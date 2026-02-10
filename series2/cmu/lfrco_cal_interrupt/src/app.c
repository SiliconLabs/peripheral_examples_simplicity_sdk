/***************************************************************************//**
 * @file
 * @brief Top level application functions
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

#include "sl_clock_manager.h"
#include "sl_power_manager.h"

#include "sl_gpio.h"

#include "em_burtc.h"

#include "pin_config.h"

/*
 * Top value for the calibration down counter.  Maximum allowed is
 * 2^20-1 = 0xFFFFF, which equates to 2^20 counts.  A larger value
 * translates into greater accuracy at the expense of a longer
 * calibration period
 *
 * In this example, calibration is run for DOWNCOUNT counts of the
 * HFXO.  At the same time, the LFRCO clocks the up counter.  At a
 * given frequency, DOWNCOUNT counts of the HFXO to take a certain
 * amount of time:
 *
 *    DOWNCOUNT
 * -------------- = calibration time
 * HFXO frequency
 *
 * It follows that the up counter will run for some number of counts
 * of the LFRCO during the same period of time:
 *
 *       UP
 * --------------- = calibration time
 * LFRCO frequency
 *
 * So, to calibrate for a specified HFXO frequency...
 *
 *      LFRCO frequency * DOWNCOUNT
 * UP = ---------------------------
 *            HXCO frequency
 *
 * For example, if the desired LFRCO frequency is 32.768 kHz, then the
 * up counter should reach...
 *
 * 32768 x 1048576
 * --------------- = 895 (rounded up from 894.78)
 *    38400000
 *
 * ...when clocked from a 38.4 MHz crystal connected to the HFXO.
 *
 * This can be run for any desired frequency to the extent that it is
 * within the range of the LFRCO.
 */
#define DOWNCOUNT 0xFFFFF

// Calibration state variables used inside the CMU interrupt handler
static bool     tuned;
static bool     lastUpGT;
static bool     lastUpLT;
static uint32_t idealCount;
static uint32_t tuningVal;
static uint32_t prevUp;
static uint32_t prevTuning;

const sl_gpio_t CLKOUT = { .port = CLKOUT_PORT, .pin = CLKOUT_PIN };

/**
 * @brief
 *    Initialize the BURTC to generate periodic interrupts every 16 seconds.
 *
 * @details
 *    The BURTC uses the LFRCO as its clock source. Although the LFRCO frequency
 *    may shift during calibration, the exact timing is not critical for this
 *    example. The BURTC simply provides periodic wakeups while the CPU remains
 *    in low power modes.
 */
static void start_burtc(void)
{
  BURTC_Init_TypeDef init = BURTC_INIT_DEFAULT;

  // Enable BURTC peripheral clock
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_BURTC);

  // Configure BURTC for a 1 Hz tick rate (LFRCO / 32768)
  init.clkDiv      = 32768;
  init.compare0Top = true;

  BURTC_Init(&init);

  // Reset counter and set COMP0 = 16 seconds
  BURTC_CounterReset();
  BURTC_CompareSet(0, 16);

  // Enable BURTC interrupt
  BURTC_IntClear(BURTC_IF_COMP);
  BURTC_IntEnable(BURTC_IEN_COMP);
  sl_interrupt_manager_clear_irq_pending(BURTC_IRQn);
  sl_interrupt_manager_enable_irq(BURTC_IRQn);
}

/**
 * @brief
 *    BURTC interrupt handler.
 *
 * @details
 *    Clears the interrupt flag and uses a DSB instruction to ensure the write
 *    completes before exiting the ISR. This prevents retriggering because the
 *    BURTC is clocked from an asynchronous domain.
 */
void BURTC_IRQHandler(void)
{
  // Disable the BURTC until the next run
  BURTC_Enable(false);
  
  // Clear the BURTC interrupt
  BURTC_IntClear(BURTC_IF_COMP);
  
  /*
   * Force the write to BURTC_IFC to complete before proceeding to
   * make sure the interrupt is not re-triggered when upon exiting this
   * IRQ handler as the BURTC is in an asynchronous clock domain.
   */
  __DSB();
}

/**
 * @brief
 *    Start an interrupt driven LFRCO calibration cycle.
 *
 * @details
 *    The CMU calibration hardware measures how many LFRCO cycles occur during
 *    a fixed number of HFXO cycles (DOWNCOUNT). When the measurement completes,
 *    the CMU generates an interrupt. The firmware adjusts the LFRCO tuning
 *    value inside the CMU interrupt handler until the measured count matches
 *    the ideal count for the target frequency.
 */
static void start_calibration(uint32_t freq)
{
  // Reset calibration state
  tuned    = false;
  lastUpGT = false;
  lastUpLT = false;

  // Compute ideal UP count for the target frequency
  idealCount = (uint32_t)(((float)freq / (float)SystemHFXOClockGet()) *
                          (float)(DOWNCOUNT + 1));

  // Read current LFRCO tuning value
  uint32_t cal_val;
  sl_clock_manager_get_rc_oscillator_calibration(SL_OSCILLATOR_LFRCO, &cal_val);
  tuningVal = cal_val;

  /*
   * Configure the calibration engine:
   *   - DOWNCOUNT HFXO cycles
   *   - LFRCO clocks the UP counter
   *   - Continuous mode enabled so the CMU automatically restarts calibration
   */
  sl_clock_manager_configure_rco_calibration(
      DOWNCOUNT,
      SL_CLOCK_MANAGER_CLOCK_CALIBRATION_HFXO,
      SL_CLOCK_MANAGER_CLOCK_CALIBRATION_LFRCO,
      true   // continuous calibration
  );

  // Begin the first calibration cycle
  sl_clock_manager_start_rco_calibration();

  // Enable CMU calibration ready interrupt
  CMU->IF_CLR = _CMU_IF_MASK;
  CMU->IEN_SET = CMU_IEN_CALRDY;
  sl_interrupt_manager_clear_irq_pending(CMU_IRQn);
  sl_interrupt_manager_enable_irq(CMU_IRQn);
}

/**
 * @brief
 *    CMU calibration interrupt handler.
 *
 * @details
 *    Each time the CMU finishes a calibration cycle, this ISR:
 *      - Waits for the calibration engine to complete
 *      - Reads the UP counter value
 *      - Compares it to the ideal count
 *      - Adjusts the LFRCO tuning value up or down
 *      - Continues calibration until the ideal count is reached
 */
void CMU_IRQHandler(void)
{
  uint32_t upCount;
  
  // Clear calibration-ready interrupt flag
  CMU->IF_CLR = CMU_IF_CALRDY;

  // Wait for calibration cycle to complete and read UP counter
  sl_clock_manager_wait_rco_calibration();
  sl_clock_manager_get_rco_calibration_count(&upCount);

  // LFRCO too slow → increase tuning value
  if (upCount < idealCount)
  {
    // Was the up counter greater than the tuned value on the last run?
    if (lastUpGT)
    {
      /*
       * If the difference between the ideal count and the up count
       * from this run is greater than it was on the last run, then
       * the last run produced a more accurate tuning, so revert to
       * the previous tuning value.  If not, the current value gets
       * us the most accurate tuning.
       */
      if ((idealCount - upCount) > (prevUp - idealCount))
        tuningVal = prevTuning;

      // Done tuning now
      tuned = true;
    }
    // Up counter for the last run not greater than the tuned value
    else
    {
      /*
       * If the difference is 1, incrementing the tuning value again
       * will only increase the frequency further away from the
       * intended target, so tuning is now complete.
       */
      if ((idealCount - upCount) == 1)
        tuned = true;
      /*
       * The difference between this run and the ideal count for the
       * desired frequency is > 1, so increase the tuning value to
       * increase the LFRCO frequency.  After the next calibration run,
       * the up counter value will increase.  Save the tuning value
       * from this run; if it's close, it might be more accurate than
       * the result from the next run.
       */
      else
      {
        prevTuning = tuningVal;
        tuningVal++;
        lastUpLT = true;  // Remember that the up counter was less than the ideal this run
        prevUp = upCount;
      }
    }
  }

  // LFRCO too fast → decrease tuning value
  if (upCount > idealCount)
  {
    // Was the up counter less than the tuned value on the last run?
    if (lastUpLT)
    {
      /*
       * If the difference between the up count and the ideal count
       * from this run is greater than it was on the last run, then
       * the last run produced a more accurate tuning, so revert to
       * the previous tuning value.  If not, the current value gets
       * the most accurate tuning.
       */
      if ((upCount - idealCount) > (idealCount - prevUp))
        tuningVal = prevTuning;

      // Done tuning now
      tuned = true;
    }
    // Up counter for the last run not less than the tuned value
    else
    {
      /*
       * If the difference is 1, decrementing the tuning value again
       * will only decrease the frequency further away from the
       * intended target, so tuning is now complete.
       */
      if ((upCount - idealCount) == 1)
        tuned = true;
      /*
       * The difference between this run and the ideal count for the
       * desired frequency is > 1, so decrease the tuning value to
       * decrease the LFRCO frequency.  After the next calibration run,
       * the up counter value will decrease.  Save the tuning value
       * from this run; if it's close, it might be more accurate than
       * the result from the next run.
       */
      else
      {
        prevTuning = tuningVal;
        tuningVal--;
        lastUpGT = true;  // Remember that the up counter was greater than the ideal this run
        prevUp = upCount;
      }
    }
  }

  // Exact match → calibration complete
  if (upCount == idealCount)
    tuned = true;
  else
    sl_clock_manager_set_rc_oscillator_calibration(
        SL_OSCILLATOR_LFRCO,
        tuningVal
    );

  // If not tuned, run calibration again, otherwise stop
  if (!tuned)
    sl_clock_manager_start_rco_calibration();
  else
    sl_clock_manager_stop_rco_calibration();
}

/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  /*
   * Enable the LFRCO register clock.  The register clock needs to be
   * enabled so that the LFRCO_CAL register is accessible to the
   * processor.  This allows an out-of-range tuning value to be set in
   * the debugger so that the calibration adjustments are more readily
   * visible on an oscilloscope.
   */
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_LFRCO);

  // Export LFRCO to CLKOUT for measurement or debugging
  sl_clock_manager_set_gpio_clock_output(
      SL_CLOCK_MANAGER_EXPORT_CLOCK_SOURCE_LFRCO,
      SL_CLOCK_MANAGER_EXPORT_CLOCK_OUTPUT_SELECT_0,
      1,
      CLKOUT_PORT,
      CLKOUT_PIN
  );
}

/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{
  // Setup calibration run
  start_calibration(32768);

  // Do other stuff while calibration is ongoing
  while (!tuned) {
    __NOP();
  }

  // Start the 16-second BURTC interrupts
  start_burtc();
}
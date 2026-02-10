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
 * In this example, calibration is run for DOWN counts of the HFXO.
 * At the same time, the LFRCO clocks the up counter.  DOWN counts
 * of the HFXO take a certain amount of time at a given frequency:
 *
 *      DOWN
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
 * So, to calibrate for a specified HFXO frequency, we should expect
 * that...
 *
 *      LFRCO frequency * DOWN
 * UP = ----------------------
 *          HFXO frequency
 *
 * For example, if the desired LFRCO frequency is 32768 Hz, then the up
 * counter should reach...
 *
 * 32768 x 1048576
 * --------------- = 895
 *    38400000
 *
 * ...when clocked from a 38.4 MHz crystal connected to the HFXO.
 *
 * This can be run for any desired frequency to the extent that it is
 * within the range of the LFRCO.
 */
#define DOWNCOUNT   0xFFFFF

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
 *    Calibrate the LFRCO to the desired frequency using a polled CMU
 *    calibration loop.
 *
 * @details
 *    The CMU calibration hardware measures how many LFRCO cycles occur during a
 *    fixed number of HFXO cycles (DOWNCOUNT). The firmware compares this
 *    measured value against the ideal value and adjusts the LFRCO tuning
 *    register until the two match.
 *
 *    This implementation performs calibration in a blocking loop. Each
 *    iteration:
 *      - Configures a one shot calibration
 *      - Waits for the calibration to complete
 *      - Reads the UP counter
 *      - Adjusts the tuning value
 *      - Repeats until the ideal count is reached
 */
static void cal_lfrco(uint32_t freq)
{
  bool tuned, lastUpGT, lastUpLT;
  uint32_t idealCount, upCount, prevUp, tuningVal, prevTuning;

  // Read the current LFRCO tuning value
  sl_clock_manager_get_rc_oscillator_calibration(SL_OSCILLATOR_LFRCO, &tuningVal);

  /*
   * Determine the ideal up counter value based on the desired
   * frequency.  Note that this is done to maintain accuracy throughout
   * the calculation.
   *
   * In most applications, it is probably better to calculate the ideal
   * up counter value(s) in advance.  For example, the ideal counter
   * value at the default LFRCO frequency is (when calibrating against
   * the HFXO and using the maximum down counter value of 0xFFFFF):
   *
   * 32768 x 1048576
   * --------------- = 895
   *    38400000
   */
  idealCount = (uint32_t)(((float)freq / (float)SystemHFXOClockGet()) *
                          (float)(DOWNCOUNT + 1));

  // Initialize calibration state
  tuned    = false;
  lastUpGT = false;
  lastUpLT = false;

  while (!tuned) {

    // Configure one shot calibration using HFXO as reference and LFRCO as target
    sl_clock_manager_configure_rco_calibration(
        DOWNCOUNT,
        SL_CLOCK_MANAGER_CLOCK_CALIBRATION_HFXO,
        SL_CLOCK_MANAGER_CLOCK_CALIBRATION_LFRCO,
        false   // one shot mode
    );

    // Run calibration and wait for completion
    sl_clock_manager_start_rco_calibration();
    sl_clock_manager_wait_rco_calibration();
    sl_clock_manager_get_rco_calibration_count(&upCount);
    sl_clock_manager_stop_rco_calibration();

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
         * the up counter value will decrease.  Save the tuning value
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
         * the up counter value will increase.  Save the tuning value
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
    if (upCount == idealCount) {
      tuned = true;
    } else {
      // Apply updated tuning value
      sl_clock_manager_set_rc_oscillator_calibration(
          SL_OSCILLATOR_LFRCO,
          tuningVal
      );
    }
  }
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

  // Start the 16-second BURTC interrupts
  start_burtc();
}

/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{
  // Run calibration
  cal_lfrco(32768);
}

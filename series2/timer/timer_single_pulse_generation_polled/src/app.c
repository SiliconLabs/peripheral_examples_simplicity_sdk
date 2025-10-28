/***************************************************************************//**
 * @file
 * @brief Top level application functions
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
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
#include "sl_gpio.h"

#include "em_timer.h"

#include "pin_config.h"

/*
 * Change this to modify the length of the delay from when the TIMER
 * starts counting to when CC0 drives the GPIO pin high.
 */
#define NUM_SEC_DELAY  3.5

// Length of the pulse high time in milliseconds
#define PULSE_WIDTH    100

// Compare values set to generate a 1 ms pulse with default clock and prescale
static uint32_t compareValue1;
static uint32_t compareValue2;

const sl_gpio_t TIMER_OUTPUT = { .port = TIMER_EXTIO_PORT,
                                 .pin  = TIMER_EXTIO_PIN };

/***************************************************************************//**
 * Initialize GPIO.
 ******************************************************************************/
void gpio_init(void)
{
  // Configure TIMER_OUTPUT
  sl_gpio_set_pin_mode(&TIMER_OUTPUT, SL_GPIO_MODE_PUSH_PULL, 0);
}

/**************************************************************************//**
 * @brief TIMER initialization
 *****************************************************************************/
void timer_init(void)
{
  uint32_t timerFreq;
  TIMER_Init_TypeDef timerInit = TIMER_INIT_DEFAULT;
  TIMER_InitCC_TypeDef timerCCInit = TIMER_INITCC_DEFAULT;

  // Enable TIMER0 bus clocks.
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_TIMER0);

  // Do not start counter upon initialization
  timerInit.enable = false;

  // Run in one-shot mode and toggle the pin on each compare match
  timerInit.oneShot = true;
  timerCCInit.mode = timerCCModeCompare;
  timerCCInit.cmoa = timerOutputActionToggle;

  TIMER_Init(TIMER0, &timerInit);

  // Route TIMER0 CC0 output to TIMER_EXTIO
  GPIO->TIMERROUTE[0].CC0ROUTE =
      (TIMER_OUTPUT.port << _GPIO_TIMER_CC0ROUTE_PORT_SHIFT)
      | (TIMER_OUTPUT.pin << _GPIO_TIMER_CC0ROUTE_PIN_SHIFT);
  GPIO->TIMERROUTE[0].ROUTEEN  = GPIO_TIMER_ROUTEEN_CC0PEN;

  /*
   * Overwrite the default of 0xFFFF in TIMER_TOP with 0xFFFFFFFF
   * because TIMER0 is 32 bits wide.
   */
  TIMER_TopSet(TIMER0, 0xFFFFFFFF);

  TIMER_InitCC(TIMER0, 0, &timerCCInit);

  // Get EM01GRPACLK frequency in MHz for compare value calculations
  sl_clock_manager_get_clock_branch_frequency(SL_CLOCK_BRANCH_EM01GRPACLK,
                                              &timerFreq);

  // Set the first compare value
  compareValue1 = (uint32_t)(timerFreq * NUM_SEC_DELAY);

  // Set compare value to the first compare value
  TIMER_CompareSet(TIMER0, 0, compareValue1);

  /*
   * Set the second compare value since its a global variable, but
   * don't actually use it here.  It gets written to the TIMER_OC
   * register in main() after the first edge happens.
   */
  compareValue2 = compareValue1 + ((timerFreq / 1000) * PULSE_WIDTH);

  // Set buffered value to next be loaded into the CCV after overflow event
  TIMER_CompareBufSet(TIMER0, 0, compareValue2);

  // Now start the TIMER
  TIMER_Enable(TIMER0, true);
}

/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  // Initialize GPIO and TIMER
  gpio_init();
  timer_init();
}

/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{
  static bool gotFirstCompare = false;

  if ((TIMER_IntGet(TIMER0) & TIMER_IF_CC0) == TIMER_IF_CC0) {
    // Clear TIMER interrupt flag
    TIMER_IntClear(TIMER0, TIMER_IF_CC0);

    if (gotFirstCompare == false) { // compare flag for first compare value
      // Set the count for the second (falling) compare value
      TIMER_CompareSet(TIMER0, 0, compareValue2);
      gotFirstCompare = true;
    }
    else { // compare flag for second compare value
      // Disable timer after falling edge
      TIMER_Enable(TIMER0, false);
    }
  }
}

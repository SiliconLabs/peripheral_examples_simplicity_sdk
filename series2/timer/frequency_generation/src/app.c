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
#include "sl_gpio.h"

#include "em_timer.h"

#include "pin_config.h"

/*
 * Desired frequency in Hz.  The default EM01GRPACLK is 19 MHz out of
 * reset, which permits a maximum frequency of 9.5 MHz.  The minimum
 * allowable frequency for this example is 1 Hz, even though TIMER0 is
 * 32 bits wide, and a minimum well below 1 Hz is possible.
 */
#define OUT_FREQ 1000

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
  uint32_t timerFreq, topValue;
  TIMER_Init_TypeDef timerInit = TIMER_INIT_DEFAULT;
  TIMER_InitCC_TypeDef timerCCInit = TIMER_INITCC_DEFAULT;

  // Enable TIMER0 bus clocks.
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_TIMER0);

  // Don't start counter on initialization
  timerInit.enable = false;

  // Configure capture/compare channel for output compare
  timerCCInit.mode = timerCCModeCompare;
  timerCCInit.cofoa = timerOutputActionToggle;

  TIMER_Init(TIMER0, &timerInit);

  // Route TIMER0 CC0 output to TIMER_EXTIO
  GPIO->TIMERROUTE[0].CC0ROUTE =
      (TIMER_OUTPUT.port << _GPIO_TIMER_CC0ROUTE_PORT_SHIFT)
      | (TIMER_OUTPUT.pin << _GPIO_TIMER_CC0ROUTE_PIN_SHIFT);
  GPIO->TIMERROUTE[0].ROUTEEN  = GPIO_TIMER_ROUTEEN_CC0PEN;

  TIMER_InitCC(TIMER0, 0, &timerCCInit);

  /*
   * Set the TOP register value.  Each time the counter overflows TOP
   * is one half of the signal period.
   */
  sl_clock_manager_get_clock_branch_frequency(SL_CLOCK_BRANCH_EM01GRPACLK,
                                              &timerFreq);
  timerFreq /= (timerInit.prescale + 1);
  topValue = timerFreq / (2 * OUT_FREQ) - 1;
  TIMER_TopSet (TIMER0, topValue);

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
}

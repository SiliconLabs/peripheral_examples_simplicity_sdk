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

const sl_gpio_t TIMER_OUTPUT = { .port = TIMER_EXTIO_PORT,
                                 .pin  = TIMER_EXTIO_PIN };

/*
 * Change this to modify the length of the delay from when the TIMER
 * starts counting to when CC0 drives the GPIO pin high.
 */
#define NUM_SECONDS_DELAY 3

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
  uint32_t timerFreq, compareValue;
  TIMER_Init_TypeDef timerInit = TIMER_INIT_DEFAULT;
  TIMER_InitCC_TypeDef timerCCInit = TIMER_INITCC_DEFAULT;

  // Enable TIMER0 bus clocks.
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_TIMER0);

  // Do not start counter upon initialization
  timerInit.enable = false;
  timerInit.prescale = timerPrescale1024;

  // Set drive output pin high upon compare match
  timerCCInit.mode = timerCCModeCompare;
  timerCCInit.cmoa = timerOutputActionSet;

  TIMER_Init(TIMER0, &timerInit);

  // Route TIMER0 CC0 output to TIMER_EXTIO
  GPIO->TIMERROUTE[0].CC0ROUTE =
      (TIMER_OUTPUT.port << _GPIO_TIMER_CC0ROUTE_PORT_SHIFT)
      | (TIMER_OUTPUT.pin << _GPIO_TIMER_CC0ROUTE_PIN_SHIFT);
  GPIO->TIMERROUTE[0].ROUTEEN  = GPIO_TIMER_ROUTEEN_CC0PEN;

  TIMER_InitCC(TIMER0, 0, &timerCCInit);

  // Set the output compare to match at NUM_SECONDS_DELAY counts
  sl_clock_manager_get_clock_branch_frequency(SL_CLOCK_BRANCH_EM01GRPACLK,
                                              &timerFreq);
  timerFreq /= (timerInit.prescale + 1);
  compareValue = timerFreq * NUM_SECONDS_DELAY;

  TIMER_CompareSet(TIMER0, 0, compareValue);

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

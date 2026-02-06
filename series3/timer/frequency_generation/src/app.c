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
#include "sl_hal_timer.h"
#include "pin_config.h"

/*
 * Desired frequency in Hz.  The default EM01GRPACLK is 38 MHz out of
 * reset, which permits a maximum frequency of 19 MHz.  The minimum
 * allowable frequency for this example is 1 Hz, even though TIMER0 is
 * 32 bits wide, and a minimum well below 1 Hz is possible.
 */
#define OUT_FREQ 1000

const sl_gpio_t TIMER_OUTPUT = { .port = EXP_TIMER_CC0_PORT,
                                 .pin  = EXP_TIMER_CC0_PIN };

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
  uint32_t timer_freq, top_value;
  sl_hal_timer_init_t init = SL_HAL_TIMER_INIT_DEFAULT;
  sl_hal_timer_channel_config_t channel_init = SL_HAL_TIMER_CHANNEL_INIT_DEFAULT;

  // Enable TIMER0 bus clocks.
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_TIMER0);

  // Configure capture/compare channel for output compare
  channel_init.channel_mode = SL_HAL_TIMER_CHANNEL_MODE_COMPARE;
  channel_init.compare_overflow_output_action = SL_HAL_TIMER_CHANNEL_OUTPUT_ACTION_TOGGLE;

  // Route TIMER0 CC0 output to output pin
  GPIO->TIMER0ROUTE[0].ROUTEEN = GPIO_TIMER0_ROUTEEN_CC0PEN;
  GPIO->TIMER0ROUTE[0].CC0ROUTE =
      TIMER_OUTPUT.pin << _GPIO_TIMER0_CC0ROUTE_PIN_SHIFT
    | TIMER_OUTPUT.port;

  sl_hal_timer_init(TIMER0, &init);
  sl_hal_timer_channel_init(TIMER0, 0, &channel_init);
  sl_hal_timer_enable(TIMER0);

  /*
   * Set the TOP register value.  Each time the counter overflows TOP
   * is one half of the signal period.
   */
  sl_clock_manager_get_clock_branch_frequency(SL_CLOCK_BRANCH_EM01GRPACLK,
                                              &timer_freq);
  timer_freq /= (init.prescaler + 1);
  top_value = timer_freq / (2 * OUT_FREQ) - 1;
  sl_hal_timer_set_top(TIMER0, top_value);

  // Start counter.
  sl_hal_timer_start(TIMER0);
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

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
#include "sl_hal_gpio.h"
#include "sl_gpio.h"
#include "sl_hal_timer.h"
#include "pin_config.h"

const sl_gpio_t GPIO_PB0 = { .port = BUTTON0_PORT, .pin = BUTTON0_PIN };
const sl_gpio_t GPIO_OUT = { .port = SLEW_RATE_OUTPUT_PORT,
                             .pin = SLEW_RATE_OUTPUT_PIN };
int32_t button0_interrupt_number = SL_GPIO_INTERRUPT_UNAVAILABLE;
uint8_t slewRate = 4;

/**************************************************************************//**
 * @brief GPIO Interrupt handler for even pins.
 *****************************************************************************/
void gpio_handler(uint8_t intNo, void * context)
{
  (void)context;

  // Check if button 0 was pressed
  if (intNo == button0_interrupt_number)
  {
    slewRate = (slewRate + 1) % 8;
    sl_gpio_set_slew_rate(&GPIO_OUT, slewRate);
  }

}

/***************************************************************************//**
 * Initialize GPIO.
 ******************************************************************************/
void gpio_init(void)
{
  // Initialize GPIO driver
  sl_gpio_init();

  // Configure timer output as push-pull with output off
  sl_gpio_set_pin_mode(&GPIO_OUT, SL_GPIO_MODE_PUSH_PULL, 0);

  // Configure Push Button 0 as input with internal pullup
  sl_gpio_set_pin_mode(&GPIO_PB0, SL_GPIO_MODE_INPUT_PULL, true);
  sl_gpio_configure_external_interrupt(&GPIO_PB0,
                                       &button0_interrupt_number,
                                       SL_GPIO_INTERRUPT_FALLING_EDGE,
                                       gpio_handler,
                                       NULL);
}

/**************************************************************************//**
 * @brief Timer init to output square wave on PC00
 *****************************************************************************/
void timer_init(void)
{
  // Enable the clock for the timer
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_TIMER0);

  // Initialize timer with update mode enabled
  sl_hal_timer_init_t init = SL_HAL_TIMER_INIT_DEFAULT;
  init.enable_update_mode = true;  // Enable manual update mode

  // Initialize a channel for PWM operation
  sl_hal_timer_channel_init_t channel_config = SL_HAL_TIMER_CHANNEL_PWM_INIT_DEFAULT;

  // Configure timer and channel
  sl_hal_timer_init(TIMER0, &init);
  sl_hal_timer_channel_init(TIMER0, 0, &channel_config);

  // Enable timer and start counting
  sl_hal_timer_enable(TIMER0);
  sl_hal_timer_wait_sync(TIMER0);
  sl_hal_timer_start(TIMER0);

  // Set initial parameters
  sl_hal_timer_set_top(TIMER0, 38);                // Set PWM period
  sl_hal_timer_channel_set_compare(TIMER0, 0, 19);  // 50% duty cycle
  sl_hal_timer_set_counter(TIMER0, 0);

  // Wait for synchronization and transfer to complete
  sl_hal_timer_wait_sync(TIMER0);  // Enable Timer0 Clock.

  //Route timer output to pin
  GPIO->TIMER0ROUTE->CC0ROUTE = (GPIO_OUT.port << _GPIO_TIMER0_CC0ROUTE_PORT_SHIFT )
                              | (GPIO_OUT.pin << _GPIO_TIMER0_CC0ROUTE_PIN_SHIFT );
  GPIO->TIMER0ROUTE->ROUTEEN = GPIO_TIMER0_ROUTEEN_CC0PEN;

}

/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  gpio_init();  
  timer_init();
}

/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{
}


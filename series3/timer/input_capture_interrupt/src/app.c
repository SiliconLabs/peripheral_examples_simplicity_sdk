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
#include "sl_interrupt_manager.h"
#include "sl_gpio.h"
#include "sl_hal_timer.h"
#include "pin_config.h"

// Define GPIO mapping for boards where LED0 and BUTTON0 share the same pins
#ifndef BUTTON0_PORT
  #define BUTTON0_PORT LED0_BUTTON0_PORT
  #define BUTTON0_PIN  LED0_BUTTON0_PIN
#endif

void TIMER_Handler(void);
const sl_gpio_t TIMER_INPUT = { .port = BUTTON0_PORT,
                                .pin  = BUTTON0_PIN };

#define BUFFERSIZE 8

// Edge capture buffer
static volatile uint32_t buffer[BUFFERSIZE];

/***************************************************************************//**
 * Initialize GPIO.
 ******************************************************************************/
void gpio_init(void)
{
  // Configure TIMER_INPUT as input
  sl_gpio_set_pin_mode(&TIMER_INPUT, SL_GPIO_MODE_INPUT_PULL_FILTER, 1);
}

/**************************************************************************//**
 * @brief
 *    TIMER initialization
 *
 * @note
 *    Prescaling the TIMER clock may or may not be necessary in a given
 *    application.  Here, prescaling the clock by 1024 (each TIMER tick
 *    is 1024 HFPERCLKs) makes it easier to see the difference between
 *    successive capture values.
 *****************************************************************************/
void timer_init(void)
{
  sl_hal_timer_init_t init = SL_HAL_TIMER_INIT_DEFAULT;
  sl_hal_timer_channel_config_t channel_init = SL_HAL_TIMER_CHANNEL_INIT_DEFAULT;

  // Enable TIMER0 bus clocks.
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_TIMER0);

  // Configure capture/compare channel for input capture
  channel_init.channel_mode = SL_HAL_TIMER_CHANNEL_MODE_CAPTURE;
  channel_init.input_capture_edge = SL_HAL_TIMER_CHANNEL_EDGE_FALLING;
  init.prescaler = SL_HAL_TIMER_PRESCALER_DIV1024;

  sl_hal_timer_init(TIMER0, &init);
  sl_hal_timer_channel_init(TIMER0, 0, &channel_init);
  sl_hal_timer_enable(TIMER0);

  // Route Button 0 to TIMER0 capture/compare channel 0
  GPIO->TIMER0ROUTE[0].ROUTEEN = GPIO_TIMER0_ROUTEEN_CC0PEN;
  GPIO->TIMER0ROUTE[0].CC0ROUTE =
      TIMER_INPUT.pin << _GPIO_TIMER0_CC0ROUTE_PIN_SHIFT
    | TIMER_INPUT.port;

  /*
   * Overwrite the default of 0xFFFF in TIMER_TOP with 0xFFFFFFFF
   * because TIMER0 is 32 bits wide.
   */
  sl_hal_timer_set_top(TIMER0, 0xFFFFFFFF);

  // Enable interrupts on capture/compare channel 0
  sl_interrupt_manager_set_irq_handler(TIMER0_IRQn, TIMER_Handler);
  sl_interrupt_manager_enable_irq(TIMER0_IRQn);
  sl_interrupt_manager_clear_irq_pending(TIMER0_IRQn);
  sl_hal_timer_clear_interrupts(TIMER0, _TIMER_IF_MASK);
  sl_hal_timer_enable_interrupts(TIMER0, TIMER_IF_CC0);

  // Start the timer
  sl_hal_timer_start(TIMER0);
}

/***************************************************************************//**
 * @brief
 *    TIMER handler
 ******************************************************************************/
void TIMER_Handler(void)
{
  static uint32_t i = 0;

  // Acknowledge the interrupt
  uint32_t flags = sl_hal_timer_get_pending_interrupts(TIMER0);
  sl_hal_timer_clear_interrupts(TIMER0, flags);

  // Check for capture event on channel 0
  if (flags & TIMER_IF_CC0) {
    // Record input capture value
    buffer[i] = sl_hal_timer_channel_get_capture(TIMER0, 0);

    // Increment index and have it wrap around
    i = (i + 1) % BUFFERSIZE;
  }
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

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
#include "sl_hal_gpio.h"
#include "sl_hal_prs.h"
#include "pin_config.h"
#include "peripheral_config.h"

#define PRS_CH_A 6
#define PRS_CH_B 1

const sl_gpio_t GPIO_LED0 = { .port = LED0_PORT, .pin = LED0_PIN };
const sl_gpio_t GPIO_BUTTON0 = { .port = BUTTON0_PORT, .pin = BUTTON0_PIN };
const sl_gpio_t GPIO_BUTTON1 = { .port = BUTTON1_PORT, .pin = BUTTON1_PIN };

/***************************************************************************//**
 * Initialize GPIO.
 ******************************************************************************/
void gpio_init(void)
{
  // Initialize GPIO driver
  sl_gpio_init();

  // Configure LED0 as push-pull with output driving LED off
  sl_gpio_set_pin_mode(&GPIO_LED0, SL_GPIO_MODE_PUSH_PULL, LED_OFF);

  // Configure Buttons as input
  sl_gpio_set_pin_mode(&GPIO_BUTTON0, SL_GPIO_MODE_INPUT_PULL, true);
  sl_gpio_set_pin_mode(&GPIO_BUTTON1, SL_GPIO_MODE_INPUT_PULL, true);

  // Configure Button pins as interrupt/prs signals
  sl_hal_gpio_configure_external_interrupt(&GPIO_BUTTON0, BUTTON0_PIN, SL_GPIO_INTERRUPT_RISING_EDGE);
  sl_hal_gpio_configure_external_interrupt(&GPIO_BUTTON1, BUTTON1_PIN, SL_GPIO_INTERRUPT_RISING_EDGE);

}

/***************************************************************************//**
 * Initialize PRS.
 ******************************************************************************/
void prs_init(void)
{
  // Enable PRS bus clock
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_PRS);

  // Configure PRS channel A and B from interrupt/prs signals
  sl_hal_prs_async_connect_channel_producer(PRS_CH_A, SL_HAL_PRS_ASYNC_GPIO_PIN1);
  sl_hal_prs_async_connect_channel_producer(PRS_CH_B, SL_HAL_PRS_ASYNC_GPIO_PIN2);

  // Combine channels with logical OR
  sl_hal_prs_async_combine_signals(PRS_CH_A, PRS_CH_B, SL_HAL_PRS_LOGIC_A_OR_B);

  // Route output to LED pin
  sl_hal_prs_pin_output(PRS_CH_A,SL_HAL_PRS_TYPE_ASYNC, LED0_PORT, LED0_PIN);
}

/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  // Initialize GPIO and PRS
  gpio_init();
  prs_init();
}

/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{
}

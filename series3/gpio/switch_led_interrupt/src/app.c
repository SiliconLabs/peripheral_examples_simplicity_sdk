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
#include "sl_hal_gpio.h"
#include "pin_config.h"

const sl_gpio_t GPIO_LED0 = { .port = LED0_PORT, .pin = LED0_PIN };
const sl_gpio_t GPIO_LED1 = { .port = LED1_PORT, .pin = LED1_PIN };
const sl_gpio_t GPIO_PB0 = { .port = BUTTON0_PORT, .pin = BUTTON0_PIN };
const sl_gpio_t GPIO_PB1 = { .port = BUTTON1_PORT, .pin = BUTTON1_PIN };
int32_t button0_interrupt_number = SL_GPIO_INTERRUPT_UNAVAILABLE;
int32_t button1_interrupt_number = SL_GPIO_INTERRUPT_UNAVAILABLE;

/**************************************************************************//**
 * @brief GPIO callback function.
 *****************************************************************************/
void gpio_callback(uint8_t intNo, void *context)
{
  (void)context;

  // Check if button 0 was pressed
  if (intNo == button0_interrupt_number)
  {
    sl_gpio_toggle_pin(&GPIO_LED0);
  }

  // Check if button 1 was pressed
  else if (intNo == button1_interrupt_number)
  {
    sl_gpio_toggle_pin(&GPIO_LED1);
  }
}

/***************************************************************************//**
 * Initialize GPIO.
 ******************************************************************************/
void gpio_init(void)
{
  // Initialize GPIO driver
  sl_gpio_init();

  // Configure LEDs as push-pull with output driving LED off
  sl_gpio_set_pin_mode(&GPIO_LED0, SL_GPIO_MODE_PUSH_PULL, LED_OFF);
  sl_gpio_set_pin_mode(&GPIO_LED1, SL_GPIO_MODE_PUSH_PULL, LED_OFF);

  // Configure Push Buttons as input with internal pullup, and configure interrupt
  sl_gpio_set_pin_mode(&GPIO_PB0, SL_GPIO_MODE_INPUT_PULL, true);
  sl_gpio_set_pin_mode(&GPIO_PB1, SL_GPIO_MODE_INPUT_PULL, true);
  sl_gpio_configure_external_interrupt(&GPIO_PB0,
                                       &button0_interrupt_number,
                                       SL_GPIO_INTERRUPT_FALLING_EDGE,
                                       gpio_callback,
                                       NULL);
  sl_gpio_configure_external_interrupt(&GPIO_PB1,
                                       &button1_interrupt_number,
                                       SL_GPIO_INTERRUPT_FALLING_EDGE,
                                       gpio_callback,
                                       NULL);
}


/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  gpio_init();
}

/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{
}

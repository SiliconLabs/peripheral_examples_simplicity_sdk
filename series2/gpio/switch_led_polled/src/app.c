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
#include "pin_config.h"

// Define GPIO mapping for boards where LED0 and button0 share the same pins
#ifndef BUTTON0_PORT
  #define BUTTON0_PORT LED0_BUTTON0_PORT
  #define BUTTON0_PIN  LED0_BUTTON0_PIN
#endif

// Define GPIO mapping for boards where LED1 and button1 share the same pins
#ifndef LED1_PORT
  #define LED1_PORT LED1_BUTTON1_PORT
  #define LED1_PIN  LED1_BUTTON1_PIN
#endif

const sl_gpio_t GPIO_LED1 = { .port = LED1_PORT, .pin = LED1_PIN };
const sl_gpio_t GPIO_PB0  = { .port = BUTTON0_PORT, .pin = BUTTON0_PIN };

/**************************************************************************//**
 * Setup GPIO for pushbuttons.
 *****************************************************************************/
static void gpioSetup(void)
{
  // Initialize GPIO driver
  sl_gpio_init();

  // Configure Button PB0 as an input
  sl_gpio_set_pin_mode(&GPIO_PB0, SL_GPIO_MODE_INPUT_PULL, true);

  // Configure LED1 as a push-pull output for LED control
  sl_gpio_set_pin_mode(&GPIO_LED1, SL_GPIO_MODE_PUSH_PULL, LED_ON);
}

/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  gpioSetup();
}

/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{
  bool button_0_pin_value;

  // Read input value of Button PB0
  sl_gpio_get_pin_input(&GPIO_PB0, &button_0_pin_value);

  // Check if Button PB0 is pressed - when pressed, value will be 0
  if (!button_0_pin_value)
  {
    sl_gpio_set_pin(&GPIO_LED1);  // Turn LED1 on (except on xG27, where it turns off)
  }
  else
  {
    sl_gpio_clear_pin(&GPIO_LED1); // Turn LED1 off (except on xG27, where it turns on)
  }
}

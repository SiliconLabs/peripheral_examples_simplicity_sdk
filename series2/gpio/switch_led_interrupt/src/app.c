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

// Remap BUTTON0_PORT to Port A for BRD4181A/EFR32MG21A010F1024IM32
// This ensures proper interrupt functionality in EM2/3 power modes
#if defined(EFR32MG21A010F1024IM32)
  #undef BUTTON0_PORT
  #undef BUTTON0_PIN
  #define BUTTON0_PORT SL_GPIO_PORT_A
  #define BUTTON0_PIN  6
#endif

const sl_gpio_t GPIO_LED1 = { .port = LED1_PORT, .pin = LED1_PIN };
const sl_gpio_t GPIO_PB0  = { .port = BUTTON0_PORT, .pin = BUTTON0_PIN };

/***************************************************************************//**
 * An internal callback called in interrupt context whenever a button changes
 * its state.
 *
 * @note The internal callback will be triggered when the Push button 0 is pressed.
 *
 ******************************************************************************/
static void button_0_change()
{
  sl_gpio_toggle_pin(&GPIO_LED1);
}

/**************************************************************************//**
 * Setup GPIO interrupt for push buttons.
 *****************************************************************************/
static void gpioSetup(void)
{
  int32_t int_no = GPIO_PB0.pin;

  // Initialize GPIO driver
  sl_gpio_init();

  // Configure Button0 as an input and enable interrupt
  sl_gpio_set_pin_mode(&GPIO_PB0, SL_GPIO_MODE_INPUT_PULL, true);
  sl_gpio_configure_external_interrupt(&GPIO_PB0, &int_no,
                                       SL_GPIO_INTERRUPT_FALLING_EDGE,
                                       (sl_gpio_irq_callback_t) button_0_change,
                                       NULL);

  // Configure LED1 as a push pull output for LED control
  sl_gpio_set_pin_mode(&GPIO_LED1, SL_GPIO_MODE_PUSH_PULL, LED_OFF);
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
}

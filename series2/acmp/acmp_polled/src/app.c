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
#include "em_acmp.h"
#include "pin_config.h"

#ifndef LED0_PORT
  #define LED0_PORT LED0_BUTTON0_PORT
  #define LED0_PIN LED0_BUTTON0_PIN
#endif

const sl_gpio_t ACMP_OUTPUT = { .port = LED0_PORT, .pin = LED0_PIN };

/**************************************************************************//**
 * @brief GPIO initialization
 *****************************************************************************/
void gpio_init(void)
{
  // Configure LED0
  sl_gpio_set_pin_mode(&ACMP_OUTPUT, SL_GPIO_MODE_PUSH_PULL, LED_OFF);
}

/**************************************************************************//**
 * @brief ACMP initialization
 *****************************************************************************/
void acmp_init(void)
{
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_ACMP0);

  // Initialize with default settings
  ACMP_Init_TypeDef init = ACMP_INIT_DEFAULT;
  ACMP_Init(ACMP0, &init);

  // Allocate the appropriate bus to ACMP0 input
  ACMP_BUS_ALLOCATION();

  /*
   * Configure ACMP0 to compare the specified input pin against the
   * selected and divided reference (which defaults to divide-by-1 if
   * the ACMP_INIT_DEFAULT settings are not overridden).
   *
   * In this example, the internal 1.25 V reference is used undivided
   * so that when the input is lower than 1.25 V, the ACMP output is 0,
   * and when it's higher than 1.25 V, the ACMP output is 1.
   */
  ACMP_ChannelSet(ACMP0, acmpInputVREFDIV1V25, ACMP_INPUT_PORT_PIN);

  // Wait for warm-up
  while (!(ACMP0->IF & ACMP_IF_ACMPRDY));
}

/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  // Initialize GPIO and ACMP
  gpio_init();
  acmp_init();
}

/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{
  // Wait for input to go low
  while (ACMP0->STATUS & _ACMP_STATUS_ACMPOUT_MASK);

  // Turn on output
#if (LED_ON == 1)
  sl_gpio_set_pin(&ACMP_OUTPUT);
#else 
  sl_gpio_clear_pin(&ACMP_OUTPUT);
#endif

  // Wait for input to go high
  while(!(ACMP0->STATUS & _ACMP_STATUS_ACMPOUT_MASK));
  
  // Turn off output
#if (LED_ON == 1)
  sl_gpio_clear_pin(&ACMP_OUTPUT);
#else 
  sl_gpio_set_pin(&ACMP_OUTPUT);
#endif
}

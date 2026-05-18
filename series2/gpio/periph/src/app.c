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
#include "pin_config.h"
#include "peripheral_config.h"

#define SLEWRATE 7  // Slew rate setting for clock output

const sl_gpio_t CLKOUT = { .port = CLKOUT_PORT, .pin = CLKOUT_PIN };

/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  // Initialize GPIO driver
  sl_gpio_init();

  // Configure CLKOUT pin as push-pull output
  sl_gpio_set_pin_mode(&CLKOUT, SL_GPIO_MODE_PUSH_PULL, 0);

  // Configure defined pin for LFRCO output
  sl_clock_manager_set_gpio_clock_output(SL_CLOCK_MANAGER_EXPORT_CLOCK_SOURCE_LFRCO,
                                         CLKOUT_SEL,
                                         1,
                                         CLKOUT.port,
                                         CLKOUT.pin);

  // Set slew rate / drive strength so there is no ringing
  sl_gpio_set_slew_rate(&CLKOUT, SLEWRATE);
}

 /***************************************************************************//**
  * App ticking function.
  ******************************************************************************/
 void app_process_action(void)
 {
 } 
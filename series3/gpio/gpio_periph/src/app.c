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
#include "pin_config.h"

const sl_gpio_t GPIO_CLOCK_OUT = { .port = SL_GPIO_PORT_B, .pin = 3 };

/***************************************************************************//**
 * Initialize GPIO.
 ******************************************************************************/
void gpio_init(void)
{
  // Initialize GPIO driver
  sl_gpio_init();

  // Configure clock out as push-pull with output off
  sl_gpio_set_pin_mode(&GPIO_CLOCK_OUT, SL_GPIO_MODE_PUSH_PULL, 0);

}

/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  gpio_init();  

  sl_clock_manager_set_gpio_clock_output(SL_CLOCK_MANAGER_EXPORT_CLOCK_SOURCE_LFRCO,
                                         SL_CLOCK_MANAGER_EXPORT_CLOCK_OUTPUT_SELECT_0,
                                         1,
                                         SL_GPIO_PORT_C,
                                         0);

  sl_gpio_set_slew_rate(&GPIO_CLOCK_OUT, 7);
}

/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{
}


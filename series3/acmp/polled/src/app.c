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
#include "sl_hal_gpio.h"
#include "sl_hal_acmp.h"
#include "pin_config.h"

const sl_gpio_t GPIO_LED0 = { .port = LED0_PORT, .pin = LED0_PIN };
const sl_gpio_t GPIO_PB0 = { .port = BUTTON0_PORT, .pin = BUTTON0_PIN };

/***************************************************************************//**
 * Initialize GPIO.
 ******************************************************************************/
void gpio_init(void)
{
  // Initialize GPIO driver
  sl_gpio_init();

  // Configure LED0 as push-pull with output driving LED off
  sl_gpio_set_pin_mode(&GPIO_LED0, SL_GPIO_MODE_PUSH_PULL, LED_OFF);

  // Configure Push Button 0 as input with internal pullup
  sl_gpio_set_pin_mode(&GPIO_PB0, SL_GPIO_MODE_INPUT_PULL, true);
}

/***************************************************************************//**
 * Initialize ACMP.
 ******************************************************************************/
void acmp_init(void)
{
  // Enable ACMP bus clock
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_ACMP0);

  // Initialize with default settings
  sl_hal_acmp_init_t init = SL_HAL_ACMP_INIT_DEFAULT;
  sl_hal_acmp_init(ACMP0, &init);
  
  // Enable ACMP
  sl_hal_acmp_enable(ACMP0);

  // Allocate ACMP0 to analog bus to be able to use the input
  GPIO->ACMP_INPUT_BUS = ACMP_INPUT_BUSALLOC;

  // In this example we want to compare an analog input to the 1.25 V
  // internal reference. The default settings resets the divider for
  // acmpInputVREFDIV1V25, which we can use as a 1.25 V reference.
  // Now we select the two inputs to compare. Here we compare the GPIO_PB0
  // input to the internal 1.25V reference. When GPIO_PB0 is lower than
  // 1.25 V then the ACMP output is 0 and when GPIO_PB0 is higher than
  // 1.25 V then the ACMP output is 1.
  sl_hal_acmp_set_input(ACMP0,
                        SL_HAL_ACMP_INPUT_VREFDIV1V25,
                        sl_hal_acmp_gpio_to_input(GPIO_PB0.port, GPIO_PB0.pin));

  // Wait for warmup
  while(!(ACMP0->IF & ACMP_IF_ACMPRDY));
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
  // Wait for ACMP output to go low (button pressed)
  while(sl_hal_acmp_get_status(ACMP0) & _ACMP_STATUS_ACMPOUT_MASK);

  // Turn on LED
  sl_gpio_set_pin(&GPIO_LED0);

  // Wait for ACMP output to go high (button released)
  while(!(sl_hal_acmp_get_status(ACMP0) & _ACMP_STATUS_ACMPOUT_MASK));

  // Turn off LED
  sl_gpio_clear_pin(&GPIO_LED0);
}

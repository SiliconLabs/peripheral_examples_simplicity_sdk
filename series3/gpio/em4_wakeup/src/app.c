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
#include "sl_power_manager.h"
#include "sl_gpio.h"
#include "sl_hal_emu.h"
#include "sl_hal_gpio.h"
#include "pin_config.h"
#include "peripheral_config.h"

const sl_gpio_t GPIO_LED0 = { .port = LED0_PORT, .pin = LED0_PIN };
const sl_gpio_t GPIO_PB0 = { .port = BUTTON0_PORT, .pin = BUTTON0_PIN };
int32_t button0_interrupt_number = SL_GPIO_INTERRUPT_UNAVAILABLE;

/***************************************************************************//**
 * Initialize GPIO.
 ******************************************************************************/
void gpio_init(void)
{
  // Initialize GPIO driver
  sl_gpio_init();

  // Configure LEDs as push-pull with output driving LED off
  sl_gpio_set_pin_mode(&GPIO_LED0, SL_GPIO_MODE_PUSH_PULL, LED_OFF);
  for(volatile int i = 0; i< 50000000; i++);

  // Configure Push Buttons as input with internal pullup
  sl_gpio_set_pin_mode(&GPIO_PB0, SL_GPIO_MODE_INPUT_PULL, true);
}


/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  uint32_t reset_cause;
  gpio_init();

  reset_cause = sl_hal_emu_get_reset_cause();
  sl_hal_emu_clear_reset_cause();
  if (reset_cause & EMU_RSTCAUSE_EM4) {
    while(1) {
      for(volatile int i = 0; i< 5000000; i ++);
      sl_gpio_toggle_pin(&GPIO_LED0);
    }
  }

  sl_hal_gpio_configure_wakeup_em4_external_interrupt(&GPIO_PB0,
                                                      button0_interrupt_number,
                                                      false);
  sl_power_manager_enter_em4();
}

/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{
}

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
#include "sl_udelay.h"
#include "sl_gpio.h"
#include "sl_hal_gpio.h"
#include "em_rmu.h"
#include "em_wdog.h"
#include "pin_config.h"

#ifndef LED0_PORT
  #define LED0_PORT LED0_BUTTON0_PORT
  #define LED0_PIN LED0_BUTTON0_PIN
#endif

#ifndef BUTTON1_PORT
  #define BUTTON1_PORT LED1_BUTTON1_PORT
  #define BUTTON1_PIN LED1_BUTTON1_PIN
#endif

const sl_gpio_t GPIO_LED0 = { .port = LED0_PORT, .pin = LED0_PIN };
const sl_gpio_t GPIO_PB1 = { .port = BUTTON1_PORT, .pin = BUTTON1_PIN };

/***************************************************************************//**
 * Initialize GPIO.
 ******************************************************************************/
void gpio_init(void)
{
  // Initialize GPIO driver
  sl_gpio_init();

  // Configure LED0 as push-pull with output driving LED off
  sl_gpio_set_pin_mode(&GPIO_LED0, SL_GPIO_MODE_PUSH_PULL, LED_OFF);

  // Configure Push Button 1 as input with internal pullup
  sl_gpio_set_pin_mode(&GPIO_PB1, SL_GPIO_MODE_INPUT_PULL, true);
}

/***************************************************************************//**
 * Initialize WDOG.
 ******************************************************************************/
void wdog_init(void)
{
  // Enable WDOG bus clock
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_WDOG0);

  // Configure WDOG to reset after 2049 ticks
  WDOG_Init_TypeDef wdogInit = WDOG_INIT_DEFAULT;
  wdogInit.perSel = wdogPeriod_2k;

  // Initialize the WDOG
  WDOGn_Init(WDOG0, &wdogInit);
}

/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  uint32_t resetCause;

  // Get and clear the MCU reset cause
  resetCause = RMU_ResetCauseGet();
  RMU_ResetCauseClear();

  // Turn on LED0 if the reset was caused by the WDOG
  if(resetCause & EMU_RSTCAUSE_WDOG0) {
    sl_gpio_init();
    sl_gpio_set_pin_mode(&GPIO_LED0, SL_GPIO_MODE_PUSH_PULL, LED_ON);
    while(1);
  }

  // Initialize GPIO and WDOG
  gpio_init();
  wdog_init();
}

/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{
  // Don't feed the WDOG if Push Button 1 is being pressed
  while(!sl_hal_gpio_get_pin_input(&GPIO_PB1));

  // Feed the WDOG and toggle LED0
  WDOGn_Feed(WDOG0);
  sl_gpio_toggle_pin(&GPIO_LED0);

  // 100 ms delay simulating user code runtime
  sl_udelay_wait(100000);
}

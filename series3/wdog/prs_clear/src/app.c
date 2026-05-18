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
#include "sl_hal_prs.h"
#include "sl_hal_wdog.h"
#include "pin_config.h"
#include "peripheral_config.h"

const sl_gpio_t GPIO_LED0 = { .port = LED0_PORT, .pin = LED0_PIN };
const sl_gpio_t GPIO_PRS_WDOG = { .port = WDOG_PRS_PORT, .pin = WDOG_PRS_PIN };

/***************************************************************************//**
 * Initialize GPIO.
 ******************************************************************************/
void gpio_init(void)
{
  int32_t interrupt_number = GPIO_PRS_WDOG.pin;

  // Initialize GPIO driver
  sl_gpio_init();

  // Configure LED0 as push-pull with output driving LED off
  sl_gpio_set_pin_mode(&GPIO_LED0, SL_GPIO_MODE_PUSH_PULL, LED_OFF);

  // Configure GPIO_PRS_WDOG as input
  sl_gpio_set_pin_mode(&GPIO_PRS_WDOG, SL_GPIO_MODE_INPUT_PULL, false);

  // Configure GPIO_PRS_WDOG interrupt for PRS
  sl_gpio_configure_external_interrupt(&GPIO_PRS_WDOG,
                                       &interrupt_number,
                                       SL_GPIO_INTERRUPT_NO_EDGE,
                                       NULL,
                                       NULL);
}

/***************************************************************************//**
 * Initialize PRS.
 ******************************************************************************/
void prs_init(void)
{
  // Enable PRS bus clock
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_PRS);

  // Configure PRS channel 0 with GPIO_PRS_WDOG pin as producer and WDOG as consumer
  sl_hal_prs_async_channel_config_t config = SL_HAL_PRS_ASYNC_INIT_DEFAULT(0,
                                                                           SL_HAL_PRS_ASYNC_GPIO_PIN0 + GPIO_PRS_WDOG.pin,
                                                                           SL_HAL_PRS_CONSUMER_WDOG0_SRC0);
  sl_hal_prs_async_init_channel(&config);
}

/***************************************************************************//**
 * Initialize WDOG.
 ******************************************************************************/
void wdog_init(void)
{
  // Enable WDOG bus clock
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_WDOG0);

  // Configure WDOG to reset after 2049 ticks and enable PRS as clear source
  sl_hal_wdog_init_t init = SL_HAL_WDOG_INIT_DEFAULT;
  init.em1_run = true;
  init.clear_source = true;
  init.period_select = SL_WDOG_PERIOD_2k;

  // Initialize the WDOG
  sl_hal_wdog_init(WDOG0, &init);

  // Start WDOG timer
  sl_hal_wdog_enable(WDOG0);
}

/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  uint32_t resetCause;

  // Get and clear the MCU reset cause
  resetCause = EMU->RSTCAUSE;
  EMU->CMD_SET = EMU_CMD_RSTCAUSECLR;

  // Turn on LED0 if the reset was caused by the WDOG
  if(resetCause & EMU_RSTCAUSE_WDOG0) {
    sl_gpio_init();
    sl_gpio_set_pin_mode(&GPIO_LED0, SL_GPIO_MODE_PUSH_PULL, LED_ON);
    while(1);
  }

  // Initialize GPIO, PRS, and WDOG
  gpio_init();
  prs_init();
  wdog_init();
}

/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{
}

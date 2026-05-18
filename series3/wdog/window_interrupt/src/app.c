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
#include "sl_hal_wdog.h"
#include "pin_config.h"
#include "peripheral_config.h"

const sl_gpio_t GPIO_LED0 = { .port = LED0_PORT, .pin = LED0_PIN };
const sl_gpio_t GPIO_PB1 = { .port = BUTTON1_PORT, .pin = BUTTON1_PIN };

volatile uint32_t usDelay = 1100000;
volatile bool clearedTooEarly = false;

/***************************************************************************//**
 * Push Button 1 falling interrupt callback.
 ******************************************************************************/
void pb1_on_change(void)
{
  usDelay =  800000;
}

/***************************************************************************//**
 * Initialize GPIO.
 ******************************************************************************/
void gpio_init(void)
{
  int32_t interrupt_number = GPIO_PB1.pin;

  // Initialize GPIO driver
  sl_gpio_init();

  // Configure LED0 as push-pull with output driving LED off
  sl_gpio_set_pin_mode(&GPIO_LED0, SL_GPIO_MODE_PUSH_PULL, LED_OFF);

  // Configure Push Button 1 as input with internal pullup
  sl_gpio_set_pin_mode(&GPIO_PB1, SL_GPIO_MODE_INPUT_PULL, true);

  // Configure Push Button 1 to trigger GPIO interrupt on falling edge
  sl_gpio_configure_external_interrupt(&GPIO_PB1,
                                       &interrupt_number,
                                       SL_GPIO_INTERRUPT_FALLING_EDGE,
                                       (sl_gpio_irq_callback_t)pb1_on_change,
                                       NULL);
}

/***************************************************************************//**
 * Initialize WDOG.
 ******************************************************************************/
void wdog_init(void)
{
  // Enable WDOG bus clock
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_WDOG0);

  // Configure WDOG to interrupt after 50% of 2049 ticks
  sl_hal_wdog_init_t init = SL_HAL_WDOG_INIT_DEFAULT;
  init.window_time_select = SL_WDOG_ILLEGAL_WINDOW_TIME50_0;
  init.period_select = SL_WDOG_PERIOD_2k;

  // Enable WDOG window interrupt
  sl_hal_wdog_enable_interrupts(WDOG0, WDOG_IEN_WIN);
  sl_interrupt_manager_enable_irq(WDOG0_IRQn);

  // Initialize the WDOG
  sl_hal_wdog_init(WDOG0, &init);

  // Start WDOG timer
  sl_hal_wdog_enable(WDOG0);
}

/***************************************************************************//**
 * Watchdog ISR triggers when fed before window level.
 ******************************************************************************/
void WDOG0_IRQHandler(void)
{
  // Clear WDOG interrupt flags
  uint32_t flags = sl_hal_wdog_get_pending_interrupts(WDOG0);
  sl_hal_wdog_clear_interrupts(WDOG0, flags);

  // Indicate that the WDOG was fed before the window level
  clearedTooEarly = true;
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

  // Initialize GPIO and WDOG
  gpio_init();
  wdog_init();
}

/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{
  // Turn on LED0 if the WDOG not cleared within 1024 ms
  if(clearedTooEarly) {
    sl_gpio_set_pin_mode(&GPIO_LED0, SL_GPIO_MODE_PUSH_PULL, LED_ON);
    while(1);
  }

  // Variable delay simulating user code runtime
  sl_udelay_wait(usDelay);

  // Feed the WDOG and toggle LED0
  sl_hal_wdog_feed(WDOG0);
  sl_gpio_toggle_pin(&GPIO_LED0);

}

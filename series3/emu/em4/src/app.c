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

#include "em_device.h"
#include "sl_power_manager.h"
#include "sl_hal_gpio.h"
#include "sl_clock_manager.h"
#include "sl_hal_burtc.h"

/*******************************************************************************
 * Select whether BURTC should run during EM4.
 *
 * 1 = Enter EM4 with BURTC enabled
 *     - BURTC continues running using EM4GRPACLK
 *     - EM4GRPACLK source is configured in SLCP (default: LFRCO @ 32.768 kHz)
 *
* 0 = Enter EM4 with BURTC disabled
*     - LF oscillators are automatically turned off by hardware when not needed
 ******************************************************************************/
#define USE_BURTC_IN_EM4   0

/***************************************************************************//**
 * @brief Disable all GPIOs to eliminate external leakage paths
 ******************************************************************************/
static void suppress_external_power_consumption(void)
{
#if defined(SL_CATALOG_MX25_FLASH_SHUTDOWN_USART_PRESENT) || defined(SL_CATALOG_MX25_FLASH_SHUTDOWN_EUSART_PRESENT)
  /* Power-down MX25 SPI flash */
  sl_mx25_flash_shutdown();
#endif

  /* Disable all GPIO */
  for (uint32_t port = 0; port < SL_HAL_GPIO_PORT_MAX; port++) {
    uint32_t pin_count = SL_HAL_GPIO_PORT_SIZE(port);
    for (uint32_t pin = 0; pin < pin_count; pin++) {
      if (SL_HAL_GPIO_PORT_PIN_IS_VALID(port, pin)) {
        const sl_gpio_t gpio = { port, pin };
        sl_hal_gpio_set_pin_mode(&gpio, SL_GPIO_MODE_DISABLED, false);
      }
    }
  }
}

#if USE_BURTC_IN_EM4
/***************************************************************************//**
 * @brief Initialize BURTC for EM4 operation
 *
 * BURTC is clocked from EM4GRPACLK. The EM4GRPACLK source is selected in the
 * project's SLCP configuration (default: LFRCO @ 32.768 kHz).
 *
 * BURAM is retained during EM4, allowing BURTC counter values and application
 * data to persist across EM4 resets.
 ******************************************************************************/
static void burtc_init(void)
{
  // Enable BURTC bus clock; BURTC runs from EM4GRPACLK (LFRCO by default in SLCP)
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_BURTC);
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_BURAM);

  // Basic BURTC configuration: free-running counter, no compare interrupt needed
  sl_hal_burtc_init_config_t init = SL_HAL_BURTC_INIT_DEFAULT;
  init.compare0_top   = false;
  init.em4_comparator = false;
  sl_hal_burtc_init(&init);
  sl_hal_burtc_enable();

  // Start BURTC from 0
  sl_hal_burtc_reset_counter();
}
#endif

/***************************************************************************//**
 * @brief Initialize application
 *
 * All GPIOs are disabled to remove external leakage paths. Depending on the
 * configuration below, the device enters EM4 either with BURTC running or with
 * no low-frequency activity. When BURTC is not enabled, the LF domain is
 * automatically powered down by hardware during EM4 entry.
 ******************************************************************************/
void app_init(void)
{
  suppress_external_power_consumption();

#if USE_BURTC_IN_EM4
  // Enter EM4 with BURTC enabled.
  // BURTC runs from EM4GRPACLK, whose source is selected in SLCP
  // (default: LFRCO @ 32.768 kHz). LF oscillator remains active because BURTC
  // requires it.
  burtc_init();

  sl_power_manager_enter_em4();

#else
  // Enter EM4 with no BURTC enabled.
  // With no peripheral requesting the LF domain, LF oscillators are
  // automatically turned off by hardware for lowest power.
  sl_power_manager_enter_em4();

#endif
}

/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{
  // Execution does not continue here after entering EM4
}
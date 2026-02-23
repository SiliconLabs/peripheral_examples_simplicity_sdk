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

#include "sl_power_manager.h"
#include "sl_clock_manager.h"
#include "sl_hal_system.h"
#include "sl_hal_gpio.h"

/*******************************************************************************
 * Select which example to run.
 *
 * USE_EM1DIV16:
 *   1 = Run EM1DIV16 example (default)
 *   0 = Run EM1 example
 ******************************************************************************/
#define USE_EM1DIV16   1

#if (USE_EM1DIV16 == 0)
/*******************************************************************************
 * EM1 clock selection (only used when EM1DIV16 is disabled)
 *
 * USE_EM1_HFRCODPLL_100MHZ:
 *   1 = EM1 using HFRCODPLL @ 100 MHz
 *   0 = EM1 using default SLCP configuration
 *       (SOCPLL @ 150 MHz, referenced to HFXO 38.4 MHz)
 ******************************************************************************/
#define USE_EM1_HFRCODPLL_100MHZ   1
#endif

/***************************************************************************//**
 * @brief Disable all GPIOs to eliminate external leakage paths
 ******************************************************************************/
static void suppress_external_power_consumption(void)
{
  for (uint32_t port = 0; port < SL_HAL_GPIO_PORT_MAX; port++) {
    uint32_t pin_count = SL_HAL_GPIO_PORT_SIZE(port);

    for (uint32_t pin = 0; pin < pin_count; pin++) {
      if (SL_HAL_GPIO_PORT_PIN_IS_VALID(port, pin)) {
        const sl_gpio_t gpio = { port, pin };

        // Disable all GPIOs
        sl_hal_gpio_set_pin_mode(&gpio, SL_GPIO_MODE_DISABLED, false);
      }
    }
  }
}

/***************************************************************************//**
 * @brief * Initialize application
 *
 * This function configures the system clocking and power manager requirements
 * depending on whether EM1 or EM1DIV16 mode is selected.
 *
 * After configuration, the system immediately enters the selected energy mode.
 ******************************************************************************/
void app_init(void)
{
  suppress_external_power_consumption();

#if USE_EM1DIV16
  /***************************************************************************
   * EM1DIV16 Example
   *
   * - SYSCLK source on entry: SOCPLL @ 150 MHz (HFXO referenced)
   * - Power manager automatically:
   *      • Switches SYSCLK to HFXO
   *      • Applies HCLKDIV16
   *      • Applies PCLKDIV2
   ***************************************************************************/

  // Switch QSPI clock to FSRCO
  sl_clock_manager_set_ext_flash_clk(SL_OSCILLATOR_FSRCO);

  // Remove EM1 requirement which allows EM1DIV16 entry
  sl_power_manager_remove_em_requirement(SL_POWER_MANAGER_EM1);

#else
  /***************************************************************************
   * EM1 Example
   *
   * This example demonstrates two selectable EM1 clock configurations:
   *
   * 1) Default SLCP configuration:
   *      - SYSCLK = SOCPLL @ 150 MHz
   *      - SOCPLL referenced to HFXO (38.4 MHz)
   *      - No runtime clock changes required
   *
   * 2) HFRCODPLL configuration:
   *      - SYSCLK = HFRCODPLL @ 100 MHz
   *      - Requires loading HFRCO factory speed calibration
   *      - Matches datasheet EM1 current‑measurement conditions
   *
   * For both EM1 options:
   *      - An EM1 requirement is added to keep the device in EM1
   ***************************************************************************/

#if USE_EM1_HFRCODPLL_100MHZ

  // Load the speed factory calibration value for HFRCO oscillator
  uint32_t freq_cal = sl_hal_system_get_hfrco_speed_calibration();
  HFRCO0->CAL = freq_cal;

  // Configure HFRCODPLL to 100 MHz
  SystemHFRCODPLLClockSet(100000000);

  // Switch SYSCLK to HFRCODPLL
  slx_clock_manager_set_sysclk_source(SL_OSCILLATOR_HFRCODPLL);

#else

  // EM1 using default SLCP clocking:
  // - SOCPLL @ 150 MHz
  // - SOCPLL referenced to HFXO (38.4 MHz)
  // No clock changes required

#endif // USE_EM1_HFRCODPLL_100MHZ

  // Add EM1 requirement
  sl_power_manager_add_em_requirement(SL_POWER_MANAGER_EM1);

#endif // USE_EM1DIV16

  // Enter the selected energy mode
  sl_power_manager_sleep();
}

/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{
  // Device remains in EM1 or EM1DIV16
}
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

#include "sl_hal_gpio.h"
#include "sl_hal_system.h"
#include "sl_clock_manager.h"

/*******************************************************************************
 * Select whether to switch to HFRCODPLL.
 *
 * 1 = Use HFRCODPLL @ 100 MHz
 * 0 = Leave system clock at reset default (SOCPLL @ 150 MHz)
 *
 * At reset, the device starts using SOCPLL at 150 MHz as configured in the
 * SLCP File (Clock Manager). When USE_HFRCODPLL_100MHZ is set to 1, the 
 * application reconfigures the system clock to HFRCODPLL at 100 MHz.
 ******************************************************************************/
#define USE_HFRCODPLL_100MHZ   1

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
        sl_hal_gpio_set_pin_mode(&gpio, SL_GPIO_MODE_DISABLED, false);
      }
    }
  }
}

/***************************************************************************//**
 * @brief Initialize application
 *
 * By default, the device boots using SOCPLL at 150 MHz as configured in
 * the clock manager settings in the SLCP file.
 *
 * If USE_HFRCODPLL_100MHZ is enabled, the application loads the HFRCO speed
 * calibration value, configures HFRCODPLL for 100 MHz, and switches SYSCLK
 * from SOCPLL to HFRCODPLL.
 ******************************************************************************/
void app_init(void)
{
  suppress_external_power_consumption();

#if USE_HFRCODPLL_100MHZ

  // Load the factory speed calibration value for HFRCO oscillator
  uint32_t freq_cal = sl_hal_system_get_hfrco_speed_calibration();
  HFRCO0->CAL = freq_cal;

  // Configure HFRCODPLL to 100 MHz
  SystemHFRCODPLLClockSet(100000000);

  // Switch system clock to HFRCODPLL
  slx_clock_manager_set_sysclk_source(SL_OSCILLATOR_HFRCODPLL);
  
  // Switch QSPI clock to FSRCO
  sl_clock_manager_set_ext_flash_clk(SL_OSCILLATOR_FSRCO);

#endif
}

/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{
  while (1) {
    // Remain in EM0 for current measurement
  }
}
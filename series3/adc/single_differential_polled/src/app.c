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
#include "sl_hal_adc.h"
#include "sl_hal_gpio.h"

#include "pin_config.h"
#include "peripheral_config.h"

const sl_gpio_t GPIO_ADC_INPUT0 = { .port = ADC_INPUT0_PORT,
                                    .pin  = ADC_INPUT0_PIN };
const sl_gpio_t GPIO_ADC_INPUT1 = { .port = ADC_INPUT1_PORT,
                                    .pin  = ADC_INPUT1_PIN };

static volatile sl_hal_adc_result_t sample;
static volatile float singleResult;

/***************************************************************************//**
 * Initialize GPIO.
 ******************************************************************************/
void gpio_init(void)
{
  // Initialize GPIO driver
  sl_gpio_init();

  /*
   * Configure ADC input in disabled mode; this is the default configuration out
   * of reset, so API call is not required; included in this example to 
   * demonstrate recommended configuration for analog input.
   */
  sl_gpio_set_pin_mode(&GPIO_ADC_INPUT0, SL_GPIO_MODE_DISABLED, false);
  sl_gpio_set_pin_mode(&GPIO_ADC_INPUT1, SL_GPIO_MODE_DISABLED, false);

  // Enable optional low noise mode for ADC input
  GPIO->P_SET[ADC_INPUT0_PORT].AMUXMODE = 1 << ADC_INPUT0_PIN;
  GPIO->P_SET[ADC_INPUT1_PORT].AMUXMODE = 1 << ADC_INPUT1_PIN;

  // Allocate the analog bus for ADC0 input
  GPIO->ADC_INPUT0_BUS |= ADC_INPUT0_BUSALLOC;
  GPIO->ADC_INPUT1_BUS |= ADC_INPUT1_BUSALLOC;
}

/***************************************************************************//**
 * Initialize ADC.
 ******************************************************************************/
void adc_init(void)
{
  // Declare initialization structures
  uint32_t adcclk_clock_freq;
  sl_hal_adc_init_t init = SL_HAL_ADC_INIT_DEFAULT;
  sl_hal_adc_config_t initConfig = SL_HAL_ADC_CONFIG_DEFAULT;
  sl_hal_adc_scan_entry_t initScanEntry = SL_HAL_ADC_SCAN_ENTRY_DEFAULT;

  // Enable ADC peripheral bus clock
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_ADC0);

  // Get ADCCLK frequency; Clock Manager used to configure ADCCLK branch
  sl_clock_manager_get_clock_branch_frequency(SL_CLOCK_BRANCH_ADCCLK,
                                              &adcclk_clock_freq);

  // Configure scan table
  initScanEntry.pos_port = ADC_INPUT0_HAL_PORT;
  initScanEntry.pos_pin  = ADC_INPUT0_PIN;
  initScanEntry.neg_port = ADC_INPUT1_HAL_PORT;
  initScanEntry.neg_pin  = ADC_INPUT1_PIN;

  // Configure gain to adjust full-scale to 3.84 V (from internal reference)
  initConfig.gain = SL_HAL_ADC_ANALOG_GAIN_0_3125;

  // Configure and enable ADC
  init.debug_halt = true;
  init.scan_trigger_action = SL_HAL_ADC_TRIGGER_ACTION_ONCE;
  init.config[SL_HAL_ADC_CONFIG_ID_0] = initConfig;
  init.entries[SL_HAL_ADC_CHANNEL_ID_0] = initScanEntry;

  sl_hal_adc_init(ADC0, &init, adcclk_clock_freq);
  sl_hal_adc_enable(ADC0);

  // Configure scan channels
  sl_hal_adc_set_scan_mask(ADC0, (1 << SL_HAL_ADC_CHANNEL_ID_0));
}

/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  // Initialize GPIO and ADC
  gpio_init();
  adc_init();
}

/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{
  // Start ADC scan
  sl_hal_adc_start(ADC0);

  /*
   * Wait for the single conversion to complete. A conversion is done when the
   * ADC->STATUS register's SCANDATADV and CONVERTING bits are 1 (conversion
   * data is in the output FIFO) and 0 (a conversion is not currently in
   * progress), respectively.
   */
  while((sl_hal_adc_get_status(ADC0) & (_ADC_STATUS_CONVERTING_MASK \
      | _ADC_STATUS_SCANDATADV_MASK)) != ADC_STATUS_SCANDATADV);

  // Pull the single conversion result from the FIFO
  sample = sl_hal_adc_pull(ADC0);

  /*
   * For differential inputs, the range is from -Vref to + Vref, i.e.,
   * for Vref = VBGR = 1.2 V, and with analog gain = 0.3125, 12 bits represents
   * (-Vref / 0.3125) to (+Vref / 0.3125) = -3.84 V to +3.84 V.
   */
  singleResult = (int16_t)sample.data * 2.0f * 1.2f / 0.3125f / 0xFFF;
}

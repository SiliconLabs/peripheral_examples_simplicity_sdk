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
#include "sl_interrupt_manager.h"

#include "sl_gpio.h"
#include "sl_hal_gpio.h"
#include "sl_hal_adc.h"

#include "pin_config.h"

/*
 * Upper and lower bound for Window Comparator
 * 16-bit left-justified format; 12-bit conversion result compared to upper
 * 12-bits of window comparator
 */
#define WINDOW_UPPER_BOUND      0xC000   // 1.815V
#define WINDOW_LOWER_BOUND      0x4000   // 0.605V

const sl_gpio_t GPIO_LED0 = { .port = LED0_PORT, .pin = LED0_PIN };
const sl_gpio_t GPIO_ADC_INPUT0 = { .port = ADC_INPUT0_PORT,
                                    .pin = ADC_INPUT0_PIN };

static volatile sl_hal_adc_result_t sample;
static volatile double singleResult;

static void ADC0_Handler(void);

/***************************************************************************//**
 * Initialize GPIO.
 ******************************************************************************/
void gpio_init(void)
{
  // Initialize GPIO driver
  sl_gpio_init();

  /*
   * Configure ADC input as disabled; this is the default configuration out
   * of reset, so API call is not required for this example
   */
  sl_gpio_set_pin_mode(&GPIO_ADC_INPUT0, SL_GPIO_MODE_DISABLED, false);

  // Configure LED0 output
  sl_gpio_set_pin_mode(&GPIO_LED0, SL_GPIO_MODE_PUSH_PULL, false);

  // Enable optional low noise mode for ADC input
  GPIO->P_SET[ADC_INPUT0_PORT].AMUXMODE = 1 << ADC_INPUT0_PIN;

  // Allocate the analog bus for ADC0 inputs
  GPIO->ADC_INPUT0_BUS |= ADC_INPUT0_BUSALLOC;
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

  // Get ADCCLK frequency
  sl_clock_manager_get_clock_branch_frequency(SL_CLOCK_BRANCH_ADCCLK,
                                              &adcclk_clock_freq);

  // Configure scan table
  initScanEntry.compare  = true;
  initScanEntry.pos_port = ADC_INPUT0_HAL_PORT;
  initScanEntry.pos_pin = ADC_INPUT0_PIN;

  // Configure gain to adjust full-scale to 2.4 V (from internal reference)
  initConfig.gain = SL_HAL_ADC_ANALOG_GAIN_0_5;

  /*
   * Configure the comparison window
   * ADC interrupt triggers when conversion results are between
   * WINDOW_LOWER_BOUND and WINDOW_UPPER_BOUND. Flip flop greater-than/less-than
   * settings to trigger when conversion result is ~outside~ the comparison
   * window.
   */
  init.greater_than = WINDOW_LOWER_BOUND;
  init.less_than = WINDOW_UPPER_BOUND;

  // Configure and enable ADC
  init.voltage_reference = SL_HAL_ADC_REFERENCE_VREFINT;
  init.scan_trigger_action = SL_HAL_ADC_TRIGGER_ACTION_CONTINUOUS;
  init.config[initScanEntry.config_id] = initConfig;
  init.entries[ADC_CHANNEL] = initScanEntry;

  sl_hal_adc_init(ADC0, &init, adcclk_clock_freq);
  sl_hal_adc_enable(ADC0);

  // Configure scan channels
  sl_hal_adc_set_scan_mask(ADC0, (1 << ADC_CHANNEL));

  // Set the ADC interrupt handler
  sl_interrupt_manager_set_irq_handler(ADC0_IRQn, ADC0_Handler);

  // Enable ADC interrupt
  sl_hal_adc_enable_interrupts(ADC0, ADC_IEN_SCANCMP);
  sl_interrupt_manager_clear_irq_pending(ADC0_IRQn);
  sl_interrupt_manager_enable_irq(ADC0_IRQn);
}

/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  // Initialize GPIO and ADC
  gpio_init();
  adc_init();

  // Start ADC scan
  sl_hal_adc_start(ADC0);
}

/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{
}

/***************************************************************************//**
 * ADC interrupt handler.
 ******************************************************************************/
void ADC0_Handler(void)
{
  /*
   * Toggle LED0 during IRQ to signal conversion result was within the compare
   * window; Set LED0 at beginning of handler
   */
  sl_gpio_set_pin(&GPIO_LED0);

  // Pull the single conversion result from the FIFO
  sample = sl_hal_adc_pull(ADC0);

  /*
   * For single-ended input, the range is 0 V to (+Vref / 0.5) = 2.4 V with
   * 12 bits for the conversion value.
   */
  singleResult = sample.data * 1.2f / 0.5f / 0xFFF;

  sl_hal_adc_clear_interrupts(ADC0, ADC_IF_SCANCMP);

  // Clear LED0 at end of handler
  sl_gpio_clear_pin(&GPIO_LED0);
}

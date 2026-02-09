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
#include "sl_hal_gpio.h"
#include "sl_hal_adc.h"

#include "pin_config.h"

/*
 * Upper and lower bound for Window Comparator
 * 16-bit left-justified format; 12-bit conversion result compared to upper
 * 12-bits of window comparator
 */
#define WINDOW_UPPER_BOUND      0x2D50 // 1.70V for config_0; 0.85V for config_1
#define WINDOW_LOWER_BOUND      0x0000 // 0.00V

const sl_gpio_t GPIO_ADC_OUTPUT0 = { .port = ADC_OUTPUT0_PORT,
                                     .pin  = ADC_OUTPUT0_PIN };
const sl_gpio_t GPIO_LED0        = { .port = LED0_PORT,
                                     .pin  = LED0_PIN };

static volatile sl_hal_adc_result_t sample;
static volatile float scanResults[4];

static void ADC0_Handler(void);

/***************************************************************************//**
 * Initialize GPIO.
 ******************************************************************************/
void gpio_init(void)
{
  // Initialize GPIO driver
  sl_gpio_init();

  // Configure LED0 output
  sl_gpio_set_pin_mode(&GPIO_LED0, SL_GPIO_MODE_PUSH_PULL, false);

  // Configure ADC conversion complete via GPIO toggle in interrupt callback
  sl_gpio_set_pin_mode(&GPIO_ADC_OUTPUT0, SL_GPIO_MODE_PUSH_PULL, false);
}

/***************************************************************************//**
 * Initialize ADC.
 ******************************************************************************/
void adc_init(void)
{
  // Declare initialization structures
  uint32_t adcclk_clock_freq;
  sl_hal_adc_init_t init = SL_HAL_ADC_INIT_DEFAULT;
  sl_hal_adc_config_t initConfig[2] = { SL_HAL_ADC_CONFIG_DEFAULT,
                                        SL_HAL_ADC_CONFIG_DEFAULT };
  sl_hal_adc_scan_entry_t initScanEntry[4] = { SL_HAL_ADC_SCAN_ENTRY_DEFAULT,
                                               SL_HAL_ADC_SCAN_ENTRY_DEFAULT,
                                               SL_HAL_ADC_SCAN_ENTRY_DEFAULT,
                                               SL_HAL_ADC_SCAN_ENTRY_DEFAULT };

  // Enable ADC peripheral bus clock
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_ADC0);

  // Get ADCCLK frequency
  sl_clock_manager_get_clock_branch_frequency(SL_CLOCK_BRANCH_ADCCLK,
                                              &adcclk_clock_freq);

  // Configure scan table
  initScanEntry[0].compare  = true;
  initScanEntry[0].pos_port = SL_HAL_ADC_PORT_POS_SUPPLY;
  initScanEntry[0].pos_pin = 0; // AVDD divided by 4
  initScanEntry[1].compare  = true;
  initScanEntry[1].pos_port = SL_HAL_ADC_PORT_POS_SUPPLY;
  initScanEntry[1].pos_pin = 1; // IOVDD divided by 4
  initScanEntry[2].compare  = true;
  initScanEntry[2].pos_port = SL_HAL_ADC_PORT_POS_SUPPLY;
  initScanEntry[2].pos_pin = 2; // DVDD divided by 4
  initScanEntry[3].compare  = true;
  initScanEntry[3].pos_port = SL_HAL_ADC_PORT_POS_SUPPLY;
  initScanEntry[3].pos_pin = 3; // DECOUPLE divided by 4
  initScanEntry[3].config_id = SL_HAL_ADC_CONFIG_ID_1;

  // Configure gain to adjust full-scale to 2.4 V (from internal reference)
  initConfig[0].gain = SL_HAL_ADC_ANALOG_GAIN_0_5;
  initConfig[1].gain = SL_HAL_ADC_ANALOG_GAIN_1;

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
  init.show_id = true;    // enable channel ID to differentiate in ADC callback
  init.debug_halt = true;
  init.voltage_reference = SL_HAL_ADC_REFERENCE_VREFINT;
  init.scan_trigger_action = SL_HAL_ADC_TRIGGER_ACTION_CONTINUOUS;
  init.config[SL_HAL_ADC_CONFIG_ID_0] = initConfig[0];
  init.config[SL_HAL_ADC_CONFIG_ID_1] = initConfig[1]; // gain adjust for DECOUPLE
  init.entries[SL_HAL_ADC_CHANNEL_ID_0] = initScanEntry[0];
  init.entries[SL_HAL_ADC_CHANNEL_ID_1] = initScanEntry[1];
  init.entries[SL_HAL_ADC_CHANNEL_ID_2] = initScanEntry[2];
  init.entries[SL_HAL_ADC_CHANNEL_ID_3] = initScanEntry[3];

  sl_hal_adc_init(ADC0, &init, adcclk_clock_freq);
  sl_hal_adc_enable(ADC0);

  // Configure scan channels
  sl_hal_adc_set_scan_mask(ADC0, ((1 << SL_HAL_ADC_CHANNEL_ID_0)
                           | (1 << SL_HAL_ADC_CHANNEL_ID_1)
                           | (1 << SL_HAL_ADC_CHANNEL_ID_2)
                           | (1 << SL_HAL_ADC_CHANNEL_ID_3)));

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
  // Set LED0 (on) when supply voltages are no longer below threshold.
  GPIO->P_SET[LED0_PORT].DOUT = 1UL << LED0_PIN;
}

/***************************************************************************//**
 * ADC interrupt handler.
 ******************************************************************************/
__attribute__((section("text_application_ram")))
static void ADC0_Handler(void)
{
  uint32_t rawData, fifoCnt;

  /*
   * Clear LED0 (off) to indicate when supplies are below threshold. IOVDD is also
   * lower than typical and LED0 will be dim if visible. Direct register writes
   * are used to optimize execution time.
   */
  GPIO->P_CLR[LED0_PORT].DOUT = 1UL << LED0_PIN;

  // Also toggle ADC output to observe callback execution time.
  GPIO->P_SET[ADC_OUTPUT0_PORT].DOUT = 1UL << ADC_OUTPUT0_PIN;

  /*
   * Clear the scan compare interrupt flag. This is done early in the interrupt
   * handler so the clear command can propagate while data is being processed.
   * Reading FIFO results does not automatically clear the interrupt flag.
   */
  ADC0->IF_CLR = ADC_IF_SCANCMP;

  /*
   * Get the number of conversion results stored in the scan FIFO.
   */
  fifoCnt = sl_hal_adc_get_fifo_count(ADC0);

  /*
   * Cycle through and process the raw conversion data, but only data which
   * triggered the interrupt. ADC conversions can continue while processing and
   * additional results can continue to fill the FIFO for the next scan complete
   * interrupt.
   */
  while ( fifoCnt > 0 )
  {
    // Pull a scan conversion result from the FIFO
    rawData = ADC0->SCANFIFODATA;
    sample.data = (rawData & (uint32_t)0x0000FFFF) >> 0;
    sample.id = (rawData & (uint32_t)0xFF000000) >> 24;

    if (sample.id > 2) { // DECOUPLE supply
      /*
       * For single-ended input, the range is 0 V to +Vref = 1.2 V with
       * 12 bits for the conversion value. Conversion must be multiplied by 4
       * to compensate for internal attenuation factor for supply inputs.
       */
      scanResults[sample.id] = sample.data * 4 * 1.2f / 0xFFF;
    } else {
      /*
       * For single-ended input, the range is 0 V to (+Vref / 0.5) = 2.4 V with
       * 12 bits for the conversion value. Conversion must be multiplied by 4
       * to compensate for internal attenuation factor for supply inputs.
       */
      scanResults[sample.id] = sample.data * 4 * 1.2f / 0.5f / 0xFFF;
    }

    // Decrement while-loop counter.
    fifoCnt--;
  }

  /*
   * Clear ADC output at end of handler. Direct register writes are used to optimize
   * execution time.
   */
  GPIO->P_CLR[ADC_OUTPUT0_PORT].DOUT = 1UL << ADC_OUTPUT0_PIN;
}

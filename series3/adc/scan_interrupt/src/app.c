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
#include "sl_hal_adc.h"
#include "sl_hal_gpio.h"

#include "pin_config.h"
#include "peripheral_config.h"

const sl_gpio_t GPIO_ADC_INPUT0  = { .port = ADC_INPUT0_PORT,
                                     .pin  = ADC_INPUT0_PIN };
const sl_gpio_t GPIO_ADC_INPUT1  = { .port = ADC_INPUT1_PORT,
                                     .pin  = ADC_INPUT1_PIN };
const sl_gpio_t GPIO_ADC_OUTPUT0 = { .port = ADC_OUTPUT0_PORT,
                                     .pin  = ADC_OUTPUT0_PIN };

static volatile sl_hal_adc_result_t sample;
static volatile float scanResults[6];

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
  sl_gpio_set_pin_mode(&GPIO_ADC_INPUT1, SL_GPIO_MODE_DISABLED, false);

  // Configure ADC conversion complete via GPIO toggle in interrupt callback
  sl_gpio_set_pin_mode(&GPIO_ADC_OUTPUT0, SL_GPIO_MODE_PUSH_PULL, false);

  // Enable optional low noise mode for ADC input
  GPIO->P_SET[ADC_INPUT0_PORT].AMUXMODE = 1 << ADC_INPUT0_PIN;
  GPIO->P_SET[ADC_INPUT1_PORT].AMUXMODE = 1 << ADC_INPUT1_PIN;

  // Allocate the analog bus for ADC0 inputs
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
  sl_hal_adc_scan_entry_t initScanEntry[6] = { SL_HAL_ADC_SCAN_ENTRY_DEFAULT,
                                               SL_HAL_ADC_SCAN_ENTRY_DEFAULT,
                                               SL_HAL_ADC_SCAN_ENTRY_DEFAULT,
                                               SL_HAL_ADC_SCAN_ENTRY_DEFAULT,
                                               SL_HAL_ADC_SCAN_ENTRY_DEFAULT,
                                               SL_HAL_ADC_SCAN_ENTRY_DEFAULT };

  // Enable ADC peripheral bus clock
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_ADC0);

  // Get ADCCLK frequency
  sl_clock_manager_get_clock_branch_frequency(SL_CLOCK_BRANCH_ADCCLK,
                                              &adcclk_clock_freq);

  // Configure scan table
  initScanEntry[0].pos_port = ADC_INPUT0_HAL_PORT;
  initScanEntry[0].pos_pin  = ADC_INPUT0_PIN;
  initScanEntry[1].pos_port = ADC_INPUT1_HAL_PORT;
  initScanEntry[1].pos_pin  = ADC_INPUT1_PIN;
  initScanEntry[2].pos_port = SL_HAL_ADC_PORT_POS_SUPPLY;
  initScanEntry[2].pos_pin  = 0; // AVDD divided by 4
  initScanEntry[3].pos_port = SL_HAL_ADC_PORT_POS_SUPPLY;
  initScanEntry[3].pos_pin  = 1; // IOVDD divided by 4
  initScanEntry[4].pos_port = SL_HAL_ADC_PORT_POS_SUPPLY;
  initScanEntry[4].pos_pin  = 2; // DVDD divided by 4
  initScanEntry[5].pos_port = SL_HAL_ADC_PORT_POS_SUPPLY;
  initScanEntry[5].pos_pin  = 3; // DECOUPLE divided by 4

  // Configure gain to adjust full-scale to 3.84 V (from internal reference)
  initConfig.gain = SL_HAL_ADC_ANALOG_GAIN_0_3125;

  // Configure and enable ADC
  init.show_id = true;  // enable channel ID to differentiate in ADC callback
  init.debug_halt = true;
  init.scan_trigger_action = SL_HAL_ADC_TRIGGER_ACTION_CONTINUOUS;
  init.config[initScanEntry[0].config_id] = initConfig;
  init.entries[SL_HAL_ADC_CHANNEL_ID_0] = initScanEntry[0];
  init.entries[SL_HAL_ADC_CHANNEL_ID_1] = initScanEntry[1];
  init.entries[SL_HAL_ADC_CHANNEL_ID_2] = initScanEntry[2];
  init.entries[SL_HAL_ADC_CHANNEL_ID_3] = initScanEntry[3];
  init.entries[SL_HAL_ADC_CHANNEL_ID_4] = initScanEntry[4];
  init.entries[SL_HAL_ADC_CHANNEL_ID_5] = initScanEntry[5];

  sl_hal_adc_init(ADC0, &init, adcclk_clock_freq);
  sl_hal_adc_enable(ADC0);

  // Configure scan channels
  sl_hal_adc_set_scan_mask(ADC0, ((1 << SL_HAL_ADC_CHANNEL_ID_0)
                           | (1 << SL_HAL_ADC_CHANNEL_ID_1)
                           | (1 << SL_HAL_ADC_CHANNEL_ID_2)
                           | (1 << SL_HAL_ADC_CHANNEL_ID_3)
                           | (1 << SL_HAL_ADC_CHANNEL_ID_4)
                           | (1 << SL_HAL_ADC_CHANNEL_ID_5)));

  // Set the ADC interrupt handler
  sl_interrupt_manager_set_irq_handler(ADC0_IRQn, ADC0_Handler);

  // Enable ADC interrupt
  sl_hal_adc_enable_interrupts(ADC0, ADC_IEN_SCANTABLEDONE);
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

  // Start scan
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
__attribute__((section("text_application_ram")))
static void ADC0_Handler(void)
{
  uint32_t rawData, fifoCnt;

  /*
   * Set GPIO on during IRQ to signal conversion result complete and start
   * of voltage conversion. Direct register writes are used to optimize
   * execution time.
   */
  GPIO->P_SET[ADC_OUTPUT0_PORT].DOUT = 1UL << ADC_OUTPUT0_PIN;

  /*
   * Clear the scan table done interrupt. This is done early in the interrupt
   * handler so the clear command can propagate while data is being processed.
   * Reading FIFO results does not automatically clear the interrupt flag.
   */
  ADC0->IF_CLR = ADC_IF_SCANTABLEDONE;

  /*
   * Get the number of conversion results stored in the scan FIFO. This should
   * be equal to the number of scan channels enabled during ADC initialization.
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
    rawData = ADC0->SCANFIFODATA;
    sample.data = (rawData & (uint32_t)0x0000FFFF) >> 0;
    sample.id = (rawData & (uint32_t)0xFF000000) >> 24;

    // Scan results 2 - 5 are configured for internal supply voltages.
    if ((sample.id > 1) && (sample.id < 6)) {
      /*
       * For single-ended input, the range is 0 V to (+Vref / 0.3125) = 3.84 V
       * with 12 bits for the conversion value. Conversion must be multiplied
       * by 4 to compensate for internal attenuation factor for supply inputs.
       */
      scanResults[sample.id] = sample.data * 1.2f * 4.0f / 0.3125f / 0xFFF;
    } else {
      /*
       * For single-ended input, the range is 0 V to (+Vref / 0.3125) = 3.84 V
       * with 12 bits for the conversion value.
       */
      scanResults[sample.id] = sample.data * 1.2f / 0.3125f / 0xFFF;
    }

    // Decrement while-loop counter.
    fifoCnt--;
  }

  /*
   * Clear GPIO at end of handler. Direct register writes are used to optimize
   * execution time.
   */
  GPIO->P_CLR[ADC_OUTPUT0_PORT].DOUT = 1UL << ADC_OUTPUT0_PIN;
}

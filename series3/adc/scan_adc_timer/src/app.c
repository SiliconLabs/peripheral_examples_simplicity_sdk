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

/*
 * Desired ADC TIMER frequency in Hz; ADC TIMER period is based on CLK_SRC_ADC,
 * which has a frequency range between 20-40 MHz. Number of CLK_SRC_ADC cycles
 * per timer even limited to 65535.
 */
#define ADC_TIMER_FREQ  1000

const sl_gpio_t GPIO_ADC_INPUT0  = { .port = ADC_INPUT0_PORT,
                                     .pin  = ADC_INPUT0_PIN };
const sl_gpio_t GPIO_ADC_INPUT1  = { .port = ADC_INPUT1_PORT,
                                     .pin  = ADC_INPUT1_PIN };
const sl_gpio_t GPIO_ADC_OUTPUT0 = { .port = ADC_OUTPUT0_PORT,
                                     .pin  = ADC_OUTPUT0_PIN };

static volatile sl_hal_adc_result_t sample;
static volatile float scanResults[2];

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
  uint32_t branch_clock_freq;
  sl_hal_adc_init_t init = SL_HAL_ADC_INIT_DEFAULT;
  sl_hal_adc_config_t initConfig = SL_HAL_ADC_CONFIG_DEFAULT;
  sl_hal_adc_scan_entry_t initScanEntry[2] = { SL_HAL_ADC_SCAN_ENTRY_DEFAULT,
                                               SL_HAL_ADC_SCAN_ENTRY_DEFAULT};

  // Enable ADC peripheral bus clock
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_ADC0);

  // Get ADCCLK frequency
  sl_clock_manager_get_clock_branch_frequency(SL_CLOCK_BRANCH_ADCCLK,
                                              &branch_clock_freq);

  // Configure scan table
  initScanEntry[0].pos_port = ADC_INPUT0_HAL_PORT;
  initScanEntry[0].pos_pin  = ADC_INPUT0_PIN;
  initScanEntry[1].pos_port = ADC_INPUT1_HAL_PORT;
  initScanEntry[1].pos_pin  = ADC_INPUT1_PIN;

  // Configure gain to adjust full-scale to 3.84 V (from internal reference)
  initConfig.gain = SL_HAL_ADC_ANALOG_GAIN_0_3125;

  // Configure and enable ADC
  init.show_id = true;    // enable channel ID to differentiate in ADC callback
  init.debug_halt = true;
  init.warmup_mode = SL_HAL_ADC_WARMUP_KEEPWARM;
  init.scan_trigger = SL_HAL_ADC_TRIGGER_TIMER;
  init.scan_trigger_action = SL_HAL_ADC_TRIGGER_ACTION_ONCE;
  init.config[SL_HAL_ADC_CONFIG_ID_0] = initConfig;  // common config_id_0
  init.entries[SL_HAL_ADC_CHANNEL_ID_0] = initScanEntry[0];
  init.entries[SL_HAL_ADC_CHANNEL_ID_1] = initScanEntry[1];

  sl_hal_adc_set_timer_period(ADC0, ((branch_clock_freq / ADC_TIMER_FREQ) - 1));
  sl_hal_adc_init(ADC0, &init, branch_clock_freq);
  sl_hal_adc_enable(ADC0);

  // Configure scan channels
  sl_hal_adc_set_scan_mask(ADC0, ((1 << SL_HAL_ADC_CHANNEL_ID_0)
                           | (1 << SL_HAL_ADC_CHANNEL_ID_1)));

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
  // Initialize GPIO, ADC and LETIMER
  gpio_init();
  adc_init();

  // Start ADC scan
  sl_hal_adc_start(ADC0);

  // Start ADC Timer
  sl_hal_adc_enable_timer(ADC0);
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
void ADC0_Handler(void)
{
  uint32_t rawData, fifoCnt;

  // Direct register writes used to optimize execution time.
  GPIO->P_TGL[ADC_OUTPUT0_PORT].DOUT = 1UL << ADC_OUTPUT0_PIN;

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
    // Pull a scan conversion result from the FIFO
    rawData = ADC0->SCANFIFODATA;
    sample.data = (rawData & (uint32_t)0x0000FFFF) >> 0;
    sample.id = (rawData & (uint32_t)0xFF000000) >> 24;

    /*
     * For single-ended input, the range is 0 V to (+Vref / 0.3125) = 3.84 V
     * with 12 bits for the conversion value.
     */
    scanResults[sample.id] = sample.data * 1.2f / 0.3125f / 0xFFF;

    // Decrement while-loop counter.
    fifoCnt--;
  }
}


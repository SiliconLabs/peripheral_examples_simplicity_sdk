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
#include "sl_hal_adc.h"
#include "sl_hal_gpio.h"
#include "sl_hal_prs.h"

#include "pin_config.h"

const sl_gpio_t GPIO_ADC_INPUT0  = { .port = ADC_INPUT0_PORT,
                                     .pin  = ADC_INPUT0_PIN };
const sl_gpio_t GPIO_ADC_OUTPUT0 = { .port = ADC_OUTPUT0_PORT,
                                     .pin  = ADC_OUTPUT0_PIN };

static volatile sl_hal_adc_result_t sample;
static volatile float singleResult;

static void ADC0_Handler(void);

/***************************************************************************//**
 * Initialize GPIO.
 ******************************************************************************/
void gpio_init(void)
{
  // Initialize GPIO driver
  sl_gpio_init();

  /*
   * Configure ADC input as disabled (default configuration out of reset);
   * API call is not required in this instance, only provided to demonstrate
   * GPIO mode recommendation.
   */
  sl_gpio_set_pin_mode(&GPIO_ADC_INPUT0, SL_GPIO_MODE_DISABLED, false);

  // Configure ADC conversion complete via PRS output
  sl_gpio_set_pin_mode(&GPIO_ADC_OUTPUT0, SL_GPIO_MODE_PUSH_PULL, false);

  // Enable optional low noise mode for ADC input
  GPIO->P_SET[ADC_INPUT0_PORT].AMUXMODE = 1 << ADC_INPUT0_PIN;

  // Allocate the analog bus for ADC0 inputs
  GPIO->ADC_INPUT0_BUS |= ADC_INPUT0_BUSALLOC;
}

/***************************************************************************//**
 * Initialize PRS.
 ******************************************************************************/
void prs_init(void)
{
  // Enable PRS peripheral bus clock
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_PRS);

  // Configure ADC scan table completion as PRS producer and output to GPIO
  sl_hal_prs_async_connect_channel_producer(ADC_ASYNC_PRS_CH,
                                           SL_HAL_PRS_ASYNC_ADC0_SCANTABLEDONE);
  sl_hal_prs_pin_output(ADC_ASYNC_PRS_CH, SL_HAL_PRS_TYPE_ASYNC,
                        ADC_OUTPUT0_PORT, ADC_OUTPUT0_PIN);
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
  sl_hal_adc_scan_entry_t initScanEntry = SL_HAL_ADC_SCAN_ENTRY_DEFAULT;

  // Enable ADC peripheral bus clock
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_ADC0);

  // Get ADCCLK frequency
  sl_clock_manager_get_clock_branch_frequency(SL_CLOCK_BRANCH_ADCCLK,
                                              &branch_clock_freq);

  // Configure scan table
  initScanEntry.pos_port = ADC_INPUT0_HAL_PORT;
  initScanEntry.pos_pin  = ADC_INPUT0_PIN;

  /*
   * Configure oversampling rate and gain to adjust full-scale to 3.84 V (from
   * internal reference)
   */
  initConfig.average = SL_HAL_ADC_AVERAGE_X16;
  initConfig.gain = SL_HAL_ADC_ANALOG_GAIN_0_3125;

  // Configure and enable ADC
  init.debug_halt = true;
  init.scan_trigger_action = SL_HAL_ADC_TRIGGER_ACTION_CONTINUOUS;
  init.config[SL_HAL_ADC_CONFIG_ID_0] = initConfig;
  init.entries[SL_HAL_ADC_CHANNEL_ID_0] = initScanEntry;

  sl_hal_adc_init(ADC0, &init, branch_clock_freq);
  sl_hal_adc_enable(ADC0);

  // Configure scan channels
  sl_hal_adc_set_scan_mask(ADC0, (1 << SL_HAL_ADC_CHANNEL_ID_0));

  // Set the ADC interrupt handler
  sl_interrupt_manager_set_irq_handler(ADC0_IRQn, ADC0_Handler);

  // Enable ADC interrupt
  sl_hal_adc_enable_interrupts(ADC0, ADC_IF_SCANTABLEDONE);
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
  prs_init();
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
  uint32_t rawData;

  /*
   * Clear the scan table done interrupt. This is done early in the interrupt
   * handler so the clear command can propagate while data is being processed.
   * Reading FIFO results does not automatically clear the interrupt flag.
   */
  ADC0->IF_CLR = ADC_IF_SCANTABLEDONE;

  // Pull a scan conversion result from the FIFO
  rawData = ADC0->SCANFIFODATA;
  sample.data = (rawData & (uint32_t)0x0000FFFF) >> 0;
  sample.id = (rawData & (uint32_t)0xFF000000) >> 24;

  /*
   * For single-ended input, the range is 0 V to (+Vref / 0.3125) = 3.84 V with
   * 16 bits for the conversion value.
   */
  singleResult = sample.data * 1.2f / 0.3125f / 0xFFFF;
}


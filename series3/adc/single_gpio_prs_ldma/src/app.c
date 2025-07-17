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

#include "dmadrv.h"

#include "sl_gpio.h"
#include "sl_hal_adc.h"
#include "sl_hal_gpio.h"
#include "sl_hal_ldma.h"
#include "sl_hal_prs.h"

#include "pin_config.h"

// How many samples to capture
#define NUM_SAMPLES   10

const sl_gpio_t GPIO_LED0 = { .port = LED0_PORT, .pin = LED0_PIN };
const sl_gpio_t GPIO_BUTTON0 = { .port = BUTTON0_PORT, .pin = BUTTON0_PIN };

const sl_gpio_t GPIO_ADC_INPUT0 = { .port = ADC_INPUT0_PORT,
                                    .pin = ADC_INPUT0_PIN };

/*******************************************************************************
 ***************************   GLOBAL VARIABLES   ******************************
 ******************************************************************************/

unsigned int channelId;

// Buffer for ADC samples
uint32_t singleBuffer[NUM_SAMPLES];

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

  // Configure LED0 output
  sl_gpio_set_pin_mode(&GPIO_LED0, SL_GPIO_MODE_PUSH_PULL, false);

  // Configure push button BTN0 input
  sl_gpio_set_pin_mode(&GPIO_BUTTON0, SL_GPIO_MODE_INPUT_PULL_FILTER, true);
  sl_hal_gpio_configure_external_interrupt(&GPIO_BUTTON0, BUTTON0_PIN,
                                           SL_GPIO_INTERRUPT_RISING_EDGE);

  // Enable optional low noise mode for ADC input
  GPIO->P_SET[ADC_INPUT0_PORT].AMUXMODE = 1 << ADC_INPUT0_PIN;

  // Allocate the analog bus for ADC0 inputs
  GPIO->ADC_INPUT0_BUS |= ADC_INPUT0_BUSALLOC;
}

void prs_init(void)
{
  // Enable PRS peripheral bus clock
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_PRS);

  // Configure GPIO input as PRS producer to initiate single ADC conversion
  sl_hal_prs_async_connect_channel_producer(ADC_ASYNC_PRS_CH,
                                            SL_HAL_PRS_ASYNC_GPIO_PIN1);
  sl_hal_prs_connect_channel_consumer(ADC_ASYNC_PRS_CH, SL_HAL_PRS_TYPE_ASYNC,
                                      SL_HAL_PRS_CONSUMER_ADC0_ASYNCSEQ);
}

void ldma_callback(void)
{
  // Toggle LED0 to notify that transfers are complete
  sl_gpio_toggle_pin(&GPIO_LED0);
}

void ldma_init(void)
{
  // Initialize DMADRV
  DMADRV_Init();

  // Halt on error if DMA channel cannot be allocated
  if (DMADRV_AllocateChannel(&channelId, NULL) != ECODE_EMDRV_DMADRV_OK) {
    __BKPT(0);
  }
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
  initScanEntry.pos_pin = ADC_INPUT0_PIN;

  // Configure gain to adjust full-scale to 3.84 V (from internal reference)
  initConfig.gain = SL_HAL_ADC_ANALOG_GAIN_0_3125;

  // Configure and enable ADC
  init.warmup_mode = SL_HAL_ADC_WARMUP_NORMAL;
  init.scan_trigger = SL_HAL_ADC_TRIGGER_PRSPOS;
  init.scan_trigger_action = SL_HAL_ADC_TRIGGER_ACTION_ONCE;
  init.config[initScanEntry.config_id] = initConfig;
  init.entries[ADC_CHANNEL] = initScanEntry;
  init.data_valid = (sl_hal_adc_data_valid_t)(NUM_SAMPLES - 1);

  sl_hal_adc_init(ADC0, &init, branch_clock_freq);
  sl_hal_adc_enable(ADC0);

  // Configure scan channels
  sl_hal_adc_set_scan_mask(ADC0, (1 << ADC_CHANNEL));
}

/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  // Initialize GPIO, PRS, ADC and LDMA
  gpio_init();
  prs_init();
  adc_init();
  ldma_init();

  // Start ADC scan
  sl_hal_adc_start(ADC0);
}

/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{
  bool active;

  // Check that ADC->memory LDMA channel is not currently active; halt if error
  if (DMADRV_TransferActive(channelId, &active) != ECODE_EMDRV_DMADRV_OK) {
    __BKPT(0);
  }

  if (!active) {
    // Start next data transfer from ADC peripheral to data buffer
    DMADRV_PeripheralMemory(channelId,
                            SL_HAL_LDMA_PERIPHERAL_SIGNAL_ADC0_SCAN,
                            singleBuffer, (void*)&(ADC0->SCANFIFODATA), true,
                            NUM_SAMPLES, dmadrvDataSize4,
                            (DMADRV_Callback_t)&ldma_callback, NULL);
  }
}

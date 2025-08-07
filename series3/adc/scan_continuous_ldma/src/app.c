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

#include "dmadrv.h"

#include "sl_gpio.h"
#include "sl_hal_adc.h"
#include "sl_hal_gpio.h"
#include "sl_hal_ldma.h"

#include "pin_config.h"

// Define number samples to capture
#define NUM_SAMPLES   10

const sl_gpio_t GPIO_ADC_INPUT0 = { .port = ADC_INPUT0_PORT,
                                    .pin  = ADC_INPUT0_PIN };
const sl_gpio_t GPIO_ADC_INPUT1 = { .port = ADC_INPUT1_PORT,
                                    .pin  = ADC_INPUT1_PIN };
const sl_gpio_t GPIO_LED0 =       { .port = LED0_PORT,
                                    .pin  = LED0_PIN };

/*******************************************************************************
 ***************************   GLOBAL VARIABLES   ******************************
 ******************************************************************************/

// Allocated DMADRV Channel ID
unsigned int channelId;

// Buffer for ADC samples
uint32_t scanBuffer[NUM_SAMPLES];

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

  // Enable optional low noise mode for ADC inputs
  GPIO->P_SET[ADC_INPUT0_PORT].AMUXMODE = 1 << ADC_INPUT0_PIN;
  GPIO->P_SET[ADC_INPUT1_PORT].AMUXMODE = 1 << ADC_INPUT1_PIN;

  // Allocate the analog bus for ADC0 inputs
  GPIO->ADC_INPUT0_BUS |= ADC_INPUT0_BUSALLOC;
  GPIO->ADC_INPUT1_BUS |= ADC_INPUT1_BUSALLOC;
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
  init.show_id = true;  // enable channel ID to differentiate in raw data
  init.debug_halt = true;
  init.warmup_mode = SL_HAL_ADC_WARMUP_KEEPWARM;
  init.scan_trigger = SL_HAL_ADC_TRIGGER_IMMEDIATE;
  init.scan_trigger_action = SL_HAL_ADC_TRIGGER_ACTION_CONTINUOUS;
  init.data_valid = (sl_hal_adc_data_valid_t)(NUM_SAMPLES - 1);
  init.config[SL_HAL_ADC_CONFIG_ID_0] = initConfig;  // common config_id_0
  init.entries[SL_HAL_ADC_CHANNEL_ID_0] = initScanEntry[0];
  init.entries[SL_HAL_ADC_CHANNEL_ID_1] = initScanEntry[1];


  sl_hal_adc_init(ADC0, &init, branch_clock_freq);
  sl_hal_adc_enable(ADC0);

  // Configure scan channels
  sl_hal_adc_set_scan_mask(ADC0, ((1 << SL_HAL_ADC_CHANNEL_ID_0)
                           | (1 << SL_HAL_ADC_CHANNEL_ID_1)));
}

/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  // Initialize peripherals
  gpio_init();
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
                            scanBuffer, (void*)&(ADC0->SCANFIFODATA), true,
                            NUM_SAMPLES, dmadrvDataSize4,
                            (DMADRV_Callback_t)&ldma_callback, NULL);
  }
}

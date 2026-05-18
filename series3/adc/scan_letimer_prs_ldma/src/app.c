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
#include "sl_hal_letimer.h"
#include "sl_hal_prs.h"

#include "pin_config.h"
#include "peripheral_config.h"

// Desired LETIMER frequency in Hz
#define LETIMER_FREQ  2

// How many samples to capture
#define NUM_SAMPLES   10

const sl_gpio_t GPIO_ADC_INPUT0 = { .port = ADC_INPUT0_PORT,
                                    .pin  = ADC_INPUT0_PIN };
const sl_gpio_t GPIO_ADC_INPUT1 = { .port = ADC_INPUT1_PORT,
                                    .pin  = ADC_INPUT1_PIN };
const sl_gpio_t GPIO_LED0       = { .port = LED0_PORT,
                                    .pin  = LED0_PIN };
const sl_gpio_t GPIO_LETIMER0   = { .port = LETIMER_OUTPUT0_PORT,
                                    .pin  = LETIMER_OUTPUT0_PIN };

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
  sl_gpio_set_pin_mode(&GPIO_ADC_INPUT1, SL_GPIO_MODE_DISABLED, false);

  // Configure LED0 output
  sl_gpio_set_pin_mode(&GPIO_LED0, SL_GPIO_MODE_PUSH_PULL, false);

  // Configure LETIMER0 output to monitor sampling rate
  sl_gpio_set_pin_mode(&GPIO_LETIMER0, SL_GPIO_MODE_PUSH_PULL, false);

  // Enable optional low noise mode for ADC input
  GPIO->P_SET[ADC_INPUT0_PORT].AMUXMODE = 1 << ADC_INPUT0_PIN;
  GPIO->P_SET[ADC_INPUT1_PORT].AMUXMODE = 1 << ADC_INPUT1_PIN;

  // Allocate the analog bus for ADC0 inputs
  GPIO->ADC_INPUT0_BUS |= ADC_INPUT0_BUSALLOC;
  GPIO->ADC_INPUT1_BUS |= ADC_INPUT1_BUSALLOC;
}

void prs_init(void)
{
/***************************************************************************//**
 * Initialize PRS.
 ******************************************************************************/
  // Enable PRS peripheral bus clock
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_PRS);

  // Configure LETIMER0 as PRS producer to initiate scan ADC conversion
  sl_hal_prs_async_connect_channel_producer(ADC_ASYNC_PRS_CH,
                                            SL_HAL_PRS_ASYNC_LETIMER0_CH1);
  sl_hal_prs_connect_channel_consumer(ADC_ASYNC_PRS_CH, SL_HAL_PRS_TYPE_ASYNC,
                                      SL_HAL_PRS_CONSUMER_ADC0_ASYNCSEQ);
}

/***************************************************************************//**
 * @brief LETIMER initialization
 ******************************************************************************/
void letimer_init(void)
{
  // Declare initialization structures
  uint32_t branch_clock_freq;
  sl_hal_letimer_config_t letimerInit = SL_HAL_LETIMER_CONFIG_DEFAULT;

  // Enable LETIMER0 peripheral bus clock
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_LETIMER0);

  // Get ADCCLK frequency
  sl_clock_manager_get_clock_branch_frequency(SL_CLOCK_BRANCH_EM23GRPACLK,
                                              &branch_clock_freq);

  // Calculate the top value (frequency) based on clock source
  uint32_t topValue = branch_clock_freq / LETIMER_FREQ;

  // Reload top on underflow, toggle output, and run in free mode
  letimerInit.enable_top = true;
  letimerInit.underflow_output0_action = \
      SL_HAL_LETIMER_UNDERFLOW_OUTPUT_ACTION_TOGGLE;
  letimerInit.underflow_output1_action = \
      SL_HAL_LETIMER_UNDERFLOW_OUTPUT_ACTION_PULSE;
  letimerInit.repeat_mode = SL_HAL_LETIMER_REPEAT_MODE_FREE;

  // Enable LETIMER0 output0
  GPIO->LETIMERROUTE_SET->ROUTEEN = GPIO_LETIMER_ROUTEEN_OUT0PEN;
  GPIO->LETIMERROUTE->OUT0ROUTE = \
      (LETIMER_OUTPUT0_PORT << _GPIO_LETIMER_OUT0ROUTE_PORT_SHIFT) \
      | (LETIMER_OUTPUT0_PIN << _GPIO_LETIMER_OUT0ROUTE_PIN_SHIFT);

  // Initialize LETIMER0
  sl_hal_letimer_init(LETIMER0, &letimerInit);

  // Enable LETIMER0; must be enabled to set top value
  sl_hal_letimer_enable(LETIMER0);

  // Set LETIMER0 top value
  sl_hal_letimer_set_top(LETIMER0, topValue);
}

void ldma_callback(void)
{
/***************************************************************************//**
 * LDMA callback function toggles LED0.
 ******************************************************************************/
  // Toggle LED0 to notify that transfers are complete
  sl_gpio_toggle_pin(&GPIO_LED0);
}

void ldma_init(void)
{
/***************************************************************************//**
 * Initialize LDMA.
 ******************************************************************************/
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
  sl_hal_adc_scan_entry_t initScanEntry[2] = {SL_HAL_ADC_SCAN_ENTRY_DEFAULT,
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
  init.show_id = true;  // enable channel ID to differentiate in raw data buffer
  init.debug_halt = true;
  init.warmup_mode = SL_HAL_ADC_WARMUP_NORMAL;
  init.scan_trigger = SL_HAL_ADC_TRIGGER_PRSPOS;
  init.scan_trigger_action = SL_HAL_ADC_TRIGGER_ACTION_ONCE;
  init.data_valid = (sl_hal_adc_data_valid_t)(NUM_SAMPLES - 1);
  init.config[initScanEntry[0].config_id] = initConfig;
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
  prs_init();
  adc_init();
  ldma_init();
  letimer_init();

  // Start ADC scan
  sl_hal_adc_start(ADC0);

  // Start the LETIMER
  sl_hal_letimer_start(LETIMER0);
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
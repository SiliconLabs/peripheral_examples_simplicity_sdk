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
#include "sl_hal_ldma.h"
#include "em_iadc.h"
#include "em_prs.h"

#include "pin_config.h"

/*******************************************************************************
 *******************************   DEFINES   ***********************************
 ******************************************************************************/

// Set CLK_ADC to 10 MHz
#define CLK_SRC_ADC_FREQ    20000000  // CLK_SRC_ADC
#define CLK_ADC_FREQ        10000000  // CLK_ADC - 10 MHz max in normal mode

// Use specified PRS channel
#define PRS_CHANNEL               0

// How many samples to capture
#define NUM_SAMPLES               10

/*
 * This example enters EM2 in the main while() loop. Setting this
 * #define to 1 enables debug connectivity in EM2, which increases
 * current consumption by about 0.5 uA.
 */
#define EM2DEBUG                  1

/* Remap BUTTON0 to Port A for BRD4181A/EFR32MG21A010F1024IM32
 * This ensures proper interrupt functionality in EM2/3 power modes
 */
#if defined(EFR32MG21A010F1024IM32)
  #undef BUTTON0_PORT
  #undef BUTTON0_PIN
  #define BUTTON0_PORT SL_GPIO_PORT_A
  #define BUTTON0_PIN  6
#endif

// Define GPIO mapping for boards where LED0 and BUTTON0 share the same pins
#ifndef BUTTON0_PORT
  #define BUTTON0_PORT LED0_BUTTON0_PORT
  #define BUTTON0_PIN  LED0_BUTTON0_PIN
#endif

// Define GPIO mapping for boards where LED1 and BUTTON1 share the same pins
#ifndef LED1_PORT
  #define LED1_PORT LED1_BUTTON1_PORT
  #define LED1_PIN  LED1_BUTTON1_PIN
#endif

const sl_gpio_t GPIO_BUTTON0 = { .port = BUTTON0_PORT, .pin = BUTTON0_PIN };
const sl_gpio_t GPIO_LED1 = { .port = LED1_PORT, .pin = LED1_PIN };

/*******************************************************************************
 ***************************   GLOBAL VARIABLES   ******************************
 ******************************************************************************/

// Buffer to store IADC samples
uint32_t scanBuffer[NUM_SAMPLES];

unsigned int channelId;
LDMA_Descriptor_t ldma_desc;

/**************************************************************************//**
 * @brief  GPIO initialization
 *****************************************************************************/
void gpio_init(void)
{
  int32_t int_no = GPIO_BUTTON0.pin;
  // Initialize GPIO driver
  sl_gpio_init();

  // Configure LED0 output
  sl_gpio_set_pin_mode(&GPIO_LED1, SL_GPIO_MODE_PUSH_PULL, false);

  // Configure pushbutton 0 as input and enable interrupt
  sl_gpio_set_pin_mode(&GPIO_BUTTON0, SL_GPIO_MODE_INPUT_PULL_FILTER, true);
  sl_gpio_configure_external_interrupt(&GPIO_BUTTON0, &int_no,
                                         SL_GPIO_INTERRUPT_NO_EDGE,
                                         NULL,
                                         NULL);
}

/**************************************************************************//**
 * @brief  PRS initialization
 *****************************************************************************/
void prs_init(void)
{
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_PRS);

  // Connect the specified PRS channel to the GPIO producer
  PRS_SourceAsyncSignalSet(PRS_CHANNEL,
                           PRS_ASYNC_CH_CTRL_SOURCESEL_GPIO,
                           GPIO_BUTTON0.pin);

  // Connect the specified PRS channel to the IADC as the consumer
  PRS_ConnectConsumer(PRS_CHANNEL,
                      prsTypeAsync,
                      prsConsumerIADC0_SCANTRIGGER);
}

/**************************************************************************//**
 * @brief  IADC initialization
 *****************************************************************************/
void iadc_init(void)
{
  // Declare initialization structures
  IADC_Init_t init = IADC_INIT_DEFAULT;
  IADC_AllConfigs_t initAllConfigs = IADC_ALLCONFIGS_DEFAULT;
  IADC_InitScan_t initScan = IADC_INITSCAN_DEFAULT;

  // Scan table structure
  IADC_ScanTable_t scanTable = IADC_SCANTABLE_DEFAULT;

  // Enable IADC0 clock tree
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_IADC0);

  // Set the prescaler needed for the intended IADC clock frequency
  init.srcClkPrescale = IADC_calcSrcClkPrescale(IADC0, CLK_SRC_ADC_FREQ, 0);

  /*
   * These two settings are modified from the defaults to reduce the
   * IADC current.  In low-frequency use cases, such as this example,
   * iadcWarmupNormal shuts down the IADC between conversions, which
   * reduces current at the expense of requiring 5 microseconds of
   * warm-up time before a conversion can begin.
   *
   * In cases where a PRS event triggers scan conversions, enabling
   * iadcClkSuspend0 gates off the ADC_CLK until the PRS trigger event
   * occurs and again upon the completion of the channel conversions
   * specified in the scan table.
   */
  init.warmup = iadcWarmupNormal;
  init.iadcClkSuspend0 = true;

  /*
   * Configuration 0 is used by both scan and single conversions by
   * default.  Use internal bandgap as the reference and specify the
   * reference voltage in mV.
   *
   * Resolution is not configurable directly but is based on the
   * selected oversampling ratio (osrHighSpeed), which defaults to
   * 2x and generates 12-bit results.
   */
  initAllConfigs.configs[0].reference = iadcCfgReferenceInt1V2;
  initAllConfigs.configs[0].vRef = IADC_getReferenceVoltage(iadcCfgReferenceInt1V2);
  initAllConfigs.configs[0].osrHighSpeed = iadcCfgOsrHighSpeed2x;
  initAllConfigs.configs[0].analogGain = iadcCfgAnalogGain0P5x;

  /*
   * CLK_SRC_ADC must be prescaled by some value greater than 1 to
   * derive the intended CLK_ADC frequency.
   *
   * Based on the default 2x oversampling rate (OSRHS)...
   *
   * conversion time = ((4 * OSRHS) + 2) / fCLK_ADC
   *
   * ...which results in a maximum sampling rate of 833 ksps with the
   * 2-clock input multiplexer switching time is included.
   */
  initAllConfigs.configs[0].adcClkPrescale = IADC_calcAdcClkPrescale(IADC0,
                                                                     CLK_ADC_FREQ,
                                                                     0,
                                                                     iadcCfgModeNormal,
                                                                     init.srcClkPrescale);

  /*
   * Trigger conversions on the PRS0 rising edge input (PRS0 is not a
   * specific channel but simply the dedicated trigger input for scan
   * conversions; PRS1 serves the same purpose for single conversions).
   *
   * Set the SCANFIFODVL flag when there are 2 entries in the scan
   * FIFO.  Note that in this example, the interrupt associated with
   * the SCANFIFODVL flag in the IADC_IF register is not used.
   *
   * Enable DMA wake-up to save the results when the specified FIFO
   * level is hit.
   *
   * Allow a scan conversion sequence to start as soon as there is a
   * trigger event.
   */
  initScan.triggerSelect = iadcTriggerSelPrs0PosEdge;
  initScan.dataValidLevel = iadcFifoCfgDvl2;
  initScan.fifoDmaWakeup = true;
  initScan.start = true;

  /*
   * Configure entries in scan table.  CH0 is single-ended from
   * input 0; CH1 is single-ended from input 1.
   */
  scanTable.entries[0].posInput = IADC_INPUT_0_PORT_PIN;
  scanTable.entries[0].negInput = iadcNegInputGnd;
  scanTable.entries[0].includeInScan = true;

  scanTable.entries[1].posInput = IADC_INPUT_2_PORT_PIN;
  scanTable.entries[1].negInput = iadcNegInputGnd;
  scanTable.entries[1].includeInScan = true;

  // Initialize IADC
  IADC_init(IADC0, &init, &initAllConfigs);

  // Initialize scan
  IADC_initScan(IADC0, &initScan, &scanTable);

  // Allocate the analog bus for ADC0 inputs
  GPIO->IADC_INPUT_0_BUS |= IADC_INPUT_0_BUSALLOC;
  GPIO->IADC_INPUT_2_BUS |= IADC_INPUT_2_BUSALLOC;
}

/**************************************************************************//**
 * @brief  LDMA Callback
 *****************************************************************************/
void ldma_callback(void)
{
  // Toggle LED0 to notify that transfers are complete
  sl_gpio_toggle_pin(&GPIO_LED1);
}

/**************************************************************************//**
 * @brief LDMA initialization
 *****************************************************************************/
void ldma_init(void)
{
  bool active;

  // Trigger LDMA transfer on IADC scan completion
  LDMA_TransferCfg_t ldma_config =
    LDMA_TRANSFER_CFG_PERIPHERAL(ldmaPeripheralSignal_IADC0_IADC_SCAN);

  /*
   * Set up a linked descriptor to save scan results to the
   * user-specified buffer.  By linking the descriptor to itself
   * (the last argument is the relative jump in terms of the number of
   * descriptors), transfers will run continuously until firmware
   * otherwise stops them.
   */
  ldma_desc =
    (LDMA_Descriptor_t)LDMA_DESCRIPTOR_LINKREL_P2M_WORD(&(IADC0->SCANFIFODATA),
                                                          scanBuffer,
                                                          NUM_SAMPLES,
                                                          0);
  // Initialize DMADRV
  DMADRV_Init();

  // Halt on error if DMA channel cannot be allocated
  if (DMADRV_AllocateChannel(&channelId, NULL) != ECODE_EMDRV_DMADRV_OK) {
    __BKPT(0);
  }

  // Check that IADC->memory LDMA channel is not currently active; halt if error
  if (DMADRV_TransferActive(channelId, &active) != ECODE_EMDRV_DMADRV_OK) {
    __BKPT(0);
  }

  // Start LDMA transfer
  if (!active) {
    DMADRV_LdmaStartTransfer(channelId,
                             &ldma_config,
                             &ldma_desc,
                             (DMADRV_Callback_t)ldma_callback,
                             NULL);
  }
}

/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  // Initialize GPIO, PRS, IADC and LDMA
  gpio_init();
  prs_init();
  iadc_init();
  ldma_init();

#ifdef EM2DEBUG
#if (EM2DEBUG == 1)
  // Enable debug connectivity in EM2
  EMU->CTRL_SET = EMU_CTRL_EM2DBGEN;
#endif
#endif
}

/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{
}

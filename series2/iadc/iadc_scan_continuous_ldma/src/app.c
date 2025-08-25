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
#include "em_ldma.h"
#include "em_iadc.h"

#include "pin_config.h"

/*******************************************************************************
 *******************************   DEFINES   ***********************************
 ******************************************************************************/

// How many samples to capture
#define NUM_SAMPLES         1024

// Set CLK_ADC to 10 MHz
#define CLK_SRC_ADC_FREQ    20000000  // CLK_SRC_ADC
#define CLK_ADC_FREQ        10000000  // CLK_ADC - 10 MHz max in normal mode

/*
 * This example enters EM2 in the main while() loop. Setting this
 * #define to 1 enables debug connectivity in EM2, which increases
 * current consumption by about 0.5 uA.
 */
#define EM2DEBUG                  1

/*******************************************************************************
 ***************************   GLOBAL VARIABLES   ******************************
 ******************************************************************************/

// Buffer to store IADC samples
uint32_t scanBuffer[NUM_SAMPLES];

unsigned int channelId;
LDMA_Descriptor_t ldma_desc;

const sl_gpio_t GPIO_OUTPUT = { .port = GPIO_LDMA_COMPLETE_PORT,
                                .pin = GPIO_LDMA_COMPLETE_PIN };

/**************************************************************************//**
 * @brief  GPIO initialization
 *****************************************************************************/
void gpio_init(void)
{
  sl_gpio_init();

  // Output toggled upon completing each LDMA transfer sequence
  sl_gpio_set_pin_mode(&GPIO_OUTPUT, SL_GPIO_MODE_PUSH_PULL, LED_OFF);
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

  // Shutdown between conversions to reduce current
  init.warmup = iadcWarmupNormal;

  // Set the HFSCLK prescale value here
  init.srcClkPrescale = IADC_calcSrcClkPrescale(IADC0, CLK_SRC_ADC_FREQ, 0);

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
   * Trigger continuously once scan is started.  Note that
   * initScan.start = false by default, so conversions must be started
   * manually with IADC_command(IADC0, iadcCmdStartScan).
   *
   * Set the SCANFIFODVL flag when scan FIFO holds 2 entries.  In this
   * example, the interrupt associated with the SCANFIFODVL flag in
   * the IADC_IF register is not used.
   *
   * Enable DMA wake-up to save the results when the specified FIFO
   * level is hit.
   */
  initScan.triggerAction = iadcTriggerActionContinuous;
  initScan.dataValidLevel = iadcFifoCfgDvl2;
  initScan.fifoDmaWakeup = true;

  /*
   * Configure entries in the scan table.  CH0 is single-ended from
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
 * @brief  LDMA IRQ Callback
 *****************************************************************************/
void ldma_callback(void)
{
  // Toggle GPIO to notify that transfers are complete
  sl_gpio_toggle_pin(&GPIO_OUTPUT);
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
  gpio_init();
  iadc_init();

#ifdef EM2DEBUG
#if (EM2DEBUG == 1)
  // Enable debug connectivity in EM2
  EMU->CTRL_SET = EMU_CTRL_EM2DBGEN;
#endif
#endif

  ldma_init();

  IADC_command(IADC0, iadcCmdStartScan);
}

/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{
}

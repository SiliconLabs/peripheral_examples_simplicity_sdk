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
#include "em_letimer.h"
#include "em_prs.h"

#include "pin_config.h"
#include "peripheral_config.h"

/*******************************************************************************
 *******************************   DEFINES   ***********************************
 ******************************************************************************/

// Set CLK_ADC to 10 MHz
#define CLK_SRC_ADC_FREQ        20000000  // CLK_SRC_ADC
#define CLK_ADC_FREQ            10000000  // CLK_ADC - 10 MHz max in normal mode

// Desired LETIMER frequency in Hz
#define LETIMER_FREQ              1

// Use specified PRS channel
#define PRS_CHANNEL               0

// How many samples to capture
#define NUM_SAMPLES               10

/*
 * This example enters EM2. Setting this #define to 1 enables
 * debug connectivity in EM2, which increases current consumption
 * by about 0.5 uA.
 */
#define EM2DEBUG                  1

// Define GPIO mapping for boards where LED0 and BUTTON0 share the same pins
#ifndef LED0_PORT
  #define LED0_PORT LED0_BUTTON0_PORT
  #define LED0_PIN  LED0_BUTTON0_PIN
#endif

/*******************************************************************************
 ***************************   GLOBAL VARIABLES   ******************************
 ******************************************************************************/

 // Buffer for IADC samples
uint32_t singleBuffer[NUM_SAMPLES];

unsigned int channelId;
LDMA_Descriptor_t ldma_desc;

const sl_gpio_t LETIMER_OUTPUT = { .port = LETIMER_OUTPUT_0_PORT, 
                                   .pin = LETIMER_OUTPUT_0_PIN };
const sl_gpio_t GPIO_LED0 = { .port = LED0_PORT, 
                              .pin = LED0_PIN };

/**************************************************************************//**
 * @brief  GPIO initialization
 *****************************************************************************/
void gpio_init(void)
{
  // Initialize GPIO driver
  sl_gpio_init();


  // Show sample completion state on LED0
  sl_gpio_set_pin_mode(&GPIO_LED0, SL_GPIO_MODE_PUSH_PULL, LED_OFF);

  // Show LETIMER activity
  sl_gpio_set_pin_mode(&LETIMER_OUTPUT, SL_GPIO_MODE_PUSH_PULL, false);
}

/**************************************************************************//**
 * @brief  PRS initialization
 *****************************************************************************/
void prs_init(void)
{
  // Enable PRS clock branch
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_PRS);

  // Connect the specified PRS channel to the LETIMER producer
  PRS_SourceAsyncSignalSet(PRS_CHANNEL,
                           PRS_ASYNC_CH_CTRL_SOURCESEL_LETIMER0,
                           PRS_LETIMER0_CH0);

  // Connect the specified PRS channel to the IADC as the consumer
  PRS_ConnectConsumer(PRS_CHANNEL,
                      prsTypeAsync,
                      prsConsumerIADC0_SINGLETRIGGER);
}

/**************************************************************************//**
 * @brief  IADC initialization
 *****************************************************************************/
void iadc_init(void)
{
  // Declare initialization structures
  IADC_Init_t init = IADC_INIT_DEFAULT;
  IADC_AllConfigs_t initAllConfigs = IADC_ALLCONFIGS_DEFAULT;
  IADC_InitSingle_t initSingle = IADC_INITSINGLE_DEFAULT;

  // Single input structure
  IADC_SingleInput_t singleInput = IADC_SINGLEINPUT_DEFAULT;

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
   * In cases where a PRS event triggers single conversions, enabling
   * iadcClkSuspend1 gates off the ADC_CLK until the PRS trigger event
   * occurs and again upon the completion of the channel converted.
   */
  init.warmup = iadcWarmupNormal;
  init.iadcClkSuspend1 = true;

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
   * CLK_SRC_ADC is prescaled to derive the intended CLK_ADC frequency.
   *
   * Based on the default 2x oversampling rate (OSRHS)...
   *
   * conversion time = ((4 * OSRHS) + 2) / fCLK_ADC
   *
   * ...which, results in a maximum single-channel sampling rate of
   * 1 Msps because there is no need to change the input multiplexer
   * (thus incurring a 2-clock switching delay).
   */
  initAllConfigs.configs[0].adcClkPrescale = IADC_calcAdcClkPrescale(IADC0,
                                                                     CLK_ADC_FREQ,
                                                                     0,
                                                                     iadcCfgModeNormal,
                                                                     init.srcClkPrescale);

  /*
   * Trigger conversions on the PRS1 rising edge input (PRS1 is not a
   * specific channel but simply the dedicated trigger input for single
   * conversions.  PRS0 serves the same purpose for scan conversions.
   * Note that the IADC_TriggerSel_t typedef specifies PRS0 even when
   * PRS1 is used because the bit field encodings in the relevant
   * IADC_TRIGGER register are the same).
   *
   * Set the SINGLEFIFODVL flag when there are 2 entries in the scan
   * FIFO.  Note that in this example, the interrupt associated with
   * the SINGLEFIFODVL flag in the IADC_IF register is not used.
   *
   * Enable DMA wake-up to save the results when the specified FIFO
   * level is hit.
   *
   * Allow a single conversion to start as soon as there is a trigger
   * event.
   */
  initSingle.triggerSelect = iadcTriggerSelPrs0PosEdge;
  initSingle.dataValidLevel = iadcFifoCfgDvl2;
  initSingle.fifoDmaWakeup = true;
  initSingle.start = true;

  /* Specify the input channel.  When negInput = iadcNegInputGnd, the
   * conversion is single-ended.
   */
  singleInput.posInput = IADC_INPUT_0_PORT_PIN;
  singleInput.negInput = iadcNegInputGnd;

  // Initialize IADC
  IADC_init(IADC0, &init, &initAllConfigs);

  // Initialize single conversion
  IADC_initSingle(IADC0, &initSingle, &singleInput);

  // Allocate the analog bus for IADC0 inputs
  GPIO->IADC_INPUT_0_BUS |= IADC_INPUT_0_BUSALLOC;
}

/**************************************************************************//**
 * @brief LETIMER initialization
 *****************************************************************************/
void letimer_init(void)
{
  LETIMER_Init_TypeDef letimerInit = LETIMER_INIT_DEFAULT;

  // Enable LETIMER0 clock tree
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_LETIMER0);

  // Calculate the top value (frequency) based on clock source
  uint32_t topValue;
  sl_clock_manager_get_clock_branch_frequency(SL_CLOCK_BRANCH_EM23GRPACLK, &topValue);
  topValue = topValue / LETIMER_FREQ;

  // Reload top on underflow, pulse output, and run in free mode
  letimerInit.comp0Top = true;
  letimerInit.topValue = topValue;
  letimerInit.ufoa0 = letimerUFOAPulse;
  letimerInit.repMode = letimerRepeatFree;

  // Enable LETIMER output 0
  GPIO_LETIMERROUTE.ROUTEEN = GPIO_LETIMER_ROUTEEN_OUT0PEN;
  GPIO_LETIMERROUTE.OUT0ROUTE = (LETIMER_OUTPUT_0_PORT << _GPIO_LETIMER_OUT0ROUTE_PORT_SHIFT) |
                                 (LETIMER_OUTPUT_0_PIN << _GPIO_LETIMER_OUT0ROUTE_PIN_SHIFT);

  // Initialize LETIMER
  LETIMER_Init(LETIMER0, &letimerInit);
}

/**************************************************************************//**
 * @brief  LDMA Callback
 *****************************************************************************/
void ldma_callback(void)
{
  // Toggle LED0 to notify that transfers are complete
  sl_gpio_toggle_pin(&GPIO_LED0);
}

/**************************************************************************//**
 * @brief LDMA initialization
 *****************************************************************************/
void ldma_init(void)
{
  bool active;

  // Trigger LDMA transfer on IADC scan completion
  LDMA_TransferCfg_t ldma_config =
    LDMA_TRANSFER_CFG_PERIPHERAL(ldmaPeripheralSignal_IADC0_IADC_SINGLE);

  /*
   * Set up a linked descriptor to save scan results to the
   * user-specified buffer.  By linking the descriptor to itself
   * (the last argument is the relative jump in terms of the number of
   * descriptors), transfers will run continuously until firmware
   * otherwise stops them.
   */
  ldma_desc =
    (LDMA_Descriptor_t)LDMA_DESCRIPTOR_LINKREL_P2M_WORD(&(IADC0->SINGLEFIFODATA),
                                                          singleBuffer,
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
  prs_init();
  iadc_init();
  ldma_init();

  // Initialize LETIMER, start conversion
  letimer_init();

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

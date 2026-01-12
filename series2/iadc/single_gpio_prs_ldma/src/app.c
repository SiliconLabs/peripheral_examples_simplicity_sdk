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
#include "em_iadc.h"
#include "em_prs.h"
#include "em_ldma.h"

#include "pin_config.h"

/*******************************************************************************
 *******************************   DEFINES   ***********************************
 ******************************************************************************/

// Set CLK_ADC to 10MHz
#define CLK_SRC_ADC_FREQ          20000000 // CLK_SRC_ADC
#define CLK_ADC_FREQ              10000000 // CLK_ADC - 10 MHz max in normal mode

// Use specified LDMA/PRS channel
#define IADC_LDMA_CH              0
#define PRS_CHANNEL               0

// How many samples to capture
#define NUM_SAMPLES               10

/* This example enters EM2 in the main while() loop; Setting this #define to 1
 * enables debug connectivity in EM2, which increases current consumption by
 * about 0.5uA
 */
#define EM2DEBUG                  1

/* Remap BUTTON0_PORT to Port A for BRD4181A/EFR32MG21A010F1024IM32
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

const sl_gpio_t GPIO_PB0  = { .port = BUTTON0_PORT, 
                              .pin = BUTTON0_PIN };
const sl_gpio_t GPIO_LED1 = { .port = LED1_PORT, 
                              .pin = LED1_PIN };

/*******************************************************************************
 ***************************   GLOBAL VARIABLES   ******************************
 ******************************************************************************/

// LDMA link descriptor
LDMA_Descriptor_t descriptor;
unsigned int channelId;

// Buffer for IADC samples
uint32_t singleBuffer[NUM_SAMPLES];

/**************************************************************************//**
 * @brief GPIO Initializer
 *****************************************************************************/
void gpio_init (void)
{
  int32_t int_no = GPIO_PB0.pin;

  // Initialize GPIO driver
  sl_gpio_init();

  // Configure LED1 as a push pull output for LED control
  sl_gpio_set_pin_mode(&GPIO_LED1, SL_GPIO_MODE_PUSH_PULL, LED_OFF);

  // Configure pushbutton 0 as input and enable interrupt
  sl_gpio_set_pin_mode(&GPIO_PB0, SL_GPIO_MODE_INPUT_PULL, true);
  sl_gpio_configure_external_interrupt(&GPIO_PB0, &int_no,
                                       SL_GPIO_INTERRUPT_NO_EDGE,
                                       NULL,
                                       NULL);
}

/**************************************************************************//**
 * @brief PRS Initializer
 *****************************************************************************/
void prs_init (void)
{
  // Enable PRS clock branch
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_PRS);

  // Set up PRS GPIO and IADC as producer and consumer respectively
  PRS_SourceAsyncSignalSet(PRS_CHANNEL, PRS_ASYNC_CH_CTRL_SOURCESEL_GPIO, BUTTON0_PIN);
  PRS_ConnectConsumer(PRS_CHANNEL, prsTypeAsync, prsConsumerIADC0_SINGLETRIGGER);
}

/**************************************************************************//**
 * @brief  IADC Initializer
 *****************************************************************************/
void iadc_init (void)
{
  // Declare init structs
  IADC_Init_t init = IADC_INIT_DEFAULT;
  IADC_AllConfigs_t initAllConfigs = IADC_ALLCONFIGS_DEFAULT;
  IADC_InitSingle_t initSingle = IADC_INITSINGLE_DEFAULT;
  IADC_SingleInput_t initSingleInput = IADC_SINGLEINPUT_DEFAULT;

  // Enable IADC0 clock branch
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_IADC0);

  // Modify init structs and initialize
  init.warmup = iadcWarmupNormal;

  // Set the HFSCLK prescale value here
  init.srcClkPrescale = IADC_calcSrcClkPrescale(IADC0, CLK_SRC_ADC_FREQ, 0);

  /* Both single and scan conversions use configuration 0 by default.
   * Use internal bandgap (supply voltage in mV) as the reference.
   */
  initAllConfigs.configs[0].reference = iadcCfgReferenceInt1V2;
  initAllConfigs.configs[0].vRef = IADC_getReferenceVoltage(iadcCfgReferenceInt1V2);
  initAllConfigs.configs[0].analogGain = iadcCfgAnalogGain0P5x;

  // Divides CLK_SRC_ADC to set the CLK_ADC frequency
  initAllConfigs.configs[0].adcClkPrescale = IADC_calcAdcClkPrescale(IADC0,
                                             CLK_ADC_FREQ,
                                             0,
                                             iadcCfgModeNormal,
                                             init.srcClkPrescale);

  // Single initialization
  initSingle.triggerSelect = iadcTriggerSelPrs0PosEdge;
  initSingle.dataValidLevel = _IADC_SINGLEFIFOCFG_DVL_VALID1;

  // Enable triggering of single conversion
  initSingle.start = true;

  // Set to run in EM2
  initSingle.fifoDmaWakeup = true;

  // Assign pins to positive and negative inputs in differential mode
  initSingleInput.posInput   = IADC_INPUT_0_PORT_PIN;
  initSingleInput.negInput   = iadcNegInputGnd;

  // Allocate the analog bus for ADC0 inputs
  GPIO->IADC_INPUT_0_BUS |= IADC_INPUT_0_BUSALLOC;

  // Initialize IADC
  IADC_init(IADC0, &init, &initAllConfigs);

  // Initialize Single
  IADC_initSingle(IADC0, &initSingle, &initSingleInput);
}

/**************************************************************************//**
 * @brief  LDMA IRQ callback
 *****************************************************************************/
void ldma_callback(void)
{
  // Toggle LED1 to notify that transfers are complete
  sl_gpio_toggle_pin(&GPIO_LED1);
}

/**************************************************************************//**
 * @brief LDMA Initializer
 *****************************************************************************/
void ldma_init(void)
{
  bool active;

  // Configure LDMA for transfer from IADC to memory
  LDMA_TransferCfg_t transferCfg =
    LDMA_TRANSFER_CFG_PERIPHERAL(ldmaPeripheralSignal_IADC0_IADC_SINGLE);

  // Set up descriptor for dual buffer transfer
  descriptor = (LDMA_Descriptor_t)LDMA_DESCRIPTOR_LINKREL_P2M_WORD(&IADC0->SINGLEFIFODATA, 
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
                             &transferCfg,
                             &descriptor,
                             (DMADRV_Callback_t)ldma_callback,
                             NULL);
  }
}

/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  // Initialize GPIO
  gpio_init();

  // Initialize PRS
  prs_init();

  // Initialize the IADC
  iadc_init();

  // Initialize LDMA
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

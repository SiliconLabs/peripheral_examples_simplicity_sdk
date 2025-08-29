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
#include "em_iadc.h"
#include "em_prs.h"

#include "pin_config.h"

/*******************************************************************************
 *******************************   DEFINES   ***********************************
 ******************************************************************************/

// Use specified PRS channel
#define IADC_PRS_CH              0

/* Set CLK_ADC to 10MHz (this corresponds to a sample rate of 77K with OSR = 32)
 * CLK_ADC; IADC_SCHEDx PRESCALE has 10 valid bits
 */
#define CLK_ADC_FREQ             10000000
// CLK_SRC_ADC; largest division is by 4
#define CLK_SRC_ADC_FREQ         20000000

// GPIO output toggle to notify IADC conversion complete
#define GPIO_OUTPUT_0_PORT        gpioPortB
#define GPIO_OUTPUT_0_PIN         1

const sl_gpio_t GPIO_OUTPUT0 = { .port = GPIO_OUTPUT_0_PORT, 
                                 .pin = GPIO_OUTPUT_0_PIN };

/*
 * This example enters EM2 in the main while() loop. Setting this #define to 1
 * enables debug connectivity in EM2, which increases current consumption by
 * about 0.5uA
 */
#define EM2DEBUG                  1

/*******************************************************************************
 ***************************   GLOBAL VARIABLES   ******************************
 ******************************************************************************/

// Stores latest ADC sample and converts to volts
static volatile IADC_Result_t sample;
static volatile double singleResult;

/**************************************************************************//**
 * @brief  GPIO Initializer
 *****************************************************************************/
void gpio_init(void)
{
  // Initialize GPIO driver
  sl_gpio_init();

  // Configure GPIO as an output to indicate conversion completion
  sl_gpio_set_pin_mode(&GPIO_OUTPUT0, SL_GPIO_MODE_PUSH_PULL, 0);
}

/**************************************************************************//**
 * @brief  PRS Initializer
 *****************************************************************************/
void prs_init(void)
{
  // Enable PRS clock branch
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_PRS);

  // Connect selected PRS channel to ADC single conversion complete producer
  PRS_SourceAsyncSignalSet(IADC_PRS_CH, 
                           PRS_ASYNC_CH_CTRL_SOURCESEL_IADC0,
                           PRS_ASYNC_CH_CTRL_SIGSEL_IADC0SINGLEDONE);

  // Route PRS channel to GPIO output to indicate a complete conversion
  PRS_PinOutput(IADC_PRS_CH, prsTypeAsync, GPIO_OUTPUT_0_PORT, GPIO_OUTPUT_0_PIN);
}

/**************************************************************************//**
 * @brief  IADC Initializer
 *****************************************************************************/
void iadc_init(void)
{
  // Declare init structs
  IADC_Init_t init = IADC_INIT_DEFAULT;
  IADC_AllConfigs_t initAllConfigs = IADC_ALLCONFIGS_DEFAULT;
  IADC_InitSingle_t initSingle = IADC_INITSINGLE_DEFAULT;
  IADC_SingleInput_t initSingleInput = IADC_SINGLEINPUT_DEFAULT;

  // Enable IADC0 clock branch
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_IADC0);

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

  // Set the HFSCLK prescale value here
  init.srcClkPrescale = IADC_calcSrcClkPrescale(IADC0, CLK_SRC_ADC_FREQ, 0);

  /* Configuration 0 is used by both scan and single conversions by default
   * Use internal bandgap (supply voltage in mV) as reference
   */
  initAllConfigs.configs[0].reference = iadcCfgReferenceInt1V2;
  initAllConfigs.configs[0].vRef = IADC_getReferenceVoltage(iadcCfgReferenceInt1V2);
  initAllConfigs.configs[0].analogGain = iadcCfgAnalogGain0P5x;

  // Divides CLK_SRC_ADC to set the CLK_ADC frequency for desired sample rate
  initAllConfigs.configs[0].adcClkPrescale = IADC_calcAdcClkPrescale(IADC0,
                                                                    CLK_ADC_FREQ,
                                                                    0,
                                                                    iadcCfgModeNormal,
                                                                    init.srcClkPrescale);

  /* Set oversampling rate to 32x
   * resolution formula res = 11 + log2(oversampling * digital averaging)
   * in this case res = 11 + log2(32*1) = 16
   */
  initAllConfigs.configs[0].osrHighSpeed = iadcCfgOsrHighSpeed32x;


  // Single initialization
  initSingle.dataValidLevel = iadcFifoCfgDvl1;

  // Set conversions to run continuously
  initSingle.triggerAction = iadcTriggerActionContinuous;

  // Set alignment to right justified with 16 bits for data field
  initSingle.alignment = iadcAlignRight16;

  // Configure Input sources for single ended conversion
  initSingleInput.posInput = IADC_INPUT_0_PORT_PIN;
  initSingleInput.negInput = iadcNegInputGnd;

  // Initialize IADC
  IADC_init(IADC0, &init, &initAllConfigs);

  // Initialize Single
  IADC_initSingle(IADC0, &initSingle, &initSingleInput);

  // Allocate the analog bus for ADC0 inputs
  GPIO->IADC_INPUT_0_BUS |= IADC_INPUT_0_BUSALLOC;

  // Enable interrupts on single conversion completion
  IADC_enableInt(IADC0, IADC_IEN_SINGLEDONE);
  sl_interrupt_manager_clear_irq_pending(IADC_IRQn);
  sl_interrupt_manager_enable_irq(IADC_IRQn);
}

/**************************************************************************//**
 * @brief  IADC interrupt handler
 *****************************************************************************/
void IADC_IRQHandler(void)
{
  // Read most recent single conversion result
  sample = IADC_readSingleResult(IADC0);

  /* For single-ended the result range is 0 to +Vref, i.e., 16 bits for the
   * conversion value.
   */
  singleResult = (double)sample.data * 2.42 / 0xFFFF;

  IADC_clearInt(IADC0, IADC_IF_SINGLEDONE);
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

#ifdef EM2DEBUG
#if (EM2DEBUG == 1)
  // Enable debug connectivity in EM2
  EMU->CTRL_SET = EMU_CTRL_EM2DBGEN;
#endif
#endif

  // Start single
  IADC_command(IADC0, iadcCmdStartSingle);
}

/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{
  // Enter EM2 sleep with power manager, woken by IADC interrupt
}

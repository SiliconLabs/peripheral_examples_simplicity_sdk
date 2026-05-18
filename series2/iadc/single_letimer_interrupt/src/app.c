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
#include "em_iadc.h"
#include "em_letimer.h"

#include "pin_config.h"
#include "peripheral_config.h"

/*******************************************************************************
 *******************************   DEFINES   ***********************************
 ******************************************************************************/

// Set CLK_ADC to 10MHz
#define CLK_SRC_ADC_FREQ          20000000 // CLK_SRC_ADC
#define CLK_ADC_FREQ              10000000 // CLK_ADC - 10MHz max in normal mode

// Desired LETIMER frequency in Hz
#define LETIMER_FREQ              1

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

// Stores latest IADC sample and conversion to Volts
static volatile IADC_Result_t sample;
static volatile double singleResult;    // Volts

const sl_gpio_t LETIMER_OUTPUT = { .port = LETIMER_OUTPUT_0_PORT, 
                                   .pin = LETIMER_OUTPUT_0_PIN };
const sl_gpio_t GPIO_LED0 = { .port = LED0_PORT, 
                              .pin = LED0_PIN };

/**************************************************************************//**
 * @brief  GPIO Initializer
 *****************************************************************************/
void gpio_init (void)
{
  // Initialize GPIO driver
  sl_gpio_init();

  // Configure LED1/LETIMER as outputs
  sl_gpio_set_pin_mode(&GPIO_LED0, SL_GPIO_MODE_PUSH_PULL, LED_OFF);
  sl_gpio_set_pin_mode(&LETIMER_OUTPUT, SL_GPIO_MODE_PUSH_PULL, false);
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

  /* Configuration 0 is used by both scan and single conversions by default
   * Use internal bandgap (supply voltage in mV) as reference
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

  // Assign pins to positive and negative inputs in differential mode
  initSingleInput.posInput   = IADC_INPUT_0_PORT_PIN;
  initSingleInput.negInput   = iadcNegInputGnd;

  // Allocate the analog bus for ADC0 inputs
  GPIO->IADC_INPUT_0_BUS |= IADC_INPUT_0_BUSALLOC;

  // Initialize IADC
  IADC_init(IADC0, &init, &initAllConfigs);

  // Initialize Single
  IADC_initSingle(IADC0, &initSingle, &initSingleInput);

  // Enable IADC single-channel done interrupts
  IADC_clearInt(IADC0, _IADC_IF_MASK);
  IADC_enableInt(IADC0, IADC_IEN_SINGLEDONE);
  sl_interrupt_manager_clear_irq_pending(IADC_IRQn);
  sl_interrupt_manager_enable_irq(IADC_IRQn);
}

/**************************************************************************//**
 * @brief  IADC interrupt handler
 *****************************************************************************/
void IADC_IRQHandler(void)
{
  uint32_t flags = IADC_getInt(IADC0);

  // Toggle GPIO
  sl_gpio_toggle_pin(&GPIO_LED0);

  // Read data from the FIFO
  sample = IADC_pullSingleFifoResult(IADC0);

  /* For single-ended the result range is 0 to +Vref, i.e., 12 bits for the
   * conversion value.
   */
  singleResult = sample.data * 2.42 / 0xFFF;

  // Clear IADC interrupt flags
  IADC_clearInt(IADC0, flags);
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


  // Reload top on underflow, PWM output, and run in free mode
  letimerInit.comp0Top = true;
  letimerInit.topValue = topValue;
  letimerInit.ufoa0 = letimerUFOAPulse;
  letimerInit.repMode = letimerRepeatFree;

  // Enable LETIMER0 output0
  GPIO_LETIMERROUTE.ROUTEEN = GPIO_LETIMER_ROUTEEN_OUT0PEN;
  GPIO_LETIMERROUTE.OUT0ROUTE = \
      (LETIMER_OUTPUT_0_PORT << _GPIO_LETIMER_OUT0ROUTE_PORT_SHIFT) \
      | (LETIMER_OUTPUT_0_PIN << _GPIO_LETIMER_OUT0ROUTE_PIN_SHIFT);

  // Initialize LETIMER
  LETIMER_Init(LETIMER0, &letimerInit);

  // Enable LETIMER underflow interrupts
  LETIMER_IntClear(LETIMER0, _LETIMER_IF_MASK);
  LETIMER_IntEnable(LETIMER0, LETIMER_IEN_UF);
  sl_interrupt_manager_clear_irq_pending(LETIMER0_IRQn);
  sl_interrupt_manager_enable_irq(LETIMER0_IRQn);
}

/**************************************************************************//**
 * @brief  LETIMER interrupt handler
 *****************************************************************************/
void LETIMER0_IRQHandler(void)
{
  uint32_t flags = LETIMER_IntGet(LETIMER0);

  // Trigger an IADC single conversion
  IADC_command(IADC0, iadcCmdStartSingle);

  // Clear LETIMER interrupt flags
  LETIMER_IntClear(LETIMER0, flags);
}

/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  gpio_init();
  iadc_init();
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

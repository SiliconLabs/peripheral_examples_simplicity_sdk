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

#include "em_iadc.h"

#include "sl_clock_manager.h"
#include "sl_gpio.h"
#include "sl_interrupt_manager.h"

#include "pin_config.h"
#include "peripheral_config.h"

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

const sl_gpio_t GPIO_LED1 = { .port = LED1_PORT, .pin = LED1_PIN };
const sl_gpio_t GPIO_PB0  = { .port = BUTTON0_PORT, .pin = BUTTON0_PIN };

/*******************************************************************************
 *******************************   DEFINES   ***********************************
 ******************************************************************************/

// Set CLK_ADC to 100 kHz (this corresponds to a sample rate of 10 ksps)
#define CLK_SRC_ADC_FREQ        9600000  // CLK_SRC_ADC
#define CLK_ADC_FREQ            100000   // CLK_ADC

// Upper and lower bound for Window Comparator
// 16-bit left-justified format; 12-bit conversion result compared to upper 12-bits of window comparator
#define WINDOW_UPPER_BOUND      0xC000   // 1.815V
#define WINDOW_LOWER_BOUND      0x4000   // 0.605V

/*******************************************************************************
 ***************************   GLOBAL VARIABLES   *******************************
 ******************************************************************************/

// Stores latest ADC sample
static volatile IADC_Result_t sample;
static volatile double singleResult;

/**************************************************************************//**
 *  Initialize GPIOs for push button and LED control.
 *****************************************************************************/
void initGPIO (void)
{
  // Initialize GPIO driver
  sl_gpio_init();

  // Configure LED1 as a push-pull output for LED control
  sl_gpio_set_pin_mode(&GPIO_LED1, SL_GPIO_MODE_PUSH_PULL, LED_OFF);
}

/**************************************************************************//**
 * @brief  IADC Initializer
 *****************************************************************************/
void initIADC (void)
{
  // Declare init structs
  IADC_Init_t init = IADC_INIT_DEFAULT;
  IADC_AllConfigs_t initAllConfigs = IADC_ALLCONFIGS_DEFAULT;
  IADC_InitSingle_t initSingle = IADC_INITSINGLE_DEFAULT;
  IADC_SingleInput_t initSingleInput = IADC_SINGLEINPUT_DEFAULT;

  // Enable IADC clock
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_IADC0);

  // Reset IADC to reset configuration in case it has been modified
  IADC_reset(IADC0);

  // Modify init structs and initialize
  init.warmup = iadcWarmupKeepWarm;

  // Set the HFSCLK prescale value here
  init.srcClkPrescale = IADC_calcSrcClkPrescale(IADC0, CLK_SRC_ADC_FREQ, 0);

  // Set lower bound for window compare
  init.greaterThanEqualThres = WINDOW_LOWER_BOUND;

  // Set upper bound for window compare
  init.lessThanEqualThres = WINDOW_UPPER_BOUND;

  // Configuration 0 is used by both scan and single conversions by default
  // Use internal bandgap (supply voltage in mV) as reference
  initAllConfigs.configs[0].reference = iadcCfgReferenceInt1V2;
  initAllConfigs.configs[0].vRef = 1210;
  initAllConfigs.configs[0].analogGain = iadcCfgAnalogGain0P5x;

  // Divide CLK_SRC_ADC to set the CLK_ADC frequency for desired sample rate
  initAllConfigs.configs[0].adcClkPrescale = IADC_calcAdcClkPrescale(IADC0,
                                                                    CLK_ADC_FREQ,
                                                                    0,
                                                                    iadcCfgModeNormal,
                                                                    init.srcClkPrescale);

  // Set conversions to run continuously
  initSingle.triggerAction = iadcTriggerActionContinuous;

  // Configure Input sources for single ended conversion
  initSingleInput.posInput = IADC_INPUT_0_PORT_PIN;
  initSingleInput.negInput = iadcNegInputGnd;

  // Enable window comparisons on this input
  initSingleInput.compare = true;

  // Initialize IADC
  IADC_init(IADC0, &init, &initAllConfigs);

  // Initialize Scan
  IADC_initSingle(IADC0, &initSingle, &initSingleInput);

  // Allocate the analog bus for ADC0 inputs
  GPIO->IADC_INPUT_0_BUS |= IADC_INPUT_0_BUSALLOC;

  // Enable interrupts on window comparison match
  IADC_enableInt(IADC0, IADC_IEN_SINGLECMP);

  // Enable ADC interrupts
  sl_interrupt_manager_clear_irq_pending(IADC_IRQn);
  sl_interrupt_manager_enable_irq(IADC_IRQn);
}

/**************************************************************************//**
 * @brief  IADC interrupt handler
 *****************************************************************************/
void IADC_IRQHandler(void)
{
  IADC_clearInt(IADC0, IADC_IF_SINGLECMP);

  // Read most recent sample
  sample = IADC_readSingleResult(IADC0);

  // Calculate input voltage:
  // For single-ended the result range is 0 to +Vref, i.e.,
  // for Vref = VBGR = 1.21V, and with analog gain = 0.5,
  // 12 bits represents 2.42V full scale IADC range.
  singleResult = sample.data * 2.42 / 0xFFF;

  // Toggle WSTK LED1 to signal compare event
  sl_gpio_toggle_pin(&GPIO_LED1);

}

/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  // Initialize GPIO
  initGPIO();

  // Initialize the IADC
  initIADC();

  // Start scan
  IADC_command(IADC0, iadcCmdStartSingle);
}

/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{

}

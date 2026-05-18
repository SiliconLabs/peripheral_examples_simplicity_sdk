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

#include <stdio.h>
#include <string.h>

#include "em_iadc.h"
#include "sl_clock_manager.h"
#include "sl_gpio.h"
#include "sl_interrupt_manager.h"
#include "pin_config.h"
#include "peripheral_config.h"

/******************************************************************************
 *******************************   DEFINES   **********************************
 *****************************************************************************/
// Set CLK_ADC to 10 MHz
#define CLK_SRC_ADC_FREQ    20000000  // CLK_SRC_ADC
#define CLK_ADC_FREQ        10000000  // CLK_ADC - 10 MHz max in normal mode

 // Define GPIO mapping for boards where LED1 and button1 share the same pins
 #ifndef LED1_PORT
   #define LED1_PORT LED1_BUTTON1_PORT
   #define LED1_PIN  LED1_BUTTON1_PIN
 #endif

const sl_gpio_t GPIO_LED1 = { .port = LED1_PORT, .pin = LED1_PIN };

/******************************************************************************
 ***************************   GLOBAL VARIABLES   *****************************
 *****************************************************************************/

// Stores latest ADC sample
static volatile IADC_Result_t sample;

// Upper and lower bound for Window Comparator
// 16-bit left-justified format; 12-bit conversion result compared to upper
// 12-bits of window comparator
static uint16_t vdd_lower_bound = 0x6AA0;  // 2V
static uint16_t vdd_upper_bound = 0x0000;  // 0V

// measured voltage from both channels using ADC
static volatile uint16_t voltagesmV[2];

/**************************************************************************//**
 *  Initialize GPIOs for push button and LED control.
 *****************************************************************************/
void initGPIO (void)
{
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
  IADC_InitScan_t initScan = IADC_INITSCAN_DEFAULT;

  // Scan table structure
  IADC_ScanTable_t scanTable = IADC_SCANTABLE_DEFAULT;

  // Enable IADC clock
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_IADC0);

  // Reset IADC to reset configuration in case it has been modified
  IADC_reset(IADC0);

  // Modify init structs and initialize
  init.warmup = iadcWarmupKeepWarm;

  // Set the clk_src_adc prescale value here
  init.srcClkPrescale = IADC_calcSrcClkPrescale(IADC0, CLK_SRC_ADC_FREQ, 0);

  // Set upper bound for window compare
  init.greaterThanEqualThres = vdd_upper_bound;

  // Set lower bound for window compare
  init.lessThanEqualThres = vdd_lower_bound;

  // Configuration 0 is used by both scan and single conversions by default
  // Use internal 1.2V bandgap as reference
  initAllConfigs.configs[0].reference = iadcCfgReferenceInt1V2;

  // Divides CLK_SRC_ADC to set the CLK_ADC frequency for desired sample rate
  initAllConfigs.configs[0].adcClkPrescale = IADC_calcAdcClkPrescale(IADC0,
                                                                     CLK_ADC_FREQ,
                                                                     0,
                                                                     iadcCfgModeNormal,
                                                                     init.srcClkPrescale);

  // Enable channel identification
  initScan.showId = true;

  // Set conversions to run continuously
  initScan.triggerAction = iadcTriggerActionContinuous;

  // Configure Input sources for the first channel for AVDD
  scanTable.entries[0].posInput = iadcPosInputAvdd;
  scanTable.entries[0].negInput = iadcNegInputGnd;
  scanTable.entries[0].includeInScan = true;

  // Enable window comparisons on this input
  scanTable.entries[0].compare = true;

  // Configure Input sources for the second channel for VDDIO
  scanTable.entries[1].posInput = iadcPosInputVddio;
  scanTable.entries[1].negInput = iadcNegInputGnd;
  scanTable.entries[1].includeInScan = true;

  // Enable window comparisons on this input
  scanTable.entries[1].compare = true;

  // Initialize IADC
  IADC_init(IADC0, &init, &initAllConfigs);

  // Initialize scan
  IADC_initScan(IADC0, &initScan, &scanTable);

  // Enable interrupts on window comparison match in channel scan
  IADC_enableInt(IADC0, IADC_IF_SCANCMP);

  // Enable ADC interrupts
  sl_interrupt_manager_clear_irq_pending(IADC_IRQn);
  sl_interrupt_manager_enable_irq(IADC_IRQn);
}

/**************************************************************************//**
 * @brief  ADC Handler
 *****************************************************************************/
void IADC_IRQHandler(void)
{
  IADC_clearInt(IADC0, IADC_IF_SCANCMP);

  // Read most recent sample
  sample = IADC_readScanResult(IADC0);
  voltagesmV[sample.id] = (sample.data)*4*1200/4095;

  sl_gpio_toggle_pin(&GPIO_LED1);
}

/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  // Initialize GPIO
  initGPIO();

  // Initialize IADC
  initIADC();

  // Start scan
  IADC_command(IADC0, iadcCmdStartScan);
}


/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{

}

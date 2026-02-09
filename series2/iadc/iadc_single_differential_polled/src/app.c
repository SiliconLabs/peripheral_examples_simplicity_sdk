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

#include "sl_gpio.h"
#include "em_iadc.h"

#include "pin_config.h"

/*******************************************************************************
 *******************************   DEFINES   ***********************************
 ******************************************************************************/

// Set CLK_ADC to 10MHz
#define CLK_SRC_ADC_FREQ          20000000 // CLK_SRC_ADC
#define CLK_ADC_FREQ              10000000 // CLK_ADC - 10MHz max in normal mode

/*******************************************************************************
 ***************************   GLOBAL VARIABLES   ******************************
 ******************************************************************************/

static volatile uint32_t sample;
static volatile double singleResult; // Volts

/**************************************************************************//**
 * @brief  Initialize IADC function
 *****************************************************************************/
void iadc_init (void)
{
  // Declare init structs
  IADC_Init_t init = IADC_INIT_DEFAULT;
  IADC_AllConfigs_t initAllConfigs = IADC_ALLCONFIGS_DEFAULT;
  IADC_InitSingle_t initSingle = IADC_INITSINGLE_DEFAULT;
  IADC_SingleInput_t initSingleInput = IADC_SINGLEINPUT_DEFAULT;

  // Enable IADC0 and GPIO clock branches
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_IADC0);
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_GPIO);


  /* Reset IADC to reset configuration in case it has been modified by
   * other code
   */
  IADC_reset(IADC0);

  // Modify init structs and initialize
  init.warmup = iadcWarmupKeepWarm;

  // Set the HFSCLK prescale value here
  init.srcClkPrescale = IADC_calcSrcClkPrescale(IADC0, CLK_SRC_ADC_FREQ, 0);

  /* Configuration 0 is used by both scan and single conversions by default
   * Use internal bandgap (supply voltage in mV) as reference
   */
  
  initAllConfigs.configs[0].reference = iadcCfgReferenceInt1V2;
  initAllConfigs.configs[0].vRef = IADC_getReferenceVoltage(iadcCfgReferenceInt1V2);;
  initAllConfigs.configs[0].analogGain = iadcCfgAnalogGain0P5x;

  // Divides CLK_SRC_ADC to set the CLK_ADC frequency
  initAllConfigs.configs[0].adcClkPrescale = IADC_calcAdcClkPrescale(IADC0,
                                             CLK_ADC_FREQ,
                                             0,
                                             iadcCfgModeNormal,
                                             init.srcClkPrescale);

  // Assign pins to positive and negative inputs in differential mode
  initSingleInput.posInput   = IADC_INPUT_0_PORT_PIN;
  initSingleInput.negInput   = IADC_INPUT_1_PORT_PIN;

  // Initialize the IADC
  IADC_init(IADC0, &init, &initAllConfigs);

  // Initialize the Single conversion inputs
  IADC_initSingle(IADC0, &initSingle, &initSingleInput);

  // Allocate the analog bus for ADC0 inputs
  GPIO->IADC_INPUT_0_BUS |= IADC_INPUT_0_BUSALLOC;
  GPIO->IADC_INPUT_1_BUS |= IADC_INPUT_1_BUSALLOC;
}
/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  // Initialize the IADC
  iadc_init();
}

/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{
  // Start IADC conversion
  IADC_command(IADC0, iadcCmdStartSingle);

  /* Wait for the single conversion to complete. A conversion is done when the
   * IADC->STATUS register's SINGLEFIFODV and CONVERTING bits are 1 (conversion
   * data is in the output FIFO) and 0 (a conversion is not currently in
   * progress), respectively.
   */
  while((IADC0->STATUS & (_IADC_STATUS_CONVERTING_MASK
              | _IADC_STATUS_SINGLEFIFODV_MASK)) != IADC_STATUS_SINGLEFIFODV);

  // Get ADC result
  sample = IADC_pullSingleFifoResult(IADC0).data;

  /* For differential inputs, the range is from -Vref to + Vref, i.e.,
   * for Vref = VBGR = 1.21 V, and with analog gain = 0.5, 12 bits represents
   * (-Vref / 0.5) to (+Vref / 0.5) = -2.42 V to +2.42 V.
   */
  singleResult = ((double)sample * 4.84) / 0xFFF;
}

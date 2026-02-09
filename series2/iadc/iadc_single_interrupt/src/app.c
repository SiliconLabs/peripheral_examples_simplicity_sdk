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

#include "pin_config.h"

/*******************************************************************************
 *******************************   DEFINES   ***********************************
 ******************************************************************************/

// Set CLK_ADC to 10 MHz
#define CLK_SRC_ADC_FREQ        20000000  // CLK_SRC_ADC
#define CLK_ADC_FREQ            10000000  // CLK_ADC - 10 MHz max in normal mode

/*
 * This example enters EM2. Setting this #define to 1 enables
 * debug connectivity in EM2, which increases current consumption
 * by about 0.5 uA.
 */
#define EM2DEBUG                  1

/*******************************************************************************
 ***************************   GLOBAL VARIABLES   ******************************
 ******************************************************************************/

// Raw IADC conversion result
static volatile IADC_Result_t sample;

// Result converted to volts
static volatile double singleResult;

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

   // Enable IADC0 clock branch.
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_IADC0);

  // Shutdown between conversions to reduce current
  init.warmup = iadcWarmupNormal;

  // Set the prescaler needed for the intended IADC clock frequency
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

  /* Specify the input channel.  When negInput = iadcNegInputGnd, the
   * conversion is single-ended.
   */
  singleInput.posInput   = IADC_INPUT_0_PORT_PIN;
  singleInput.negInput   = iadcNegInputGnd;

  // Allocate the analog bus for ADC0 inputs
  GPIO->IADC_INPUT_0_BUS |= IADC_INPUT_0_BUSALLOC;

  // Initialize IADC
  IADC_init(IADC0, &init, &initAllConfigs);

  // Initialize a single-channel conversion
  IADC_initSingle(IADC0, &initSingle, &singleInput);

  // Enable IADC single-channel done interrupts
  IADC_clearInt(IADC0, _IADC_IF_MASK);
  IADC_enableInt(IADC0, IADC_IEN_SINGLEDONE);
  sl_interrupt_manager_clear_irq_pending(IADC_IRQn);
  sl_interrupt_manager_enable_irq(IADC_IRQn);
}

/**************************************************************************//**
 * @brief  IADC IRQ Handler
 *****************************************************************************/
void IADC_IRQHandler(void)
{
  // Read a result from the FIFO
  sample = IADC_pullSingleFifoResult(IADC0);

  /*
   * Calculate the voltage converted as follows:
   *
   * For single-ended conversions, the result can range from 0 to
   * +Vref, i.e., for Vref = VBGR = 1.21V, and with analog gain = 0.5
   * 0xFFF represents the full scale value of 2.42V.
   */
  singleResult = sample.data * 2.42 / 0xFFF;

  /* Clear the single conversion complete interrupt.  Reading FIFO
   * results does not do this automatically.
   */
  IADC_clearInt(IADC0, IADC_IF_SINGLEDONE);
}


/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  sl_gpio_init();
  iadc_init();

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
  /*
   * Care must be taken when using the SINGLEDONE interrupt as a way
   * to exit low-energy modes, especially EM2/3.  Because the IADC
   * is has its own clock  (it runs from the IADC_CLK vs. the PCLK as
   * is the case for most other peripherals), register accesses
   * must be synchronized and require additional clock cycles.
   *
   * This matters because if the ADC_CLK is fast enough, it is
   * possible to start a single conversion, have it finish, and
   * request an interrupt before the device can enter EM2/3.
   *
   * ADC_CLK = 1 MHz (minimum conversion time = 10 microseconds) in
   * this example allows sufficient time to start the conversion
   * and enter EM2 so that the SINGLEDONE interrupt wakes the
   * device.  However, if ADC_CLK is sufficiently fast relative to
   * the CPU clock frequency, this may not be possible.
   */
  IADC_command(IADC0, iadcCmdStartSingle);
}

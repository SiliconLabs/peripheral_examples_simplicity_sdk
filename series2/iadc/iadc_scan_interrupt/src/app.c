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

// Set CLK_ADC to 10MHz
#define CLK_SRC_ADC_FREQ      20000000  // CLK_SRC_ADC
#define CLK_ADC_FREQ          10000000  // CLK_ADC - 10 MHz max in normal mode

/*
 * This example enters EM2; Setting this #define
 * to 1 enables debug connectivity in EM2, which increases current
 * consumption by about 0.5 uA.
 */
#define EM2DEBUG                  1

/*******************************************************************************
 ***************************   GLOBAL VARIABLES   ******************************
 ******************************************************************************/

static volatile double scanResult[IADC_SCAN_NUM_INPUTS];  // Volts

/**************************************************************************//**
 * @brief  IADC Initializer
 *****************************************************************************/
void iadc_init(void)
{
  // Declare init structs
  IADC_Init_t init = IADC_INIT_DEFAULT;
  IADC_AllConfigs_t initAllConfigs = IADC_ALLCONFIGS_DEFAULT;
  IADC_InitScan_t initScan = IADC_INITSCAN_DEFAULT;
  IADC_ScanTable_t initScanTable = IADC_SCANTABLE_DEFAULT;    // Scan Table

  // Enable IADC0 and GPIO clock branches.
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_IADC0);

  // Modify init structures and initialize
  init.warmup = iadcWarmupKeepWarm;

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

  // Divide CLK_SRC_ADC to set the CLK_ADC frequency
  initAllConfigs.configs[0].adcClkPrescale = IADC_calcAdcClkPrescale(IADC0,
                                             CLK_ADC_FREQ,
                                             0,
                                             iadcCfgModeNormal,
                                             init.srcClkPrescale);

  /*
   * Set the SCANFIFODVL flag when there are 4 entries in the scan
   * FIFO.  Note that in this example, the interrupt associated with
   * the SCANFIFODVL flag in the IADC_IF register is not used.
   *
   * Similarly, the fifoDmaWakeup member of the initScan structure
   * is left at its default setting of false, so LDMA service is not
   * requested when the FIFO holds the specified number of samples.
   */
  initScan.dataValidLevel = IADC_SCANFIFO_DVL;

  // Tag FIFO entry with scan table entry id.
  initScan.showId = true;

  /*
   * Configure entries in scan table.  CH0 is single-ended from
   * input 0; CH1 is single-ended from input 1.
   */
  initScanTable.entries[0].posInput = IADC_INPUT_0_PORT_PIN;
  initScanTable.entries[0].negInput = iadcNegInputGnd;
  initScanTable.entries[0].includeInScan = true;

  initScanTable.entries[1].posInput = IADC_INPUT_2_PORT_PIN;
  initScanTable.entries[1].negInput = iadcNegInputGnd;
  initScanTable.entries[1].includeInScan = true;

  initScanTable.entries[2].posInput = iadcPosInputAvdd;      // Add AVDD to scan for demonstration purposes
  initScanTable.entries[2].negInput = iadcNegInputGnd | 1;   // When measuring a supply, PINNEG must be odd (1, 3, 5,...)
  initScanTable.entries[2].includeInScan = true;

  initScanTable.entries[3].posInput = iadcPosInputVddio;     // Add VDDIO to scan for demonstration purposes
  initScanTable.entries[3].negInput = iadcNegInputGnd | 1;   // When measuring a supply, PINNEG must be odd (1, 3, 5,...)
  initScanTable.entries[3].includeInScan = true;

#if IADC_SCAN_NUM_INPUTS == 8
  #ifndef EFR32FG25B222F1920IM56
  initScanTable.entries[4].posInput = iadcPosInputVss;     // Add VSS to scan for demonstration purposes
  initScanTable.entries[4].negInput = iadcNegInputGnd | 1; // When measuring a supply, PINNEG must be odd (1, 3, 5,...)
  initScanTable.entries[4].includeInScan = true;

  initScanTable.entries[5].posInput = iadcPosInputVssaux;  // Add VSSAUX (same as VSS) to scan for demonstration purposes
  initScanTable.entries[5].negInput = iadcNegInputGnd | 1; // When measuring a supply, PINNEG must be odd (1, 3, 5,...)
  initScanTable.entries[5].includeInScan = true;
  #else // EFR32FG25B222F1920IM56
  initScanTable.entries[4].posInput = iadcPosInputVddio;   // VSS is not available on FG25, use VDDIO instead
  initScanTable.entries[4].negInput = iadcNegInputGnd | 1; // See entry 3
  initScanTable.entries[4].includeInScan = true;

  initScanTable.entries[5].posInput = iadcPosInputDvdd;    // VSSAUX is not available on FG25, use DVDD instead
  initScanTable.entries[5].negInput = iadcNegInputGnd | 1; // See entry 6
  initScanTable.entries[5].includeInScan = true;
  #endif

  initScanTable.entries[6].posInput = iadcPosInputDvdd;      // Add DVDD to scan for demonstration purposes
  initScanTable.entries[6].negInput = iadcNegInputGnd | 1;   // When measuring a supply, PINNEG must be odd (1, 3, 5,...)
  initScanTable.entries[6].includeInScan = true;

  initScanTable.entries[7].posInput = iadcPosInputDecouple;  // Add DECOUPLE to scan for demonstration purposes
  initScanTable.entries[7].negInput = iadcNegInputGnd | 1;   // When measuring a supply, PINNEG must be odd (1, 3, 5,...)
  initScanTable.entries[7].includeInScan = true;
#endif

  // Initialize IADC
  IADC_init(IADC0, &init, &initAllConfigs);

  // Initialize scan
  IADC_initScan(IADC0, &initScan, &initScanTable);

  // Allocate the analog bus for ADC0 inputs
  GPIO->IADC_INPUT_0_BUS |= IADC_INPUT_0_BUSALLOC;
  GPIO->IADC_INPUT_2_BUS |= IADC_INPUT_2_BUSALLOC;

  // Enable IADC scan interrupts
  IADC_clearInt(IADC0, _IADC_IF_MASK);
  IADC_enableInt(IADC0, IADC_IEN_SCANTABLEDONE);
  sl_interrupt_manager_clear_irq_pending(IADC_IRQn);
  sl_interrupt_manager_enable_irq(IADC_IRQn);
}

/**************************************************************************//**
 * @brief  IADC interrupt handler
 *****************************************************************************/
void IADC_IRQHandler(void)
{
  IADC_Result_t result = {0, 0};

  // While the FIFO count is non-zero...
  while (IADC_getScanFifoCnt(IADC0))
  {
    // Pull a scan result from the FIFO
    result = IADC_pullScanFifoResult(IADC0);

    /*
     * Calculate the voltage converted as follows:
     *
     * For single-ended conversions, the result can range from 0 to
     * +Vref, i.e., for Vref = VBGR = 1.21V, and with analog gain = 0.5
     * 0xFFF represents the full scale value of 2.42V.
     */
    scanResult[result.id] = result.data * 2.42 / 0xFFF;

    /*
     * Scan results 2 - 6 are for external supply voltages, which are
     * presented to the IADC divided by 4 for conversion.  Back this
     * out to get the correct result in volts.  Note that DECOUPLE,
     * scan table entry 7 in this example, is an internal supply (the
     * output of the core supply regulator) and is connected directly
     * to the IADC without a divide-by-4 stage.
     */
    if ((result.id > 1) && (result.id < 7)) {
      scanResult[result.id] *= 4;
    }
  }

  /*
   * Clear the scan table complete interrupt.  Reading FIFO results
   * does not do this automatically.
   */
  IADC_clearInt(IADC0, IADC_IF_SCANTABLEDONE);
}

/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
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
  // Start scan
  IADC_command(IADC0, iadcCmdStartScan);
}

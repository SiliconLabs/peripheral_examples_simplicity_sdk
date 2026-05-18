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

// Set CLK_ADC to 10 MHz
#define CLK_SRC_ADC_FREQ        20000000  // CLK_SRC_ADC
#define CLK_ADC_FREQ            10000000  // CLK_ADC - 10 MHz max in normal mode

// Desired LETIMER frequency in Hz
#define LETIMER_FREQ              1

/*
 * This example enters EM2. Setting this #define
 * to 1 enables debug connectivity in EM2, which increases
 * current consumption by about 0.5 uA.
 */
#define EM2DEBUG                  1

// Define GPIO mapping for boards where LED0 and BUTTON0 share the same pins
#ifndef LED1_PORT
  #define LED1_PORT LED1_BUTTON1_PORT
  #define LED1_PIN  LED1_BUTTON1_PIN
#endif

const sl_gpio_t LETIMER_OUTPUT = { .port = LETIMER_OUTPUT_0_PORT, 
                                   .pin = LETIMER_OUTPUT_0_PIN };
const sl_gpio_t GPIO_LED1 = { .port = LED1_PORT,
                              .pin = LED1_PIN };

/*******************************************************************************
 ***************************   GLOBAL VARIABLES   ******************************
 ******************************************************************************/

static volatile double scanResult[IADC_SCAN_NUM_INPUTS];  // Volts

/**************************************************************************//**
 * @brief  GPIO Initializer
 *****************************************************************************/
void gpio_init (void)
{
  // Initialize GPIO driver
  sl_gpio_init();

  // Configure LED1/LETIMER as outputs
  sl_gpio_set_pin_mode(&GPIO_LED1, SL_GPIO_MODE_PUSH_PULL, LED_OFF);
  sl_gpio_set_pin_mode(&LETIMER_OUTPUT, SL_GPIO_MODE_PUSH_PULL, false);
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

  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_IADC0);

  // Set the prescaler needed for the intended IADC clock frequency
  init.srcClkPrescale = IADC_calcSrcClkPrescale(IADC0, CLK_SRC_ADC_FREQ, 0);

  // Shutdown between conversions to reduce current
  init.warmup = iadcWarmupNormal;

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
   * ...which, results in a maximum sampling rate of 833 ksps with the
   * 2-clock input multiplexer switching time is included.
   */
  initAllConfigs.configs[0].adcClkPrescale = IADC_calcAdcClkPrescale(IADC0,
                                                                     CLK_ADC_FREQ,
                                                                     0,
                                                                     iadcCfgModeNormal,
                                                                     init.srcClkPrescale);

  /*
   * Set the SCANFIFODVL flag when there are 8 entries in the scan
   * FIFO.
   *
   * Tag FIFO entries with scan table entry IDs.
   *
   * Allow a scan conversion sequence to start as soon as there is a
   * trigger event.
   */
  initScan.dataValidLevel = IADC_SCANFIFO_DVL;
  initScan.showId = true;
  initScan.start = true;

  /*
   * Configure entries in scan table.  CH0 is single-ended from
   * input 0; CH1 is single-ended from input 1.
   */
  scanTable.entries[0].posInput = IADC_INPUT_0_PORT_PIN;
  scanTable.entries[0].negInput = iadcNegInputGnd;
  scanTable.entries[0].includeInScan = true;

  scanTable.entries[1].posInput = IADC_INPUT_2_PORT_PIN;
  scanTable.entries[1].negInput = iadcNegInputGnd;
  scanTable.entries[1].includeInScan = true;

  scanTable.entries[2].posInput = iadcPosInputAvdd;      // Add AVDD to scan for demonstration purposes
  scanTable.entries[2].negInput = iadcNegInputGnd | 1;   // When measuring a supply, PINNEG must be odd (1, 3, 5,...)
  scanTable.entries[2].includeInScan = true;

  scanTable.entries[3].posInput = iadcPosInputVddio;     // Add VDDIO to scan for demonstration purposes
  scanTable.entries[3].negInput = iadcNegInputGnd | 1;   // When measuring a supply, PINNEG must be odd (1, 3, 5,...)
  scanTable.entries[3].includeInScan = true;

#if IADC_SCAN_NUM_INPUTS == 8
  #ifndef EFR32FG25B222F1920IM56
    scanTable.entries[4].posInput = iadcPosInputVss;       // Add VSS to scan for demonstration purposes
    scanTable.entries[4].negInput = iadcNegInputGnd | 1;   // When measuring a supply, PINNEG must be odd (1, 3, 5,...)
    scanTable.entries[4].includeInScan = true;            // FIFO is only 4 entries deep

    scanTable.entries[5].posInput = iadcPosInputVssaux;    // Add VSSAUX (same as VSS) to scan for demonstration purposes
    scanTable.entries[5].negInput = iadcNegInputGnd | 1;   // When measuring a supply, PINNEG must be odd (1, 3, 5,...)
    scanTable.entries[5].includeInScan = true;
  #else // EFR32FG25B222F1920IM56
    scanTable.entries[4].posInput = iadcPosInputVddio;     // VSS is not available on the FG25, use VDDIO instead
    scanTable.entries[4].negInput = iadcNegInputGnd | 1;   // See entry 3
    scanTable.entries[4].includeInScan = true;            // FIFO is only 4 entries deep

    scanTable.entries[5].posInput = iadcPosInputDvdd;      // VSSAUX is not available on the FG25, use DVDD instead
    scanTable.entries[5].negInput = iadcNegInputGnd | 1;   // See entry 6
    scanTable.entries[5].includeInScan = true;
  #endif

  scanTable.entries[6].posInput = iadcPosInputDvdd;      // Add DVDD to scan for demonstration purposes
  scanTable.entries[6].negInput = iadcNegInputGnd | 1;   // When measuring a supply, PINNEG must be odd (1, 3, 5,...)
  scanTable.entries[6].includeInScan = true;

  scanTable.entries[7].posInput = iadcPosInputDecouple;  // Add DECOUPLE to scan for demonstration purposes
  scanTable.entries[7].negInput = iadcNegInputGnd | 1;   // When measuring a supply, PINNEG must be odd (1, 3, 5,...)
  scanTable.entries[7].includeInScan = true;
#endif

  // Initialize IADC
  IADC_init(IADC0, &init, &initAllConfigs);

  // Initialize scan
  IADC_initScan(IADC0, &initScan, &scanTable);

  // Allocate the analog bus for IADC0 inputs
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

  // Toggle LED0 to notify that transfers are complete
  sl_gpio_toggle_pin(&GPIO_LED1);

  // While the FIFO count is non-zero...
  while (IADC_getScanFifoCnt(IADC0))
  {
    // Pull a scan result from the FIFO
    result = IADC_pullScanFifoResult(IADC0);

    /*
     * Calculate the voltage converted as follows:
     *
     * For single-ended conversions, the result can range from 0 to
     * +Vref, i.e., for Vref = VBGR = 1.21V, and with analog gain = 0.5,
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
 * @brief  LETIMER IRQ Handler
 *****************************************************************************/
void LETIMER0_IRQHandler(void)
{
  uint32_t flags = LETIMER_IntGet(LETIMER0);

  // Trigger an IADC scan conversion
  IADC_command(IADC0, iadcCmdStartScan);

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

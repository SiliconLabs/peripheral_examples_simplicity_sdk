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

// Set CLK_ADC to 40 MHz - this will be adjusted to HFXO frequency in the
// initialization process
#define CLK_SRC_ADC_FREQ        40000000  // CLK_SRC_ADC - 40 MHz max
#define CLK_ADC_FREQ            10000000  // CLK_ADC - 10 MHz max in normal mode

// Number of scan channels
#define NUM_INPUTS 2

/*******************************************************************************
***************************   GLOBAL VARIABLES   *******************************
 ******************************************************************************/

static volatile double scanResult[NUM_INPUTS];

const sl_gpio_t GPIO_OUTPUT0 = { .port = GPIO_IADC0_EOC_PORT,
                                 .pin = GPIO_IADC0_EOC_PIN };

/**************************************************************************//**
 * @brief  GPIO initialization
 *****************************************************************************/
void gpio_init(void)
{
  sl_gpio_init();

  // Output toggled upon completing each LDMA transfer sequence
  sl_gpio_set_pin_mode(&GPIO_OUTPUT0, SL_GPIO_MODE_PUSH_PULL, LED_OFF);
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

  // Enable IADC0 clock tree
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_IADC0);

  // Shutdown between conversions to reduce current
  init.warmup = iadcWarmupNormal;

  // Set the HFSCLK prescale value here
  init.srcClkPrescale = IADC_calcSrcClkPrescale(IADC0, CLK_SRC_ADC_FREQ, 0);

  /*
   * The IADC local timer runs at CLK_SRC_ADC_FREQ, which is at least
   * 2x CLK_ADC_FREQ. CLK_SRC_ADC_FREQ in this example is equal to the
   * HFXO frequency. Dividing the frequency of the HFXO by 1000 will give
   * the tick count for 1 ms trigger rate.
   * For example - if HFXO freq = 38.4 MHz,
   *
   * ticks for 1 ms trigger = 38400000 / 1000
   * ticks =  38400
   */
  uint32_t iadcFreq;
  sl_clock_manager_get_clock_branch_frequency(SL_CLOCK_BRANCH_IADCCLK, &iadcFreq);
  init.timerCycles = iadcFreq/1000;

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

  /*
   * The IADC local timer triggers conversions.
   *
   * Set the SCANFIFODVL flag when scan FIFO holds 2 entries.  In this
   * example, the interrupt associated with the SCANFIFODVL flag in
   * the IADC_IF register is not used.
   *
   * Tag each FIFO entry with scan table entry ID.
   */
  initScan.triggerSelect = iadcTriggerSelTimer;
  initScan.dataValidLevel = iadcFifoCfgDvl2;
  initScan.showId = true;

  /*
   * Configure entries in the scan table.  CH0 is single-ended from
   * input 0; CH1 is single-ended from input 1.
   */
  scanTable.entries[0].posInput = IADC_INPUT_0_PORT_PIN;
  scanTable.entries[0].negInput = iadcNegInputGnd;
  scanTable.entries[0].includeInScan = true;

  scanTable.entries[1].posInput = IADC_INPUT_2_PORT_PIN;
  scanTable.entries[1].negInput = iadcNegInputGnd;
  scanTable.entries[1].includeInScan = true;

  // Initialize IADC
  IADC_init(IADC0, &init, &initAllConfigs);

  // Initialize scan
  IADC_initScan(IADC0, &initScan, &scanTable);

  // Enable the IADC timer (must be after the IADC is initialized)
  IADC_command(IADC0, iadcCmdEnableTimer);

  // Allocate the analog bus for ADC0 inputs
  GPIO->IADC_INPUT_0_BUS |= IADC_INPUT_0_BUSALLOC;
  GPIO->IADC_INPUT_2_BUS |= IADC_INPUT_2_BUSALLOC;

  // Enable IADC scan interrupts
  IADC_clearInt(IADC0, _IADC_IF_MASK);
  IADC_enableInt(IADC0, IADC_IEN_SCANFIFODVL);
  sl_interrupt_manager_clear_irq_pending(IADC_IRQn);
  sl_interrupt_manager_enable_irq(IADC_IRQn);
}

/**************************************************************************//**
 * @brief  IADC interrupt handler
 *****************************************************************************/
void IADC_IRQHandler(void)
{
  IADC_Result_t sample;

  // Toggle GPIO to signal scan completion
  sl_gpio_toggle_pin(&GPIO_OUTPUT0);

  // While the FIFO count is non-zero...
  while (IADC_getScanFifoCnt(IADC0))
  {
    // Pull a scan result from the FIFO
    sample = IADC_pullScanFifoResult(IADC0);

    /*
     * Calculate the voltage converted as follows:
     *
     * For single-ended conversions, the result can range from 0 to
     * +Vref, i.e., for Vref = VBGR = 1.21V, and with analog gain = 0.5,
     * 0xFFF represents the full scale value of 2.42V.
     */
    scanResult[sample.id] = sample.data * 2.42 / 0xFFF;
  }

  /*
   * Clear the scan table complete interrupt.  Reading from the FIFO
   * does not do this automatically.
   */
  IADC_clearInt(IADC0, IADC_IF_SCANFIFODVL);
}

/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  gpio_init();
  iadc_init();

  // Start the IADC scan
  IADC_command(IADC0, iadcCmdStartScan);
}

/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{
}

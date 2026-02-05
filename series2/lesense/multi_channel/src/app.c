/***************************************************************************//**
 * @file
 * @brief LESENSE multi channel demo for EFR32ZG23. This example uses LESENSE
 *        to scan four IADC scan channels in low energy mode. The LESENSE is
 *        configured to detect when each input signal crosses over a threshold
 *        and will trigger an interrupt to toggle an the LED.
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 *******************************************************************************
 * # Evaluation Quality
 * This code has been minimally tested to ensure that it builds and is suitable 
 * as a demonstration for evaluation purposes only. This code will be maintained
 * at the sole discretion of Silicon Labs.
 ******************************************************************************/
 
#include "em_iadc.h"
#include "em_lesense.h"

#include "sl_clock_manager.h"
#include "sl_gpio.h"
#include "pin_config.h"

#define PD01REGNORETAIN  1  // EM0/1 peripheral register retention

// Set CLK_ADC to 1 MHz (set to max for shortest IADC conversion/power-up time)
#define CLK_SRC_ADC_FREQ          1000000 // CLK_SRC_ADC
#define CLK_ADC_FREQ              1000000 // CLK_ADC

// Set the LESENSE scan frequency
#define LESENSE_SCAN_FREQ         32 // All enabled channels are scanned each
                                     // scan period; corresponds to 32 samples
                                     // per second per channel

// IADC/LESENSE comparison thresholds; upper and low bounds for added hysteresis
// The comparison defines are used for all channels; however, each input is
// capable of being individually configured
#define IADC_COMP_THRESH_UPPER    0xC00
#define IADC_COMP_THRESH_LOWER    0x200

const sl_gpio_t GPIO_BUTTON0 = { .port = BUTTON0_PORT,  .pin = BUTTON0_PIN };
const sl_gpio_t GPIO_BUTTON1 = { .port = BUTTON1_PORT,  .pin = BUTTON1_PIN };
const sl_gpio_t GPIO_LED0 = { .port = LED0_PORT, .pin = LED0_PIN };
const sl_gpio_t GPIO_LED1 = { .port = LED1_PORT, .pin = LED1_PIN };

/***************************************************************************//**
 * @brief LESENSE interrupt handler
 *        This function acknowledges the interrupt and toggles LED0.
 ******************************************************************************/
void LESENSE_IRQHandler(void)
{
  // Clear all LESENSE interrupt flag
  uint32_t flags = LESENSE_IntGet();
  LESENSE_IntClear(flags);

  if ((flags & LESENSE_IF_CH0) == LESENSE_IF_CH0) {
    // Toggle LED0
    sl_gpio_toggle_pin(&GPIO_LED0);
  }

  if ((flags & LESENSE_IF_CH1) == LESENSE_IF_CH1) {
    // Toggle LED1
    sl_gpio_toggle_pin(&GPIO_LED1);
  }

  if ((flags & LESENSE_IF_CH2) == LESENSE_IF_CH2) {
    // Toggle LED0
    sl_gpio_toggle_pin(&GPIO_LED0);
  }

  if ((flags & LESENSE_IF_CH3) == LESENSE_IF_CH3) {
    // Toggle LED1
    sl_gpio_toggle_pin(&GPIO_LED1);
  }
}

/***************************************************************************//**
 * @brief escape_hatch()
 * When developing or debugging code that enters EM2 or
 * lower, it's a good idea to have an "escape hatch" type
 * mechanism, e.g. a way to pause the device so that a debugger can
 * connect in order to erase flash, among other things.
 *
 * Before proceeding with this example, make sure PB0 is not pressed.
 * If the PB0 pin is low, turn on LED0 and execute the breakpoint
 * instruction to stop the processor in EM0 and allow a debug
 * connection to be made.
 ******************************************************************************/
void escape_hatch(void)
{
  bool pin_value;

  // Initialize GPIO
  sl_gpio_init();

  sl_gpio_set_pin_mode(&GPIO_BUTTON0, SL_GPIO_MODE_INPUT_PULL_FILTER, 1);

  sl_gpio_get_pin_input(&GPIO_BUTTON0, &pin_value);
  if (pin_value == 0) {
    sl_gpio_set_pin_mode(&GPIO_LED0, SL_GPIO_MODE_PUSH_PULL, LED_ON);
    __BKPT(0);
  }
  else {
    // Pin not asserted, so disable input
    sl_gpio_set_pin_mode(&GPIO_BUTTON0, SL_GPIO_MODE_DISABLED, 0);
  }
}

/***************************************************************************//**
 * @brief  IADC Initializer
 ******************************************************************************/
void iadc_init(void)
{
  // Declare init structs
  IADC_Init_t init = IADC_INIT_DEFAULT;
  IADC_AllConfigs_t initAllConfigs = IADC_ALLCONFIGS_DEFAULT;
  IADC_InitScan_t initScan = IADC_INITSCAN_DEFAULT;
  IADC_ScanTable_t initScanTable = IADC_SCANTABLE_DEFAULT; // Scan Table

  // Enable IADC0 clock branch
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_IADC0);

  // Reset IADC to reset configuration in case it has been modified by
  // other code
  IADC_reset(IADC0);

  // Modify init structs and initialize
  init.warmup = iadcWarmupNormal;

  // Set the HFSCLK prescale value here
  init.srcClkPrescale = IADC_calcSrcClkPrescale(IADC0, CLK_SRC_ADC_FREQ, 0);

  // Configuration 0 is used by both scan and single conversions by default
  // Use unbuffered AVDD as reference
  initAllConfigs.configs[0].reference = iadcCfgReferenceVddx;

  // Divides CLK_SRC_ADC to set the CLK_ADC frequency
  initAllConfigs.configs[0].adcClkPrescale = IADC_calcAdcClkPrescale(IADC0,
                                             CLK_ADC_FREQ,
                                             0,
                                             iadcCfgModeNormal,
                                             init.srcClkPrescale);

  // Fixed unipolar for LESENSE threshold comparisons
  initAllConfigs.configs[0].twosComplement = iadcCfgTwosCompUnipolar;

  // Scan triggered by LESENSE request
  initScan.triggerSelect = iadcTriggerSelLesense;

  // Configure entries in scan table, CH0-3 are single-ended from inputs 0-3
  // set by the defines at the beginning of this source
  initScanTable.entries[0].posInput = LESENSE_IADC_INPUT_0_PORT_PIN;
  initScanTable.entries[0].negInput = iadcNegInputGnd;
  initScanTable.entries[0].includeInScan = true;

  initScanTable.entries[1].posInput = LESENSE_IADC_INPUT_1_PORT_PIN;
  initScanTable.entries[1].negInput = iadcNegInputGnd;
  initScanTable.entries[1].includeInScan = true;

  initScanTable.entries[2].posInput = LESENSE_IADC_INPUT_2_PORT_PIN;
  initScanTable.entries[2].negInput = iadcNegInputGnd;
  initScanTable.entries[2].includeInScan = true;

  initScanTable.entries[3].posInput = LESENSE_IADC_INPUT_3_PORT_PIN;
  initScanTable.entries[3].negInput = iadcNegInputGnd;
  initScanTable.entries[3].includeInScan = true;

  // Initialize IADC
  IADC_init(IADC0, &init, &initAllConfigs);

  // Initialize Scan
  IADC_initScan(IADC0, &initScan, &initScanTable);

  // Allocate the analog bus for ADC0 inputs
  GPIO->LESENSE_IADC_INPUT_0_BUS |= LESENSE_IADC_INPUT_0_BUSALLOC;
  GPIO->LESENSE_IADC_INPUT_1_BUS |= LESENSE_IADC_INPUT_1_BUSALLOC;
  GPIO->LESENSE_IADC_INPUT_2_BUS |= LESENSE_IADC_INPUT_2_BUSALLOC;
  GPIO->LESENSE_IADC_INPUT_3_BUS |= LESENSE_IADC_INPUT_3_BUSALLOC;

  // Enable triggering of the scan queue
  IADC_command(IADC0, iadcCmdStartScan);
}

/***************************************************************************//**
 * @brief GPIO initialization
 *        This function configures the following two GPIOs:
 *        1. LED0 set to push-pull output with pull-down enabled.
 *        2. LED1 set to push-pull output with pull-down enabled.
 ******************************************************************************/
void gpio_init(void)
{
  // Initialize GPIO
  sl_gpio_init();

  // Configure LEDs for output
  sl_gpio_set_pin_mode(&GPIO_LED0, SL_GPIO_MODE_PUSH_PULL, 0);
  sl_gpio_set_pin_mode(&GPIO_LED1, SL_GPIO_MODE_PUSH_PULL, 0);
}

/***************************************************************************//**
 * @brief LESENSE Initialization
 *        This functions configures and enables the LESENSE block
 ******************************************************************************/
void lesense_init(void)
{
  /*****************************************************************************
   * Enable clock for LESENSE
   * Enable PRS clock as it is needed for LESENSE initialization
  *****************************************************************************/
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_LESENSE);
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_PRS);

  // LESENSE default configurations
  LESENSE_Init_TypeDef initLesense = LESENSE_INIT_DEFAULT;
  LESENSE_ChDesc_TypeDef initLesenseCh = LESENSE_CH_CONF_DEFAULT;

  // Do not store scan result
  initLesense.coreCtrl.storeScanRes = true;
  initLesense.coreCtrl.scanConfSel = lesenseScanConfToggle; // Allows for some
                                                            // hysteresis

  initLesense.timeCtrl.delayAuxStartup = true; // delay startup of AUXHFRCO
                                         // until the system enters excite phase

  // Channel Configuration
  initLesenseCh.enaScanCh = true;  // Enable scan channel
  initLesenseCh.enaInt = true;     // Enable interrupt
  initLesenseCh.sampleDelay = 0x0; // 0+1 LF Clock cycle sample delay
  initLesenseCh.sampleMode = lesenseSampleModeADC;
  initLesenseCh.intMode = (LESENSE_ChIntMode_TypeDef)
      LESENSE_CH_INTERACT_SETIF_BOTHEDGES; // Interrupt on both sensor edges
  initLesenseCh.compMode = lesenseCompModeLess;

  // Initialize LESENSE interface
  LESENSE_Init(&initLesense, true);

  // Configure channel 0-3
  LESENSE_ChannelConfig(&initLesenseCh, 0);
  LESENSE_ChannelConfig(&initLesenseCh, 1);
  LESENSE_ChannelConfig(&initLesenseCh, 2);
  LESENSE_ChannelConfig(&initLesenseCh, 3);

  // Configure alternate channels
  initLesenseCh.enaScanCh = false;  // not actually scanning these channels;
                                    // only configuring as complimentary
                                    // thresholds for channels 0-4
  initLesenseCh.compMode = lesenseCompModeLess; // comparison mode remains less
                                    // than threshold; only threshold value is
                                    // adjusted in alternate channel configs

  // Configure alternate channels 8-11
  LESENSE_ChannelConfig(&initLesenseCh, 8);
  LESENSE_ChannelConfig(&initLesenseCh, 9);
  LESENSE_ChannelConfig(&initLesenseCh, 10);
  LESENSE_ChannelConfig(&initLesenseCh, 11);

  LESENSE_ScanFreqSet(0, LESENSE_SCAN_FREQ);

  // Wait for SYNCBUSY clear
  while(LESENSE->SYNCBUSY);

  // Disable LESENSE; needed in order to configure the offset and threshold
  LESENSE->EN_CLR = LESENSE_EN_EN;
  while (LESENSE->EN & _LESENSE_EN_DISABLING_MASK);

  // LESENSE OFFSET determines which entry specified in the IADC scan table is
  // converted
  LESENSE->CH_SET[0].INTERACT = (0 << _LESENSE_CH_INTERACT_OFFSET_SHIFT);
  LESENSE->CH_SET[1].INTERACT = (1 << _LESENSE_CH_INTERACT_OFFSET_SHIFT);
  LESENSE->CH_SET[2].INTERACT = (2 << _LESENSE_CH_INTERACT_OFFSET_SHIFT);
  LESENSE->CH_SET[3].INTERACT = (3 << _LESENSE_CH_INTERACT_OFFSET_SHIFT);

  // Configure the comparison threshold for each channel; this threshold
  // coincides with SCANRES 0 or IADC conversion IS NOT less than the threshold
  // (value is greater than or equal); lower threshold used to trigger change
  LESENSE->CH_SET[0].EVALTHRES = IADC_COMP_THRESH_LOWER;
  LESENSE->CH_SET[1].EVALTHRES = IADC_COMP_THRESH_LOWER;
  LESENSE->CH_SET[2].EVALTHRES = IADC_COMP_THRESH_LOWER;
  LESENSE->CH_SET[3].EVALTHRES = IADC_COMP_THRESH_LOWER;

  // for hysteresis, configure the alternate channels as well
  LESENSE->CH_SET[8].INTERACT = (0 << _LESENSE_CH_INTERACT_OFFSET_SHIFT);
  LESENSE->CH_SET[9].INTERACT = (1 << _LESENSE_CH_INTERACT_OFFSET_SHIFT);
  LESENSE->CH_SET[10].INTERACT = (2 << _LESENSE_CH_INTERACT_OFFSET_SHIFT);
  LESENSE->CH_SET[11].INTERACT = (3 << _LESENSE_CH_INTERACT_OFFSET_SHIFT);

  // Configure the comparison threshold for each channel; this threshold
  // coincides with SCANRES 1 or IADC conversion IS less than the threshold
  // (value is less than threshold); upper threshold used to trigger change
  LESENSE->CH_SET[8].EVALTHRES = IADC_COMP_THRESH_UPPER;
  LESENSE->CH_SET[9].EVALTHRES = IADC_COMP_THRESH_UPPER;
  LESENSE->CH_SET[10].EVALTHRES = IADC_COMP_THRESH_UPPER;
  LESENSE->CH_SET[11].EVALTHRES = IADC_COMP_THRESH_UPPER;

  LESENSE->EN_SET = LESENSE_EN_EN;  // Enable LESENSE
  while(LESENSE->SYNCBUSY);         // SYNCBUSY check;

  // Enable interrupt
  sl_interrupt_manager_clear_irq_pending(LESENSE_IRQn);
  sl_interrupt_manager_enable_irq(LESENSE_IRQn);

  // Disable PRS clock as it is no longer needed
  sl_clock_manager_disable_bus_clock(SL_BUS_CLOCK_PRS);

  // Start continuous scan
  LESENSE_ScanStart();
}

/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  // EM2 escape hatch to re-enable debug control
  escape_hatch();

/*****************************************************************************
 * The reference manual section 13.3.6 states that the EM0 / EM1 peripheral
 * register retention can be optionally disabled if the application does not
 * need to retain the EM0 / EM1 peripheral register setting upon low energy mode
 * wake up. This can be done by setting bit 0 in the EMU_PD1PARETCTRL register.
 *
 * Note this will cause the the EM0 / EM1 peripheral register interface to reset
 * upon exit to EM0 and will need to be reconfigured. This example does not use
 * any peripherals that operates in EM0 / EM1 only, and therefore has this bit
 * configured for lower current consumption.
*****************************************************************************/
#if PD01REGNORETAIN
  EMU->PD1PARETCTRL_SET = 0x1;  // provides around 0.2-0.5uA of current saving
#endif

  // Initialize GPIO
  gpio_init();

  // Initialize ACMP
  iadc_init();
  
  // Initialize LESENSE
  lesense_init();
}

/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{
}

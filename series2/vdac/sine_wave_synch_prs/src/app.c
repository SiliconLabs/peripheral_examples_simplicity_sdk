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
#include "sl_power_manager.h"

#include "sl_gpio.h"

#include "em_vdac.h"
#include "em_prs.h"

#include "pin_config.h"

// Note: The sine wave output is always generated on Channel 0. Channel 1 may
// still be used independently as a single-ended DAC output; however, its
// configuration is ignored whenever differential mode is enabled.
#define CHANNEL_NUM 0

// Configure the VDAC to operate at its maximum 1 MHz clock rate.
// The sine wave frequency is given by:
// f_sine = f_EM01GRPACLK / (32 * (PRESCALE + 1))
// With f_EM01GRPACLK = 19 MHz and PRESCALE = 18 (fastest VDAC clock),
// the resulting sine wave frequency is approximately 31 kHz.
#define CLK_VDAC_FREQ              1000000

// Select the PRS channel used to capture the raw GPIO signal.
// In this example, PRS channel 7 receives the button input, and
// PRS channel 8 is used to generate the inverted version of that signal.
#define PRS_CHANNEL               7

const sl_gpio_t BUTTON1 = { .port = BUTTON1_PORT,
                                 .pin  = BUTTON1_PIN };
								 
/*
 * The VDAC output port and pin are configured through the VDAC_OUTCTRL register.
 * The ABUSPORTSELCHx fields in this register select the GPIO port used for the
 * CHx ABUS output. The field values are defined as follows:
 *
 *   0  No GPIO selected for CHx ABUS output
 *   1  Port A selected
 *   2  Port B selected
 *   3  Port C selected
 *   4  Port D selected
 *
 * When using the main VDAC output, no additional port/pin configuration is
 * required. Refer to the device Reference Manual and Datasheet for further
 * information. In this example, CH0 is routed to the main output pin PB00.
 */
const sl_gpio_t VDAC_CH0_MAINOUT = { .port = VDAC0_CH0_MAINOUT_PORT,
                                     .pin  = VDAC0_CH0_MAINOUT_PIN };

/**************************************************************************//**
 * @brief  GPIO initialization
 *****************************************************************************/
void gpio_init(void)
{
  int32_t int_no = BUTTON1.pin;

  // Initialize GPIO driver
  sl_gpio_init();

  // Configure pushbutton 1 as input and enable interrupt
  sl_gpio_set_pin_mode(&BUTTON1, SL_GPIO_MODE_INPUT_PULL_FILTER, true);
  sl_gpio_configure_external_interrupt(&BUTTON1, &int_no,
                                         SL_GPIO_INTERRUPT_NO_EDGE,
                                         NULL,
                                         NULL);

}

/**************************************************************************//**
 * @brief  PRS initialization
 *****************************************************************************/
void prs_init(void)
{
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_PRS);

  // Route BUTTON1 input into PRS channel 7 (raw GPIO level)
  PRS_SourceAsyncSignalSet(PRS_CHANNEL,
                           PRS_ASYNC_CH_CTRL_SOURCESEL_GPIO,
                           BUTTON1.pin);

  // Generate inverted logic on PRS channel 8: CH8 = NOT(CH7)
  PRS_Combine(PRS_CHANNEL + 1,
              PRS_CHANNEL,
              prsLogic_NOT_B);

  // Use PRS channel 8 as the VDAC asynchronous trigger source
  PRS_ConnectConsumer(PRS_CHANNEL + 1,
                      prsTypeAsync,
                      prsConsumerVDAC0_ASYNCTRIGCH0);
}

/**************************************************************************//**
 * @brief
 *    VDAC initialization
 *****************************************************************************/
void vdac_init(void)
{
  VDAC_Init_TypeDef init = VDAC_INIT_SINE_GENERATION_MODE;
  VDAC_InitChannel_TypeDef ch = VDAC_INITCHANNEL_DEFAULT;

  // Enable VDAC clock
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_VDAC0);

  // Configure VDAC clock for 1 MHz operation
  init.prescaler = VDAC_PrescaleCalc(VDAC0, 1000000);

  // Use internal 1.25 V reference
  init.reference = vdacRef1V25;

  // Enable sine mode with PRS control
  init.sineEnable = true;
  init.sineModePrsEnable = true;
  init.sineReset = true;

  // Channel configuration
  ch.trigMode = vdacTrigModeAsyncPrs;     // Triggered by PRS CH8
  ch.sampleOffMode = false;               // Continuous output mode
  ch.highCapLoadEnable = false;           // No high-capacitance load
  ch.chRefreshSource = vdacRefreshSrcNone;

  // Disable main output pin
  sl_gpio_set_pin_mode(&VDAC_CH0_MAINOUT, SL_GPIO_MODE_DISABLED, 0);

  // Initialize VDAC and channel
  VDAC_Init(VDAC0, &init);
  VDAC_InitChannel(VDAC0, &ch, 0);

  // Enable VDAC channel
  VDAC_Enable(VDAC0, 0, true);
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

  // Initialize the VDAC peripheral and channel
  vdac_init();
}

/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{
}
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

#include "pin_config.h"

// Note: The sine wave output is always generated on Channel 0. Channel 1 may
// still be used independently as a single-ended DAC output; however, its
// configuration is ignored whenever differential mode is enabled.
#define CHANNEL_NUM 0

// Set the VDAC to its maximum 1 MHz clock frequency
// f_sinewave = f_EM01GRPACLK / (32 * (PRESCALE + 1))
// With f_EM01GRPACLK = 19 MHz and PRESCALE = 18 (fastest VDAC clock),
// the resulting sine wave frequency is approximately 31 kHz.
#define CLK_VDAC_FREQ              1000000

const sl_gpio_t LED0 = { .port = LED0_PORT,
                         .pin  = LED0_PIN };

const sl_gpio_t ESCAPE_HATCH = { .port = BUTTON0_PORT,
                                 .pin  = BUTTON0_PIN };

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
 * information. In this example, CH0 is routed to the main output pin PB00
 * on all devices except BRD4270B (xG25), where CH0 must instead use the
 * auxiliary output on PA06. On BRD4270B, the CH0 main output is tied to an
 * external pullup resistor, capacitor, and pushbutton on the WSTK, which
 * interferes with the analog output signal; therefore, the auxiliary output
 * provides a clean, usable path.
 */
#if defined(EFR32FG25B222F1920IM56)
const sl_gpio_t VDAC_CH0_AUXOUT = { .port = SL_GPIO_PORT_A,
                                 .pin  = 6 };
#else
const sl_gpio_t VDAC_CH0_MAINOUT = { .port = VDAC0_CH0_MAINOUT_PORT,
                                     .pin  = VDAC0_CH0_MAINOUT_PIN };
#endif

/**************************************************************************//**
 * @brief
 *    VDAC initialization
 *****************************************************************************/
void vdac_init(void)
{
  // Load default VDAC configuration structures
  VDAC_Init_TypeDef        init        = VDAC_INIT_DEFAULT;
  VDAC_InitChannel_TypeDef initChannel = VDAC_INITCHANNEL_DEFAULT;

  // Sine mode requires the VDAC to run at its fastest continuous-mode configuration,
  // so EM01GRPACLK must be selected as the VDAC0 clock source in the clock manager.
  // Enable the VDAC clock
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_VDAC0);

  // Compute the prescaler value required to achieve a 1 MHz VDAC clock
  init.prescaler = VDAC_PrescaleCalc(VDAC0, (uint32_t)CLK_VDAC_FREQ);

  // Set reference to internal 1.25V reference
  init.reference = vdacRef1V25;

  // Enable sine mode
  init.sineEnable = true;

  // Set the output mode to continuous as required for the sine mode
  initChannel.sampleOffMode = false;

  // Since the minimum load requirement for high capacitance mode is 25 nF, turn
  // this mode off
  initChannel.highCapLoadEnable = false;

  // Trigger mode and refresh source should be programmed to None for the sine
  // mode to avoid interference in sine output generation from other triggers
  initChannel.trigMode = vdacTrigModeNone;
  initChannel.chRefreshSource = vdacRefreshSrcNone;

#if defined(EFR32FG25B222F1920IM56)
  // Enable the GPIO clock
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_GPIO);

  // Disable PA06 input
  sl_gpio_set_pin_mode(&VDAC_CH0_AUXOUT, SL_GPIO_MODE_DISABLED, false);

  // Connect VDAC0 CH0 to an even pin on Port A via the ABUS
  GPIO->ABUSALLOC = GPIO_ABUSALLOC_AEVEN0_VDAC0CH0;

  // Disable Main output
  initChannel.mainOutEnable = false;

  // Enable Auxiliary output
  initChannel.auxOutEnable = true;

  // Output to PA06
  initChannel.port = vdacChPortA;
  initChannel.pin = 6;
#else
  // Disable VDAC0 main output pin
  sl_gpio_set_pin_mode(&VDAC_CH0_MAINOUT, SL_GPIO_MODE_DISABLED, 0);
#endif

  // Initialize the VDAC and VDAC channel
  VDAC_Init(VDAC0, &init);
  VDAC_InitChannel(VDAC0, &initChannel, CHANNEL_NUM);

  // Enable the VDAC
  VDAC_Enable(VDAC0, CHANNEL_NUM, true);
}

/**************************************************************************//**
 * @brief
 *    Provide an "escape hatch" mechanism when developing or debugging code
 *    that enters EM2 or lower power modes.
 *
 * @note
 *    This mechanism allows the device to pause so a debugger can connect,
 *    erase flash, or perform other recovery actions.
 *
 * Before running this example, ensure that PB0 is not pressed. If the PB0
 * pin is detected low, LED0 is turned on and a breakpoint instruction is
 * executed. This halts the processor in EM0, allowing a debug connection
 * to be established.
 *****************************************************************************/
void escape_hatch(void)
{
  bool pin_value;

  // Configure Escape Hatch button as an input pulled high
  sl_gpio_set_pin_mode(&ESCAPE_HATCH, SL_GPIO_MODE_INPUT_PULL_FILTER, true);

  // Check Escape Hatch button state: if pressed, enable LED0 and trigger breakpoint
  sl_gpio_get_pin_input(&ESCAPE_HATCH, &pin_value);
  if (pin_value == 0)
  {
    sl_gpio_set_pin_mode(&LED0, SL_GPIO_MODE_PUSH_PULL, LED_ON);
    __BKPT(0);  // Halt execution for debugging
  }
  else
  {
    // Disable Escape Hatch button digital input when not in use
    sl_gpio_set_pin_mode(&ESCAPE_HATCH, SL_GPIO_MODE_DISABLED, false);
  }
}

/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  // Run Escape Hatch check before entering low-power modes
  escape_hatch();

  // Initialize the VDAC peripheral and channel
  vdac_init();

  // Start sine mode
  VDAC_SineModeStart(VDAC0, true);
}

/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{
}
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
#include "peripheral_config.h"

// Select which VDAC channel to use (0 or 1)
#define CHANNEL_NUM 0

// VDAC clock is 32.768 kHz using EM23GRPACLK sourced by LFRCO
#define CLK_VDAC_FREQ             32768

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

  // Enable VDAC clock
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_VDAC0);

  // Compute prescaler for a 32.768 kHz VDAC clock
  init.prescaler = VDAC_PrescaleCalc(VDAC0, (uint32_t)CLK_VDAC_FREQ);

  // Use internal 1.25 V low-noise reference
  init.reference = vdacRef1V25;

  /* When using EM23GRPACLK, the clock source cannot be made "on demand";
   * Setting this bool/bitfield to true/logic 1 disables "on demand"
   */
  init.onDemandClk = true;

  // Set VDAC channel refresh period
  init.refresh = vdacRefresh32;

  // Since the VDAC runs in EM3, low-power mode should be enabled
  initChannel.powerMode = vdacPowerModeLowPower;

  // Set the output mode to sample/off. In sample/off mode, the VDAC will only
  // drive the output for a limited time per conversion.
  initChannel.sampleOffMode = true;

  // The VDAC will drive the output for 10 prescaled VDAC clock cycles before
  // tri-stating the output again
  initChannel.holdOutTime = 10;

  // Set the trigger mode to SW so that the internal timer or a software
  // trigger will trigger the VDAC conversion
  initChannel.trigMode = vdacTrigModeSw;

  // A conversion will start on an overflow of the refresh timer. This is an
  // internal low power refresh timer that is automatically started. It will
  // count the number of clock refresh cycles programmed in the init
  // configuration before wrapping and generating a refresh trigger.
  initChannel.chRefreshSource = vdacRefreshSrcRefreshTimer;

  // Since the minimum load requirement for high capacitance mode is 25 nF, turn
  // this mode off
  initChannel.highCapLoadEnable = false;

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
  sl_gpio_set_pin_mode(&VDAC_CH0_MAINOUT, SL_GPIO_MODE_DISABLED, false);
#endif

  // Initialize the VDAC and VDAC channel
  VDAC_Init(VDAC0, &init);
  VDAC_InitChannel(VDAC0, &initChannel, CHANNEL_NUM);

  // Enable the VDAC
  VDAC_Enable(VDAC0, CHANNEL_NUM, true);
}

/**************************************************************************//**
 * @brief
 *    Calculate the digital value that maps to the desired output voltage
 *
 * @note
 *    The vRef parameter must match the reference voltage selected during
 *    initialization
 *
 * @param [in] vOut
 *    Desired output voltage
 *
 * @param [in] vRef
 *    Reference voltage used by the VDAC
 *
 * @return
 *    The digital value that maps to the desired output voltage
 *****************************************************************************/
uint32_t vdac_get_value(float vOut, float vRef)
{
  return (uint32_t)((vOut * 4095) / vRef);
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

  // Calculate the 12-bit output value for 0.5 V
  uint32_t vdacValue = vdac_get_value(0.5, 1.25);

  // Write the output value to VDAC DATA register
  VDAC_ChannelOutputSet(VDAC0, CHANNEL_NUM, vdacValue);
}

/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{
}
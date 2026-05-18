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

// Set the VDAC to max frequency of 1 MHz
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
 * information. For this example, the CH0 main output on PB00 and and
 * CH1 main output PB001 are used.
 */
const sl_gpio_t VDAC_CH0_MAINOUT = { .port = VDAC0_CH0_MAINOUT_PORT,
                                     .pin  = VDAC0_CH0_MAINOUT_PIN };

/**************************************************************************//**
 * @brief
 *    VDAC initialization
 *****************************************************************************/
void vdac_init(void)
{
  // Load default VDAC configuration structures
  VDAC_Init_TypeDef        init        = VDAC_INIT_DEFAULT;
  VDAC_InitChannel_TypeDef initChannel = VDAC_INITCHANNEL_DEFAULT;

  // Enable HFRCOEM23 on-demand operation for EM2/EM3 wake-up
  HFRCOEM23->CTRL =
      (HFRCOEM23->CTRL & ~_HFRCO_CTRL_EM23ONDEMAND_MASK)
      | (1 << _HFRCO_CTRL_EM23ONDEMAND_SHIFT);

  // Enable the HFRCOEM23 and VDAC peripheral clocks
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_HFRCOEM23);
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_VDAC0);

  // Compute the prescaler value required to achieve a 1 MHz VDAC clock
  init.prescaler = VDAC_PrescaleCalc(VDAC0, CLK_VDAC_FREQ);

  // Set the output mode to differential instead of single-ended
  init.diff = true;

  // Clocking is requested on demand
  init.onDemandClk = false;

  // Disable High Capacitance Load mode
  initChannel.highCapLoadEnable = false;

  // Configure the channel for Low-Power mode
  initChannel.powerMode = vdacPowerModeLowPower;

  // Disable VDAC0 main output pin
  sl_gpio_set_pin_mode(&VDAC_CH0_MAINOUT, SL_GPIO_MODE_DISABLED, false);

  // Initialize the VDAC and VDAC channel
  VDAC_Init(VDAC0, &init);
  VDAC_InitChannel(VDAC0, &initChannel, 0);
  VDAC_InitChannel(VDAC0, &initChannel, 1);

  // Enable the VDAC
  VDAC_Enable(VDAC0, 0, true);
  VDAC_Enable(VDAC0, 1, true);
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
  return (uint32_t)((vOut * 2047) / vRef);
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
  VDAC_ChannelOutputSet(VDAC0, 0, vdacValue);

}

/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{
}
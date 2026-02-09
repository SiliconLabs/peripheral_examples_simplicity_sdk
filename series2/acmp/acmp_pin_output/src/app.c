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
#include "sl_hal_gpio.h"
#include "em_acmp.h"
#include "pin_config.h"

#ifndef BUTTON0_PORT
  #define BUTTON0_PORT LED0_BUTTON0_PORT
  #define BUTTON0_PIN LED0_BUTTON0_PIN
#endif

const sl_gpio_t ACMP_OUTPUT = { .port = ACMP_OUTPUT0_PORT, .pin = ACMP_OUTPUT0_PIN };
const sl_gpio_t GPIO_PB0 = { .port = BUTTON0_PORT, .pin = BUTTON0_PIN };

/**************************************************************************//**
 * @brief escapeHatch()
 *
 * When developing or debugging code that enters EM2 or lower, it's a
 * good idea to have an "escape hatch" type mechanism, e.g. a way to
 * pause the device so that a debugger can connect in order to erase
 * flash, among other things.
 *
 * Before proceeding with this example, make sure PB0 is not pressed.
 * If the PB0 pin is low, turn on LED0 and execute the breakpoint (BKPT)
 * instruction to stop the processor in EM0 and allow a debug
 * connection to be made.
 *****************************************************************************/
void escapeHatch(void)
{
  // Configure PB0 pin as an input pulled high
  sl_gpio_set_pin_mode(&GPIO_PB0, SL_GPIO_MODE_INPUT_PULL_FILTER, true);

  // Check for PB0 low; if so halt the CPU
  if (sl_hal_gpio_get_pin_input(&GPIO_PB0) == 0) {
    sl_gpio_set_pin_mode(&ACMP_OUTPUT, SL_GPIO_MODE_PUSH_PULL, LED_ON);
    __BKPT(0);
  }
  // Pin not asserted; disable the PB0 digital input
  else {
    sl_gpio_set_pin_mode(&GPIO_PB0, SL_GPIO_MODE_DISABLED, false);
  }
}

/**************************************************************************//**
 * @brief GPIO initialization
 *****************************************************************************/
void gpio_init(void)
{
  /*
   * Enable LED0 pin as an output and drive low.  Note that LED0 remains
   * off until the input falls below the comparator reference voltage.
   */
  sl_gpio_set_pin_mode(&ACMP_OUTPUT, SL_GPIO_MODE_PUSH_PULL, LED_OFF);
}
 

/**************************************************************************//**
 * @brief ACMP initialization
 *****************************************************************************/
void acmp_init(void)
{
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_ACMP0);

  // Initialize with default settings
  ACMP_Init_TypeDef init = ACMP_INIT_DEFAULT;
  ACMP_Init(ACMP0, &init);

  // Allocate the appropriate bus to ACMP0 input
  ACMP_BUS_ALLOCATION();

  /*
   * Configure ACMP0 to compare the specified input pin against the
   * selected and divided reference (which defaults to divide-by-1 if
   * the ACMP_INIT_DEFAULT settings are not overridden).
   *
   * In this example, the internal 1.25 V reference is used undivided
   * so that when the input is lower than 1.25 V, the ACMP output is 1,
   * and when it's higher than 1.25 V, the ACMP output is 0.
   */
  ACMP_ChannelSet(ACMP0, acmpInputVREFDIV1V25, ACMP_INPUT_PORT_PIN);

  // Enable and drive inverted comparator on specified pin
#if (LED_ON == 1)
  ACMP_GPIOSetup(ACMP0, ACMP_OUTPUT.port, ACMP_OUTPUT.pin, true, true);
#else 
  ACMP_GPIOSetup(ACMP0, ACMP_OUTPUT.port, ACMP_OUTPUT.pin, true, false);
#endif

  // Wait for warm-up
  while (!(ACMP0->IF & ACMP_IF_ACMPRDY));
}

/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  // Halt if PB0 held before entering EM3 to allow debug access
  escapeHatch();

  // Initialize GPIO
  gpio_init();

  // Initialize ACMP
  acmp_init();
}

/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{
}

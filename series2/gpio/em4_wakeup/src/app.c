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
#include "pin_config.h"
#include "peripheral_config.h"
#include "em_rmu.h"
#include "sl_main_init.h"
#include "sl_power_manager.h"

// Define GPIO mapping for boards where LED0 and button0 share the same pins
#ifndef LED0_PORT
  #define LED0_PORT LED0_BUTTON0_PORT
  #define LED0_PIN LED0_BUTTON0_PIN
#endif

#ifndef BUTTON0_PORT
  #define BUTTON0_PORT LED0_BUTTON0_PORT
  #define BUTTON0_PIN LED0_BUTTON0_PIN
#endif

// Define GPIO mapping for boards where LED1 and button1 share the same pins
#ifndef BUTTON1_PORT
  #define BUTTON1_PORT LED1_BUTTON1_PORT
  #define BUTTON1_PIN LED1_BUTTON1_PIN
#endif

#define EM4WUEN (1<<EM4WUENx)

// Conditional compilation to configure GPIO mappings based on the target microcontroller
#if defined(EFR32MG21A010F1024IM32)
const sl_gpio_t GPIO_ESCAPE_HATCH = { .port = BUTTON1_PORT, .pin = BUTTON1_PIN };
const sl_gpio_t GPIO_EM4WU = { .port = BUTTON0_PORT, .pin = BUTTON0_PIN };
#else
const sl_gpio_t GPIO_ESCAPE_HATCH = { .port = BUTTON0_PORT, .pin = BUTTON0_PIN };
const sl_gpio_t GPIO_EM4WU = { .port = BUTTON1_PORT, .pin = BUTTON1_PIN };
#endif

const sl_gpio_t GPIO_LED0 = { .port = LED0_PORT, .pin = LED0_PIN };

/***************************************************************************//**
 *  User defined weak function for performing early application initialization.
 *  This is called from sl_main_init.
 ******************************************************************************/
void app_init_early(void)
{
  // Release pin state after EM4 exit (reset).
  sl_power_manager_em4_unlatch_pin_retention();
}

/**************************************************************************//**
 * When developing or debugging code that enters EM2 or lower, it's a
 * good idea to have an "escape hatch" type mechanism, e.g. a way to
 * pause the device so that a debugger can connect in order to erase
 * flash, among other things.
 *
 * Before proceeding with this example, make sure Escape Hatch button is not
 * pressed. If the Escape Hatch button pin is low, turn on LED0 and execute the
 * breakpoint (BKPT) instruction to stop the processor in EM0 and allow a debug
 * connection to be made.
 *****************************************************************************/
void escapeHatch(void)
{
  bool pin_value;

  // Initialize GPIO driver
  sl_gpio_init();

  // Configure Escape Hatch button as an input pulled high
  sl_gpio_set_pin_mode(&GPIO_ESCAPE_HATCH, SL_GPIO_MODE_INPUT_PULL_FILTER, true);

  // Check Escape Hatch button state: if pressed, enable LED0 and trigger breakpoint
  sl_gpio_get_pin_input(&GPIO_ESCAPE_HATCH, &pin_value);
  if (pin_value == 0)
  {
    sl_gpio_set_pin_mode(&GPIO_LED0, SL_GPIO_MODE_PUSH_PULL, LED_ON);
    __BKPT(0);  // Halt execution for debugging
  }
  else
  {
    // Disable Escape Hatch button digital input when not in use
    sl_gpio_set_pin_mode(&GPIO_ESCAPE_HATCH, SL_GPIO_MODE_DISABLED, false);
  }
}

/**************************************************************************//**
 * Configure GPIOs for EM4 wake-up button and LED0.
 *****************************************************************************/
void gpioSetup(void)
{
  // Initialize GPIO driver
  sl_gpio_init();

  // Configure EM4 wake-up button as an input and set it as an EM4 wake-up source
  sl_gpio_set_pin_mode(&GPIO_EM4WU, SL_GPIO_MODE_INPUT_PULL_FILTER, true);
  sl_gpio_enable_pin_em4_wakeup(EM4WUEN, 0);

  // Configure LED0 as a push-pull output for LED control
  sl_gpio_set_pin_mode(&GPIO_LED0, SL_GPIO_MODE_PUSH_PULL, LED_OFF);
}

/**************************************************************************//**
 * Toggle LED indefinitely.
 *****************************************************************************/
void toggleLED(void)
{
  while(1)
  {
     sl_gpio_toggle_pin(&GPIO_LED0);

     // Arbitrary delay between toggles
     for(volatile uint32_t delay = 0; delay < 0xFFFFF; delay++);
  }
}

/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  // Halt execution if Escape Hatch button is pressed before entering EM4
  escapeHatch();

  // GPIO setup
  gpioSetup();

  // Retrieve and clear the last reset cause
  uint32_t resetCause = RMU_ResetCauseGet();
  RMU_ResetCauseClear();

  // If the last reset was due to EM4 Wake-up, toggle LED; otherwise, enter EM4
  if (resetCause & EMU_RSTCAUSE_EM4)
  {
    toggleLED();
  }
  else
  {
    // Enter EM4
    sl_power_manager_enter_em4();
  }
}

/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{
}

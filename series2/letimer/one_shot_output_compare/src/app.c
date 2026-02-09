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
#include "em_letimer.h"
#include "pin_config.h"

// Desired frequency in Hz
#define OUT_FREQ 1000

// Desired repeat count
#define REPEAT_COUNT 10

#ifndef BUTTON0_PORT
  #define BUTTON0_PORT LED0_BUTTON0_PORT
  #define BUTTON0_PIN LED0_BUTTON0_PIN
#endif

#ifndef LED0_PORT
  #define LED0_PORT LED0_BUTTON0_PORT
  #define LED0_PIN LED0_BUTTON0_PIN
#endif

const sl_gpio_t LETIMER_OUTPUT = { .port = EXP_LET0_O0_PORT, .pin = EXP_LET0_O0_PIN };
const sl_gpio_t GPIO_PB0 = { .port = BUTTON0_PORT, .pin = BUTTON0_PIN };
const sl_gpio_t GPIO_LED0 = { .port = LED0_PORT, .pin = LED0_PIN };

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
    sl_gpio_set_pin_mode(&GPIO_LED0, SL_GPIO_MODE_PUSH_PULL, LED_ON);
    __BKPT(0);
  }
  // Pin not asserted; disable the PB0 digital input
  else {
    sl_gpio_set_pin_mode(&GPIO_PB0, SL_GPIO_MODE_DISABLED, false);
  }
}

/***************************************************************************//**
 * Initialize GPIO.
 ******************************************************************************/
void gpio_init(void)
{
  // Configure LET0_O0 output
  sl_gpio_set_pin_mode(&LETIMER_OUTPUT, SL_GPIO_MODE_PUSH_PULL, 0);
}

/**************************************************************************//**
 * @brief LETIMER initialization
 *****************************************************************************/
void letimer_init(void)
{
  LETIMER_Init_TypeDef letimerInit = LETIMER_INIT_DEFAULT;

  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_LETIMER0);

  // Calculate the top value (frequency) based on clock source
  uint32_t topValue;
  sl_clock_manager_get_clock_branch_frequency(SL_CLOCK_BRANCH_EM23GRPACLK, &topValue);
  topValue = topValue / OUT_FREQ;

  // Reload top on underflow, set output, and run in one-shot mode
  letimerInit.comp0Top = true;
  letimerInit.topValue = topValue;
  letimerInit.ufoa0 = letimerUFOAToggle;
  letimerInit.repMode = letimerRepeatOneshot;

  // Enable LETIMER0 output0 on configured output
  GPIO_LETIMERROUTE.ROUTEEN = GPIO_LETIMER_ROUTEEN_OUT0PEN;
  GPIO_LETIMERROUTE.OUT0ROUTE = \
      (EXP_LET0_O0_PORT << _GPIO_LETIMER_OUT0ROUTE_PORT_SHIFT) \
      | (EXP_LET0_O0_PIN << _GPIO_LETIMER_OUT0ROUTE_PIN_SHIFT);

  // repeat 10 times
  LETIMER_RepeatSet(LETIMER0, 0, REPEAT_COUNT);

  // Initialize and enable LETIMER
  LETIMER_Init(LETIMER0, &letimerInit);
}

/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  // Halt if PB0 held before entering EM2/3 to allow debug access
  escapeHatch();

  // Initialize GPIO
  gpio_init();

  // Initialize LETIMER
  letimer_init();
}

/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{
}

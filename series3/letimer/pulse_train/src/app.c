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
#include "sl_hal_letimer.h"
#include "pin_config.h"
#include "peripheral_config.h"

// Desired frequency in Hz
#define OUT_FREQ 1000

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
 * @brief escape_hatch()
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
void escape_hatch(void)
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
  sl_hal_letimer_init_t init = SL_HAL_LETIMER_INIT_DEFAULT;

  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_LETIMER0);

  // Calculate the top value (frequency) based on clock source
  uint32_t topValue;
  sl_clock_manager_get_clock_branch_frequency(SL_CLOCK_BRANCH_EM23GRPACLK, &topValue);
  topValue = topValue / OUT_FREQ;

  // Reload top on underflow, set output, and run in free mode
  init.enable_top = true;
  init.underflow_output0_action = SL_HAL_LETIMER_UNDERFLOW_OUTPUT_ACTION_PULSE;
  init.repeat_mode = SL_HAL_LETIMER_REPEAT_MODE_FREE;

  // Route LETIMER output to pin
  GPIO->LETIMERROUTE[0].ROUTEEN = GPIO_LETIMER_ROUTEEN_OUT0PEN;
  GPIO->LETIMERROUTE[0].OUT0ROUTE = EXP_LET0_O0_PIN << _GPIO_LETIMER_OUT0ROUTE_PIN_SHIFT | EXP_LET0_O0_PORT;

  // Initialize, enable, and start LETIMER
  sl_hal_letimer_init(LETIMER0, &init);
  sl_hal_letimer_set_top(LETIMER0, topValue);
  sl_hal_letimer_enable(LETIMER0);
  sl_hal_letimer_start(LETIMER0);
}

/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  // Halt if PB0 held before entering EM3 to allow debug access
  escape_hatch();

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

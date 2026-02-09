/*******************************************************************************
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
#include "pin_config.h"
#include "em_prs.h"

#ifndef BUTTON0_PORT
#define BUTTON0_PORT LED0_BUTTON0_PORT
#define BUTTON0_PIN LED0_BUTTON0_PIN
#endif

#ifndef BUTTON1_PORT
#define BUTTON1_PORT LED1_BUTTON1_PORT
#define BUTTON1_PIN LED1_BUTTON1_PIN
#endif

const sl_gpio_t BUTTON0 = { .port = BUTTON0_PORT, .pin = BUTTON0_PIN };
const sl_gpio_t BUTTON1 = { .port = BUTTON1_PORT, .pin = BUTTON1_PIN };

const sl_gpio_t PRS_OUTPUT = { .port = PRS_OUTPUT_PORT, .pin = PRS_OUTPUT_PIN };

/**************************************************************************//**
 * @brief GPIO initialization
 *****************************************************************************/
void gpio_init(void)
{
  // Set Push Buttons as input
  sl_gpio_set_pin_mode(&BUTTON0, SL_GPIO_MODE_INPUT_PULL_FILTER, true);
  sl_gpio_set_pin_mode(&BUTTON1, SL_GPIO_MODE_INPUT_PULL_FILTER, true);

  // Configure Push Buttons to create interrupt signals
  sl_hal_gpio_configure_external_interrupt(&BUTTON0, BUTTON0_PIN, SL_GPIO_INTERRUPT_NO_EDGE);
  sl_hal_gpio_configure_external_interrupt(&BUTTON1, BUTTON1_PIN, SL_GPIO_INTERRUPT_NO_EDGE);

  // Set PRS output
  sl_gpio_set_pin_mode(&PRS_OUTPUT, SL_GPIO_MODE_PUSH_PULL, LED_OFF);
}

/**************************************************************************//**
 * @brief PRS initialization
 *****************************************************************************/
void prs_init(void)
{
  // Enable PRS clock
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_PRS);

  // Use Push Buttons as PRS source
  // Push Button 0
  PRS_SourceAsyncSignalSet(
      PRS_INPUT_CH_PB0,
      PRS_ASYNC_CH_CTRL_SOURCESEL_GPIO,
      BUTTON0_PIN);

  // Push Button 1
  PRS_SourceAsyncSignalSet(
      PRS_INPUT_CH_PB1,
      PRS_ASYNC_CH_CTRL_SOURCESEL_GPIO,
      BUTTON1_PIN);

  // Configure PRS logic
  PRS_Combine(PRS_INPUT_CH_PB1, PRS_INPUT_CH_PB0, prsLogic_A_OR_B);

  // Route output to LED1
  PRS_PinOutput(PRS_INPUT_CH_PB1, prsTypeAsync, PRS_OUTPUT_PORT, PRS_OUTPUT_PIN);
  /* Note that there are certain restrictions to where a PRS channel can be
   *   routed. Consult the datasheet of the device to see if a channel can be
   *   routed to the requested GPIO pin.
   */
}

/*******************************************************************************
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  gpio_init();
  prs_init();
}

/*******************************************************************************
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{
  // No action necessary, stay in EM1
}

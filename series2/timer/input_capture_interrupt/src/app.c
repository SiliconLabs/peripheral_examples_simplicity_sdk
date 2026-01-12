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

#include "em_timer.h"

#include "pin_config.h"

// Define GPIO mapping for boards where LED0 and BUTTON0 share the same pins
#ifndef BUTTON0_PORT
  #define BUTTON0_PORT LED0_BUTTON0_PORT
  #define BUTTON0_PIN  LED0_BUTTON0_PIN
#endif

const sl_gpio_t TIMER_INPUT = { .port = BUTTON0_PORT,
                                .pin  = BUTTON0_PIN };

#define BUFFERSIZE 8

// Edge capture buffer
static volatile uint32_t buffer[BUFFERSIZE];

/***************************************************************************//**
 * Initialize GPIO.
 ******************************************************************************/
void gpio_init(void)
{
  // Configure TIMER_INPUT as input
  sl_gpio_set_pin_mode(&TIMER_INPUT, SL_GPIO_MODE_INPUT_PULL_FILTER, 1);
}

/**************************************************************************//**
 * @brief
 *    TIMER initialization
 *
 * @note
 *    Prescaling the TIMER clock may or may not be necessary in a given
 *    application.  Here, prescaling the clock by 1024 (each TIMER tick
 *    is 1024 HFPERCLKs) makes it easier to see the difference between
 *    successive capture values.
 *****************************************************************************/
void timer_init(void)
{
  // Initialize TIMER0
  TIMER_Init_TypeDef timerInit = TIMER_INIT_DEFAULT;
  timerInit.prescale = timerPrescale1024;
  timerInit.enable = false;               // Don't start the timer yet

  // Configure TIMER0 capture/compare channel 0 for input capture
  TIMER_InitCC_TypeDef timerCCInit = TIMER_INITCC_DEFAULT;
  timerCCInit.edge = timerEdgeFalling;
  timerCCInit.mode = timerCCModeCapture;

  // Enable TIMER0 bus clocks.
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_TIMER0);

  TIMER_Init(TIMER0, &timerInit);

  // Route the Button 0 to TIMER0 capture/compare channel 0 and enable
  GPIO->TIMERROUTE[0].CC0ROUTE =
      (TIMER_INPUT.port << _GPIO_TIMER_CC0ROUTE_PORT_SHIFT)
      | (TIMER_INPUT.pin << _GPIO_TIMER_CC0ROUTE_PIN_SHIFT);
  GPIO->TIMERROUTE[0].ROUTEEN  = GPIO_TIMER_ROUTEEN_CC0PEN;

  TIMER_InitCC(TIMER0, 0, &timerCCInit);

  /*************************************************************************//**
   * POLLED MODE INSTRUCTIONS
   * -------------------------
   *
   * To use this example in polled mode instead of interrupt mode -
   *
   * 1. Remove TIMER_IntEnable(TIMER0, TIMER_IEN_CC0);
   * 2. Remove sl_interrupt_manager_enable_irq(TIMER0_IRQn);
   *
   ****************************************************************************/

  // Enable interrupts on capture/compare channel 0
  TIMER_IntClear(TIMER0, _TIMER_IF_MASK);
  TIMER_IntEnable(TIMER0, TIMER_IEN_CC0);
  sl_interrupt_manager_clear_irq_pending(TIMER0_IRQn);
  sl_interrupt_manager_enable_irq(TIMER0_IRQn);

  // Now start the TIMER
  TIMER_Enable(TIMER0, true);
}

/***************************************************************************//**
 * POLLED MODE INSTRUCTIONS
 * -------------------------
 *
 * To use this example in polled mode instead of interrupt mode, remove
 * the TIMER0_IRQHandler(void) function.
 *
 ******************************************************************************/

/***************************************************************************//**
 * @brief
 *    TIMER 0 handler
 ******************************************************************************/
void TIMER0_IRQHandler(void)
{
  static uint32_t i = 0;

  // Acknowledge the interrupt
  uint32_t flags = TIMER_IntGet(TIMER0);
  TIMER_IntClear(TIMER0, flags);

  // Check for capture event on channel 0
  if (flags & TIMER_IF_CC0) {

    // Record input capture value
    buffer[i] = TIMER_CaptureGet(TIMER0, 0);

    // Increment index and have it wrap around
    i = (i + 1) % BUFFERSIZE;
  }
}

/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  // Initialize GPIO and TIMER
  gpio_init();
  timer_init();

  /**************************************************************************//**
   * POLLED MODE INSTRUCTIONS
   * -------------------------
   *
   * To use this example in polled mode instead of interrupt mode.
   *
   * 2. Remove Power Manager software component from the project.
   *
   *****************************************************************************/
}

/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{
  /*************************************************************************//**
  * POLLED MODE INSTRUCTIONS
  * -------------------------
  *
  * To use this example in polled mode instead of interrupt mode,
  * insert the following code...
  *
  * static uint32_t i = 0;
  *
  * // Wait for a capture event to happen
  * while (TIMER0->STATUS & TIMER_STATUS_ICFEMPTY0);
  *
  * // Record input capture value
  * buffer[i] = TIMER_CaptureGet(TIMER0, 0);
  *
  * // Increment index and have it wrap around
  * i = (i + 1) % BUFFERSIZE;
  *
  *****************************************************************************/
}

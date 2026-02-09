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

// Stored edges from interrupt
volatile uint32_t firstEdge;
volatile uint32_t secondEdge;

const sl_gpio_t TIMER_INPUT = { .port = TIMER_EXTIO_PORT,
                                .pin  = TIMER_EXTIO_PIN };

/***************************************************************************//**
 * Initialize GPIO.
 ******************************************************************************/
void gpio_init(void)
{
  // Configure TIMER_INPUT as input
  sl_gpio_set_pin_mode(&TIMER_INPUT, SL_GPIO_MODE_INPUT, 0);
}

/**************************************************************************//**
 * @brief TIMER initialization
 *****************************************************************************/
void timer_init(void)
{
  // Initialize the timer
  TIMER_Init_TypeDef timerInit = TIMER_INIT_DEFAULT;

  // Configure TIMER0 for input capture mode
  TIMER_InitCC_TypeDef timerCCInit = TIMER_INITCC_DEFAULT;

  // Enable TIMER0 bus clocks.
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_TIMER0);

  timerCCInit.mode = timerCCModeCapture;
  timerCCInit.edge = timerEdgeBoth;                 // Input capture on every edge
  timerCCInit.eventCtrl = timerEventEvery2ndEdge;   // Interrupt on every other edge
  timerInit.enable = false;

  TIMER_Init(TIMER0, &timerInit);

  // Route TIMER0 CC0 input to TIMER_EXTIO
  GPIO->TIMERROUTE[0].CC0ROUTE =
      (TIMER_EXTIO_PORT << _GPIO_TIMER_CC0ROUTE_PORT_SHIFT)
      | (TIMER_EXTIO_PIN << _GPIO_TIMER_CC0ROUTE_PIN_SHIFT);
  GPIO->TIMERROUTE[0].ROUTEEN  = GPIO_TIMER_ROUTEEN_CC0PEN;

  TIMER_InitCC(TIMER0, 0, &timerCCInit);

  // Enable TIMER0 interrupts
  TIMER_IntClear(TIMER0, _TIMER_IF_MASK);
  TIMER_IntEnable(TIMER0, TIMER_IEN_CC0);
  sl_interrupt_manager_clear_irq_pending(TIMER0_IRQn);
  sl_interrupt_manager_enable_irq(TIMER0_IRQn);;

  // Now enable the TIMER
  TIMER_Enable(TIMER0, true);
}

/**************************************************************************//**
 * @brief
 *    Interrupt handler for TIMER0
 *****************************************************************************/
void TIMER0_IRQHandler(void)
{
  // Acknowledge the interrupt
  uint32_t flags = TIMER_IntGet(TIMER0);
  TIMER_IntClear(TIMER0, flags);

  /*
   * Read the last two captured edges.  Note that this interrupt
   * occurs after the second edge.
   */
  firstEdge = TIMER_CaptureGet(TIMER0, 0);
  secondEdge = TIMER_CaptureGet(TIMER0, 0);
}

/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  // Initialize GPIO and TIMER
  gpio_init();
  timer_init();
}

/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{
}

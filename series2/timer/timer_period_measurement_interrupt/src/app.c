/***************************************************************************//**
 * @file
 * @brief Top level application functions
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
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

// Most recent measured period in microseconds
uint32_t measuredPeriod;

// Edge pair times stored in the IRQ handler
uint32_t firstEdge;
uint32_t secondEdge;

// Was there an overflow between the edge pair
bool overflow = false;

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

  // Configure TIMER0 for input period measurement
  TIMER_InitCC_TypeDef timerCCInit = TIMER_INITCC_DEFAULT;

  // Enable TIMER0 bus clocks.
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_TIMER0);

  timerCCInit.mode = timerCCModeCapture;
  timerCCInit.edge = timerEdgeFalling;              // Input capture on falling edges
  timerCCInit.eventCtrl = timerEventEvery2ndEdge;   // Interrupt on every other edge
  timerInit.enable = false;

  TIMER_Init(TIMER0, &timerInit);

  // Route TIMER0 CC0 input to TIMER_EXTIO
  GPIO->TIMERROUTE[0].CC0ROUTE =
      (TIMER_INPUT.port << _GPIO_TIMER_CC0ROUTE_PORT_SHIFT)
      | (TIMER_INPUT.pin << _GPIO_TIMER_CC0ROUTE_PIN_SHIFT);
  GPIO->TIMERROUTE[0].ROUTEEN  = GPIO_TIMER_ROUTEEN_CC0PEN;

  TIMER_InitCC(TIMER0, 0, &timerCCInit);

  /*
   * Overwrite the default of 0xFFFF in TIMER_TOP with 0xFFFFFFFF
   * because TIMER0 is 32 bits wide.
   */
  TIMER_TopSet(TIMER0, 0xFFFFFFFF);

  // Enable TIMER0 interrupts
  TIMER_IntClear(TIMER0, _TIMER_IF_MASK);
  TIMER_IntEnable(TIMER0, TIMER_IEN_CC0);
  sl_interrupt_manager_clear_irq_pending(TIMER0_IRQn);
  sl_interrupt_manager_enable_irq(TIMER0_IRQn);

  // Enable the TIMER
  TIMER_Enable(TIMER0, true);
}

/**************************************************************************//**
 * @brief
 *    Interrupt handler for TIMER0
 *
 * @note
 *    Because TIMER0 has a 32-bit counter and because the variables used
 *    are also 32-bit, the calculated period is limited to 2^32 - 1 ticks
 *    of the counter clock.
 *****************************************************************************/
void TIMER0_IRQHandler(void)
{
  // Get the interrupt flags
  uint32_t flags = TIMER_IntGet(TIMER0);

  // Get the time at which each falling edge was captured
  firstEdge = TIMER_CaptureGet(TIMER0, 0);
  secondEdge = TIMER_CaptureGet(TIMER0, 0);

  // Was there an overflow between edges?
  if (flags & TIMER_IF_OF)
    overflow = true;

  // Clear the interrupt flags and exit
  TIMER_IntClear(TIMER0, flags);
}

/**************************************************************************//**
 * @brief
 *    Calculate the waveform period from the captured pair of edges
 *
 * @return
 *    The period of the input waveform
 *****************************************************************************/
uint32_t calculatePeriod(void)
{
  uint32_t countsBetweenEdges, timerFreq, timerClockMHz;

  /*
   * Calculate the frequency of TIMER0 from the bus clock.  This
   * assumes the prescaler remains at the default value of 1.
   */
  sl_clock_manager_get_clock_branch_frequency(SL_CLOCK_BRANCH_EM01GRPACLK,
                                              &timerFreq);
  timerClockMHz = timerFreq / 1000000;

  /*
   * Calculate the count between edges depending on whether or not
   * there was an overflow.
   */
  if (overflow)
    countsBetweenEdges = TIMER_TopGet(TIMER0) - firstEdge + 1 + secondEdge;
  else
    countsBetweenEdges = secondEdge - firstEdge;

  // Reset the overflow flag
  overflow = false;

  // Convert the count between edges to a period in microseconds
  return (countsBetweenEdges / timerClockMHz);
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
  // Record the period into the global variable
  measuredPeriod = calculatePeriod();
}

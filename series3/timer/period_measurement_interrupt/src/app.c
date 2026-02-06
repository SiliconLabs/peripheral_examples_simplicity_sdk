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

#include "sl_core.h"
#include "sl_clock_manager.h"
#include "sl_gpio.h"
#include "sl_hal_timer.h"
#include "pin_config.h"

// Most recent measured period in microseconds
uint32_t measured_period;

// Edge pair times stored in the IRQ handler
uint32_t first_edge;
uint32_t second_edge;
bool new_edge = false;

// Was there an overflow between the edge pair
bool overflow = false;

void TIMER_Handler(void);
const sl_gpio_t TIMER_INPUT = { .port = EXP_TIMER_CC0_PORT,
                                .pin  = EXP_TIMER_CC0_PIN };

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
  sl_hal_timer_init_t init = SL_HAL_TIMER_INIT_DEFAULT;
  sl_hal_timer_channel_config_t channel_init = SL_HAL_TIMER_CHANNEL_INIT_DEFAULT;

  // Enable TIMER0 bus clocks.
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_TIMER0);

  // Configure capture/compare channel for input capture
  channel_init.channel_mode = SL_HAL_TIMER_CHANNEL_MODE_CAPTURE;
  channel_init.input_capture_edge = SL_HAL_TIMER_CHANNEL_EDGE_FALLING;
  channel_init.input_capture_event = SL_HAL_TIMER_CHANNEL_EVENT_EVERY_SECONDE_EDGE;

  sl_hal_timer_init(TIMER0, &init);
  sl_hal_timer_channel_init(TIMER0, 0, &channel_init);
  sl_hal_timer_enable(TIMER0);

  // Route Button 0 to TIMER0 capture/compare channel 0
  GPIO->TIMER0ROUTE[0].ROUTEEN = GPIO_TIMER0_ROUTEEN_CC0PEN;
  GPIO->TIMER0ROUTE[0].CC0ROUTE =
      TIMER_INPUT.pin << _GPIO_TIMER0_CC0ROUTE_PIN_SHIFT
    | TIMER_INPUT.port;

  /*
   * Overwrite the default of 0xFFFF in TIMER_TOP with 0xFFFFFFFF
   * because TIMER0 is 32 bits wide.
   */
  sl_hal_timer_set_top(TIMER0, 0xFFFFFFFF);

  // Enable interrupts on capture/compare channel 0
  sl_interrupt_manager_set_irq_handler(TIMER0_IRQn, TIMER_Handler);
  sl_interrupt_manager_enable_irq(TIMER0_IRQn);
  sl_interrupt_manager_clear_irq_pending(TIMER0_IRQn);
  sl_hal_timer_clear_interrupts(TIMER0, _TIMER_IF_MASK);
  sl_hal_timer_enable_interrupts(TIMER0, TIMER_IF_CC0);

  // Start counter.
  sl_hal_timer_start(TIMER0);
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
void TIMER_Handler(void)
{
  // Get the interrupt flags
  uint32_t flags = sl_hal_timer_get_pending_interrupts(TIMER0);

  // Get the time at which each falling edge was captured
  first_edge = sl_hal_timer_channel_get_capture(TIMER0, 0);
  second_edge = sl_hal_timer_channel_get_capture(TIMER0, 0);

  // Was there an overflow between edges?
  if (flags & TIMER_IF_OF)
    overflow = true;

  // Set flag for main loop
  new_edge = true;

  // Clear the interrupt flags and exit
  sl_hal_timer_clear_interrupts(TIMER0, flags);
}

/**************************************************************************//**
 * @brief
 *    Calculate the waveform period from the captured pair of edges
 *
 * @return
 *    The period of the input waveform
 *****************************************************************************/
uint32_t calculate_period(void)
{
  CORE_irqState_t state;
  uint32_t counts_between_edges, timer_freq, timer_clock_MHz;

  /*
   * Calculate the frequency of TIMER0 from the bus clock.  This
   * assumes the prescaler remains at the default value of 1.
   */
  sl_clock_manager_get_clock_branch_frequency(SL_CLOCK_BRANCH_EM01GRPACLK,
                                              &timer_freq);
  timer_clock_MHz = timer_freq / 1000000;

  /*
   * Calculate the count between edges depending on whether or not
   * there was an overflow.  Critical section to keep edge variables
   * from being updated in the middle of the calculation.
   */
  state = CORE_EnterCritical();
  if (overflow)
    counts_between_edges = sl_hal_timer_get_top(TIMER0) - first_edge + 1 + second_edge;
  else
    counts_between_edges = second_edge - first_edge;
  CORE_ExitCritical(state);

  // Reset the overflow flag
  overflow = false;

  // Convert the count between edges to a period in microseconds
  return (counts_between_edges / timer_clock_MHz);
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
  if (new_edge == true) {
    // Record the period into the global variable
    measured_period = calculate_period();
  }
}

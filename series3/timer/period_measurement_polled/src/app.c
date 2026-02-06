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
#include "sl_hal_timer.h"
#include "pin_config.h"

// Most recent measured period in microseconds
static volatile uint32_t measured_period;

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

  // Clear TIMER flags
  sl_hal_timer_clear_interrupts(TIMER0, _TIMER_IF_MASK);

  // Start counter.
  sl_hal_timer_start(TIMER0);
}

/**************************************************************************//**
 * @brief
 *    Calculate the waveform period from the captured pair of edges
 *
 * @return
 *    The period of the input waveform
 *****************************************************************************/
uint32_t calculate_period(uint32_t fe, uint32_t se, bool ovf)
{
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
   * there was an overflow.
   */
  if (ovf)
    counts_between_edges = sl_hal_timer_get_top(TIMER0) - fe + 1 + se;
  else
    counts_between_edges = se - fe;

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
  // Edge pair times
  volatile uint32_t first_edge;
  volatile uint32_t second_edge;

  // Overflow flag between the edge pair
  bool overflow = false;

  // Check for a capture event
  if ((sl_hal_timer_get_pending_interrupts(TIMER0) & TIMER_IF_CC0) == TIMER_IF_CC0) {
    // Get the time at which each falling edge was captured
    first_edge = sl_hal_timer_channel_get_capture(TIMER0, 0);
    second_edge = sl_hal_timer_channel_get_capture(TIMER0, 0);

    // Was there an overflow between edges?
    if (sl_hal_timer_get_pending_interrupts(TIMER0) & TIMER_IF_OF)
    {
      overflow = true;
      sl_hal_timer_clear_interrupts(TIMER0, TIMER_IF_OF);
    }

    // Record the period into the global variable
    measured_period = calculate_period(first_edge, second_edge, overflow);

    // Clear the TIMER0 CC0 flag
    sl_hal_timer_clear_interrupts(TIMER0, TIMER_IF_CC0);
  }
}

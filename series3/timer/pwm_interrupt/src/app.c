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

const sl_gpio_t TIMER_OUTPUT = { .port = EXP_TIMER_CC0_PORT,
                                 .pin  = EXP_TIMER_CC0_PIN };

// Desired PWM frequency and initial duty cycle
#define PWM_FREQ            1000
#define INITIAL_DUTY_CYCLE  30

void TIMER_Handler(void);

// Duty cycle global variable for IRQ handler use
static volatile float duty_cycle;

/***************************************************************************//**
 * Initialize GPIO.
 ******************************************************************************/
void gpio_init(void)
{
  // Configure TIMER_OUTPUT
  sl_gpio_set_pin_mode(&TIMER_OUTPUT, SL_GPIO_MODE_PUSH_PULL, 0);
}

/**************************************************************************//**
 * @brief TIMER initialization
 *****************************************************************************/
void timer_init(void)
{
  uint32_t timer_freq, top_value, duty_count;
  sl_hal_timer_init_t init = SL_HAL_TIMER_INIT_DEFAULT;
  sl_hal_timer_channel_config_t channel_init = SL_HAL_TIMER_CHANNEL_INIT_DEFAULT;

  // Enable TIMER0 bus clocks.
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_TIMER0);

  // Configure capture/compare channel for input capture
  channel_init.channel_mode = SL_HAL_TIMER_CHANNEL_MODE_PWM;

  sl_hal_timer_init(TIMER0, &init);
  sl_hal_timer_channel_init(TIMER0, 0, &channel_init);
  sl_hal_timer_enable(TIMER0);

  // Route Button 0 to TIMER0 capture/compare channel 0
  GPIO->TIMER0ROUTE[0].ROUTEEN = GPIO_TIMER0_ROUTEEN_CC0PEN;
  GPIO->TIMER0ROUTE[0].CC0ROUTE =
      TIMER_OUTPUT.pin << _GPIO_TIMER0_CC0ROUTE_PIN_SHIFT
    | TIMER_OUTPUT.port;

  // Set top value to overflow at the desired PWM_FREQ frequency
  sl_clock_manager_get_clock_branch_frequency(SL_CLOCK_BRANCH_EM01GRPACLK,
                                              &timer_freq);
  timer_freq /= (init.prescaler + 1);
  top_value = (timer_freq / PWM_FREQ);
  sl_hal_timer_set_top(TIMER0, top_value);

  // Set duty_cycle global variable and compare value for initial duty cycle
  duty_cycle = INITIAL_DUTY_CYCLE;
  duty_count = (top_value * INITIAL_DUTY_CYCLE) / 100;
  sl_hal_timer_channel_set_compare(TIMER0, 0, duty_count);

  // Enable interrupts on capture/compare channel 0
  sl_interrupt_manager_set_irq_handler(TIMER0_IRQn, TIMER_Handler);
  sl_interrupt_manager_enable_irq(TIMER0_IRQn);
  sl_interrupt_manager_clear_irq_pending(TIMER0_IRQn);
  sl_hal_timer_clear_interrupts(TIMER0, _TIMER_IF_MASK);
  sl_hal_timer_enable_interrupts(TIMER0, TIMER_IF_CC0);

  // Start the timer
  sl_hal_timer_start(TIMER0);
}

/**************************************************************************//**
 * @brief
 *    Interrupt handler for TIMER0
 *
 * @note
 *    In this example, the duty cycle of the output waveform does not
 *    change unless the value of the duty_cycle global variable is
 *    modified outside the scope of this function.
 *
 *    Alternatively, other code could be inserted here to modify the
 *    duty cycle based on some other input, e.g. the voltage measured
 *    by the IADC on a given input channel.
 *****************************************************************************/
void TIMER_Handler(void)
{
  uint32_t new_duty_count;

  // Acknowledge the interrupt
  uint32_t flags = sl_hal_timer_get_pending_interrupts(TIMER0);
  sl_hal_timer_clear_interrupts(TIMER0, flags);

  // Calculate new duty cycle count
  new_duty_count = (uint32_t)((sl_hal_timer_get_top(TIMER0) * duty_cycle) / 100);

  // Write OCB to update the duty cycle of the next waveform period
  sl_hal_timer_channel_set_compare_buffer(TIMER0, 0, new_duty_count);
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

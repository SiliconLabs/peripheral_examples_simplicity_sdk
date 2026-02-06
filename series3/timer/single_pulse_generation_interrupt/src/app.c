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

/*
 * Change this to modify the length of the delay from when the TIMER
 * starts counting to when CC0 drives the GPIO pin high.
 */
#define NUM_SEC_DELAY 1

void TIMER_Handler(void);

// Length of the pulse high time in milliseconds
#define PULSE_WIDTH   100

// Rising and falling edges compare values (for debugger visibility)
static uint32_t timer_freq, compare_value1;
static volatile uint32_t compare_value2;

const sl_gpio_t TIMER_OUTPUT = { .port = EXP_TIMER_CC0_PORT,
                                 .pin  = EXP_TIMER_CC0_PIN };

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
  sl_hal_timer_init_t init = SL_HAL_TIMER_INIT_DEFAULT;
  sl_hal_timer_channel_config_t channel_init = SL_HAL_TIMER_CHANNEL_INIT_DEFAULT;

  // Enable TIMER0 bus clocks.
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_TIMER0);

  // Configure capture/compare channel for output compare
  init.count_one_shot = true;
  channel_init.channel_mode = SL_HAL_TIMER_CHANNEL_MODE_COMPARE;
  channel_init.compare_match_output_action = SL_HAL_TIMER_CHANNEL_OUTPUT_ACTION_TOGGLE;

  // Route TIMER0 CC0 output to output pin
  GPIO->TIMER0ROUTE[0].ROUTEEN = GPIO_TIMER0_ROUTEEN_CC0PEN;
  GPIO->TIMER0ROUTE[0].CC0ROUTE =
      TIMER_OUTPUT.pin << _GPIO_TIMER0_CC0ROUTE_PIN_SHIFT
    | TIMER_OUTPUT.port;

  sl_hal_timer_init(TIMER0, &init);
  sl_hal_timer_channel_init(TIMER0, 0, &channel_init);
  sl_hal_timer_enable(TIMER0);

  /*
   * Overwrite the default of 0xFFFF in TIMER_TOP with 0xFFFFFFFF
   * because TIMER0 is 32 bits wide.
   */
  sl_hal_timer_set_top(TIMER0, 0xFFFFFFFF);

  // Set compare value to the desired delay
  sl_clock_manager_get_clock_branch_frequency(SL_CLOCK_BRANCH_EM01GRPACLK,
                                              &timer_freq);
  timer_freq /= (init.prescaler + 1);
  compare_value1 = timer_freq * NUM_SEC_DELAY;
  sl_hal_timer_channel_set_compare(TIMER0, 0, compare_value1);

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
 * @brief TIMER0 handler
 *****************************************************************************/
void TIMER_Handler(void)
{
  // Acknowledge the interrupt
  uint32_t flags = sl_hal_timer_get_pending_interrupts(TIMER0);
  sl_hal_timer_clear_interrupts(TIMER0, flags);

  // Load compare register with second compare value
  compare_value2 = compare_value1 + ((timer_freq / 1000) * PULSE_WIDTH);
  sl_hal_timer_channel_set_compare(TIMER0, 0, compare_value2);
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

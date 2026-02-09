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

const sl_gpio_t TIMER_OUTPUT = { .port = TIMER_EXTIO_PORT,
                                 .pin  = TIMER_EXTIO_PIN };

// Desired PWM frequency and initial duty cycle
#define PWM_FREQ            1000
#define INITIAL_DUTY_CYCLE  30

// Duty cycle global variable for IRQ handler use
static volatile float dutyCycle;

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
  uint32_t timerFreq, topValue, dutyCount;
  TIMER_Init_TypeDef timerInit = TIMER_INIT_DEFAULT;
  TIMER_InitCC_TypeDef timerCCInit = TIMER_INITCC_DEFAULT;

  // Enable TIMER0 bus clocks.
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_TIMER0);

  // Don't start counter on initialization
  timerInit.enable = false;

  // PWM mode sets/clears the output on compare/overflow events
  timerCCInit.mode = timerCCModePWM;

  TIMER_Init(TIMER0, &timerInit);

  // Route TIMER0 CC0 output to TIMER_EXTIO
  GPIO->TIMERROUTE[0].CC0ROUTE =
      (TIMER_OUTPUT.port << _GPIO_TIMER_CC0ROUTE_PORT_SHIFT)
      | (TIMER_OUTPUT.pin << _GPIO_TIMER_CC0ROUTE_PIN_SHIFT);
  GPIO->TIMERROUTE[0].ROUTEEN  = GPIO_TIMER_ROUTEEN_CC0PEN;

  TIMER_InitCC(TIMER0, 0, &timerCCInit);

  // Set top value to overflow at the desired PWM_FREQ frequency
  sl_clock_manager_get_clock_branch_frequency(SL_CLOCK_BRANCH_EM01GRPACLK,
                                              &timerFreq);
  timerFreq /= (timerInit.prescale + 1);
  topValue = (timerFreq / PWM_FREQ);
  TIMER_TopSet(TIMER0, topValue);

  // Set dutyCycle global variable and compare value for initial duty cycle
  dutyCycle = INITIAL_DUTY_CYCLE;
  dutyCount = (topValue * INITIAL_DUTY_CYCLE) / 100;
  TIMER_CompareSet(TIMER0, 0, dutyCount);

  // Now start the TIMER
  TIMER_Enable(TIMER0, true);

  // Enable TIMER0 compare event interrupts to update the duty cycle
  TIMER_IntClear(TIMER0, _TIMER_IF_MASK);
  TIMER_IntEnable(TIMER0, TIMER_IEN_CC0);
  sl_interrupt_manager_clear_irq_pending(TIMER0_IRQn);
  sl_interrupt_manager_enable_irq(TIMER0_IRQn);
}

/**************************************************************************//**
 * @brief
 *    Interrupt handler for TIMER0
 *
 * @note
 *    In this example, the duty cycle of the output waveform does not
 *    change unless the value of the dutyCycle global variable is
 *    modified outside the scope of this function.
 *
 *    Alternatively, other code could be inserted here to modify the
 *    duty cycle based on some other input, e.g. the voltage measured
 *    by the IADC on a given input channel.
 *****************************************************************************/
void TIMER0_IRQHandler(void)
{
  uint32_t newDutyCount;

  // Acknowledge the interrupt
  uint32_t flags = TIMER_IntGet(TIMER0);
  TIMER_IntClear(TIMER0, flags);

  // Calculate new duty cycle count
  newDutyCount = (uint32_t)((TIMER_TopGet(TIMER0) * dutyCycle) / 100);

  // Write OCB to update the duty cycle of the next waveform period
  TIMER_CompareBufSet(TIMER0, 0, newDutyCount);
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

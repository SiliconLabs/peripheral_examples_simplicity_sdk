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
#include "pin_config.h"
#include "em_timer.h"

// Define GPIO mapping for boards where LED0 and button0 share the same pins
#ifndef BUTTON0_PORT
  #define BUTTON0_PORT LED0_BUTTON0_PORT
  #define BUTTON0_PIN  LED0_BUTTON0_PIN
#endif

#define OUTPUT_FREQ_Hz   (1000000UL)  // Output frequency of 1 MHz
#define TIMER_PRESCALER  2           // Timer prescaler value

volatile uint32_t slewRate = 6;  // Default slew rate setting

const sl_gpio_t GPIO_PB0     = { .port = BUTTON0_PORT, .pin = BUTTON0_PIN };
const sl_gpio_t SQUARE_WAVE  = { .port = SQUARE_WAVE_PORT, .pin = SQUARE_WAVE_PIN };

/***************************************************************************//**
 * An internal callback called in interrupt context whenever a button changes
 * its state.
 *
 * @note The internal callback will be triggered when the Push button 0 is pressed.
 * Increments the slew rate setting in a cyclic manner.
 *
 ******************************************************************************/
static void button_0_change()
{
  slewRate = (slewRate + 1) % 8;
}

/***************************************************************************//**
 * Initialize GPIO.
 ******************************************************************************/
void gpioSetup(void)
{
  int32_t int_no = GPIO_PB0.pin;

  // Initialize GPIO driver
  sl_gpio_init();

  // Configure Button0 as an input and enable interrupt
  sl_gpio_set_pin_mode(&GPIO_PB0, SL_GPIO_MODE_INPUT_PULL, true);
  sl_gpio_configure_external_interrupt(&GPIO_PB0, &int_no,
                                       SL_GPIO_INTERRUPT_FALLING_EDGE,
                                       (sl_gpio_irq_callback_t) button_0_change,
                                       NULL);

  // Set up GPIO to output CCO 1MHz waveform
  GPIO->TIMERROUTE[0].ROUTEEN |= GPIO_TIMER_ROUTEEN_CC0PEN;
  GPIO->TIMERROUTE[0].CC0ROUTE = (SQUARE_WAVE_PORT << _GPIO_TIMER_CC0ROUTE_PORT_SHIFT)
                               | (SQUARE_WAVE_PIN << _GPIO_TIMER_CC0ROUTE_PIN_SHIFT);

  // Set up GPIO to push-pull output to output 1MHz square wave
  sl_gpio_set_pin_mode(&SQUARE_WAVE, SL_GPIO_MODE_PUSH_PULL, true);
}

/***************************************************************************//**
 * Initialize timer.
 ******************************************************************************/
void timerSetup(void)
{
  uint32_t frequency;
  
  // Enable Timer0 clock
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_TIMER0);

  // Set compare output
  TIMER_InitCC_TypeDef initCh = TIMER_INITCC_DEFAULT;
  initCh.cofoa = timerOutputActionToggle;
  initCh.mode = timerCCModeCompare;
  TIMER_InitCC(TIMER0, 0, &initCh);

  // Set max value of timer to cause period to be 1MHz
  // Timer counts from 0 - value and  overflows and toggles the pin
  sl_clock_manager_get_clock_branch_frequency(SL_CLOCK_BRANCH_EM01GRPACLK, &frequency);
  TIMER_TopSet(TIMER0, frequency / TIMER_PRESCALER / OUTPUT_FREQ_Hz / TIMER_PRESCALER - 1);

  // Init timer after setting up CC as timer stops running after CC init
  TIMER_Init_TypeDef init = TIMER_INIT_DEFAULT;
  init.enable = true;
  init.prescale = (TIMER_Prescale_TypeDef) _TIMER_CFG_PRESC_DIV2;
  TIMER_Init(TIMER0, &init);
}

/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  gpioSetup();
  timerSetup();
}

/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{
  sl_gpio_set_slew_rate(&SQUARE_WAVE, slewRate);
}
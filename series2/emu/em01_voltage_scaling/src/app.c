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

#include "em_emu.h"
#include "em_ramfunc.h"
#include "pin_config.h"
#include "sl_gpio.h"

#ifndef BUTTON0_PORT
  #define BUTTON0_PORT LED0_BUTTON0_PORT
  #define BUTTON0_PIN LED0_BUTTON0_PIN
#endif

const sl_gpio_t BUTTON0 = { .port = BUTTON0_PORT, .pin = BUTTON0_PIN };

/**************************************************************************//**
 * @brief GPIO IRQHandler
 *****************************************************************************/
#if (BUTTON0_PIN & 1)
void gpio_odd_irq_callback(uint8_t irq_idx, void* context)
#else
void gpio_even_irq_callback(uint8_t irq_idx, void* context)
#endif
{
  // Eliminate "unused parameter" warnings
  (void)context;
  (void)irq_idx;

  EMU_EM01Init_TypeDef vsInit = EMU_EM01INIT_DEFAULT;
  EMU_VScaleEM01_TypeDef vscale;

  // Get the current voltage scaling
  vscale = EMU_VScaleGet();

  if (vscale == emuVScaleEM01_HighPerformance)
  {
    // Currently running at VS2 (high performance), so scale down
    vsInit.vScaleEM01LowPowerVoltageEnable = true;
  }
  else
  {
    // Currently running at VS1 (low power), so scale up
    vsInit.vScaleEM01LowPowerVoltageEnable = false;
  }

  /*
   * Perform voltage scaling and set the appropriate number of flash
   * wait states.
   */
  EMU_EM01Init(&vsInit);

}
/**************************************************************************//**
 * @brief
 *    GPIO initialization
 *****************************************************************************/
void gpio_init(void)
{
  int32_t interrupt_number = BUTTON0_PIN;

  // Configure button 0 pin as an input
  sl_gpio_set_pin_mode(&BUTTON0, SL_GPIO_MODE_INPUT_PULL, 1);

#if (BUTTON0_PIN & 1)
  // Interrupt on button 0 rising edge to start transfers
  sl_gpio_configure_external_interrupt(&BUTTON0, &interrupt_number, SL_GPIO_INTERRUPT_RISING_EDGE, &gpio_odd_irq_callback, (void*)0);

  // Enable NVIC GPIO interrupt
  sl_interrupt_manager_clear_irq_pending(GPIO_ODD_IRQn);
  sl_interrupt_manager_enable_irq(GPIO_ODD_IRQn);
#else
  // Interrupt on button 0 rising edge to start transfers
  sl_gpio_configure_external_interrupt(&BUTTON0, &interrupt_number, SL_GPIO_INTERRUPT_RISING_EDGE, &gpio_even_irq_callback, (void*)0);

  // Enable NVIC GPIO interrupt
  sl_interrupt_manager_clear_irq_pending(GPIO_EVEN_IRQn);
  sl_interrupt_manager_enable_irq(GPIO_EVEN_IRQn);
#endif
}

/**************************************************************************//**
 * @brief  Calculates the n-th Fibonacci number recursively.
 *****************************************************************************/
SL_RAMFUNC_DEFINITION_BEGIN
uint32_t fib(uint32_t n)
{
  if (n < 2)
  {
    return 1;
  }
  return (fib(n-1) + fib(n-2));
}
SL_RAMFUNC_DEFINITION_END

/**************************************************************************//**
 * @brief  Exercise the CPU and RAM in order to observe the difference
 * in current draw when core VDD is (not) scaled.
 *****************************************************************************/
SL_RAMFUNC_DEFINITION_BEGIN
int fibLoop(void)
{
  // Infinite loop
  while(1)
  {
    volatile uint32_t temp;
    for(uint32_t i = 0; i < 0x3FF; i++)
    {
      temp = fib(i);
    }
    (void)temp;
  }
}
SL_RAMFUNC_DEFINITION_END

/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  gpio_init();

  // Run the Fibonacci code to exercise the CPU and RAM subsystems.
  fibLoop();
}

/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{
}

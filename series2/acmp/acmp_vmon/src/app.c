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
#include "sl_interrupt_manager.h"
#include "sl_gpio.h"
#include "sl_hal_gpio.h"
#include "em_acmp.h"
#include "pin_config.h"

#ifndef BUTTON0_PORT
  #define BUTTON0_PORT LED0_BUTTON0_PORT
  #define BUTTON0_PIN LED0_BUTTON0_PIN
#endif

const sl_gpio_t ACMP_OUTPUT0 = { .port = ACMP_OUTPUT0_PORT, .pin = ACMP_OUTPUT0_PIN };
const sl_gpio_t ACMP_OUTPUT1 = { .port = ACMP_OUTPUT1_PORT, .pin = ACMP_OUTPUT1_PIN };
const sl_gpio_t GPIO_PB0 = { .port = BUTTON0_PORT, .pin = BUTTON0_PIN };

/**************************************************************************//**
 * @brief GPIO initialization
 *****************************************************************************/
void gpio_init(void)
{
  /*
   * Configure LED pins as outputs.
   *
   * Note that both LEDs remain off until the VMCU supply (combined
   * AVDD and IOVDD on the development board) falls below the scaled
   * comparator reference voltage.
   */
  sl_gpio_set_pin_mode(&ACMP_OUTPUT0, SL_GPIO_MODE_PUSH_PULL, LED_OFF);
  sl_gpio_set_pin_mode(&ACMP_OUTPUT1, SL_GPIO_MODE_PUSH_PULL, LED_OFF);
}

/**************************************************************************//**
 * @brief escapeHatch()
 *
 * When developing or debugging code that enters EM2 or lower, it's a
 * good idea to have an "escape hatch" type mechanism, e.g. a way to
 * pause the device so that a debugger can connect in order to erase
 * flash, among other things.
 *
 * Before proceeding with this example, make sure PB0 is not pressed.
 * If the PB0 pin is low, turn on LED0 and execute the breakpoint (BKPT)
 * instruction to stop the processor in EM0 and allow a debug
 * connection to be made.
 *****************************************************************************/
void escapeHatch(void)
{
  // Configure PB0 pin as an input pulled high
  sl_gpio_set_pin_mode(&GPIO_PB0, SL_GPIO_MODE_INPUT_PULL_FILTER, true);

  // Check for PB0 low; if so halt the CPU
  if (sl_hal_gpio_get_pin_input(&GPIO_PB0) == 0) {
    sl_gpio_set_pin_mode(&ACMP_OUTPUT0, SL_GPIO_MODE_PUSH_PULL, LED_ON);
    __BKPT(0);
  }
  // Pin not asserted; disable the PB0 digital input
  else {
    sl_gpio_set_pin_mode(&GPIO_PB0, SL_GPIO_MODE_DISABLED, false);
  }
}

/**************************************************************************//**
 * @brief ACMP initialization
 *****************************************************************************/
void acmp_init(void)
{
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_ACMP0);

  // Initialize ACMP with settings for supply monitoring
  ACMP_Init_TypeDef init =
  {
    0x2,                      // Use reduced bias for lower current
    acmpInputRangeReduced,    // Input range reduced to 0 to AVDD - 0.7 V
    acmpAccuracyHigh,         // High accuracy mode to minimize wake-ups
    acmpHysteresisDisabled,
    false,                    // Output 0 when ACMP is inactive
    0x10,                     // Scaled VREFDIV for comparison against VSENSE
    true                      // Enable after initialization
  };

  ACMP_Init(ACMP0, &init);

  /*
   * For supply monitoring, the selected reference is the negative input
   * and the desired VSENSE channel is the positive input.
   *
   * When ACMP0 is used, VSENSE0 is the AVDD supply and VSENSE1 is the
   * IOVDD supply (IOVDD0 on a device that also has an IOVDD1).  In
   * either case, the 1DIV4 designation indicates that the input voltage
   * to the comparator is divided by 4.
   *
   * In this example, the 2.5 V internal reference is multiplied by
   * 16 (0x10) / 63 (0x3F) to set a comparison threshold of 0.6349 V.
   * When AVDD = 2.54 V, VENSE0 = 2.54 V / 4 = 0.635 V.
   */
  ACMP_ChannelSet(ACMP0, acmpInputVREFDIV2V5, acmpInputVSENSE01DIV4);

  // Wait for warm-up
  while (!(ACMP0->IF & ACMP_IF_ACMPRDY));

  // Clear pending ACMP interrupts
  ACMP_IntClear(ACMP0, _ACMP_IF_MASK);
  sl_interrupt_manager_clear_irq_pending(ACMP0_IRQn);

  // Enable ACMP falling and rising interrupts
  ACMP_IntEnable(ACMP0, (ACMP_IEN_FALL | ACMP_IEN_RISE));
  sl_interrupt_manager_enable_irq(ACMP0_IRQn);
}

/**************************************************************************//**
 * @brief ACMP0 interrupt handler
 *****************************************************************************/
void ACMP0_IRQHandler(void)
{
  if ((ACMP_IntGet(ACMP0) & _ACMP_IF_FALL_MASK) == ACMP_IF_FALL) {
    // Supply below threshold
#if (LED_ON == 1)
    sl_gpio_set_pin(&ACMP_OUTPUT0);
    sl_gpio_clear_pin(&ACMP_OUTPUT1);
#else 
    sl_gpio_clear_pin(&ACMP_OUTPUT0);
    sl_gpio_set_pin(&ACMP_OUTPUT1);
#endif
  }
  else if ((ACMP_IntGet(ACMP0) & _ACMP_IF_RISE_MASK) == ACMP_IF_RISE){
    // Supply above threshold
#if (LED_ON == 1)
    sl_gpio_set_pin(&ACMP_OUTPUT1);
    sl_gpio_clear_pin(&ACMP_OUTPUT0);
#else 
    sl_gpio_clear_pin(&ACMP_OUTPUT1);
    sl_gpio_set_pin(&ACMP_OUTPUT0);
#endif
  }

  // Clear interrupt flags
  ACMP_IntClear(ACMP0, (ACMP_IF_FALL |ACMP_IF_RISE));
}

/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  // Halt if PB0 held before entering EM3 to allow debug access
  escapeHatch();

  // Initialize GPIO
  gpio_init();
  
  // Initialize ACMP
  acmp_init();
}

/***************************************************************************//**
 * App ticking function.
 *
 * In this example we are comparing AVDD (VSENSE0) input to the internal 2.5V
 * reference. When AVDD falls below threshold, LED0 lights up. If AVDD is
 * above threshold, LED1 lights up.
 *
 * Note: VREFOUT = VREFIN * (VREFDIV/63). Since VSENSE0 is divided by 4 we need
 * to also divide VREF to be comparable to VSENSE0. In this example we use
 * VREFDIV = 16 (0x10) to get the reference around 0.7V. The threshold can
 * be calculated with VREFDIV * (16/63) * 4 = 2.54V.
 ******************************************************************************/
void app_process_action(void)
{
}

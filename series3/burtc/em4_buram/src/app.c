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
#include "sl_power_manager.h"
#include "sl_iostream.h"
#include "sl_iostream_eusart_vcom_config.h"

#include "sl_gpio.h"
#include "sl_hal_burtc.h"
#include "sl_hal_emu.h"
#include "sl_hal_eusart.h"

#include "stdio.h"

#include "pin_config.h"
#include "peripheral_config.h"

// Number of 1 KHz ULFRCO clocks between BURTC interrupts
#define BURTC_IRQ_PERIOD  3000

const sl_gpio_t GPIO_LED0 = { .port = LED0_PORT, .pin = LED0_PIN };
const sl_gpio_t GPIO_PB0 = { .port = BUTTON0_PORT, .pin = BUTTON0_PIN };

/***************************************************************************//**
 * Initialize GPIO.
 ******************************************************************************/
void gpio_init(void)
{
  // Initialize GPIO driver
  sl_gpio_init();

  // Configure LED0 as push-pull with output driving LED on
  sl_gpio_set_pin_mode(&GPIO_LED0, SL_GPIO_MODE_PUSH_PULL, LED_ON);

  // Configure Push Button 0 as input with internal pullup
  sl_gpio_set_pin_mode(&GPIO_PB0, SL_GPIO_MODE_INPUT_PULL, true);
}

/***************************************************************************//**
 * BURTC interrupt triggers on compare match and toggles LED0.
 ******************************************************************************/
void BURTC_IRQHandler(void)
{
  // Clear BURTC interrupt
  sl_hal_burtc_clear_interrupts(BURTC_IF_COMP);

  // Toggle LED0
  sl_gpio_toggle_pin(&GPIO_LED0);
}

/***************************************************************************//**
 * Initialize BURTC.
 ******************************************************************************/
void burtc_init(void)
{
  // Enable BURTC and BURAM bus clocks
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_BURTC);
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_BURAM);

  // Configure BURTC to reset the counter when it reaches the compare value
  // and to wake from EM4 on a compare interrupt
  sl_hal_burtc_init_config_t init = SL_HAL_BURTC_INIT_DEFAULT;
  init.compare0_top = true;
  init.em4_comparator = true;
  sl_hal_burtc_init(&init);
  sl_hal_burtc_enable();

  // Set BURTC compare value to BURTC_IRQ_PERIOD
  sl_hal_burtc_set_compare(BURTC_IRQ_PERIOD);

  // Enable BURTC compare match interrupt
  sl_hal_burtc_enable_interrupts(BURTC_IEN_COMP);
  sl_interrupt_manager_enable_irq(BURTC_IRQn);

  // Start BURTC counting from 0
  sl_hal_burtc_reset_counter();
}

/**************************************************************************//**
 * Check RSTCAUSE for EM4 wakeups (reset) and save wakeup count to BURAM
 *****************************************************************************/
void check_reset_cause(void)
{
  // Get and clear latest reset cause
  uint32_t cause = sl_hal_emu_get_reset_cause();
  sl_hal_emu_clear_reset_cause();

  // Print reset cause
  if (cause & EMU_RSTCAUSE_PIN)
  {
    // reset EM4 wakeup counter
    BURAM->RET[0].REG = 0;
    printf("-- RSTCAUSE = PIN \r\n");
  }
  else if (cause & EMU_RSTCAUSE_EM4)
  {
    // increment EM4 wakeup counter
    BURAM->RET[0].REG += 1;
    printf("-- RSTCAUSE = EM4 wakeup \r\n");
  }

  // Print # of EM4 wakeups
  printf("-- Number of EM4 wakeups = %ld \r\n", BURAM->RET[0].REG);
  printf("-- BURTC ISR will toggle LED every ~3 seconds \r\n");
}

/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  bool pin_value = true;

  // Unlatch pins that were retained in EM4
  sl_power_manager_em4_unlatch_pin_retention();

  printf("In EM0 \r\n");

  // Initialize peripherals
  gpio_init();
  burtc_init();

  // Check RESETCAUSE, update and print EM4 wakeup count
  check_reset_cause();

  // Wait for user to press PB0
  printf("Press PB0 to enter EM4 \r\n");
  while (pin_value) {
    sl_gpio_get_pin_input(&GPIO_PB0, &pin_value);
  }
  printf("-- Button pressed \r\n");

  // Reset BURTC counter to wait full ~3 sec before EM4 wakeup
  sl_hal_burtc_reset_counter();
  printf("-- BURTC counter reset \r\n");

  // Flush iostream buffer before entering EM4
  printf("Entering EM4 and wake on BURTC compare in ~3 seconds \r\n\r\n");
  while (!(sl_hal_eusart_get_status(SL_IOSTREAM_EUSART_VCOM_PERIPHERAL) & EUSART_STATUS_TXIDLE));
  sl_power_manager_enter_em4();
}

/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{
  // This line should never be reached
}


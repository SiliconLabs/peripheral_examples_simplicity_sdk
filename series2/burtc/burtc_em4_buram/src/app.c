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
#include <stdio.h>
#include <string.h>

#include "sl_clock_manager.h"
#include "sl_gpio.h"
#include "pin_config.h"
#include "em_burtc.h"
#include "em_rmu.h"
#include "sl_main_init.h"
#include "sl_power_manager.h"
#include "sl_iostream.h"
#include "sl_iostream_init_instances.h"
#include "sl_iostream_handles.h"

// Number of 1 KHz ULFRCO clocks between BURTC interrupts (~3 seconds)
#define BURTC_IRQ_PERIOD  3000

 // Define GPIO mapping for boards where LED0 and button0 share the same pins
 #ifndef BUTTON0_PORT
   #define BUTTON0_PORT LED0_BUTTON0_PORT
   #define BUTTON0_PIN  LED0_BUTTON0_PIN
 #endif

 // Define GPIO mapping for boards where LED1 and button1 share the same pins
 #ifndef LED1_PORT
   #define LED1_PORT LED1_BUTTON1_PORT
   #define LED1_PIN  LED1_BUTTON1_PIN
 #endif

const sl_gpio_t GPIO_PB0 = { .port = BUTTON0_PORT, .pin = BUTTON0_PIN };
const sl_gpio_t GPIO_LED1 = { .port = LED1_PORT, .pin = LED1_PIN };

/***************************************************************************//**
 *  User defined weak function for performing early application initialization.
 *  This is called from sl_main_init.
 ******************************************************************************/
void app_init_early(void)
{
  // Release GPIO pin retention caused by EM4 wake-up/exit (reset).
  sl_power_manager_em4_unlatch_pin_retention();
}

/**************************************************************************//**
 * BURTC interrupt handler — toggles LED on compare match.
 *****************************************************************************/
void BURTC_IRQHandler(void)
{
  // Clear compare match interrupt
  BURTC_IntClear(BURTC_IF_COMP);

  // Toggle LED1
  sl_gpio_toggle_pin(&GPIO_LED1);
}

/**************************************************************************//**
 * Initialize GPIOs for push button and LED control.
 *****************************************************************************/
void initGPIO(void)
{
  // Initialize GPIO driver
  sl_gpio_init();

  // Configure Button0 as an input
  sl_gpio_set_pin_mode(&GPIO_PB0, SL_GPIO_MODE_INPUT, true);

  // Configure LED1 as a push pull output for LED control  and turn it on
  sl_gpio_set_pin_mode(&GPIO_LED1, SL_GPIO_MODE_PUSH_PULL, LED_ON);
}

/**************************************************************************//**
 * Configure BURTC to interrupt every BURTC_IRQ_PERIOD and wake from EM4
 *****************************************************************************/
void initBURTC(void)
{
  // Enable clocks to BURTC and BURAM
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_BURTC);
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_BURAM);

  // Configure BURTC
  BURTC_Init_TypeDef burtcInit = BURTC_INIT_DEFAULT;
  burtcInit.compare0Top = true; // Reset counter when reaching compare value
  burtcInit.em4comp = true;     // Allow BURTC compare to wake from EM4
  BURTC_Init(&burtcInit);

  BURTC_CounterReset();
  BURTC_CompareSet(0, BURTC_IRQ_PERIOD);
  BURTC_IntEnable(BURTC_IEN_COMP);
  sl_interrupt_manager_enable_irq(BURTC_IRQn);

  BURTC_Enable(true);
}

/**************************************************************************//**
 * Check RSTCAUSE, update and print EM4 wake-up count stored in BURAM.
 *****************************************************************************/
void checkResetCause (void)
{
  uint32_t resetCause = RMU_ResetCauseGet();
  RMU_ResetCauseClear();

  // Print reset cause
  if (resetCause & EMU_RSTCAUSE_PIN)
  {
    printf("-- RSTCAUSE = PIN \r\n");
    BURAM->RET[0].REG = 0; // reset EM4 wakeup counter
  }
  else if (resetCause & EMU_RSTCAUSE_EM4)
  {
    printf("-- RSTCAUSE = EM4 wakeup \r\n");
    BURAM->RET[0].REG += 1; // increment EM4 wakeup counter
  }

  // Print # of EM4 wakeups
  printf("-- Number of EM4 wakeups = %ld \r\n", BURAM->RET[0].REG);
  printf("-- BURTC ISR will toggle LED every ~3 seconds \r\n");
}

/***************************************************************************//**
 * Initialize the application and enter EM4 after user interaction.
 ******************************************************************************/
void app_init(void)
{
  bool pinValue;

  initGPIO();
  initBURTC();
  
  // Check RESETCAUSE, update and print EM4 wakeup count
  checkResetCause();
  
  // Wait for user to press PB0, reset BURTC counter
  printf("Press PB0 to enter EM4 \r\n");
  do {
    sl_gpio_get_pin_input(&GPIO_PB0, &pinValue);
  } while (pinValue == 1);

  printf("-- Button pressed \r\n");

  // Restart BURTC counter to wait full ~3 sec before entering EM4
  BURTC_CounterReset();
  printf("-- BURTC counter reset \r\n");

  // Enter EM4
  printf("Entering EM4 and wake on BURTC compare in ~3 seconds \r\n\r\n");

  // Add a 5 ms delay to ensure UART has finished transmitting
  sl_sleeptimer_delay_millisecond(5);

  sl_power_manager_enter_em4();
}

/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{
}

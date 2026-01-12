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
#include "sl_hal_gpio.h"
#include "em_msc.h"
#include "pin_config.h"

// Define GPIO mapping for boards where LED0 and button0 share the same pins
#ifndef LED0_PORT
  #define LED0_PORT LED0_BUTTON0_PORT
  #define LED0_PIN LED0_BUTTON0_PIN
#endif

const sl_gpio_t GPIO_LED0 = { .port = LED0_PORT, .pin = LED0_PIN };

/***************************************************************************//**
 *  User defined weak function for performing early application initialization.
 *  This is called from sl_main_init.
 ******************************************************************************/
void app_init_early(void)
{
  /*
   * Lock the specified main flash page as early as possible.
   * Ideal for critical early-stage configuration.
   *
   * NOTE: In the future, the plan is to move this code to a 
   * generalized SystemInit2() hook once it becomes available.
   */
#if !defined(_SILICON_LABS_32B_SERIES_2_CONFIG_1)
  CMU->CLKEN1_SET = CMU_CLKEN1_MSC;
#endif

  MSC->PAGELOCKn = LASTLOCK;

  /*
   * Pulse the LED0 pin high to measure the timing of this code
   * relative to other early register writes.
   */
#if (LED0_PORT > 7)
  GPIO->P_SET[LED0_PORT].MODEH = _GPIO_P_MODEL_MODE0_PUSHPULL << (4 * LED0_PIN);
#else
  GPIO->P_SET[LED0_PORT].MODEL = _GPIO_P_MODEL_MODE0_PUSHPULL << (4 * LED0_PIN);
#endif

#if LED_ON
    // Turn on LED (active high)
    GPIO->P_SET[LED0_PORT].DOUT = 1 << LED0_PIN;
#else
    // Turn on LED (active low)
    GPIO->P_CLR[LED0_PORT].DOUT = 1 << LED0_PIN;
#endif
}

/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  volatile MSC_Status_TypeDef flashStatus;

  // Lock user data page by writing the lock bit
  MSC->MISCLOCKWORD_SET = MSC_MISCLOCKWORD_UDLOCKBIT;

  /*
   * Pull LED0 low to mark the moment of writing the lock word.
   * Useful for debugging with an oscilloscope.
   */
  #if LED_ON
      // Turn off LED (active high)
      GPIO->P_CLR[LED0_PORT].DOUT = 1 << LED0_PIN;
  #else
      // Turn off LED (active low)
      GPIO->P_SET[LED0_PORT].DOUT = 1 << LED0_PIN;
  #endif

  // Initialize the MSC to prepare for flash write/erase operations
  MSC_Init();

  // Attempt to erase the last page of main flash
  flashStatus = MSC_ErasePage((uint32_t*)(FLASH_BASE + FLASH_SIZE - FLASH_PAGE_SIZE));

  /*
   * Halt on any flash operation status other than mscReturnLocked.
   * The expectation is that the erase should fail because the page
   * in question was locked.
   */
  switch (flashStatus)
  {
    case mscReturnOk:
    case mscReturnInvalidAddr:
    case mscReturnTimeOut:
    case mscReturnUnaligned:    __BKPT(0);
         break;
    case mscReturnLocked:
         break;
  }

  // Attempt to erase the user data page
  flashStatus = MSC_ErasePage((uint32_t*)USERDATA_BASE);

  /*
   * Halt on any flash operation status other than mscReturnLocked.
   * The expectation is that the erase should fail because the user
   * data page was locked by the write to MSC_MISCLOCKWORD above.
   */
  switch (flashStatus)
  {
    case mscReturnOk:
    case mscReturnInvalidAddr:
    case mscReturnTimeOut:
    case mscReturnUnaligned:    __BKPT(1);
         break;
    case mscReturnLocked:
         break;
  }
}

/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{
  // If both pages are locked, code will loop here
}

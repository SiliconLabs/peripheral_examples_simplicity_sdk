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
#include "em_cmu.h"
#include "em_burtc.h"
#include "em_rmu.h"
#include "em_burtc.h"
#include "sl_clock_manager.h"
#include "sl_gpio.h"
#include "pin_config.h"

#define BURTC_LFXO_EN (0)

#ifndef BUTTON0_PORT
  #define BUTTON0_PORT LED0_BUTTON0_PORT
  #define BUTTON0_PIN LED0_BUTTON0_PIN
#endif

#ifndef LED1_PORT
  #define LED1_PORT LED1_BUTTON1_PORT
  #define LED1_PIN LED1_BUTTON1_PIN
#endif

const sl_gpio_t BUTTON0 =  { .port = BUTTON0_PORT, .pin = BUTTON0_PIN };
const sl_gpio_t LED1 = { .port = LED1_PORT, .pin = LED1_PIN };

/***************************************************************************//**
 * @brief
 *   Enter EM4 with BURTC running on a LFXO
 *
 * @details
 *   Parameter:
 *     EM4H. Hibernate Mode.@n
 *   Condition:
 *     BURTC, 128 byte RAM, 32.768 kHz LFXO.@n
 *
 ******************************************************************************/
void em_EM4_LfxoBURTC(void)
{
  CMU_LFXOInit_TypeDef lfxoInit = CMU_LFXOINIT_DEFAULT;
  CMU_LFXOInit(&lfxoInit);

  // Select LFXO as the BURTC clock source.
  CMU_ClockSelectSet(cmuClock_EM4GRPACLK, cmuSelect_LFXO);

  // Setup BURTC.
  BURTC_Init_TypeDef burtcInit = BURTC_INIT_DEFAULT;
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_BURTC);
  BURTC_Init(&burtcInit);

  // Enter EM4
  EMU_EM4Init_TypeDef em4Init = EMU_EM4INIT_DEFAULT;
  em4Init.retainLfxo = true;
  em4Init.pinRetentionMode = emuPinRetentionLatch;
  EMU_EM4Init(&em4Init);
  EMU_EnterEM4();
}

/***************************************************************************//**
 * @brief
 *   Enter EM4 without BURTC.
 *
 * @details
 *   Parameter:
 *     EM4.@n
 *   Condition:
 *     No BURTC, 128 byte BURAM.@n
 *
 ******************************************************************************/
void em_EM4(void)
{
  // Enter EM4
  EMU_EM4Init_TypeDef em4Init = EMU_EM4INIT_DEFAULT;
  em4Init.pinRetentionMode = emuPinRetentionLatch;
  EMU_EM4Init(&em4Init);
  EMU_EnterEM4();
}

/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  uint32_t cause;
  cause = RMU_ResetCauseGet();
  RMU_ResetCauseClear();
  while ((cause & EMU_RSTCAUSE_EM4)) {
  }

  /*
   * When developing or debugging code that enters EM2 or
   *  lower, it's a good idea to have an "escape hatch" type
   * mechanism, e.g. a way to pause the device so that a debugger can
   * connect in order to erase flash, among other things.
   *
   * Before proceeding with this example, make sure PB0 is not pressed.
   * If the PB0 pin is low, turn on LED0 and execute the breakpoint
   * instruction to stop the processor in EM0 and allow a debug
   * connection to be made.
   */
  bool buttonIn;
  sl_gpio_set_pin_mode(&BUTTON0, SL_GPIO_MODE_INPUT_PULL_FILTER, true);
  sl_gpio_get_pin_input(&BUTTON0, &buttonIn);

  if (buttonIn == 0) {
    sl_gpio_set_pin_mode(&LED1, SL_GPIO_MODE_PUSH_PULL, LED_ON);
    __BKPT(0);
  }
  else {
    // Pin not asserted, so disable input
    sl_gpio_set_pin_mode(&BUTTON0, SL_GPIO_MODE_DISABLED, true);
  }

  // Enter EM4
  if(BURTC_LFXO_EN){
      em_EM4_LfxoBURTC();
  }
  else{
      em_EM4();
  }

}

/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{
}

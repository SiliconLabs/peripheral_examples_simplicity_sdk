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
#include "pin_config.h"
#include "sl_interrupt_manager.h"
#include "sl_gpio.h"

// test mode control signals
volatile bool low_voltage = false;

#ifndef LED1_PORT
  #define LED1_PORT LED1_BUTTON1_PORT
  #define LED1_PIN LED1_BUTTON1_PIN
#endif

const sl_gpio_t LED1 = { .port = LED1_PORT, .pin = LED1_PIN };

/**************************************************************************//**
 * @brief  GPIO Initializer
 *****************************************************************************/
void gpio_init(void)
{
  // Configure LED1 pin as an output
  sl_gpio_set_pin_mode(&LED1, SL_GPIO_MODE_PUSH_PULL, LED_OFF);
}

/*****************************************************************************/
/*@brief: DCDC interrupt handler
 *****************************************************************************/
void DCDC_IRQHandler(void)
{
  volatile uint32_t flag = DCDC -> IF;

  // Clear all interrupt flag
  DCDC -> IF_CLR = flag;

  // Check for interrupt flag
  if (flag & DCDC_IF_VREGINLOW) {
    low_voltage = true;
    DCDC -> IEN_CLR |= DCDC_IEN_VREGINLOW;
  }
  if (flag & DCDC_IF_VREGINHIGH) {
    low_voltage = false;
    DCDC -> IEN_CLR |= DCDC_IEN_VREGINHIGH;
  }
}

/*****************************************************************************/
/*@brief DCDC Initialization
 *****************************************************************************/
void dcdc_init(void)
{
  EMU_DCDCInit_TypeDef dcdcInit = EMU_DCDCINIT_DEFAULT;

  // Change dcdc threshold voltage level to 2.2V
  dcdcInit.cmpThreshold = emuVreginCmpThreshold_2v2;

  // Initializing DCDC with regulator on
  EMU_DCDCInit(&dcdcInit);

  // Clear all DCDC interrupt
  DCDC->IF_CLR |= _DCDC_IF_MASK;

  // Enable VREGIN low detection interrupt
  DCDC->IEN_SET |= DCDC_IEN_VREGINLOW;

  // Enable DCDC interrupt vector.
  sl_interrupt_manager_clear_irq_pending(DCDC_IRQn);
  sl_interrupt_manager_enable_irq(DCDC_IRQn);
}

/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  // Initialize GPIO
  gpio_init();

  // Initialize DCDC
  dcdc_init();
}

/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{
  // If below threshold, switch to bypass mode, turn on LED
  // Enable interrupt flags to detect VREGIN > VTHRESHOLD
  if (low_voltage) {
    EMU_DCDCModeSet(emuDcdcMode_Bypass);
    sl_gpio_set_pin(&LED1);
    DCDC -> IEN_SET |= DCDC_IEN_VREGINHIGH;
  }
  else {
  // If above threshold, switch to regulator on mode, turn off LED
  // Enable interrupt flags to detect VREGIN < VTHRESHOLD
    EMU_DCDCModeSet(emuDcdcMode_Regulation);
    sl_gpio_clear_pin(&LED1);
    DCDC -> IEN_SET |= DCDC_IEN_VREGINLOW;
  }
}

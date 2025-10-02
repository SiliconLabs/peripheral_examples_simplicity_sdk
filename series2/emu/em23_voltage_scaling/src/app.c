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
#include "em_burtc.h"
#include "em_emu.h"
#include "pin_config.h"
#include "sl_interrupt_manager.h"
#include "sl_clock_manager.h"
#include "sl_gpio.h"
#include "sl_power_manager.h"

// Number of 1 kHz ULFRCO clocks between BURTC interrupts
#define BURTC_IRQ_PERIOD 4000

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

void burtc_init()
{
  // Setup BURTC.
  BURTC_Init_TypeDef burtcInit = BURTC_INIT_DEFAULT;
  burtcInit.compare0Top = true;   // Reset counter at compare value
  burtcInit.start = false;        // Don't start counting yet
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_BURTC);
  BURTC_Init(&burtcInit);

  // Interrupt on compare match
  BURTC_IntEnable(BURTC_IEN_COMP);
  sl_interrupt_manager_enable_irq(BURTC_IRQn);

  // Set the compare value and start the counter.
  BURTC_CompareSet(0, BURTC_IRQ_PERIOD);
  BURTC_Start();
}

/**************************************************************************/
/**
 * @brief  BURTC Handler
 *****************************************************************************/
void BURTC_IRQHandler(void)
{
  uint32_t vscale;
  EMU_EM23Init_TypeDef vsInit = EMU_EM23INIT_DEFAULT;

  // Get the current EM2/3 voltage scaling
  vscale = EMU->CTRL & _EMU_CTRL_EM23VSCALE_MASK;
  if (vscale == EMU_CTRL_EM23VSCALE_VSCALE2) {
    // Currently running at VS2 (high performance), so scale down
    vsInit.vScaleEM23Voltage = emuVScaleEM23_LowPower;
  } else {
    // Currently running at VS0 (low power), so scale up
    vsInit.vScaleEM23Voltage = emuVScaleEM23_FastWakeup;
  }

  // Switch to the new EM2/3 voltage scaling setting
  EMU_EM23Init(&vsInit);
  BURTC_IntClear(BURTC_IF_COMP);
}


/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
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

  burtc_init();

  // Power down all eligible RAM blocks
  EMU_RamPowerDown(SRAM_BASE, RAM_POWER_DOWN_END);
}

/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{
}

/***************************************************************************//**
 * @file app.c
 * @brief This project demonstrates use of the RTCC to wake from EM2/3. See
 * readme.txt for details.
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 *******************************************************************************
 * # Evaluation Quality
 * This code has been minimally tested to ensure that it builds and is suitable 
 * as a demonstration for evaluation purposes only. This code will be maintained
 * at the sole discretion of Silicon Labs.
 ******************************************************************************/

#include "em_cmu.h"
#include "em_emu.h"
#include "em_prs.h"
#include "em_rtcc.h"

#include "sl_gpio.h"
#include "sl_clock_manager.h"
#include "sl_interrupt_manager.h"
#include "sl_power_manager.h"

#include "pin_config.h"

/**************************************************************************//**
 * The defines below select the RTCC clock source, the PRS output channel and
 * the wake up interval from EM2/3.
 * If the RTCC Clock source is defined as LFXO, the device will go into EM2. To
 * test the example in EM3, the RTCC Clock source must be changed to
 * cmuSelect_ULFRCO.
 *****************************************************************************/

#define RTCC_CLOCK              cmuSelect_LFXO    // RTCC clock source
#define RTCC_PRS_CH             0                 // RTCC PRS output channel
#define WAKEUP_INTERVAL_MS      500               // Wake up interval in ms

const sl_gpio_t PRS_OUTPUT = { .port = SL_GPIO_PORT_B, .pin = 0};

/**************************************************************************//**
 * @brief
 *   RTCC Interrupt Handler, clears the flag.
 *****************************************************************************/
void RTCC_IRQHandler(void)
{
  // Clear interrupt source CC1
  RTCC_IntClear(RTCC_IF_CC1);
}

/**************************************************************************//**
 * @brief  
 *   Setup RTCC with selected clock source. The clock source can be modified by
 *   changing the definition of RTCC_CLOCK.
 * @param[in] rtccClock
 *   Select clock source, valid values are cmuSelect_LFRCO, cmuSelect_LFXO, and
 *   cmuSelect_ULFRCO.
 *****************************************************************************/
void setupRtcc()
{
  // Configure the RTCC with default parameters
  RTCC_Init_TypeDef rtccInit = RTCC_INIT_DEFAULT;
  RTCC_CCChConf_TypeDef rtccInitCompareChannel = RTCC_CH_INIT_COMPARE_DEFAULT;

  // Setting RTCC clock source
  CMU_ClockSelectSet(cmuClock_RTCCCLK, RTCC_CLOCK);

  // Enable RTCC bus clock
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_RTCC);

  // Initialize the RTCC
  rtccInit.cntWrapOnCCV1 = true;        // Clear counter on CC1 compare match
  rtccInit.presc = rtccCntPresc_1;      // Prescaler 1

  // Initialize and start counting
  RTCC_Init(&rtccInit);

  // Initialize Capture Compare Channel 1 to toggle PRS output on compare match
  rtccInitCompareChannel.compMatchOutAction = rtccCompMatchOutActionToggle;
  RTCC_ChannelInit(1, &rtccInitCompareChannel);

  /**********************************************************************//**
   * Set RTCC Capture Compare Value to the frequency of the
   * LFXO/ULFRCO(minus 1).
   *
   * When the pre-counter (RTCC_PRECNT) matches this value, the counter
   * (RTCC_CNT) increments by one.
   *************************************************************************/
  if (RTCC_CLOCK == cmuSelect_ULFRCO) {
    //Setting the CC1 compare value of the RTCC
    RTCC_ChannelCCVSet(1, (1000 * WAKEUP_INTERVAL_MS) / 1000 - 1);
  } else {
    //Setting the CC1 compare value of the RTCC
    RTCC_ChannelCCVSet(1, (32768 * WAKEUP_INTERVAL_MS) / 1000 - 1);
  }

  // Enabling Interrupt from RTCC CC1
  RTCC_IntEnable(RTCC_IEN_CC1);
  sl_interrupt_manager_clear_irq_pending(RTCC_IRQn);
  sl_interrupt_manager_enable_irq(RTCC_IRQn);
}

/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{

  // Enable peripheral clock
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_PRS);
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_GPIO);

  // Setting up RTCC
  setupRtcc();

  /**********************************************************************//**
   * RTCC CC1 PRS output on LED1. While all GPIO pins retain their state in
   * EM2, only pins on port A and B remain fully functional in EM2/3.
   *
   * In this example, pin PB0 is used as the output of the PRS channel. This
   * pin is routed to the expansion header on the WSTK (EXP Pin 7).
   *************************************************************************/
  sl_gpio_set_pin_mode(&PRS_OUTPUT, SL_GPIO_MODE_PUSH_PULL, 1);

  PRS_SourceAsyncSignalSet(RTCC_PRS_CH, PRS_ASYNC_CH_CTRL_SOURCESEL_RTCC,
                           PRS_ASYNC_CH_CTRL_SIGSEL_RTCCCCV1);
  PRS_PinOutput(RTCC_PRS_CH, prsTypeAsync, SL_GPIO_PORT_B, 0);

  /**********************************************************************//**
   * Power down first 24 KB to reduce current in EM2/3.
   *
   * NOTE: The top 8 KB of RAM (BLK1) **MUST** remain powered in EM2/3
   * (SYSCFG_DMEM0RETNCTRL_RAMRETNCTRL != 2) or the device is liable to
   * hard fault on wake-up depending on what data may have been saved
   * on the stack, e.g. the return address from the EMU_EnterEM3() call
   * below.
   *************************************************************************/
  EMU_RamPowerDown(SRAM_BASE, SRAM_BASE + SRAM_SIZE - 0x2000);

}

/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{

}

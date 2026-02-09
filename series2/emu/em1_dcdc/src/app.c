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

#include "em_emu.h"

/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  /* In FG25 device, EMU handles Voltage Scaling automatically,
   * so this initialization is not possible.
   */
  #ifndef EFR32FG25B222F1920IM56
    // Enable voltage downscaling in EM01 energy modes
    EMU_EM01Init_TypeDef em01Init = EMU_EM01INIT_DEFAULT;
    em01Init.vScaleEM01LowPowerVoltageEnable = true;

    // Initialize EM01 energy modes
    EMU_EM01Init(&em01Init);
  #endif // EFR32FG25B222F1920IM56
}

/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{
}

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

#include "sl_clock_manager.h"
#include "sl_interrupt_manager.h"
#include "sl_gpio.h"
#include "sl_hal_gpio.h"
#include "sl_se_manager_util.h"

// Pointer to the base of the User Data page in flash
#define USERDATA ((uint32_t*)USERDATA_BASE)

uint32_t Cleared_value;  // Holds the flash word after erase
uint32_t Set_value;      // Holds the flash word after writing a value

sl_se_command_context_t cmd_ctx;

/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  // Declare the value to be stored in flash
  uint32_t value = 32;

  // Clear the UserData page of any previous data stored
  sl_se_erase_user_data(&cmd_ctx);

  // Read the initial value in the cleared page
  Cleared_value = USERDATA[3];

  // Write the value into the 4th word of the Userdata portion of the flash
  sl_se_write_user_data(&cmd_ctx, 3*4, &value, 4);

  // Read the written data from the flash location it was stored in
  Set_value = USERDATA[3];
}

/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{
}

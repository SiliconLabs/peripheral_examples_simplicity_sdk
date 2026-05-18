/***************************************************************************//**
 * @file app.c
 * @brief This example demonstrates the LDMA descriptor list with looping. See
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

#include <stddef.h>
#include "dmadrv.h"
#include "sl_gpio.h"
#include "pin_config.h"
#include "peripheral_config.h"

// DMA channel used for the example
#define LDMA_CHANNEL        0
#define LDMA_CH_MASK        (1 << LDMA_CHANNEL)

// Memory to memory transfer buffer size
#define BUFFER_SIZE         4

// Descriptor linked list size
#define LIST_SIZE           3

// Number of iterations of A and B.
#define NUM_ITERATIONS      4

// Constant for loop transfer
// NUM_ITERATIONS - 1 (for first iteration)
#define LOOP_COUNT          NUM_ITERATIONS - 1

// Define GPIO mapping for boards where LED0 and BUTTON0 share the same pins
#ifndef LED0_PORT
  #define LED0_PORT LED0_BUTTON0_PORT
  #define LED0_PIN  LED0_BUTTON0_PIN
#endif

// Descriptor linked list for LDMA transfer
sl_hal_ldma_descriptor_t descriptor_link[LIST_SIZE];

// Buffers for memory to memory transfer
uint8_t dest_buffer[BUFFER_SIZE];

uint8_t source_a[BUFFER_SIZE] = "AAaa";
uint8_t source_b[BUFFER_SIZE] = "BBbb";
uint8_t source_c[BUFFER_SIZE] = "CCcc";

const sl_gpio_t GPIO_LED0 = { .port = LED0_PORT, .pin = LED0_PIN };

/***************************************************************************//**
 * @brief
 *   LDMA Callback Function
 ******************************************************************************/
void ldma_callback(void)
{
  // Toggle GPIO to notify that transfer is complete
  sl_gpio_toggle_pin(&GPIO_LED0);

  // Request next transfer
  LDMA0->SWREQ |= LDMA_CH_MASK;
}

/***************************************************************************//**
 * @brief
 *   Setup LED0 as indicator for complete transfer
 ******************************************************************************/
void init_gpio(void)
{
  // Initialize GPIO
  sl_gpio_init();

  // Show completion state on LED0
  sl_gpio_set_pin_mode(&GPIO_LED0, SL_GPIO_MODE_PUSH_PULL, LED_OFF);
}

/***************************************************************************//**
 * @brief
 *   Initialize the LDMA controller for descriptor list with looping
 ******************************************************************************/
void init_ldma(void)
{
  unsigned int channel_id = LDMA_CHANNEL;
  bool active;
  uint32_t i;

  // Initialize buffers for memory transfer
  for (i = 0; i < BUFFER_SIZE; i++)
  {
    dest_buffer[i] = 0;
  }

  // Use looped memory transfer configuration macro
  sl_hal_ldma_transfer_config_t transfer_tx =
      SL_HAL_LDMA_TRANSFER_CFG_MEMORY_LOOP(LOOP_COUNT);

  // LINK descriptor macros for looping
  descriptor_link[0] = (sl_hal_ldma_descriptor_t)
      SL_HAL_LDMA_DESCRIPTOR_LINKREL_M2M(SL_HAL_LDMA_CTRL_SIZE_BYTE,
                                         &source_a,
                                         &dest_buffer,
                                         BUFFER_SIZE,
                                         1);
  descriptor_link[1] = (sl_hal_ldma_descriptor_t)
      SL_HAL_LDMA_DESCRIPTOR_LINKREL_M2M(SL_HAL_LDMA_CTRL_SIZE_BYTE,
                                         &source_b,
                                         &dest_buffer,
                                         BUFFER_SIZE,
                                         -1);

  // SINGLE descriptor macro for single transfer
  descriptor_link[2] = (sl_hal_ldma_descriptor_t)
      SL_HAL_LDMA_DESCRIPTOR_SINGLE_M2M(SL_HAL_LDMA_CTRL_SIZE_BYTE,
                                        &source_c,
                                        &dest_buffer,
                                        BUFFER_SIZE);

  // Enable looping
  descriptor_link[1].xfer.dec_loop_count = 1;

  // Enable interrupts
  descriptor_link[0].xfer.done_ifs = true;
  descriptor_link[1].xfer.done_ifs = true;
  descriptor_link[2].xfer.done_ifs = true;

  // Disable automatic triggers
  descriptor_link[0].xfer.struct_req = false;
  descriptor_link[1].xfer.struct_req = false;
  descriptor_link[2].xfer.struct_req = false;

  // Initialize DMADRV
  DMADRV_Init();

  // Halt on error if DMA channel cannot be allocated
  if (DMADRV_AllocateChannel(&channel_id, NULL) != ECODE_EMDRV_DMADRV_OK) {
    __BKPT(0);
  }

  // Check that IADC->memory LDMA channel is not currently active; halt if error
  if (DMADRV_TransferActive(channel_id, &active) != ECODE_EMDRV_DMADRV_OK) {
    __BKPT(0);
  }

  // Start LDMA transfer
  if (!active) {
    DMADRV_LdmaStartTransfer(channel_id,
                             &transfer_tx,
                             descriptor_link,
                             (DMADRV_Callback_t)ldma_callback,
                             NULL);
  }

  // Request first transfer
  LDMA0->SWREQ |= LDMA_CH_MASK;
}

/*******************************************************************************
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  // Initialize LED
  init_gpio();

  // Initialize LDMA
  init_ldma();
}

/*******************************************************************************
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{

}

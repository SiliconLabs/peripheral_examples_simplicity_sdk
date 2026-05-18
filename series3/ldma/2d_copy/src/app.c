/***************************************************************************//**
 * @file app.c
 * @brief This example demonstrates the LDMA 2D copy. See readme.txt for details
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
#include "sl_hal_ldma.h"
#include "pin_config.h"
#include "peripheral_config.h"

// DMA channel used for the example
#define LDMA_CHANNEL        0
#define LDMA_CH_MASK        (1 << LDMA_CHANNEL)

// 2D buffer size and constants for 2D copy
#define BUFFER_2D_WIDTH     10
#define BUFFER_2D_HEIGHT    8
#define TRANSFER_WIDTH      3
#define TRANSFER_HEIGHT     4
#define SRC_ROW_INDEX       1
#define SRC_COL_INDEX       0
#define DST_ROW_INDEX       1
#define DST_COL_INDEX       2

// Define GPIO mapping for boards where LED0 and BUTTON0 share the same pins
#ifndef LED0_PORT
  #define LED0_PORT LED0_BUTTON0_PORT
  #define LED0_PIN  LED0_BUTTON0_PIN
#endif

// Descriptor linked list for LDMA transfer
sl_hal_ldma_descriptor_t descriptor_link[2];

// Buffers for 2D copy transfer
uint16_t src2d[BUFFER_2D_HEIGHT][BUFFER_2D_WIDTH];
uint16_t dst2d[BUFFER_2D_HEIGHT][BUFFER_2D_WIDTH];

const sl_gpio_t GPIO_LED0 = { .port = LED0_PORT, .pin = LED0_PIN };

/***************************************************************************//**
 * @brief
 *   LDMA Callback Function
 ******************************************************************************/
void ldma_callback(void)
{
  // Toggle GPIO to notify that transfer is complete
  sl_gpio_toggle_pin(&GPIO_LED0);
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
 *   Initialize the LDMA controller for 2D copy
 ******************************************************************************/
void init_ldma(void)
{
  unsigned int channel_id = LDMA_CHANNEL;
  bool active;
  uint32_t x, y;

  // Initialize buffers for 2D copy
  for (x = 0; x < BUFFER_2D_HEIGHT; x++)
  {
    for (y = 0; y < BUFFER_2D_WIDTH; y++)
    {
      src2d[x][y] = x*BUFFER_2D_WIDTH + y;
      dst2d[x][y] = 0;
    }
  }

  // Use looped memory transfer configuration macro
  sl_hal_ldma_transfer_init_t mem_transfer = SL_HAL_LDMA_TRANSFER_CFG_MEMORY_LOOP(TRANSFER_HEIGHT-2);

  // First descriptor gets the absolute source and destination address
  descriptor_link[0] = (sl_hal_ldma_descriptor_t)SL_HAL_LDMA_DESCRIPTOR_LINKREL_M2M(SL_HAL_LDMA_CTRL_SIZE_HALF,
                                                                                    &src2d[SRC_ROW_INDEX][SRC_COL_INDEX],
                                                                                    &dst2d[DST_ROW_INDEX][DST_COL_INDEX],
                                                                                    TRANSFER_WIDTH,
                                                                                    1);

  // Second descriptor uses relative addressing and performs looping
  descriptor_link[1] = (sl_hal_ldma_descriptor_t)SL_HAL_LDMA_DESCRIPTOR_LINKREL_M2M(SL_HAL_LDMA_CTRL_SIZE_HALF,
                                                                                   (BUFFER_2D_WIDTH - TRANSFER_WIDTH)*2,
                                                                                   (BUFFER_2D_WIDTH - TRANSFER_WIDTH)*2,
                                                                                   TRANSFER_WIDTH,
                                                                                   0);

  // Use relative addressing for source & destination, enable looping
  descriptor_link[1].xfer.src_addr_mode = SL_HAL_LDMA_CTRL_SRC_ADDR_MODE_REL;
  descriptor_link[1].xfer.dst_addr_mode = SL_HAL_LDMA_CTRL_DST_ADDR_MODE_REL;;
  descriptor_link[1].xfer.dec_loop_count = 1;

  // Stop after looping
  descriptor_link[1].xfer.link = 0;

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
                             &mem_transfer,
                             descriptor_link,
                             (DMADRV_Callback_t)ldma_callback,
                             NULL);
  }
}

/*******************************************************************************
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  // Initialize LED0
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

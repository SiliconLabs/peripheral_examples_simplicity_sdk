/***************************************************************************//**
 * @file app.c
 * @brief This example demonstrates the LDMA descriptor linked list. See
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

// DMA channel used for the example
#define LDMA_CHANNEL      0
#define LDMA_CH_MASK      (1 << LDMA_CHANNEL)

// Memory to memory transfer buffer size
#define BUFFER_SIZE       32

// Descriptor linked list size
#define LIST_SIZE         4

// Define GPIO mapping for boards where LED0 and BUTTON0 share the same pins
#ifndef LED0_PORT
  #define LED0_PORT LED0_BUTTON0_PORT
  #define LED0_PIN  LED0_BUTTON0_PIN
#endif

// Descriptor linked list for LDMA transfer
LDMA_Descriptor_t descLink[LIST_SIZE];

// Buffers for linked transfer
uint16_t srcBuffer[LIST_SIZE][BUFFER_SIZE];
uint16_t dstBuffer[LIST_SIZE][BUFFER_SIZE];

const sl_gpio_t GPIO_LED0 = {.port = LED0_PORT, .pin = LED0_PIN};

/***************************************************************************//**
 * @brief
 *   LDMA Callback Function
 ******************************************************************************/
void ldmaCallback(void)
{
  // Toggle GPIO to notify that transfer is complete
  sl_gpio_toggle_pin(&GPIO_LED0);

  // Request next transfer, this transfers the remaining two descriptor
  LDMA->SWREQ |= LDMA_CH_MASK;
}

/***************************************************************************//**
 * @brief
 *   Setup LED0 as indicator for complete transfer
 ******************************************************************************/
void initGPIO(void)
{
  // Initialize GPIO
  sl_gpio_init();

  // Show completion state on LED0
  sl_gpio_set_pin_mode(&GPIO_LED0, SL_GPIO_MODE_PUSH_PULL, LED_OFF);
}

/***************************************************************************//**
 * @brief
 *   Initialize the LDMA controller for descriptor linked list
 ******************************************************************************/
void initLdma(void)
{
  unsigned int channelId = LDMA_CHANNEL;
  bool active;
  uint32_t i;

  // Fill buffers
  for (i = 0; i < BUFFER_SIZE * 4; i++)
  {
    srcBuffer[i / BUFFER_SIZE][i % BUFFER_SIZE] = i;
    dstBuffer[i / BUFFER_SIZE][i % BUFFER_SIZE] = 0;
  }

  // Use memory transfer configuration macro
  LDMA_TransferCfg_t periTransferTx = LDMA_TRANSFER_CFG_MEMORY();

  // LINK descriptor macros to form linked list
  for (i = 0; i < LIST_SIZE - 1; i++)
  {
    descLink[i] = (LDMA_Descriptor_t)LDMA_DESCRIPTOR_LINKREL_M2M_HALF(&srcBuffer[i],
                                                                      &dstBuffer[i],
                                                                      BUFFER_SIZE, 1);
  }

  // SINGLE descriptor macro for the last one
  descLink[LIST_SIZE - 1] = (LDMA_Descriptor_t)LDMA_DESCRIPTOR_SINGLE_M2M_HALF(&srcBuffer[LIST_SIZE - 1],
                                                                               &dstBuffer[LIST_SIZE - 1],
                                                                               BUFFER_SIZE);

  // Turn on Done interrupts for Descriptor 2
  descLink[1].xfer.doneIfs = true;
  // Disable automatic triggers for Descriptor 3, wait for software trigger
  descLink[2].xfer.structReq = false;

  // Initialize DMADRV
  DMADRV_Init();

  // Halt on error if DMA channel cannot be allocated
  if (DMADRV_AllocateChannel(&channelId, NULL) != ECODE_EMDRV_DMADRV_OK) {
    __BKPT(0);
  }

  // Check that IADC->memory LDMA channel is not currently active; halt if error
  if (DMADRV_TransferActive(channelId, &active) != ECODE_EMDRV_DMADRV_OK) {
    __BKPT(0);
  }

  // Start LDMA transfer
  if (!active) {
    DMADRV_LdmaStartTransfer(channelId,
                             &periTransferTx,
                             descLink,
                             (DMADRV_Callback_t)ldmaCallback,
                             NULL);
  }
}

/*******************************************************************************
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  // Initialize LED
  initGPIO();

  // Initialize LDMA
  initLdma();
}

/*******************************************************************************
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{

}

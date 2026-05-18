/***************************************************************************//**
 * @file app.c
 * @brief This example demonstrates the LDMA single descriptor looped transfer.
 * See readme.txt for details.
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
#define LDMA_CHANNEL      0
#define LDMA_CH_MASK      (1 << LDMA_CHANNEL)

// Memory to memory transfer buffer size
#define BUFFER_SIZE       4

// Number of sets of BUFFER_SIZE elements to send.
// Total words sent = BUFFER_SIZE * NUM_ITERATIONS
#define NUM_ITERATIONS    4

// Constant for loop transfer
// NUM_ITERATIONS - 1 (for first descriptor) - 1 (for first iteration)
#define LOOP_COUNT        NUM_ITERATIONS - 1

// Define GPIO mapping for boards where LED0 and BUTTON0 share the same pins
#ifndef LED0_PORT
  #define LED0_PORT LED0_BUTTON0_PORT
  #define LED0_PIN  LED0_BUTTON0_PIN
#endif

// Descriptor linked list for LDMA transfer
LDMA_Descriptor_t descLink;

// Buffers for memory to memory transfer
uint32_t srcBuffer[BUFFER_SIZE];
uint32_t dstBuffer[NUM_ITERATIONS][BUFFER_SIZE];

const sl_gpio_t GPIO_LED0 = { .port = LED0_PORT, .pin = LED0_PIN };

/***************************************************************************//**
 * @brief
 *   LDMA Callback Function
 ******************************************************************************/
void ldmaCallback(void)
{
  // Toggle GPIO to notify that transfer is complete
  sl_gpio_toggle_pin(&GPIO_LED0);

  // Start next Transfer
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
 *   Initialize the LDMA controller for single descriptor looped transfer
 ******************************************************************************/
void initLdma(void)
{
  unsigned int channelId = LDMA_CHANNEL;
  bool active;
  uint32_t i, j;

  // Initialize buffers for memory transfer
  for (i = 0; i < BUFFER_SIZE; i++)
  {
    srcBuffer[i] = i;
    for (j = 0; j < NUM_ITERATIONS; j++)
    {
      dstBuffer[j][i] = 0;
    }
  }

  // Use looped memory transfer configuration macro
  LDMA_TransferCfg_t periTransferTx = LDMA_TRANSFER_CFG_MEMORY_LOOP(LOOP_COUNT);

  // Use LINK descriptor macro for initialization and looping
  descLink = (LDMA_Descriptor_t)LDMA_DESCRIPTOR_LINKREL_M2M_WORD(&srcBuffer,
                                                                 0,
                                                                 BUFFER_SIZE,
                                                                 0);

  descLink.xfer.blockSize   = ldmaCtrlBlockSizeUnit4;   // Set block sizes to 4
  descLink.xfer.doneIfs     = true;                     // Enable interrupts
  descLink.xfer.structReq   = false;                    // Disable auto-requests
  descLink.xfer.decLoopCnt  = 1;                        // Enable loops
  descLink.xfer.link        = 0;                        // End of linked list
  // Each consecutive transfer uses the previous destination
  descLink.xfer.dstAddrMode = ldmaCtrlDstAddrModeRel;
  // Set request mode to Block instead of all
  descLink.xfer.reqMode     = ldmaCtrlReqModeBlock;

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

  // Start transfers at dstBuffer
  LDMA->CH[LDMA_CHANNEL].DST = (uint32_t)&dstBuffer;

  // Start LDMA transfer
  if (!active) {
    DMADRV_LdmaStartTransfer(channelId,
                             &periTransferTx,
                             &descLink,
                             (DMADRV_Callback_t)ldmaCallback,
                             NULL);
  }

  // Send software request
  LDMA->SWREQ |= LDMA_CH_MASK;
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

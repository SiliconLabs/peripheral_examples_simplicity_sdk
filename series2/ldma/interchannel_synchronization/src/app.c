/***************************************************************************//**
 * @file app.c
 * @brief This example demonstrates the LDMA inter-channel synchronization. See
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
#include "em_prs.h"

#include "sl_gpio.h"
#include "sl_hal_gpio.h"
#include "sl_clock_manager.h"
#include "pin_config.h"
#include "peripheral_config.h"

// Constants for inter-channel sync transfer
#define SYNC_SET            0x80
#define SYNC_CLEAR          0x80
#define MATCH_VALUE         0x80

// Channels used for the example
#define LDMA_CHANNEL        0
#define GPIO_PRS_CHANNEL    1

// Memory to memory transfer buffer size
#define BUFFER_SIZE         4

// Define GPIO mapping for boards where LED0 and BUTTON0 share the same pins
#ifndef BUTTON0_PORT
  #define BUTTON0_PORT LED0_BUTTON0_PORT
  #define BUTTON0_PIN  LED0_BUTTON0_PIN
#endif

// Define GPIO mapping for boards where LED0 and BUTTON0 share the same pins
#ifndef LED0_PORT
  #define LED0_PORT SL_GPIO_PORT_B
  #define LED0_PIN  2
#endif

// Define GPIO mapping for boards where LED1 and BUTTON1 share the same pins
#ifndef BUTTON1_PORT
  #define BUTTON1_PORT LED1_BUTTON1_PORT
  #define BUTTON1_PIN  LED1_BUTTON1_PIN
#endif

// Descriptor linked lists for LDMA transfer
LDMA_Descriptor_t descLink0[3];
LDMA_Descriptor_t descLink1[2];

// Buffers for memory to memory transfer
uint8_t srcA[BUFFER_SIZE] = "AAaa";
uint8_t srcC[BUFFER_SIZE] = "CCcc";
uint8_t srcY[BUFFER_SIZE] = "YYyy";

uint8_t dstBuffer[BUFFER_SIZE];

const sl_gpio_t GPIO_PB0 = { .port = BUTTON0_PORT, .pin = BUTTON0_PIN };
const sl_gpio_t GPIO_PB1 = { .port = BUTTON1_PORT, .pin = BUTTON1_PIN };
const sl_gpio_t GPIO_LED0 = { .port = LED0_PORT, .pin = LED0_PIN };

/***************************************************************************//**
 * @brief
 *   LDMA Callback Function
 ******************************************************************************/
void ldmaCallback(void)
{
  // Toggle GPIO to notify that transfer is complete
  sl_gpio_toggle_pin(&GPIO_LED0);
}

/***************************************************************************//**
 * @brief
 *   Setup push button PB0 and PB1 as PRS source for DMAREQ0 and DMAREQ1.
 ******************************************************************************/
static void gpioPrsSetup(void)
{
  // Enable GPIO and PRS clocks
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_GPIO);
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_PRS);

  // Configure push buttons as input
  sl_gpio_set_pin_mode(&GPIO_PB0, SL_GPIO_MODE_INPUT_PULL_FILTER, 1);
  sl_gpio_set_pin_mode(&GPIO_PB1, SL_GPIO_MODE_INPUT_PULL_FILTER, 1);

  // Show completion state on LED0
  sl_gpio_set_pin_mode(&GPIO_LED0, SL_GPIO_MODE_PUSH_PULL, LED_OFF);

  // Configure PRS sensing on push buttons
  sl_hal_gpio_configure_external_interrupt(&GPIO_PB0, BUTTON0_PIN, SL_GPIO_INTERRUPT_NO_EDGE);
  sl_hal_gpio_configure_external_interrupt(&GPIO_PB1, BUTTON1_PIN, SL_GPIO_INTERRUPT_NO_EDGE);

  // Select GPIO as PRS source and push buttons as signals for PRS channels
  PRS_SourceAsyncSignalSet(GPIO_PRS_CHANNEL, PRS_ASYNC_CH_CTRL_SOURCESEL_GPIO,
                           BUTTON0_PIN);
  PRS_SourceAsyncSignalSet(GPIO_PRS_CHANNEL+1, PRS_ASYNC_CH_CTRL_SOURCESEL_GPIO,
                           BUTTON1_PIN);

  // DMA request is high active, invert PRS signals to default low
  PRS_Combine(GPIO_PRS_CHANNEL, GPIO_PRS_CHANNEL, prsLogic_NOT_A);
  PRS_Combine(GPIO_PRS_CHANNEL+1, GPIO_PRS_CHANNEL+1, prsLogic_NOT_A);

  // Select PRS channels for DMA request 0 and 1
  PRS_ConnectConsumer(GPIO_PRS_CHANNEL, prsTypeAsync, prsConsumerLDMA_REQUEST0);
  PRS_ConnectConsumer(GPIO_PRS_CHANNEL+1, prsTypeAsync,
                      prsConsumerLDMA_REQUEST1);
}

/***************************************************************************//**
 * @brief
 *   Initialize the LDMA controller for inter-channel synchronization
 ******************************************************************************/
void initLdma(void)
{
  unsigned int channelId0 = LDMA_CHANNEL, channelId1 = LDMA_CHANNEL + 1;
  bool active0, active1;
  uint32_t i;

  // Initialize buffers for memory transfer
  for (i = 0; i < BUFFER_SIZE; i++)
  {
    dstBuffer[i] = 0;
  }

  // Use peripheral transfer configuration macro for DMA channels
  LDMA_TransferCfg_t periTransferPB0 =
    LDMA_TRANSFER_CFG_PERIPHERAL(LDMAXBAR_CH_REQSEL_SIGSEL_LDMAXBARPRSREQ0
                                 | LDMAXBAR_CH_REQSEL_SOURCESEL_LDMAXBAR);
  LDMA_TransferCfg_t periTransferPB1 =
    LDMA_TRANSFER_CFG_PERIPHERAL(LDMAXBAR_CH_REQSEL_SIGSEL_LDMAXBARPRSREQ1
                                 | LDMAXBAR_CH_REQSEL_SOURCESEL_LDMAXBAR);
  
  // Set up descriptor links
  // Transfer AAaa
  descLink0[0] = (LDMA_Descriptor_t)
    LDMA_DESCRIPTOR_LINKREL_M2M_BYTE (&srcA, &dstBuffer, BUFFER_SIZE, 1);

  // Wait for MATCH_VALUE to be set
  descLink0[1] = (LDMA_Descriptor_t)
    LDMA_DESCRIPTOR_LINKREL_SYNC(0, 0, MATCH_VALUE, MATCH_VALUE, 1);

  // Transfer CCcc
  descLink0[2] = (LDMA_Descriptor_t)
    LDMA_DESCRIPTOR_SINGLE_M2M_BYTE(&srcC, &dstBuffer, BUFFER_SIZE);

  // Transfer YYyy
  descLink1[0] = (LDMA_Descriptor_t)
    LDMA_DESCRIPTOR_LINKREL_M2M_BYTE(&srcY, &dstBuffer, BUFFER_SIZE, 1);

  // Set SYNC_SET value
  descLink1[1] = (LDMA_Descriptor_t)
    LDMA_DESCRIPTOR_SINGLE_SYNC(SYNC_SET, 0, 0, 0);

  // First transfer in each set waits for request
  descLink0[0].xfer.structReq = false;
  descLink1[0].xfer.structReq = false;

  // Initialize DMADRV
  DMADRV_Init();

  // Halt on error if DMA channel cannot be allocated
  if ((DMADRV_AllocateChannel(&channelId0, NULL) != ECODE_EMDRV_DMADRV_OK)
      || (DMADRV_AllocateChannel(&channelId1, NULL) != ECODE_EMDRV_DMADRV_OK)) {
    __BKPT(0);
  }

  // Check that IADC->memory LDMA channel is not currently active; halt if error
  if ((DMADRV_TransferActive(channelId0, &active0) != ECODE_EMDRV_DMADRV_OK)
      || (DMADRV_TransferActive(channelId1, &active1) != ECODE_EMDRV_DMADRV_OK)) {
    __BKPT(0);
  }

  // Start LDMA transfer on both channels
  if (!active0 && !active1) {
    DMADRV_LdmaStartTransfer(channelId0,
                             &periTransferPB0,
                             descLink0,
                             (DMADRV_Callback_t)ldmaCallback,
                             NULL);

    DMADRV_LdmaStartTransfer(channelId1,
                             &periTransferPB1,
                             descLink1,
                             (DMADRV_Callback_t)ldmaCallback,
                             NULL);
  }
}

/*******************************************************************************
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  // Initialize GPIO for PRS
  gpioPrsSetup();

  // Initialize LDMA
  initLdma();
}

/*******************************************************************************
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{

}


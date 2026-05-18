/***************************************************************************//**
 * @file app.c
 * @brief This example demonstrates the single direct register LDMA transfer.
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

#include "dmadrv.h"
#include "em_prs.h"
#include "sl_clock_manager.h"
#include "sl_gpio.h"

#include "pin_config.h"
#include "peripheral_config.h"

// DMA channel used for the example
#define LDMA_CHANNEL        0
#define LDMA_CH_MASK        (1 << LDMA_CHANNEL)

// Memory to memory transfer buffer size and constants for GPIO PRS
#define BUFFER_SIZE         128
#define TRANSFER_SIZE       BUFFER_SIZE
#define USE_GPIO_PRS        0
#define GPIO_PRS_CHANNEL    1

// Define GPIO mapping for boards where LED1 and BUTTON1 share the same pins
#ifndef BUTTON1_PORT
  #define BUTTON1_PORT LED1_BUTTON1_PORT
  #define BUTTON1_PIN  LED1_BUTTON1_PIN
#endif

// Define GPIO mapping for boards where LED0 and BUTTON0 share the same pins
#ifndef LED0_PORT
  #define LED0_PORT LED0_BUTTON0_PORT
  #define LED0_PIN  LED0_BUTTON0_PIN
#endif

// Buffers for memory to memory transfer
uint16_t srcBuffer[BUFFER_SIZE];
uint16_t dstBuffer[BUFFER_SIZE];

LDMA_Descriptor_t desc;

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

#if (USE_GPIO_PRS == 1)
/***************************************************************************//**
 * @brief
 *   Setup push button PB1 as PRS source for DMAREQ0.
 ******************************************************************************/
static void gpioPrsSetup(void)
{
  // Enable peripheral clocks
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_PRS);

  // Configure push button PB1 as input
  GPIO_PinModeSet(BUTTON1_PORT, BUTTON1_PIN,
                  gpioModeInputPullFilter, 1);

  // Configure PRS sensing on push button PB1
  GPIO_ExtIntConfig(BUTTON1_PORT, BUTTON1_PIN, BUTTON1_PIN,
                    false, false, false);

  // Select GPIO as PRS source and push button PB1 as signal for PRS channel
  PRS_SourceAsyncSignalSet(GPIO_PRS_CHANNEL,
                           PRS_ASYNC_CH_CTRL_SOURCESEL_GPIO,
                           BUTTON1_PIN);

  // DMA request is high active, invert PRS signal to default low
  PRS_Combine(GPIO_PRS_CHANNEL, GPIO_PRS_CHANNEL, prsLogic_NOT_A);

  // Select PRS channel for DMA request 0
  PRS_ConnectConsumer(GPIO_PRS_CHANNEL, prsTypeAsync, prsConsumerLDMA_REQUEST0);
}
#endif

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
 *   Setup LDMA to be triggered either by software or by pressing PB1
 ******************************************************************************/
void initLdma(void)
{
  unsigned int channelId = LDMA_CHANNEL;
  bool active;

  uint32_t i;

  // Initialize buffers for memory transfer
  for (i = 0; i < BUFFER_SIZE; i++)
  {
    srcBuffer[i] = i;
    dstBuffer[i] = 0;
  }

#if (USE_GPIO_PRS==1)
  // Use peripheral transfer configuration macro for DMA channels
  LDMA_TransferCfg_t periTransferConfigPB1 =
    LDMA_TRANSFER_CFG_PERIPHERAL(LDMAXBAR_CH_REQSEL_SIGSEL_LDMAXBARPRSREQ0
                                 | LDMAXBAR_CH_REQSEL_SOURCESEL_LDMAXBAR);
#else
  LDMA_TransferCfg_t periTransferConfigPB1 = LDMA_TRANSFER_CFG_MEMORY();
#endif

  // Set up descriptor links
  desc = (LDMA_Descriptor_t)LDMA_DESCRIPTOR_SINGLE_M2M_WORD(&srcBuffer,
                                                           &dstBuffer,
                                                           TRANSFER_SIZE);

  // Transfer half word size
  desc.xfer.size = ldmaCtrlSizeHalf;
  desc.xfer.reqMode = ldmaCtrlReqModeAll;

#if (USE_GPIO_PRS==1)
  // Disables SW trigger if using push button
  desc.xfer.structReq = false;
#endif

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
                             &periTransferConfigPB1,
                             &desc,
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

#if (USE_GPIO_PRS == 1)
  // Initialize GPIO for PRS
  gpioPrsSetup();
#endif
  
  // Initialize LDMA
  initLdma();
}

/*******************************************************************************
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{

}

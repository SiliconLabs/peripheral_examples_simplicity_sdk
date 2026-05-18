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

#include "dmadrv.h"
#include "sl_hal_prs.h"
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
sl_hal_ldma_descriptor_t descriptor_link0[3];
sl_hal_ldma_descriptor_t descriptor_link1[2];

// Buffers for memory to memory transfer
uint8_t source_a[BUFFER_SIZE] = "AAaa";
uint8_t source_c[BUFFER_SIZE] = "CCcc";
uint8_t source_y[BUFFER_SIZE] = "YYyy";

uint8_t dest_buffer[BUFFER_SIZE];

const sl_gpio_t GPIO_PB0 = { .port = BUTTON0_PORT, .pin = BUTTON0_PIN };
const sl_gpio_t GPIO_PB1 = { .port = BUTTON1_PORT, .pin = BUTTON1_PIN };
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
 *   Setup push button PB0 and PB1 as PRS source for DMAREQ0 and DMAREQ1.
 ******************************************************************************/
void init_gpio(void)
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
  sl_hal_gpio_configure_external_interrupt(&GPIO_PB0,
                                           BUTTON0_PIN,
                                           SL_GPIO_INTERRUPT_NO_EDGE);
  sl_hal_gpio_configure_external_interrupt(&GPIO_PB1,
                                           BUTTON1_PIN,
                                           SL_GPIO_INTERRUPT_NO_EDGE);

  // Select GPIO as PRS source and push buttons as signals for PRS channels
  sl_hal_prs_async_connect_channel_producer(GPIO_PRS_CHANNEL,
                                            SL_HAL_PRS_ASYNC_GPIO_PIN1);
  sl_hal_prs_async_connect_channel_producer(GPIO_PRS_CHANNEL+1,
                                            SL_HAL_PRS_ASYNC_GPIO_PIN2);

  // DMA request is high active, invert PRS signals to default low
  sl_hal_prs_async_combine_signals(GPIO_PRS_CHANNEL,
                                   GPIO_PRS_CHANNEL,
                                   SL_HAL_PRS_LOGIC_NOT_A);
  sl_hal_prs_async_combine_signals(GPIO_PRS_CHANNEL+1,
                                   GPIO_PRS_CHANNEL+1,
                                   SL_HAL_PRS_LOGIC_NOT_A);

  // Select PRS channels for DMA request 0 and 1
  sl_hal_prs_connect_channel_consumer(GPIO_PRS_CHANNEL,
                                      SL_HAL_PRS_TYPE_ASYNC,
                                      SL_HAL_PRS_CONSUMER_LDMAXBAR0_DMAREQ0);
  sl_hal_prs_connect_channel_consumer(GPIO_PRS_CHANNEL+1,
                                      SL_HAL_PRS_TYPE_ASYNC,
                                      SL_HAL_PRS_CONSUMER_LDMAXBAR0_DMAREQ1);
}

/***************************************************************************//**
 * @brief
 *   Initialize the LDMA controller for inter-channel synchronization
 ******************************************************************************/
void init_ldma(void)
{
  unsigned int channel_id_0 = LDMA_CHANNEL, channel_id_1 = LDMA_CHANNEL + 1;
  bool active0, active1;
  uint32_t i;

  // Initialize buffers for memory transfer
  for (i = 0; i < BUFFER_SIZE; i++)
  {
    dest_buffer[i] = 0;
  }

  // Use peripheral transfer configuration macro for DMA channels
  sl_hal_ldma_transfer_config_t peripheral_transfer_PB0 =
    SL_HAL_LDMA_TRANSFER_CFG_PERIPHERAL(SL_HAL_LDMA_PERIPHERAL_SIGNAL_LDMAXBAR0_PRSREQ0);
  sl_hal_ldma_transfer_config_t periTransferPB1 =
    SL_HAL_LDMA_TRANSFER_CFG_PERIPHERAL(SL_HAL_LDMA_PERIPHERAL_SIGNAL_LDMAXBAR0_PRSREQ1);
  
  // Set up descriptor links
  // Transfer AAaa
  descriptor_link0[0] = (sl_hal_ldma_descriptor_t)
    SL_HAL_LDMA_DESCRIPTOR_LINKREL_M2M (SL_HAL_LDMA_CTRL_SIZE_BYTE,
                                        &source_a,
                                        &dest_buffer,
                                        BUFFER_SIZE,
                                        1);

  // Wait for MATCH_VALUE to be set
  descriptor_link0[1] = (sl_hal_ldma_descriptor_t)
    SL_HAL_LDMA_DESCRIPTOR_LINKREL_SYNC(0, 0, MATCH_VALUE, MATCH_VALUE, 1);

  // Transfer CCcc
  descriptor_link0[2] = (sl_hal_ldma_descriptor_t)
    SL_HAL_LDMA_DESCRIPTOR_SINGLE_M2M(SL_HAL_LDMA_CTRL_SIZE_BYTE,
                                      &source_c,
                                      &dest_buffer,
                                      BUFFER_SIZE);

  // Transfer YYyy
  descriptor_link1[0] = (sl_hal_ldma_descriptor_t)
    SL_HAL_LDMA_DESCRIPTOR_LINKREL_M2M(SL_HAL_LDMA_CTRL_SIZE_BYTE,
                                       &source_y,
                                       &dest_buffer,
                                       BUFFER_SIZE,
                                       1);

  // Set SYNC_SET value
  descriptor_link1[1] = (sl_hal_ldma_descriptor_t)
    SL_HAL_LDMA_DESCRIPTOR_SINGLE_SYNC(SYNC_SET, 0, 0, 0);

  // First transfer in each set waits for request
  descriptor_link0[0].xfer.struct_req = false;
  descriptor_link1[0].xfer.struct_req = false;

  // Initialize DMADRV
  DMADRV_Init();

  // Halt on error if DMA channel cannot be allocated
  if ((DMADRV_AllocateChannel(&channel_id_0, NULL) != ECODE_EMDRV_DMADRV_OK)
      || (DMADRV_AllocateChannel(&channel_id_1, NULL) != ECODE_EMDRV_DMADRV_OK)) {
    __BKPT(0);
  }

  // Check that IADC->memory LDMA channel is not currently active; halt if error
  if ((DMADRV_TransferActive(channel_id_0, &active0) != ECODE_EMDRV_DMADRV_OK)
      || (DMADRV_TransferActive(channel_id_1, &active1) != ECODE_EMDRV_DMADRV_OK)) {
    __BKPT(0);
  }

  // Start LDMA transfer on both channels
  if (!active0 && !active1) {
    DMADRV_LdmaStartTransfer(channel_id_0,
                             &peripheral_transfer_PB0,
                             descriptor_link0,
                             (DMADRV_Callback_t)ldma_callback,
                             NULL);

    DMADRV_LdmaStartTransfer(channel_id_1,
                             &periTransferPB1,
                             descriptor_link1,
                             (DMADRV_Callback_t)ldma_callback,
                             NULL);
  }
}

/*******************************************************************************
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  // Initialize GPIO for PRS
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


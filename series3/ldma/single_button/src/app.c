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
#include "sl_hal_prs.h"
#include "sl_gpio.h"
#include "sl_hal_ldma.h"
#include "sl_clock_manager.h"
#include "sl_device_peripheral.h"
#include "pin_config.h"
#include "peripheral_config.h"

// DMA channel used for the example
#define LDMA_CHANNEL        0
#define LDMA_CH_MASK        (1 << LDMA_CHANNEL)

// Memory to memory transfer buffer size and constants for GPIO PRS
#define BUFFER_SIZE         128
#define TRANSFER_SIZE       BUFFER_SIZE
#define USE_GPIO_PRS        0
#define GPIO_PRS_CHANNEL    2

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
uint16_t source_buffer[BUFFER_SIZE];
uint16_t dest_buffer[BUFFER_SIZE];

sl_hal_ldma_descriptor_t desc;

const sl_gpio_t GPIO_LED0 = { .port = LED0_PORT, .pin = LED0_PIN };
const sl_gpio_t GPIO_PB1 = { .port = BUTTON1_PORT, .pin = BUTTON1_PIN };

/***************************************************************************//**
 * @brief
 *   LDMA Callback Function
 ******************************************************************************/
void ldma_callback(void)
{
  // Toggle GPIO to notify that transfer is complete
  sl_gpio_toggle_pin(&GPIO_LED0);
}

#if (USE_GPIO_PRS == 1)
/***************************************************************************//**
 * @brief
 *   Setup push button PB1 as PRS source for DMAREQ0.
 ******************************************************************************/
void init_prs(void)
{
  // Enable peripheral clocks
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_PRS);

  // Configure push button PB1 as input
  sl_gpio_set_pin_mode(&GPIO_PB1, SL_GPIO_MODE_INPUT_PULL, true);

  // Configure PRS sensing on push button PB1
  sl_hal_gpio_configure_external_interrupt(&GPIO_PB1,
                                           BUTTON1_PIN,
                                           SL_GPIO_INTERRUPT_RISING_EDGE);

  // Select GPIO as PRS source and push button PB1 as signal for PRS channel
  sl_hal_prs_async_connect_channel_producer(GPIO_PRS_CHANNEL,  SL_HAL_PRS_ASYNC_GPIO_PIN2);

  // DMA request is high active, invert PRS signal to default low
  sl_hal_prs_async_combine_signals(GPIO_PRS_CHANNEL,
                                   GPIO_PRS_CHANNEL,
                                   SL_HAL_PRS_LOGIC_NOT_A);

  // Select PRS channel for DMA request 0
  sl_hal_prs_connect_channel_consumer(GPIO_PRS_CHANNEL,
                                      SL_HAL_PRS_TYPE_ASYNC,
                                      SL_HAL_PRS_CONSUMER_LDMAXBAR0_DMAREQ0);
}
#endif

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
 *   Setup LDMA to be triggered either by software or by pressing PB1
 ******************************************************************************/
void init_ldma(void)
{
  unsigned int channel_id = LDMA_CHANNEL;
  bool active;

  uint32_t i;

  // Initialize buffers for memory transfer
  for (i = 0; i < BUFFER_SIZE; i++)
  {
    source_buffer[i] = i;
    dest_buffer[i] = 0;
  }

#if (USE_GPIO_PRS==1)
  // Use peripheral transfer configuration macro for DMA channels
  sl_hal_ldma_transfer_config_t peripheral_transfer_config_PB1 =
            SL_HAL_LDMA_TRANSFER_CFG_PERIPHERAL(SL_HAL_LDMA_PERIPHERAL_SIGNAL_LDMAXBAR0_PRSREQ0);
#else
  sl_hal_ldma_transfer_config_t peripheral_transfer_config_PB1 = SL_HAL_LDMA_TRANSFER_CFG_MEMORY();
#endif

  // Set up descriptor links
  desc = (sl_hal_ldma_descriptor_t)SL_HAL_LDMA_DESCRIPTOR_SINGLE_M2M(SL_HAL_LDMA_CTRL_SIZE_HALF,
                                                                   &source_buffer,
                                                                   &dest_buffer,
                                                                   TRANSFER_SIZE);

  // Transfer half word size
  desc.xfer.req_mode = SL_HAL_LDMA_CTRL_REQ_MODE_ALL;

#if (USE_GPIO_PRS==1)
  // Disables SW trigger if using push button
  desc.xfer.struct_req = false;
#endif

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
                             &peripheral_transfer_config_PB1,
                             &desc,
                             (DMADRV_Callback_t)ldma_callback,
                             NULL);
  }
}

/*******************************************************************************
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  // Initialize LED
  init_gpio();

#if (USE_GPIO_PRS == 1)
  // Initialize PRS
  init_prs();
#endif
  
  // Initialize LDMA
  init_ldma();
}

/*******************************************************************************
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{

}

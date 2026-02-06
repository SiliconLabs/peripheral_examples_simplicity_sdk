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

#include "sl_clock_manager.h"
#include "sl_gpio.h"
#include "dmadrv.h"
#include "sl_hal_timer.h"
#include "sl_hal_ldma.h"
#include "pin_config.h"

// Define GPIO mapping for boards where LED0 and BUTTON0 share the same pins
#ifndef LED0_PORT
  #define LED0_PORT LED0_BUTTON0_PORT
  #define LED0_PIN  LED0_BUTTON0_PIN
#endif

const sl_gpio_t GPIO_LED0 = { .port = LED0_PORT, .pin = LED0_PIN };

const sl_gpio_t TIMER_INPUT = { .port = EXP_TIMER_CC0_PORT,
                                .pin  = EXP_TIMER_CC0_PIN };

#define BUFFERSIZE 512

// Edge capture buffer
static volatile uint32_t buffer[BUFFERSIZE];

// Globally declared DMADRV channel ID and link descriptor
unsigned int channel_id;

/*
 * Set up a linked descriptor to save CC0 input capture register to
 * user-specified buffer.  By linking the descriptor to itself
 * (the last argument is the relative jump in terms of the number of
 * descriptors), transfers will run continuously until firmware
 * otherwise stops them.
 */
sl_hal_ldma_descriptor_t ldma_desc = SL_HAL_LDMA_DESCRIPTOR_LINKREL_P2M(
        SL_HAL_LDMA_CTRL_SIZE_WORD,
        &TIMER0->CC[0].ICF,
        &buffer,
        BUFFERSIZE,
        0);

/**************************************************************************//**
 * @brief  LDMA Callback
 *****************************************************************************/
void ldma_callback(void)
{
  // Toggle LED0 to notify that transfers are complete
  sl_gpio_toggle_pin(&GPIO_LED0);
}

/**************************************************************************//**
 * @brief LDMA initialization
 *****************************************************************************/
void ldma_init(void)
{
  bool active;

  // Trigger LDMA transfer on CC0 peripheral requests
  sl_hal_ldma_transfer_init_t ldma_config =
      SL_HAL_LDMA_TRANSFER_CFG_PERIPHERAL(SL_HAL_LDMA_PERIPHERAL_SIGNAL_TIMER0_CC0);

  // Do not ignore single requests.  Transfer data on every request.
  ldma_desc.xfer.ignore_single_req = 0;

  // Initialize DMADRV
  DMADRV_Init();

  // Halt on error if DMA channel cannot be allocated
  if (DMADRV_AllocateChannel(&channel_id, NULL) != ECODE_EMDRV_DMADRV_OK) {
    __BKPT(0);
  }

  // Check that LDMA channel is not currently active; halt if error
  if (DMADRV_TransferActive(channel_id, &active) != ECODE_EMDRV_DMADRV_OK) {
    __BKPT(0);
  }

  // Start LDMA transfer
  if (!active) {
    DMADRV_LdmaStartTransfer(channel_id,
                             &ldma_config,
                             &ldma_desc,
                             (DMADRV_Callback_t)ldma_callback,
                             NULL);
  }
}

/***************************************************************************//**
 * Initialize GPIO.
 ******************************************************************************/
void gpio_init(void)
{
  // Configure LED0 output
  sl_gpio_set_pin_mode(&GPIO_LED0, SL_GPIO_MODE_PUSH_PULL, false);

  // Configure TIMER_INPUT as input
  sl_gpio_set_pin_mode(&TIMER_INPUT, SL_GPIO_MODE_INPUT, 0);
}

/**************************************************************************//**
 * @brief TIMER initialization
 *****************************************************************************/
void timer_init(void)
{
  sl_hal_timer_init_t init = SL_HAL_TIMER_INIT_DEFAULT;
  sl_hal_timer_channel_config_t channel_init = SL_HAL_TIMER_CHANNEL_INIT_DEFAULT;

  // Enable TIMER0 bus clocks.
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_TIMER0);

  // Configure capture/compare channel for input capture
  channel_init.channel_mode = SL_HAL_TIMER_CHANNEL_MODE_CAPTURE;
  channel_init.input_capture_edge = SL_HAL_TIMER_CHANNEL_EDGE_BOTH;
  channel_init.input_capture_event = SL_HAL_TIMER_CHANNEL_EVENT_EVERY_EDGE;
  init.prescaler = SL_HAL_TIMER_PRESCALER_DIV1024;

  sl_hal_timer_init(TIMER0, &init);
  sl_hal_timer_channel_init(TIMER0, 0, &channel_init);
  sl_hal_timer_enable(TIMER0);

  // Route Button 0 to TIMER0 capture/compare channel 0
  GPIO->TIMER0ROUTE[0].ROUTEEN = GPIO_TIMER0_ROUTEEN_CC0PEN;
  GPIO->TIMER0ROUTE[0].CC0ROUTE =
      TIMER_INPUT.pin << _GPIO_TIMER0_CC0ROUTE_PIN_SHIFT
    | TIMER_INPUT.port;

  /*
   * Overwrite the default of 0xFFFF in TIMER_TOP with 0xFFFFFFFF
   * because TIMER0 is 32 bits wide.
   */
  sl_hal_timer_set_top(TIMER0, 0xFFFFFFFF);

  // Enable interrupts on capture/compare channel 0
  sl_hal_timer_clear_interrupts(TIMER0, _TIMER_IF_MASK);
  sl_hal_timer_enable_interrupts(TIMER0, TIMER_IF_CC0);

  // Start the timer
  sl_hal_timer_start(TIMER0);
}

/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  // Initialize GPIO, LDMA, and TIMER
  gpio_init();
  ldma_init();
  timer_init();
}

/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{
}

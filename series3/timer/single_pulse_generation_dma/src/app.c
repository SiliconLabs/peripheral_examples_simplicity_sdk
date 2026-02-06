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
#include "dmadrv.h"
#include "sl_gpio.h"
#include "sl_hal_timer.h"
#include "sl_hal_ldma.h"
#include "pin_config.h"

/*
 * Change this to modify the length of the delay from when the TIMER
 * starts counting to when CC0 drives the GPIO pin high.
 */
#define NUM_SECONDS_DELAY  3

// Length of the pulse high time in milliseconds
#define PULSE_WIDTH        1

const sl_gpio_t TIMER_OUTPUT = { .port = EXP_TIMER_CC0_PORT,
                                 .pin  = EXP_TIMER_CC0_PIN };

// Compare values set to generate a 1 ms pulse
static uint32_t compare_value1;
static uint32_t compare_value2;

// Globally declared DMADRV channel ID and link descriptor
unsigned int channelId;
sl_hal_ldma_descriptor_t ldma_desc;

/**************************************************************************//**
 * @brief LDMA initialization
 *****************************************************************************/
void ldma_init(void)
{
  bool active;

  // Trigger LDMA transfer on CC0 peripheral requests
  sl_hal_ldma_transfer_init_t ldma_config =
      SL_HAL_LDMA_TRANSFER_CFG_PERIPHERAL(SL_HAL_LDMA_PERIPHERAL_SIGNAL_TIMER0_CC0);

  ldma_desc = (sl_hal_ldma_descriptor_t)SL_HAL_LDMA_DESCRIPTOR_SINGLE_M2P(
        SL_HAL_LDMA_CTRL_SIZE_WORD, // Transfer size
        &compare_value2,            // Memory source address
        &TIMER0->CC[0].OC,          // Output compare buffer register
        1);                         // Number of transfers to make

  // Do not ignore single requests.  Transfer data on every request.
  ldma_desc.xfer.ignore_single_req = 0;

  // Initialize DMADRV
  DMADRV_Init();

  // Halt on error if DMA channel cannot be allocated
  if (DMADRV_AllocateChannel(&channelId, NULL) != ECODE_EMDRV_DMADRV_OK) {
    __BKPT(0);
  }

  // Check that LDMA channel is not currently active; halt if error
  if (DMADRV_TransferActive(channelId, &active) != ECODE_EMDRV_DMADRV_OK) {
    __BKPT(0);
  }

  // Start LDMA transfer
  if (!active) {
    DMADRV_LdmaStartTransfer(channelId,
                             &ldma_config,
                             &ldma_desc,
                             NULL,
                             NULL);
  }
}

/***************************************************************************//**
 * Initialize GPIO.
 ******************************************************************************/
void gpio_init(void)
{
  // Configure TIMER_OUTPUT
  sl_gpio_set_pin_mode(&TIMER_OUTPUT, SL_GPIO_MODE_PUSH_PULL, 0);
}

/**************************************************************************//**
 * @brief TIMER initialization
 *****************************************************************************/
void timer_init(void)
{
  uint32_t timer_freq;
  sl_hal_timer_init_t init = SL_HAL_TIMER_INIT_DEFAULT;
  sl_hal_timer_channel_config_t channel_init = SL_HAL_TIMER_CHANNEL_INIT_DEFAULT;

  // Enable TIMER0 bus clocks.
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_TIMER0);

  // Configure capture/compare channel for output compare
  init.count_one_shot = true;
  channel_init.channel_mode = SL_HAL_TIMER_CHANNEL_MODE_COMPARE;
  channel_init.compare_match_output_action = SL_HAL_TIMER_CHANNEL_OUTPUT_ACTION_TOGGLE;

  // Route TIMER0 CC0 output to output pin
  GPIO->TIMER0ROUTE[0].ROUTEEN = GPIO_TIMER0_ROUTEEN_CC0PEN;
  GPIO->TIMER0ROUTE[0].CC0ROUTE =
      TIMER_OUTPUT.pin << _GPIO_TIMER0_CC0ROUTE_PIN_SHIFT
    | TIMER_OUTPUT.port;

  sl_hal_timer_init(TIMER0, &init);
  sl_hal_timer_channel_init(TIMER0, 0, &channel_init);
  sl_hal_timer_enable(TIMER0);

  /*
   * Overwrite the default of 0xFFFF in TIMER_TOP with 0xFFFFFFFF
   * because TIMER0 is 32 bits wide.
   */
  sl_hal_timer_set_top(TIMER0, 0xFFFFFFFF);

  sl_clock_manager_get_clock_branch_frequency(SL_CLOCK_BRANCH_EM01GRPACLK,
                                              &timer_freq);
  timer_freq /= (init.prescaler + 1);
  compare_value1 = timer_freq * NUM_SECONDS_DELAY;
  sl_hal_timer_channel_set_compare(TIMER0, 0, compare_value1);

  /*
   * Set the second compare value since its a global variable, but
   * don't actually use it here.  It gets written to the TIMER_OC
   * register by LDMA after the first edge happens.
   */
  compare_value2 = compare_value1 + ((timer_freq / 1000) * PULSE_WIDTH);

  // Start counter.
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

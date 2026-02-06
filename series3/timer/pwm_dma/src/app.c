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

const sl_gpio_t TIMER_OUTPUT = { .port = EXP_TIMER_CC0_PORT,
                                 .pin  = EXP_TIMER_CC0_PIN };

//  Desired frequency in Hz
#define PWM_FREQ 1000

/*
 * This table holds the time calculated for each given duty cycle value
 * expressed as a percent.  Note that BUFFER_SIZE must match the number
 * of values in duty_cycle_percentages[BUFFER_SIZE].
 */
#define BUFFERSIZE 11
static uint32_t buffer[BUFFERSIZE];

// Each change in duty cycle expressed as a percent
static const uint32_t duty_cycle_percentages[BUFFERSIZE] =
    {0, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100};

// Globally declared DMADRV channel ID and link descriptor
unsigned int channel_id;
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

  ldma_desc = (sl_hal_ldma_descriptor_t)SL_HAL_LDMA_DESCRIPTOR_LINKREL_M2P(
                              SL_HAL_LDMA_CTRL_SIZE_WORD,  // Transfer size
                              &buffer,                     // Memory source address
                              &TIMER0->CC[0].OCB,          // Output compare buffer register
                              BUFFERSIZE,                  // Number of transfers to make
                              0);

  // Do not ignore single requests.  Transfer data on every request.
  ldma_desc.xfer.ignore_single_req = 0;

  // Do not request an interrupt on completion of all transfers
  ldma_desc.xfer.done_ifs  = 0;

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
  uint32_t timer_freq, top_value;
  sl_hal_timer_init_t init = SL_HAL_TIMER_INIT_DEFAULT;
  sl_hal_timer_channel_config_t channel_init = SL_HAL_TIMER_CHANNEL_INIT_DEFAULT;

  // Enable TIMER0 bus clocks.
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_TIMER0);

  // Configure capture/compare channel for PWM
  channel_init.channel_mode = SL_HAL_TIMER_CHANNEL_MODE_PWM;

  // Route TIMER0 CC0 output to output pin
  GPIO->TIMER0ROUTE[0].ROUTEEN = GPIO_TIMER0_ROUTEEN_CC0PEN;
  GPIO->TIMER0ROUTE[0].CC0ROUTE =
      TIMER_OUTPUT.pin << _GPIO_TIMER0_CC0ROUTE_PIN_SHIFT
    | TIMER_OUTPUT.port;

  sl_hal_timer_init(TIMER0, &init);
  sl_hal_timer_channel_init(TIMER0, 0, &channel_init);
  sl_hal_timer_enable(TIMER0);

  // Set top value to overflow at the desired PWM_FREQ frequency
  sl_clock_manager_get_clock_branch_frequency(SL_CLOCK_BRANCH_EM01GRPACLK,
                                              &timer_freq);
  timer_freq /= (init.prescaler + 1);
  top_value = (timer_freq / PWM_FREQ);
  sl_hal_timer_set_top(TIMER0, top_value);

  // Start counter.
  sl_hal_timer_start(TIMER0);
}

/**************************************************************************//**
 * @brief
 *    Populate buffer with calculated duty cycle time values
 *****************************************************************************/
void populate_buffer(void)
{
  uint32_t i, topVal;

  // 100% duty cycle is the maximum count value
  topVal = sl_hal_timer_get_top(TIMER0);

  for (i = 0; i < BUFFERSIZE; i++) {
    buffer[i] = (topVal * duty_cycle_percentages[i]) / 100;
  }
}

/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  // Initialize GPIO and TIMER
  gpio_init();
  timer_init();

  // Initialize DMA only after buffer is populated
  populate_buffer();
  ldma_init();
}

/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{
}

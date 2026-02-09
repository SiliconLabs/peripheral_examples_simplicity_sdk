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

#include "em_timer.h"

#include "pin_config.h"

const sl_gpio_t TIMER_OUTPUT = { .port = TIMER_EXTIO_PORT,
                                 .pin  = TIMER_EXTIO_PIN };

//  Desired frequency in Hz
#define PWM_FREQ 1000

/*
 * This table holds the time calculated for each given duty cycle value
 * expressed as a percent.  Note that BUFFER_SIZE must match the number
 * of values in dutyCyclePercentages[BUFFER_SIZE].
 */
#define BUFFERSIZE 11
static uint32_t buffer[BUFFERSIZE];

// Each change in duty cycle expressed as a percent
static const uint32_t dutyCyclePercentages[BUFFERSIZE] =
    {0, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100};

// Globally declared DMADRV channel ID and link descriptor
unsigned int channelId;
LDMA_Descriptor_t ldma_desc;

/**************************************************************************//**
 * @brief LDMA initialization
 *****************************************************************************/
void ldma_init(void)
{
  bool active;

  // Trigger LDMA transfer on CC0 peripheral requests
  LDMA_TransferCfg_t ldma_config =
      LDMA_TRANSFER_CFG_PERIPHERAL(ldmaPeripheralSignal_TIMER0_CC0);

  /*
   * Set up a linked descriptor to save CC0 input capture register to
   * user-specified buffer.  By linking the descriptor to itself
   * (the last argument is the relative jump in terms of the number of
   * descriptors), transfers will run continuously until firmware
   * otherwise stops them.
   */
  ldma_desc =
    (LDMA_Descriptor_t)LDMA_DESCRIPTOR_LINKREL_M2P_BYTE(
        &buffer,             // Memory source address
        &TIMER0->CC[0].OCB,  // Output compare buffer register
        BUFFERSIZE,          // Number of transfers to make
        0);                  // Link to same descriptor

  // Transfer one word per request
  ldma_desc.xfer.size = ldmaCtrlSizeWord;

  // Do not ignore single requests.  Transfer data on every request.
  ldma_desc.xfer.ignoreSrec = 0;

  // Do not request an interrupt on completion of all transfers
  ldma_desc.xfer.doneIfs  = 0;

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
  uint32_t timerFreq, topValue;
  TIMER_Init_TypeDef timerInit = TIMER_INIT_DEFAULT;
  TIMER_InitCC_TypeDef timerCCInit = TIMER_INITCC_DEFAULT;

  // Enable TIMER0 bus clocks.
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_TIMER0);

  // Don't start counter on initialization
  timerInit.enable = false;

  // PWM mode sets/clears the output on compare/overflow events
  timerCCInit.mode = timerCCModePWM;

  TIMER_Init(TIMER0, &timerInit);

  // Route TIMER0 CC0 output to TIMER_EXTIO
  GPIO->TIMERROUTE[0].CC0ROUTE =
      (TIMER_OUTPUT.port << _GPIO_TIMER_CC0ROUTE_PORT_SHIFT)
      | (TIMER_OUTPUT.pin << _GPIO_TIMER_CC0ROUTE_PIN_SHIFT);
  GPIO->TIMERROUTE[0].ROUTEEN  = GPIO_TIMER_ROUTEEN_CC0PEN;

  TIMER_InitCC(TIMER0, 0, &timerCCInit);

  // Set top value to overflow at the desired PWM_FREQ frequency
  sl_clock_manager_get_clock_branch_frequency(SL_CLOCK_BRANCH_EM01GRPACLK,
                                              &timerFreq);
  timerFreq /= (timerInit.prescale + 1);
  topValue = (timerFreq / PWM_FREQ);
  TIMER_TopSet(TIMER0, topValue);

  // Now start the TIMER
  TIMER_Enable(TIMER0, true);

  // Trigger DMA on compare event to set CCVB to update duty cycle on next period
  TIMER_IntEnable(TIMER0, TIMER_IEN_CC0);
}

/**************************************************************************//**
 * @brief
 *    Populate buffer with calculated duty cycle time values
 *****************************************************************************/
void populate_buffer(void)
{
  uint32_t i, topVal;

  // 100% duty cycle is the maximum count value
  topVal = TIMER_TopGet(TIMER0);

  for (i = 0; i < BUFFERSIZE; i++) {
    buffer[i] = (topVal * dutyCyclePercentages[i]) / 100;
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

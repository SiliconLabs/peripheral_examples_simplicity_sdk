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

/*
 * Change this to modify the length of the delay from when the TIMER
 * starts counting to when CC0 drives the GPIO pin high.
 */
#define NUM_SECONDS_DELAY  3

// Compare count for first edge (assumes 19 MHz HFRCO)
#define COMPARE_VALUE1     (NUM_SECONDS_DELAY * 19000000)

// Length of the pulse high time in milliseconds
#define PULSE_WIDTH        1

// Compare count for second edge
#define COMPARE_VALUE2     (COMPARE_VALUE1 + (PULSE_WIDTH * 19000000 / 1000))

// Globally stored value for DMA transfer
uint32_t value = COMPARE_VALUE2;

const sl_gpio_t TIMER_OUTPUT = { .port = TIMER_EXTIO_PORT,
                                 .pin  = TIMER_EXTIO_PIN };

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
  ldma_desc = (LDMA_Descriptor_t)LDMA_DESCRIPTOR_SINGLE_M2P_BYTE(
      &value,            // Memory source address
      &TIMER0->CC[0].OC, // Output compare register
      1);                // Just 1 transfer

  // Transfer one word per request
  ldma_desc.xfer.size = ldmaCtrlSizeWord;

  // Do not ignore single requests.  Transfer data on every request.
  ldma_desc.xfer.ignoreSrec = 0;

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
  TIMER_Init_TypeDef timerInit = TIMER_INIT_DEFAULT;
  TIMER_InitCC_TypeDef timerCCInit = TIMER_INITCC_DEFAULT;

  // Enable TIMER0 bus clocks.
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_TIMER0);

  // Do not start counter upon initialization
  timerInit.enable = false;

  // Set drive output pin to toggle upon compare match
  timerCCInit.mode = timerCCModeCompare;
  timerCCInit.cmoa = timerOutputActionToggle;

  TIMER_Init(TIMER0, &timerInit);

  // Route TIMER0 CC0 output to TIMER_EXTIO
  GPIO->TIMERROUTE[0].CC0ROUTE =
      (TIMER_OUTPUT.port << _GPIO_TIMER_CC0ROUTE_PORT_SHIFT)
      | (TIMER_OUTPUT.pin << _GPIO_TIMER_CC0ROUTE_PIN_SHIFT);
  GPIO->TIMERROUTE[0].ROUTEEN  = GPIO_TIMER_ROUTEEN_CC0PEN;

  /*
   * Overwrite the default of 0xFFFF in TIMER_TOP with 0xFFFFFFFF
   * because TIMER0 is 32 bits wide.
   */
  TIMER_TopSet(TIMER0, 0xFFFFFFFF);

  TIMER_InitCC(TIMER0, 0, &timerCCInit);

  // Set the count for the first output compare to match
  TIMER_CompareSet(TIMER0, 0, COMPARE_VALUE1);

  // Now start the TIMER
  TIMER_Enable(TIMER0, true);
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

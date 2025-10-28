/***************************************************************************//**
 * @file
 * @brief Top level application functions
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
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

// Define GPIO mapping for boards where LED0 and BUTTON0 share the same pins
#ifndef LED0_PORT
  #define LED0_PORT LED0_BUTTON0_PORT
  #define LED0_PIN  LED0_BUTTON0_PIN
#endif

const sl_gpio_t GPIO_LED0 = { .port = LED0_PORT, .pin = LED0_PIN };

const sl_gpio_t TIMER_INPUT = { .port = TIMER_EXTIO_PORT,
                                .pin  = TIMER_EXTIO_PIN };

// Globally declared DMADRV channel ID and link descriptor
unsigned int channelId;
LDMA_Descriptor_t ldma_desc;

#define BUFFERSIZE 512

// Edge capture buffer
static volatile uint32_t buffer[BUFFERSIZE];

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
    (LDMA_Descriptor_t)LDMA_DESCRIPTOR_LINKREL_P2M_BYTE(&TIMER0->CC[0].ICF,
                                                        &buffer,
                                                        BUFFERSIZE,
                                                        0);

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
  TIMER_Init_TypeDef timerInit = TIMER_INIT_DEFAULT;
  TIMER_InitCC_TypeDef timerCCInit = TIMER_INITCC_DEFAULT;

  // Enable TIMER0 bus clocks.
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_TIMER0);

  // Don't start counter on initialization
  timerInit.enable = false;

  // Set event on every edge
  timerCCInit.eventCtrl = timerEventEveryEdge;
  timerCCInit.edge = timerEdgeBoth;
  timerCCInit.mode = timerCCModeCapture;

  TIMER_Init(TIMER0, &timerInit);

  // Route TIMER0 CC0 input to TIMER_EXTIO
  GPIO->TIMERROUTE[0].CC0ROUTE =
      (TIMER_INPUT.port << _GPIO_TIMER_CC0ROUTE_PORT_SHIFT)
      | (TIMER_INPUT.pin << _GPIO_TIMER_CC0ROUTE_PIN_SHIFT);
  GPIO->TIMERROUTE[0].ROUTEEN  = GPIO_TIMER_ROUTEEN_CC0PEN;

  TIMER_InitCC(TIMER0, 0, &timerCCInit);

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

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
#include "dmadrv.h"

#include "sl_clock_manager.h"
#include "sl_interrupt_manager.h"

#include "em_eusart.h"
#include "em_ldma.h"
#include "sl_gpio.h"

#include "pin_config.h"

/*******************************************************************************
 *******************************   DEFINES   ***********************************
 ******************************************************************************/
// Size of the buffer for received data
#define BUFLEN   10
// In low-frequency mode, the maximum EUSART baud rate is 9600
#define BAUDRATE 9600

/*******************************************************************************
 ***************************   GLOBAL VARIABLES   ******************************
 ******************************************************************************/
// Receive data buffer
uint8_t buffer[BUFLEN];

unsigned int channel_id_rx;

unsigned int channel_id_tx;

const sl_gpio_t EUSART0_TX  = { .port = EUSART0_TX_PORT,
                                . pin = EUSART0_TX_PIN };

const sl_gpio_t EUSART0_RX  = { .port = EUSART0_RX_PORT,
                                . pin = EUSART0_RX_PIN };

// LDMA descriptor and transfer configuration structures for TX channel
LDMA_Descriptor_t ldmaTXDescriptor;
LDMA_TransferCfg_t ldmaTXConfig;

// LDMA descriptor and transfer configuration structures for RX channel
LDMA_Descriptor_t ldmaRXDescriptor;
LDMA_TransferCfg_t ldmaRXConfig;

volatile bool receive; // true: receive, false: transmit 

/**************************************************************************//**
 * @brief  Initialize GPIO
 *****************************************************************************/
void gpio_init(void)
{
  // Configure the EUSART TX pin to the board controller as an output
  sl_gpio_set_pin_mode(&EUSART0_TX, SL_GPIO_MODE_PUSH_PULL, 1);
  // Configure the EUSART RX pin to the board controller as an input
  sl_gpio_set_pin_mode(&EUSART0_RX, SL_GPIO_MODE_INPUT, 0);
  // Route EUSART0 TX and RX to the board controller TX and RX pins
  GPIO->EUSARTROUTE[0].TXROUTE = (EUSART0_TX_PORT << _GPIO_EUSART_TXROUTE_PORT_SHIFT)
                                  | (EUSART0_TX_PIN << _GPIO_EUSART_TXROUTE_PIN_SHIFT);
  GPIO->EUSARTROUTE[0].RXROUTE = (EUSART0_RX_PORT << _GPIO_EUSART_RXROUTE_PORT_SHIFT)
                                  | (EUSART0_RX_PIN << _GPIO_EUSART_RXROUTE_PIN_SHIFT);
  // Enable RX and TX signals now that they have been routed
  GPIO->EUSARTROUTE[0].ROUTEEN = GPIO_EUSART_ROUTEEN_RXPEN | GPIO_EUSART_ROUTEEN_TXPEN;
}

/**************************************************************************//**
 * @brief  Initialize EUSART
 *****************************************************************************/
void eusart_init(void)
{
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_EUSART0);
  // Initialize the EUSART0 module
  EUSART_UartInit_TypeDef init = EUSART_UART_INIT_DEFAULT_LF;
  EUSART_AdvancedInit_TypeDef advance_init = EUSART_ADVANCED_INIT_DEFAULT;

  init.baudrate = BAUDRATE;
  init.advancedSettings = &advance_init;
  init.advancedSettings->dmaWakeUpOnRx = true;
  init.advancedSettings->dmaHaltOnError = true;

  // Set FIFO watermark lvl; wake when ready to transfer from RX FIFO to memory
  advance_init.RxFifoWatermark = (EUSART_RxFifoWatermark_TypeDef)((BUFLEN - 1) << _EUSART_CFG1_RXFIW_SHIFT);

  // Configure and enable EUSART0 for low-frequency (EM2) operation
  EUSART_UartInitLf(EUSART0, &init);

  // Clear and enable transmit complete interrupt
  EUSART_IntClear(EUSART0, EUSART_IF_TXC);
  EUSART_IntEnable(EUSART0, EUSART_IEN_TXC);
  // Enable USART Interrupts
  sl_interrupt_manager_clear_irq_pending(EUSART0_RX_IRQn);
  sl_interrupt_manager_enable_irq(EUSART0_RX_IRQn);

  sl_interrupt_manager_clear_irq_pending(EUSART0_TX_IRQn);
  sl_interrupt_manager_enable_irq(EUSART0_TX_IRQn);
}

/**************************************************************************//**
 * @brief EUSART Transmit Interrupt Handler
 *****************************************************************************/
void EUSART0_TX_IRQHandler(void)
{
  /*
   * The EUSART differs from the USART in that the TX complete
   * interrupt flag must be explicitly cleared even after a write to
   * the FIFO.
   */
  EUSART_IntClear(EUSART0, EUSART_IF_TXC);
  receive = true;
}

/**************************************************************************//**
 * @brief  Initialize LDMA
 *****************************************************************************/
void ldma_init(void)
{
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_LDMA0);

  // Source is buffer, destination is EUSART0_TXDATA, and length is BUFLEN
  ldmaTXDescriptor = (LDMA_Descriptor_t)LDMA_DESCRIPTOR_SINGLE_M2P_BYTE(buffer, &(EUSART0->TXDATA), BUFLEN);

  // Transfer a byte on free space in the EUSART FIFO
  ldmaTXConfig = (LDMA_TransferCfg_t)LDMA_TRANSFER_CFG_PERIPHERAL(ldmaPeripheralSignal_EUSART0_TXFL);

  // Source is EUSART0_RXDATA, destination is buffer, and length is BUFLEN
  ldmaRXDescriptor = (LDMA_Descriptor_t)LDMA_DESCRIPTOR_SINGLE_P2M_BYTE(&(EUSART0->RXDATA), buffer, BUFLEN);

  // Transfer a byte on receive FIFO level event
  ldmaRXConfig = (LDMA_TransferCfg_t)LDMA_TRANSFER_CFG_PERIPHERAL(ldmaPeripheralSignal_EUSART0_RXFL);

  // Initialize DMADRV
  DMADRV_Init();

  // Halt on error if DMA channel cannot be allocated
  if (DMADRV_AllocateChannel(&channel_id_rx, NULL) != ECODE_EMDRV_DMADRV_OK) {
    __BKPT(0);
  }

  // Halt on error if DMA channel cannot be allocated
  if (DMADRV_AllocateChannel(&channel_id_tx, NULL) != ECODE_EMDRV_DMADRV_OK) {
    __BKPT(0);
  }
}

/**************************************************************************//**
 * @brief LDMA Callback on RX
 *****************************************************************************/
void ldma_callback_rx(void)
{
  receive = false;
}

/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  gpio_init();

  eusart_init();

  ldma_init();
  receive = true;
}

/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{
  volatile uint32_t i;
  // Receive data
  if(receive){
    // Zero out buffer
    for (i = 0; i < BUFLEN; i++) {
      buffer[i] = 0;
    }
    DMADRV_LdmaStartTransfer(channel_id_rx,
                             &ldmaRXConfig,
                             &ldmaRXDescriptor,
                             (DMADRV_Callback_t)ldma_callback_rx,
                             NULL);   
  // Transmit data 
  } else {
    DMADRV_LdmaStartTransfer(channel_id_tx,
                             &ldmaTXConfig,
                             &ldmaTXDescriptor,
                             NULL,
                             NULL);
    // Disable the channel interrupt; automatically enabled with previous API
    LDMA_IntDisable(1 << channel_id_tx);
  }
} 

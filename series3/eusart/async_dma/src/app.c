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
#include "dmadrv.h"

#include "sl_clock_manager.h"
#include "sl_interrupt_manager.h"
#include "sl_udelay.h"

#include "sl_hal_eusart.h"
#include "sl_hal_ldma.h"
#include "sl_gpio.h"

#include "pin_config.h"

/*******************************************************************************
 *******************************   DEFINES   ***********************************
 ******************************************************************************/
// Size of the buffer for received data
#define BUFLEN   10

/*******************************************************************************
 ***************************   GLOBAL VARIABLES   ******************************
 ******************************************************************************/
// Receive data buffer
uint8_t buffer[BUFLEN];

// LDMA channel IDs
unsigned int rx_channel_id;
unsigned int tx_channel_id;

// In low-frequency mode, the maximum EUSART baud rate is 9600
uint32_t baudrate = 9600;

// LDMA descriptor and transfer configuration structures for TX channel
sl_hal_ldma_descriptor_t tx_ldma_descriptor;
sl_hal_ldma_transfer_config_t tx_ldma_transfer_config;

// LDMA descriptor and transfer configuration structures for RX channel
sl_hal_ldma_descriptor_t rx_ldma_descriptor;
sl_hal_ldma_transfer_config_t rx_ldma_transfer_config;

// RX and TX state flags
volatile bool rx_start = false;
volatile bool tx_start = false;

const sl_gpio_t EUSART0_TX  = { .port = EUSART0_TX_PORT,
                                . pin = EUSART0_TX_PIN };

const sl_gpio_t EUSART0_RX  = { .port = EUSART0_RX_PORT,
                                . pin = EUSART0_RX_PIN };

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
  uint32_t freq;

  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_EUSART0);
  // Initialize the EUSART0 module
  sl_hal_eusart_uart_init_t init = SL_HAL_EUSART_UART_INIT_DEFAULT_LF;
  sl_hal_eusart_uart_advanced_init_t advance_init = SL_HAL_EUSART_UART_ADVANCED_INIT_DEFAULT;
  init.advanced_config = &advance_init;
  init.advanced_config->dma_halt_on_error = true;
  // Set FIFO watermark lvl; wake when ready to transfer from RX FIFO to memory
  advance_init.rx_fifo_watermark = (sl_hal_eusart_fifo_interrupt_watermark_t)((BUFLEN - 1) << _EUSART_CFG1_RXFIW_SHIFT);

  // Configure desired baudrate 
  sl_clock_manager_get_clock_branch_frequency(SL_CLOCK_BRANCH_EUSART0CLK, &freq);
  init.clock_div = sl_hal_eusart_uart_calculate_clock_div(freq, baudrate, init.oversampling);

  // Configure EUSART0 for low-frequence operation
  sl_hal_eusart_init_uart_lf(EUSART0, &init);

  // Enable EUSART0
  sl_hal_eusart_enable(EUSART0);
  sl_hal_eusart_enable_rx(EUSART0);
  sl_hal_eusart_enable_tx(EUSART0);

  // Clear and enable transmit complete interrupt
  sl_hal_eusart_clear_interrupts(EUSART0, EUSART_IF_TXC);
  sl_hal_eusart_enable_interrupts(EUSART0, EUSART_IEN_TXC);

  // Enable EUSART Interrupts
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
  sl_hal_eusart_clear_interrupts(EUSART0, EUSART_IF_TXC);
}

/**************************************************************************//**
 * @brief  Initialize LDMA
 *****************************************************************************/
void ldma_init(void)
{
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_LDMA0);
  // Source is buffer, destination is EUSART0_TXDATA, and length is BUFLEN
  tx_ldma_descriptor = (sl_hal_ldma_descriptor_t)SL_HAL_LDMA_DESCRIPTOR_SINGLE_M2P(SL_HAL_LDMA_CTRL_SIZE_BYTE,
                                                                                   buffer,
                                                                                   &(EUSART0->TXDATA),
                                                                                   BUFLEN);
  // Transfer a byte on free space in the EUSART FIFO
  tx_ldma_transfer_config = (sl_hal_ldma_transfer_config_t)SL_HAL_LDMA_TRANSFER_CFG_PERIPHERAL(SL_HAL_LDMA_PERIPHERAL_SIGNAL_EUSART0_TXFL);


  // Source is EUSART0_RXDATA, destination is buffer, and length is BUFLEN
  rx_ldma_descriptor = (sl_hal_ldma_descriptor_t)SL_HAL_LDMA_DESCRIPTOR_SINGLE_P2M(SL_HAL_LDMA_CTRL_SIZE_BYTE,
                                                                                   &(EUSART0->RXDATA),
                                                                                   buffer,
                                                                                   BUFLEN);
  // Transfer a byte on receive FIFO level event
  rx_ldma_transfer_config = (sl_hal_ldma_transfer_config_t)SL_HAL_LDMA_TRANSFER_CFG_PERIPHERAL(SL_HAL_LDMA_PERIPHERAL_SIGNAL_EUSART0_RXFL);

  // Initialize DMADRV
  DMADRV_Init();

  // Halt on error if DMA channel cannot be allocated
  if (DMADRV_AllocateChannel(&rx_channel_id, NULL) != ECODE_EMDRV_DMADRV_OK) {
    __BKPT(0);
  }

  // Halt on error if DMA channel cannot be allocated
  if (DMADRV_AllocateChannel(&tx_channel_id, NULL) != ECODE_EMDRV_DMADRV_OK) {
    __BKPT(0);
  }
}

/**************************************************************************//**
 * @brief LDMA Callback on RX
 *****************************************************************************/
void ldma_callback_rx(void)
{
  tx_start = true;
}

/**************************************************************************//**
 * @brief LDMA Callback on TX
 *****************************************************************************/
void ldma_callback_tx(void)
{
  rx_start = true;
}

/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  gpio_init();

  eusart_init();

  ldma_init();

  rx_start = true;
}

/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{
  // Receive data
  if(rx_start){
    DMADRV_LdmaStartTransfer(rx_channel_id,
                             &rx_ldma_transfer_config,
                             &rx_ldma_descriptor,
                             (DMADRV_Callback_t)ldma_callback_rx,
                             NULL);
    rx_start = false;
  }
  // Send out data
  if(tx_start){
    DMADRV_LdmaStartTransfer(tx_channel_id,
                             &tx_ldma_transfer_config,
                             &tx_ldma_descriptor,
                             (DMADRV_Callback_t)ldma_callback_tx,
                             NULL);
    tx_start = false;
  }
}

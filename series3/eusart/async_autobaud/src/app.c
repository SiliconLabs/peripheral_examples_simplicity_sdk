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
#include "sl_interrupt_manager.h"
#include "sl_power_manager.h"

#include "sl_hal_eusart.h"
#include "sl_gpio.h"

#include "pin_config.h"

/*******************************************************************************
 *******************************   DEFINES   ***********************************
 ******************************************************************************/
// Size of the buffer for received data
#define BUFLEN  80

/*******************************************************************************
 ***************************   GLOBAL VARIABLES   ******************************
 ******************************************************************************/
// Receive data buffer
uint8_t buffer[BUFLEN];

// Current position in buffer
uint32_t in_pos = 0;
uint32_t out_pos = 0;

const sl_gpio_t EUSART0_TX = { .port = EUSART0_TX_PORT,
                               . pin = EUSART0_TX_PIN };

const sl_gpio_t EUSART0_RX = { .port = EUSART0_RX_PORT,
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
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_EUSART0);
  // Initialize the EUSART0 module
  sl_hal_eusart_uart_init_t init = SL_HAL_EUSART_UART_INIT_DEFAULT_HF;
  // Configure EUSART0 for high-frequency (EM0/1) operation
  sl_hal_eusart_init_uart_hf(EUSART0,&init);
  // Configure auto baudrate detection functionality
  EUSART0->CFG0 |= EUSART_CFG0_AUTOBAUDEN;
  
  // Enable EUSART
  sl_hal_eusart_enable(EUSART0);
  sl_hal_eusart_enable_rx(EUSART0);
  sl_hal_eusart_enable_tx(EUSART0);

  // Enable EUSART Interrupts
  sl_interrupt_manager_clear_irq_pending(EUSART0_RX_IRQn);
  sl_interrupt_manager_enable_irq(EUSART0_RX_IRQn);
  sl_interrupt_manager_clear_irq_pending(EUSART0_TX_IRQn);
  sl_interrupt_manager_enable_irq(EUSART0_TX_IRQn);
}

/**************************************************************************//**
 * @brief
 *    The EUSART0 receive interrupt saves incoming characters.
 *****************************************************************************/
void EUSART0_RX_IRQHandler(void)
{
  uint32_t flags;
  flags = sl_hal_eusart_get_pending_interrupts(EUSART0);
  // Get the character just received
  buffer[in_pos] = (uint8_t)EUSART0->RXDATA;
  // Check for autobaud done interrupt
  if(flags & EUSART_IF_AUTOBAUDDONE) {
    // Disable Auto Baud Complete interrupt
    sl_hal_eusart_disable_interrupts(EUSART0, EUSART_IEN_AUTOBAUDDONE);
    // Enable receive FIFO level interrupt
    sl_hal_eusart_enable_interrupts(EUSART0, EUSART_IEN_RXFL);
    // Otherwise buffer characters on RX FIFO interrupt
  } else {
    // Exit loop on new line or buffer full
    if((buffer[in_pos] != '\r') && (in_pos < BUFLEN)) {
      in_pos++;
    } else {
      // Disable receive FIFO level interrupt
      sl_hal_eusart_disable_interrupts(EUSART0, EUSART_IEN_RXFL);
      // Enable transmit FIFO level interrupt
      sl_hal_eusart_enable_interrupts(EUSART0, EUSART_IEN_TXFL);
    }
  }
  /*
   * The EUSART differs from the USART in that explicit clearing of
   * RX interrupt flags is required even after emptying the RX FIFO.
   */
  sl_hal_eusart_clear_interrupts(EUSART0, EUSART_IF_RXFL | EUSART_IF_AUTOBAUDDONE);
}

/**************************************************************************//**
 * @brief
 *    The EUSART0 transmit interrupt outputs characters.
 *****************************************************************************/
void EUSART0_TX_IRQHandler(void)
{
  // Send a previously received character
  if(out_pos < in_pos) {
    EUSART0->TXDATA = buffer[out_pos];
    out_pos++;
    /*
     * The EUSART differs from the USART in that the TX FIFO interrupt
     * flag must be explicitly cleared even after a write to the FIFO.
     */
    sl_hal_eusart_clear_interrupts(EUSART0, EUSART_IF_TXFL);
  } else {
    // Enable receive FIFO level interrupt
    sl_hal_eusart_enable_interrupts(EUSART0, EUSART_IEN_RXFL);

    uint32_t i;
    // Zero out buffer
    for(i = 0; i < BUFLEN; i++) {
      buffer[i] = 0;
    }
    // Reset buffer indices
    in_pos = 0;
    out_pos = 0;
    /*
     * Need to disable the transmit FIFO level interrupt in this IRQ
     * handler when done or it will immediately trigger again upon exit
     * even though there is no data left to send.
     */
    sl_hal_eusart_disable_interrupts(EUSART0, EUSART_IEN_TXFL);
  }
}

/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  gpio_init();

  eusart_init();
  
  // Enable receive FIFO level interrupt
  sl_hal_eusart_enable_interrupts(EUSART0, EUSART_IEN_RXFL);

  uint32_t i;
  // Zero out buffer
  for(i = 0; i < BUFLEN; i++){
    buffer[i] = 0;
  }
}

/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{
}

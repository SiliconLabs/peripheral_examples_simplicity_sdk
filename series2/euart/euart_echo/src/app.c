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
#include "em_gpio.h"
#include "em_eusart.h"

#include "pin_config.h"

// Size of the buffer for received data
#define BUFLEN      80

// EUART baudrate
#define BAUDRATE    115200

// Receive data buffer
uint8_t buffer[BUFLEN];

// Current position in buffer
uint32_t inpos = 0;
uint32_t outpos = 0;

// True while receiving data (waiting for CR or BUFLEN characters)
bool receive = true;

const sl_gpio_t EUART0_TX  = { .port = EUART0_TX_PORT,
                                .pin = EUART0_TX_PIN };
const sl_gpio_t EUART0_RX  = { .port = EUART0_RX_PORT,
                                .pin = EUART0_RX_PIN };

/**************************************************************************//**
 * @brief
 *    GPIO initialization
 *****************************************************************************/
void gpio_init(void)
{
  // Configure the EUSART TX pin
  sl_gpio_set_pin_mode(&EUART0_TX, SL_GPIO_MODE_PUSH_PULL, 1);
  sl_gpio_set_pin_mode(&EUART0_RX, SL_GPIO_MODE_INPUT, 0);

  // Route EUART0 TX and RX to the board controller TX and RX pins
  GPIO->EUARTROUTE->TXROUTE = (EUART0_TX_PORT << _GPIO_EUART_TXROUTE_PORT_SHIFT)
                            | (EUART0_TX_PIN << _GPIO_EUART_TXROUTE_PIN_SHIFT);
  GPIO->EUARTROUTE->RXROUTE = (EUART0_RX_PORT << _GPIO_EUART_TXROUTE_PORT_SHIFT)
                            | (EUART0_RX_PIN << _GPIO_EUART_TXROUTE_PIN_SHIFT);

  // Enable TX signal
  GPIO->EUARTROUTE->ROUTEEN = GPIO_EUART_ROUTEEN_TXPEN;
}

/**************************************************************************//**
 * @brief
 *    EUART initialization
 *****************************************************************************/
void euart0_init(void)
{
  // Enable EUART0 Bus Clock
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_EUART0);

  // Initialize the EUART0 module
  EUSART_UartInit_TypeDef init = EUSART_UART_INIT_DEFAULT_HF;
  init.baudrate = BAUDRATE;
  EUSART_UartInitHf(EUART0, &init);

  // Enable EUART0 IRQs
  sl_interrupt_manager_clear_irq_pending(EUART0_RX_IRQn);
  sl_interrupt_manager_enable_irq(EUART0_RX_IRQn);
  sl_interrupt_manager_clear_irq_pending(EUART0_TX_IRQn);
  sl_interrupt_manager_enable_irq(EUART0_TX_IRQn);
}

/**************************************************************************//**
 * @brief
 *    The EUART0 receive interrupt saves incoming characters.
 *****************************************************************************/
void EUART0_RX_IRQHandler(void)
{
  // Get the character just received
  buffer[inpos] = EUART0->RXDATA;

  // Exit loop on new line or buffer full
  if ((buffer[inpos] != '\r') && (inpos < BUFLEN)) {
    inpos++;
  } else {
    // Disable RX FIFO Level Interrupt
    EUSART_IntDisable(EUART0, EUSART_IEN_RXFLIEN);
    // Enable TX FIFO Level Interrupt
    EUSART_IntEnable(EUART0, EUSART_IEN_TXFLIEN);
  }
  
  // Clear the requesting interrupt before exiting the handler
  EUSART_IntClear(EUART0, EUSART_IF_RXFLIF);
}

/**************************************************************************//**
 * @brief
 *    The EUART0 Transmit Interrupt Handler - Outputs characters
 *****************************************************************************/
void EUART0_TX_IRQHandler(void)
{
  // Send a previously received character
  if (outpos < inpos)
    EUART0->TXDATA = buffer[outpos++];
  else
  /*
   * Need to disable the transmit buffer level interrupt in this IRQ
   * handler when done or it will immediately trigger again upon exit
   * even though there is no data left to send.
   */
  {
    EUSART_IntDisable(EUART0, EUSART_IEN_TXFLIEN); // Disable TX FIFO Level Interrupt
    // Enable receive data valid interrupt
    EUSART_IntEnable(EUART0, EUSART_IEN_RXFLIEN);
    inpos = outpos = 0;
  }
  
  // Clear the requesting interrupt before exiting the handler
  EUSART_IntClear(EUART0, EUSART_IF_TXFLIF);
}

/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  gpio_init();
  euart0_init();

  // Enable receive data valid interrupt
  EUSART_IntEnable(EUART0, EUSART_IEN_RXFLIEN);

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

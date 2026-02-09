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
#include "sl_interrupt_manager.h"
#include "sl_gpio.h"
#include "sl_hal_gpio.h"
#include "pin_config.h"
#include "em_usart.h"
#include "sl_board_control_config.h"

// Size of the buffer for received data
#define BUFLEN  80

uint8_t buffer[BUFLEN];

const sl_gpio_t UART_TX = { .port = EXP_UART_TX_PORT, .pin = EXP_UART_TX_PIN };
const sl_gpio_t UART_RX = { .port = EXP_UART_RX_PORT, .pin = EXP_UART_RX_PIN };
const sl_gpio_t VCOM_ENABLE = { .port = SL_BOARD_ENABLE_VCOM_PORT, .pin = SL_BOARD_ENABLE_VCOM_PIN };

/**************************************************************************//**
 * @brief
 *    GPIO initialization
 *****************************************************************************/
void initGPIO(void)
{
  // GPIO driver initialization.
  sl_gpio_init();

  // Configure the USART TX pin to the board controller as an output
  sl_gpio_set_pin_mode(&UART_TX, SL_GPIO_MODE_PUSH_PULL, true);

  // Configure the USART RX pin to the board controller as an input
  sl_gpio_set_pin_mode(&UART_RX, SL_GPIO_MODE_INPUT, false);

  /*
   * Configure the BCC_ENABLE pin as output and set high.  This enables
   * the virtual COM port (VCOM) connection to the board controller and
   * permits serial port traffic over the debug connection to the host
   * PC.
   *
   * To disable the VCOM connection and use the pins on the kit
   * expansion (EXP) header, comment out the following line.
   */
  sl_gpio_set_pin_mode(&VCOM_ENABLE, SL_GPIO_MODE_PUSH_PULL, true);
}

/**************************************************************************//**
 * @brief
 *    USART0 initialization
 *****************************************************************************/
void initUSART0(void)
{
  // Enable clock to USART0
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_USART0);

  // Default asynchronous initializer (115.2 Kbps, 8N1, no flow control)
  USART_InitAsync_TypeDef init = USART_INITASYNC_DEFAULT;

  // Route USART0 TX and RX to the board controller TX and RX pins
  GPIO->USARTROUTE[0].TXROUTE = (EXP_UART_TX_PORT << _GPIO_USART_TXROUTE_PORT_SHIFT)
            | (EXP_UART_TX_PIN << _GPIO_USART_TXROUTE_PIN_SHIFT);
  GPIO->USARTROUTE[0].RXROUTE = (EXP_UART_RX_PORT << _GPIO_USART_RXROUTE_PORT_SHIFT)
            | (EXP_UART_RX_PIN << _GPIO_USART_RXROUTE_PIN_SHIFT);

  // Enable RX and TX signals now that they have been routed
  GPIO->USARTROUTE[0].ROUTEEN = GPIO_USART_ROUTEEN_RXPEN | GPIO_USART_ROUTEEN_TXPEN;

  // Configure and enable USART0
  USART_InitAsync(USART0, &init);
}

/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  // Initialize GPIO and USART0
  initGPIO();
  initUSART0();
}

/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{
  uint32_t i;
  // Zero out buffer
  for (i = BUFLEN-1; i > 0; --i) {
    buffer[i] = 0;
  }

  // Receive BUFLEN characters unless a new line is received first
  do {
    // Wait for a character
    buffer[i] = USART_Rx(USART0);

    // Exit loop on new line
    if (buffer[i] != '\r') {
      i++;
    }
    else {
      break;
    }
  } while (i < BUFLEN);

  // Output received characters
  for (uint32_t j = 0; j < i; j++) {
    USART_Tx(USART0, buffer[j]);
  }
}

/*******************************************************************************
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
#include "sl_hal_gpio.h"
#include "pin_config.h"
#include "em_usart.h"
#include "sl_board_control_config.h"
#include "sl_power_manager.h"

// Size of the buffer for received data
#define BUFLEN  80

typedef enum {
  STANDBY,
  RECEIVE_COMPLETE,
  EM1
} AppState_t;

AppState_t appState;

uint8_t buffer[BUFLEN];

// Current position ins buffer
uint32_t inpos = 0;
uint32_t outpos = 0;

const sl_gpio_t UART_TX = { .port = EXP_UART_TX_PORT, .pin = EXP_UART_TX_PIN };
const sl_gpio_t UART_RX = { .port = EXP_UART_RX_PORT, .pin = EXP_UART_RX_PIN };
const sl_gpio_t VCOM_ENABLE = { .port = SL_BOARD_ENABLE_VCOM_PORT, .pin = SL_BOARD_ENABLE_VCOM_PIN };

/******************************************************************************
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

/******************************************************************************
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

  // Enable NVIC USART sources
  sl_interrupt_manager_clear_irq_pending(USART0_RX_IRQn);
  sl_interrupt_manager_enable_irq(USART0_RX_IRQn);
  sl_interrupt_manager_clear_irq_pending(USART0_TX_IRQn);
  sl_interrupt_manager_enable_irq(USART0_TX_IRQn);
}

/******************************************************************************
 * @brief
 *    The USART0 receive interrupt saves incoming characters.
 *****************************************************************************/
void USART0_RX_IRQHandler(void)
{
  // Get the character just received
  buffer[inpos] = USART0->RXDATA;
  // Exit loop on new line or buffer full
  if ((buffer[inpos] != '\r') && (inpos < BUFLEN)) {
      inpos++;
      appState = EM1; // Wait in EM1 while receiving to reduce current draw
    }
  else {
      appState = RECEIVE_COMPLETE; // Stop receiving on CR
    }
}

/******************************************************************************
 * @brief
 *    The USART0 transmit interrupt outputs characters.
 *****************************************************************************/
void USART0_TX_IRQHandler(void)
{
  // Send a previously received character
  if (outpos < inpos) {
      USART0->TXDATA = buffer[outpos++];
      appState = EM1; // Wait in EM1 while transmitting to reduce current draw
    }
  else {
  /*
   * Need to disable the transmit buffer level interrupt in this IRQ
   * handler when done or it will immediately trigger again upon exit
   * even though there is no data left to send.
   */
    USART_IntDisable(USART0, USART_IEN_TXBL);
    appState = STANDBY; // Go back into standby when all is sent
  }
}

/*******************************************************************************
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  // Initialize GPIO and USART0
  initGPIO();
  initUSART0();
  appState = STANDBY;
}

/*******************************************************************************
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{
  switch (appState) {
    case STANDBY:
      uint32_t i;

      // Reset buffer indices
      inpos = outpos = 0;

      // Zero out buffer
      for (i = 0; i < BUFLEN; i++) {
        buffer[i] = 0;
      }

      // Enable receive data valid interrupt
      USART_IntEnable(USART0, USART_IEN_RXDATAV);
      break;
    case RECEIVE_COMPLETE:
      // Disable receive data valid interrupt
      USART_IntDisable(USART0, USART_IEN_RXDATAV);

      // Enable transmit buffer level interrupt
      USART_IntEnable(USART0, USART_IEN_TXBL);
      break;
    case EM1:
      /*
       * This state is used to allow the main.c while() loop to go into EM1
       * in order to reduce current draw while the peripheral is running.
       */
      break;
    default:
      break;
  }
}

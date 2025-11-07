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

#include "em_eusart.h"
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
  // Default asynchronous initializer (115.2 Kbps, 8N1, no flow control)
  EUSART_UartInit_TypeDef init = EUSART_UART_INIT_DEFAULT_HF;

  // Configure and enable EUSART0 for high-frequency (EM0/1) operation
  EUSART_UartInitHf(EUSART0, &init);
}

/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  gpio_init();

  eusart_init();
}

/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{
  uint32_t i, j;
  // Zero out buffer
  for(i = BUFLEN - 1; i > 0; --i){
    buffer[i] = 0;
  }
  // Receive BUFLEN characters unless a new line is received first
  do {
    // Wait for a character
    buffer[i] = EUSART_Rx(EUSART0);
    // Exit loop on new line
    if(buffer[i] != '\r'){
      i++;
    } else {
      break;
    }
  } while(i < BUFLEN);
  // Output received characters
  for(j = 0; j <= i; j++){
    EUSART_Tx(EUSART0, buffer[j]);
  }
} 
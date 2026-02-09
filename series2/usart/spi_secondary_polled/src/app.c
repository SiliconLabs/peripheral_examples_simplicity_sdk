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
#include "sl_hal_gpio.h"
#include "pin_config.h"
#include "em_usart.h"
#include "sl_board_control_config.h"

// Size of the data buffers
#define BUFLEN  10

// Outgoing data
uint8_t buffer[BUFLEN];

const sl_gpio_t US0COPI = { .port = EXP_SPI_COPI_PORT, .pin = EXP_SPI_COPI_PIN };
const sl_gpio_t US0CIPO = { .port = EXP_SPI_CIPO_PORT, .pin = EXP_SPI_CIPO_PIN };
const sl_gpio_t US0CLK = { .port = EXP_SPI_SCK_PORT, .pin = EXP_SPI_SCK_PIN };
const sl_gpio_t US0CS = { .port = EXP_SPI_CS_PORT, .pin = EXP_SPI_CS_PIN };

/**************************************************************************//**
 * @brief
 *    GPIO initialization
 *****************************************************************************/
void initGPIO(void)
{
  // Configure COPI pin as an input
  sl_gpio_set_pin_mode(&US0COPI, SL_GPIO_MODE_INPUT, 0);

  // Configure CIPO pin as an output
  sl_gpio_set_pin_mode(&US0CIPO, SL_GPIO_MODE_PUSH_PULL, 0);

  // Configure CLK pin as an input
  sl_gpio_set_pin_mode(&US0CLK, SL_GPIO_MODE_INPUT, 0);

  // Configure CS pin as input
  sl_gpio_set_pin_mode(&US0CS, SL_GPIO_MODE_INPUT, 1);

  /*
   * Configure CS pin for high-to-low edge detection, but don't enable
   * the interrupt.
   */
  sl_hal_gpio_configure_external_interrupt(&US0CS, EXP_SPI_CS_PIN, SL_GPIO_INTERRUPT_FALLING_EDGE);
  sl_hal_gpio_disable_interrupts(EXP_SPI_CS_PIN);
}

/**************************************************************************//**
 * @brief
 *    USART0 initialization
 *****************************************************************************/
void initUSART0(void)
{
  // Enable peripheral clock
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_USART0);

  // Default asynchronous initializer (main mode, 1 Mbps, 8-bit data)
  USART_InitSync_TypeDef init = USART_INITSYNC_DEFAULT;

  init.master = false;  // Operate as a secondary
  init.msbf = true;     // MSB first transmission for SPI compatibility

  /*
   * Route USART0 RX, TX, CLK and CS to the specified pins.
   */
  GPIO->USARTROUTE[0].RXROUTE = (EXP_SPI_CIPO_PORT << _GPIO_USART_RXROUTE_PORT_SHIFT)
      | (EXP_SPI_CIPO_PIN << _GPIO_USART_RXROUTE_PIN_SHIFT);
  GPIO->USARTROUTE[0].TXROUTE = (EXP_SPI_COPI_PORT << _GPIO_USART_TXROUTE_PORT_SHIFT)
      | (EXP_SPI_COPI_PIN << _GPIO_USART_TXROUTE_PIN_SHIFT);
  GPIO->USARTROUTE[0].CLKROUTE = (EXP_SPI_SCK_PORT << _GPIO_USART_CLKROUTE_PORT_SHIFT)
      | (EXP_SPI_SCK_PIN << _GPIO_USART_CLKROUTE_PIN_SHIFT);
  GPIO->USARTROUTE[0].CSROUTE = (EXP_SPI_CS_PORT << _GPIO_USART_CSROUTE_PORT_SHIFT)
      | (EXP_SPI_CS_PIN << _GPIO_USART_CSROUTE_PIN_SHIFT);

  // Enable USART interface pins
  GPIO->USARTROUTE[0].ROUTEEN = GPIO_USART_ROUTEEN_RXPEN  |    // CIPO
                                GPIO_USART_ROUTEEN_TXPEN  |    // COPI
                                GPIO_USART_ROUTEEN_CLKPEN |
                                GPIO_USART_ROUTEEN_CSPEN;

  // Configure and enable USART0
  USART_InitSync(USART0, &init);
}


/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  uint32_t i;
  // Zero buffer
  for (i = 0; i < BUFLEN; i++) {
      buffer[i] = 0;
  }

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

  // Clear the flag and wait for a falling edge on US0CS_PIN
  sl_hal_gpio_clear_interrupts(1 << EXP_SPI_CS_PIN);

  while ((sl_hal_gpio_get_pending_interrupts() & (1 << EXP_SPI_CS_PIN)) == 0);

  /*
   * After getting the falling edge, clear the RX buffer before
   * receiving the desired number of bytes.
   */
  USART0->CMD = USART_CMD_CLEARRX;

  /*
   * Perform the specified number of single byte SPI transfers
   * (transmission and reception).
   *
   * Data to be queued for transmission is sent with USART_Tx(), which
   * checks that there is room in the transmit FIFO but does not
   * actually wait for transmit complete (USART_STATUS_TXC).
   *
   * Instead, incoming data is read from the FIFO with USART_Rx(),
   * which polls USART_STATUS,RXDATAV to make sure data has arrived.
   */
  for (i = 0; i < BUFLEN; i++) {
    USART_Tx(USART0, buffer[i]);
    buffer[i] = USART_Rx(USART0);
  }
}

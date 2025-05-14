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
#include "sl_hal_gpio.h"
#include "pin_config.h"
#include "em_usart.h"
#include "sl_board_control_config.h"
#include "sl_power_manager.h"

#include "sl_udelay.h"

#ifndef BUTTON0_PORT
  #define BUTTON0_PORT LED0_BUTTON0_PORT
  #define BUTTON0_PIN LED0_BUTTON0_PIN
#endif

// Size of the data buffers
#define BUFLEN  10

typedef enum {
  INIT,
  RECEIVE,
  START_SEND,
  SEND_DONE
} AppState_t;

AppState_t appState;

// Outgoing data
uint8_t outbuf[BUFLEN];

// Incoming data
uint8_t inbuf[BUFLEN];

// Position in the buffer
uint32_t bufpos;

const sl_gpio_t US0COPI = { .port = EXP_SPI_COPI_PORT, .pin = EXP_SPI_COPI_PIN };
const sl_gpio_t US0CIPO = { .port = EXP_SPI_CIPO_PORT, .pin = EXP_SPI_CIPO_PIN };
const sl_gpio_t US0CLK = { .port = EXP_SPI_SCK_PORT, .pin = EXP_SPI_SCK_PIN };
const sl_gpio_t US0CS = { .port = EXP_SPI_CS_PORT, .pin = EXP_SPI_CS_PIN };
const sl_gpio_t BUTTON0 = { .port = BUTTON0_PORT, .pin = BUTTON0_PIN };

/**************************************************************************//**
 * @brief GPIO IRQHandler
 *****************************************************************************/
#if (BUTTON0_PIN & 1)
void gpio_odd_irq_callback(uint8_t irq_idx, void* context)
#else
void gpio_even_irq_callback(uint8_t irq_idx, void* context)
#endif
{
  appState = START_SEND;
  // Eliminate "unused parameter" warnings
  (void)context;
  (void)irq_idx;
}

/**************************************************************************//**
 * @brief
 *    GPIO initialization
 *****************************************************************************/
void initGPIO(void)
{
  int32_t interrupt_number = BUTTON0_PIN;

  // Configure MOSI pin as an output
  sl_gpio_set_pin_mode(&US0COPI, SL_GPIO_MODE_PUSH_PULL, 0);

  // Configure MISO pin as an input
  sl_gpio_set_pin_mode(&US0CIPO, SL_GPIO_MODE_INPUT, 0);

  // Configure CLK pin as an output
  sl_gpio_set_pin_mode(&US0CLK, SL_GPIO_MODE_PUSH_PULL, 0);

  // Configure CS pin as an output initially high
  sl_gpio_set_pin_mode(&US0CS, SL_GPIO_MODE_PUSH_PULL, 1);

  // Configure button 0 pin as an input
  sl_gpio_set_pin_mode(&BUTTON0, SL_GPIO_MODE_INPUT_PULL, 1);

#if (BUTTON0_PIN & 1)
  // Interrupt on button 0 rising edge to start transfers
  sl_gpio_configure_external_interrupt(&BUTTON0, &interrupt_number, SL_GPIO_INTERRUPT_RISING_EDGE, &gpio_odd_irq_callback, (void*)0);

  // Enable NVIC GPIO interrupt
  sl_interrupt_manager_clear_irq_pending(GPIO_ODD_IRQn);
  sl_interrupt_manager_enable_irq(GPIO_ODD_IRQn);
#else
  // Interrupt on button 0 rising edge to start transfers
  sl_gpio_configure_external_interrupt(&BUTTON0, &interrupt_number, SL_GPIO_INTERRUPT_RISING_EDGE, &gpio_even_irq_callback, (void*)0);

  // Enable NVIC GPIO interrupt
  sl_interrupt_manager_clear_irq_pending(GPIO_EVEN_IRQn);
  sl_interrupt_manager_enable_irq(GPIO_EVEN_IRQn);
#endif
}

/**************************************************************************//**
 * @brief
 *    USART0 initialization
 *****************************************************************************/
void initUSART0(void)
{
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_USART0);

  // Default synchronous initializer (main mode, 1 Mbps, 8-bit data)
  USART_InitSync_TypeDef init = USART_INITSYNC_DEFAULT;

  init.msbf = true;     // MSB first transmission for SPI compatibility

  // Configure and enable USART0
  USART_InitSync(USART0, &init);

  /*
   * Route USART0 RX, TX, and CLK to the specified pins.  Note that CS
   * is not controlled by the USART in this case but as a GPIO under
   * software control in the main loop.
   */
  GPIO->USARTROUTE[0].TXROUTE = (EXP_SPI_COPI_PORT << _GPIO_USART_TXROUTE_PORT_SHIFT)
      | (EXP_SPI_COPI_PIN << _GPIO_USART_TXROUTE_PIN_SHIFT);
  GPIO->USARTROUTE[0].RXROUTE = (EXP_SPI_CIPO_PORT << _GPIO_USART_RXROUTE_PORT_SHIFT)
      | (EXP_SPI_CIPO_PIN << _GPIO_USART_RXROUTE_PIN_SHIFT);
  GPIO->USARTROUTE[0].CLKROUTE = (EXP_SPI_SCK_PORT << _GPIO_USART_CLKROUTE_PORT_SHIFT)
      | (EXP_SPI_SCK_PIN << _GPIO_USART_CLKROUTE_PIN_SHIFT);

  // Enable USART interface pins
  GPIO->USARTROUTE[0].ROUTEEN = GPIO_USART_ROUTEEN_RXPEN  |    // MISO
                                GPIO_USART_ROUTEEN_TXPEN  |    // MOSI
                                GPIO_USART_ROUTEEN_CLKPEN;

  // Enable NVIC USART sources
  sl_interrupt_manager_clear_irq_pending(USART0_TX_IRQn);
  sl_interrupt_manager_enable_irq(USART0_TX_IRQn);

  // Enable transmit complete interrupt
  USART_IntEnable(USART0, USART_IEN_TXC);
}

/**************************************************************************//**
 * @brief
 *    USART0 transmit interrupt handler
 *****************************************************************************/
void USART0_TX_IRQHandler(void)
{
  /*
   * Save byte received concurrent with the transmission of the last bit of
   * the previous outgoing byte, and increment the buffer position to the
   * next byte.
   */
  inbuf[bufpos++] = USART0->RXDATA;

  // If there are still bytes left to send, transmit the next one
  if (bufpos < BUFLEN) {
    USART0->TXDATA = outbuf[bufpos];
  }
  else {
      appState = SEND_DONE;
  }

  // Clear the requesting interrupt before exiting the handler
  USART_IntClear(USART0, USART_IF_TXC);
}

/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  initGPIO();
  initUSART0();

  appState = INIT;
}

/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{
  switch (appState) {
    case INIT:
    case RECEIVE:
      /*
       * This state is used to allow the main.c while() loop to go into EM1
       * in order to reduce current draw while the peripheral is running.
       */
      break;
    case SEND_DONE:
      // De-assert chip select upon transfer completion (drive high)
      sl_gpio_set_pin(&US0CS);

      /*
       * Insert a short delay after CS de-assertion.  When this code is
       * running on a faster main (e.g. either higher clock rate or a
       * faster CPU such as a Cortex-M3), the secondary implementation
       * running on a slower device (e.g. either lower clock rate or a
       * slower CPU such as a Cortex-M0+) needs extra time to prepare
       * the input and output buffers for the next round of bytes to be
       * transferred.
       */
      sl_udelay_wait(15); // wait 15 us
      /*
       * Fall through is intentional in order to continue with
       * the next frame after sending has been completed
       */
    case START_SEND:
      uint32_t i;
      // Zero incoming buffer and populate outgoing data array
      for (i = 0; i < BUFLEN; i++) {
        inbuf[i] = 0;
        outbuf[i] = (uint8_t)i;
      }

      // Start at the beginning of the buffer
      bufpos = 0;

      // Assert chip select (drive low)
      sl_gpio_clear_pin(&US0CS);

      /*
       * Because this example is most likely going to be running with
       * another EFM32/EFR32 device on the secondary side, it must insert
       * a delay between chip select assertion and sending the first
       * byte.
       *
       * On Series 1 and Series 2 EFM32/EFR32 devices, this delay needs
       * to be between 7 and 10 us in order for the downstream firmware
       * to enable SPI reception and pre-load the first byte to be
       * transmitted.
       *
       * Similar delays are not uncommon for things like high-precision
       * delta-sigma A-to-D converters where the falling chip select
       * wakes the device from a low-power state, starts a conversion,
       * and can return data after some set delay.
       */
      sl_udelay_wait(15); // wait 15 us

      /*
       * Transmit the first byte, then go into EM1.  The IRQ handler will
       * receive each incoming byte and transmit the next outgoing byte.
       */
      USART0->TXDATA = outbuf[bufpos];
      appState = RECEIVE;
      break;
  }
}

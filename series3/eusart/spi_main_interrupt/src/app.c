/***************************************************************************//**
 * @file app.c
 *
 * @brief This project demonstrates interrupt-driven use of the EUSART in
 * synchronous (SPI) main (formerly master) mode. The main loop transmits the
 * specified number of bytes and receives the byte that is shifted in with
 * each outgoing one.
 *
 * The pins used in this example are defined below and are described in the
 * accompanying readme.txt file.
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 *******************************************************************************
 * # Evaluation Quality
 * This code has been minimally tested to ensure that it builds and is suitable
 * as a demonstration for evaluation purposes only. This code will be maintained
 * at the sole discretion of Silicon Labs.
 ******************************************************************************/

#include "sl_clock_manager.h"
#include "sl_gpio.h"
#include "sl_hal_eusart.h"
#include "sl_hal_gpio.h"
#include "sl_udelay.h"

#include "pin_config.h"

// Size of the data buffers
#define BUFLEN  64
#define RXFIW_VAL 16
#define TXFIW_VAL 16

// SPI ports and pins
const sl_gpio_t GPIO_EUS0MOSI = {.port = EUS0MOSI_PORT,
                             .pin = EUS0MOSI_PIN};
const sl_gpio_t GPIO_EUS0MISO = {.port = EUS0MISO_PORT,
                             .pin = EUS0MISO_PIN};
const sl_gpio_t GPIO_EUS0SCLK = {.port = EUS0SCLK_PORT,
                             .pin = EUS0SCLK_PIN};
const sl_gpio_t GPIO_EUS0CS = {.port = EUS0CS_PORT,
                             .pin = EUS0CS_PIN};

const sl_gpio_t GPIO_BUTTON0 = {.port = BUTTON0_PORT, .pin = BUTTON0_PIN};

typedef enum {
  IDLE,
  RX_COMPLETE,
  TX_COMPLETE,
  SEND
} AppState_t;

/*******************************************************************************
 ***************************   GLOBAL VARIABLES   ******************************
 ******************************************************************************/

volatile AppState_t app_state;

// Outgoing data
uint8_t outbuf[BUFLEN];

// Incoming data
volatile uint8_t inbuf[BUFLEN];

// Position in each buffer
volatile uint32_t bufpos_rx, bufpos_tx;

void EUSART0_RX_Handler(void);
void EUSART0_TX_Handler(void);

/**************************************************************************//**
 * @brief GPIO IRQHandler
 *****************************************************************************/
void gpio_button0_callback(void)
{
  // Start transmission
  app_state = SEND;
}

/**************************************************************************//**
 * @brief
 *    GPIO initialization
 *****************************************************************************/
void gpio_init(void)
{
  int32_t int_no = GPIO_BUTTON0.pin;

  // Initialize GPIO driver
  sl_gpio_init();

  // Configure MOSI (TX) pin as an output
  sl_gpio_set_pin_mode(&GPIO_EUS0MOSI, SL_GPIO_MODE_PUSH_PULL, 0);

  // Configure MISO (RX) pin as an input
  sl_gpio_set_pin_mode(&GPIO_EUS0MISO, SL_GPIO_MODE_INPUT, 0);

  // Configure SCLK pin as an output low (CPOL = 0)
  sl_gpio_set_pin_mode(&GPIO_EUS0SCLK, SL_GPIO_MODE_PUSH_PULL, 0);

  // Configure CS pin as an output initially high
  sl_gpio_set_pin_mode(&GPIO_EUS0CS, SL_GPIO_MODE_PUSH_PULL, 1);

  // Configure button 0 pin as an input
  sl_gpio_set_pin_mode(&GPIO_BUTTON0, SL_GPIO_MODE_INPUT_PULL, 1);

  // Interrupt on button 0 rising edge to start transfers
  sl_gpio_configure_external_interrupt(&GPIO_BUTTON0,
                                         &int_no,
                                         SL_GPIO_INTERRUPT_RISING_EDGE,
                                         (sl_gpio_irq_callback_t)gpio_button0_callback,
                                         NULL);
}

/**************************************************************************//**
 * @brief
 *    EUSART0 initialization
 *****************************************************************************/
void eusart0_init(void)
{
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_EUSART0);

  // SPI advanced configuration (part of the initializer)
  sl_hal_eusart_spi_advanced_config_t adv = SL_HAL_EUSART_SPI_ADVANCED_INIT_DEFAULT;

  adv.msb_first = true;        // SPI standard MSB first
  adv.auto_cs_enable = false;  // Turn off auto CS since it is manually changed by TX

  // Default asynchronous initializer (main/master mode and 8-bit data)
  sl_hal_eusart_spi_config_t init = SL_HAL_EUSART_SPI_MASTER_INIT_DEFAULT_HF;

  // Calculate the clock divider from clock frequency for 1000000 baud rate
  uint32_t ref_freq;
  sl_clock_manager_get_clock_branch_frequency(SL_CLOCK_BRANCH_EUSART0CLK, &ref_freq);
  init.clock_div = sl_hal_eusart_spi_calculate_clock_div(ref_freq, 1000000);

  init.advanced_config = &adv;  // Advanced settings structure

  /*
   * Route EUSART0 MOSI, MISO, and SCLK to the specified pins.  CS is
   * not controlled by EUSART0 so there is no write to the corresponding
   * EUSARTROUTE register to do this.
   */
  GPIO->EUSARTROUTE[0].TXROUTE = (EUS0MOSI_PORT << _GPIO_EUSART_TXROUTE_PORT_SHIFT)
                               | (EUS0MOSI_PIN << _GPIO_EUSART_TXROUTE_PIN_SHIFT);
  GPIO->EUSARTROUTE[0].RXROUTE = (EUS0MISO_PORT << _GPIO_EUSART_RXROUTE_PORT_SHIFT)
                               | (EUS0MISO_PIN << _GPIO_EUSART_RXROUTE_PIN_SHIFT);
  GPIO->EUSARTROUTE[0].SCLKROUTE = (EUS0SCLK_PORT << _GPIO_EUSART_SCLKROUTE_PORT_SHIFT)
                                 | (EUS0SCLK_PIN << _GPIO_EUSART_SCLKROUTE_PIN_SHIFT);

  // Enable EUSART interface pins
  GPIO->EUSARTROUTE[0].ROUTEEN = GPIO_EUSART_ROUTEEN_RXPEN |    // MISO
                                 GPIO_EUSART_ROUTEEN_TXPEN |    // MOSI
                                 GPIO_EUSART_ROUTEEN_SCLKPEN;

  // Configure and enable EUSART0
  sl_hal_eusart_init_spi(EUSART0, &init);

  // Configure RXFL and TXFL for 16 frames to optimize time in EM1
  EUSART0->CFG1_SET = (TXFIW_VAL-1) << _EUSART_CFG1_TXFIW_SHIFT;
  EUSART0->CFG1_SET = (RXFIW_VAL-1) << _EUSART_CFG1_RXFIW_SHIFT;

  // Enable EUSART
  sl_hal_eusart_enable(EUSART0);
  sl_hal_eusart_enable_tx(EUSART0);
  sl_hal_eusart_enable_rx(EUSART0);

  // Set and enable EUSART RX interrupt handlers
  sl_interrupt_manager_set_irq_handler(EUSART0_RX_IRQn, EUSART0_RX_Handler);
  sl_interrupt_manager_clear_irq_pending(EUSART0_RX_IRQn);
  sl_interrupt_manager_enable_irq(EUSART0_RX_IRQn);

  // Set and enable EUSART TX interrupt handlers
  sl_interrupt_manager_set_irq_handler(EUSART0_TX_IRQn, EUSART0_TX_Handler);
  sl_interrupt_manager_clear_irq_pending(EUSART0_TX_IRQn);
  sl_interrupt_manager_enable_irq(EUSART0_TX_IRQn);
}

/**************************************************************************//**
 * @brief
 *    EUSART0 receive interrupt handler
 *****************************************************************************/
__attribute__((section("text_application_ram")))
void EUSART0_RX_Handler(void)
{
  // Fill incoming data array with RX buffer while there is still data left
  if(bufpos_rx < BUFLEN){
    for(int i = 0; i < RXFIW_VAL; i++)
      inbuf[bufpos_rx++] = EUSART0->RXDATA;
  }

  // Receive is complete when the buffer position reaches the length of the transmitted data
  if (bufpos_rx == BUFLEN){
    app_state = RX_COMPLETE;

    // Disable the requesting interrupt before exiting the handler
    sl_hal_eusart_disable_interrupts(EUSART0, EUSART_IF_RXFL);

    // De-assert chip select upon final byte being received (drive high)
    sl_gpio_set_pin(&GPIO_EUS0CS);
  }

  // Clear the requesting interrupt before exiting the handler
  sl_hal_eusart_clear_interrupts(EUSART0, EUSART_IF_RXFL);
  sl_interrupt_manager_clear_irq_pending(EUSART0_RX_IRQn);
}

/**************************************************************************//**
 * @brief
 *    EUSART0 transmit interrupt handler
 *****************************************************************************/
__attribute__((section("text_application_ram")))
void EUSART0_TX_Handler(void)
{
  // Fill TX buffer with output array data while there is still data left
  if(EUSART0->IEN & EUSART_IEN_TXFL){
    if((EUSART0->IF & EUSART_IF_TXFL) && bufpos_tx < BUFLEN){
      EUSART0->TXDATA = outbuf[bufpos_tx++];
    }

    if(bufpos_tx == BUFLEN){
      // Change interrupt source from TXFL to TXC on final transmitted byte
      sl_hal_eusart_disable_interrupts(EUSART0, EUSART_IEN_TXFL);
      sl_hal_eusart_enable_interrupts(EUSART0, EUSART_IEN_TXC);
    }

    // Clear the requesting interrupt before exiting the handler
    sl_hal_eusart_clear_interrupts(EUSART0, EUSART_IF_TXFL);
  }
  else{
    app_state = TX_COMPLETE;

    // Clear and disable the requesting interrupt before exiting the handler
    sl_hal_eusart_disable_interrupts(EUSART0, EUSART_IF_TXC);
  }
}

/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  // Initialize GPIO and EUSART0
  gpio_init();
  eusart0_init();

  app_state = IDLE;

  // Fill TX buffer with values to be transmitted
  for (int i = 0; i < BUFLEN; i++)
  {
    outbuf[i] = (uint8_t)i;
  }
}

/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{
  switch (app_state){
    case IDLE:
      /*
       * This state is used to allow the main.c while() loop to go into EM1
       * in order to reduce current draw while the peripheral is running.
       */
      break;

    case RX_COMPLETE:
      /*
       * Insert a short delay after CS de-assertion.  When this code is
       * running on a faster main (e.g. either higher clock rate or a
       * faster CPU such as a Cortex-M3), the secondary implementation
       * running on a slower device (e.g. either lower clock rate or a
       * slower CPU such as a Cortex-M0+) needs extra time to prepare
       * the input and output buffers for the next round of bytes to be
       * transferred.
       */

      sl_udelay_wait(15);

      break;

    case TX_COMPLETE:
      /*
       * This state is to let the user know when TX has completed
       */
      break;

    case SEND:
      uint32_t i;

      // Zero incoming buffer and populate outgoing data array
      for (i = 0; i < BUFLEN; i++)
      {
        inbuf[i] = 0;
        outbuf[i] = (uint8_t)i;
      }

      // Start at the beginning of the buffer
      bufpos_tx = 0;
      bufpos_rx = 0;

      // Assert chip select (drive low)
      sl_gpio_clear_pin(&GPIO_EUS0CS);

      /*
       * Because this example is most likely going to be running with
       * another EFM32/EFR32 device on the secondary side, it must
       * insert a delay between chip select assertion and sending the
       * first byte.
       *
       * On Series 3 devices, this delay needs to be between 20 and 35 us
       * in order to accommodate for the slower memory access.
       */
      sl_udelay_wait(35);

      // Enable RXFL and TXFL interrupts to start transmitting and receiving
      sl_hal_eusart_enable_interrupts(EUSART0, EUSART_IEN_TXFL);
      sl_hal_eusart_enable_interrupts(EUSART0, EUSART_IEN_RXFL);

      app_state = IDLE;
      break;
    }
}

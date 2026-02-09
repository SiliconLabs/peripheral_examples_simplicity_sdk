/***************************************************************************//**
 * @file app.c
 *
 * @brief This project demonstrates interrupt-driven use of the EUSART in
 * synchronous (SPI) secondary (formerly slave) mode.  The main loop
 * transmits the specified number of bytes and receives the byte that is
 * shifted in by the main/master with each outgoing one.
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
#include "sl_hal_gpio.h"
#include "sl_hal_eusart.h"
#include "sl_hal_gpio.h"

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

/*
 * The TIMEPORT/TIMEPIN is not part of the SPI bus.  It shows when the
 * CPU responds to the main before, during, and after data transfer.
 * Use a logic analyzer to capture the activity on this pin along with
 * the bus traffic to understand the timing relationship between the
 * CPU and the SPI during interrupt-driven transfers.
 */
const sl_gpio_t TIME = { .port = SPI_TIME_PORT, .pin = SPI_TIME_PIN };

typedef enum {
  INIT,
  RECEIVE,
  RX_COMPLETE,
  TX_COMPLETE
} AppState_t;

/*******************************************************************************
 ***************************   GLOBAL VARIABLES   ******************************
 ******************************************************************************/

volatile AppState_t app_state;

// Outgoing data
uint8_t outbuf[BUFLEN];

// Incoming data
volatile uint8_t inbuf[BUFLEN];

// Position in the buffer
volatile uint32_t bufpos_tx, bufpos_rx;

void EUSART0_RX_Handler(void);
void EUSART0_TX_Handler(void);

/**************************************************************************//**
 * @brief GPIO IRQHandler
 *****************************************************************************/
void gpio_cs_callback(void)
{
  // Prevent re-entrancy while this frame runs
  sl_gpio_disable_interrupts(1 << EUS0CS_PIN);

  sl_gpio_set_pin(&TIME);

  // Start transmission
  sl_hal_eusart_enable(EUSART0);
  sl_hal_eusart_enable_tx(EUSART0);
  sl_hal_eusart_enable_rx(EUSART0);
  sl_hal_eusart_enable_interrupts(EUSART0, EUSART_IEN_RXFL);
  sl_hal_eusart_enable_interrupts(EUSART0, EUSART_IEN_TXFL);

  app_state = RECEIVE;

  // Drive the activity pin low when ready to receive data
  sl_gpio_clear_pin(&TIME);
}

/**************************************************************************//**
 * @brief
 *    GPIO initialization
 *****************************************************************************/
void gpio_init(void)
{
  int32_t int_no = GPIO_EUS0CS.pin;

  // Initialize GPIO driver
  sl_gpio_init();

  // Configure MOSI (TX) pin as an input
  sl_gpio_set_pin_mode(&GPIO_EUS0MOSI, SL_GPIO_MODE_INPUT, 0);

  // Configure MISO (RX) pin as an output
  sl_gpio_set_pin_mode(&GPIO_EUS0MISO, SL_GPIO_MODE_PUSH_PULL, 0);

  // Configure SCLK pin as an input
  sl_gpio_set_pin_mode(&GPIO_EUS0SCLK, SL_GPIO_MODE_INPUT, 0);

  // Configure CS pin as an input pulled high
  sl_gpio_set_pin_mode(&GPIO_EUS0CS, SL_GPIO_MODE_INPUT, 1);

  // Generate an interrupt on a CS pin high-to-low transition.
  sl_gpio_configure_external_interrupt(&GPIO_EUS0CS,
                                         &int_no,
                                         SL_GPIO_INTERRUPT_FALLING_EDGE,
                                         (sl_gpio_irq_callback_t)gpio_cs_callback,
                                         NULL);

  // Enable the activity pin
  sl_gpio_set_pin_mode(&TIME, SL_GPIO_MODE_PUSH_PULL, 0);
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

  // Default asynchronous initializer (secondary/slave mode and 8-bit data)
  sl_hal_eusart_spi_config_t init = SL_HAL_EUSART_SPI_SLAVE_INIT_DEFAULT_HF;

  init.advanced_config = &adv;   // Advanced settings structure

  // Route EUSART0RX, TX, SCLK, and CS to the specified pins.
  GPIO->EUSARTROUTE[0].RXROUTE = (EUS0MISO_PORT << _GPIO_EUSART_RXROUTE_PORT_SHIFT)
                               | (EUS0MISO_PIN << _GPIO_EUSART_RXROUTE_PIN_SHIFT);
  GPIO->EUSARTROUTE[0].TXROUTE = (EUS0MOSI_PORT << _GPIO_EUSART_TXROUTE_PORT_SHIFT)
                               | (EUS0MOSI_PIN << _GPIO_EUSART_TXROUTE_PIN_SHIFT);
  GPIO->EUSARTROUTE[0].SCLKROUTE = (EUS0SCLK_PORT << _GPIO_EUSART_SCLKROUTE_PORT_SHIFT)
                               | (EUS0SCLK_PIN << _GPIO_EUSART_SCLKROUTE_PIN_SHIFT);
  GPIO->EUSARTROUTE[0].CSROUTE = (EUS0CS_PORT << _GPIO_EUSART_CSROUTE_PORT_SHIFT)
                               | (EUS0CS_PIN << _GPIO_EUSART_CSROUTE_PIN_SHIFT);

  // Enable EUSART interface pins
  GPIO->EUSARTROUTE[0].ROUTEEN = GPIO_EUSART_ROUTEEN_RXPEN   |
                                 GPIO_EUSART_ROUTEEN_TXPEN   |
                                 GPIO_EUSART_ROUTEEN_SCLKPEN |
                                 GPIO_EUSART_ROUTEEN_CSPEN;

  // Configure and enable EUSART0
  sl_hal_eusart_init_spi(EUSART0, &init);

  /*
   * In polled mode, there should be a delay of reasonable length
   * between chip select assertion and when the main device begins
   * clocking the secondary.  This delay is needed to allow the
   * secondary to recognize the CSn falling edge in order to exit a
   * low-power mode (especially EM2/3) and to prepare data for
   * transmission back to the main.
   *
   * Unfortunately, the EUSART default behavior assumes that an
   * extended delay means that the secondary should transmit a default
   * frame back to the main, which is probably unnecessary.
   *
   * To avoid this, the FORCELOAD bit the in the CFG2 register must
   * be set, but this requires disabling the EUSART because the sl_hal
   * advanced initializer for SPI mode does not include an option to
   * set this (even though it includes a way to set the default frame).
   */
  EUSART0->EN_CLR = EUSART_EN_EN;

  // Wait while the EUSART disables itself
  while ((EUSART0->EN & EUSART_EN_DISABLING));

  // Configure RXFL and TXFL for 16 frames to optimize time in EM1
  EUSART0->CFG1_SET = (TXFIW_VAL-1) << _EUSART_CFG1_TXFIW_SHIFT;
  EUSART0->CFG1_SET = (RXFIW_VAL-1) << _EUSART_CFG1_RXFIW_SHIFT;

  // Set FORCELOAD, then re-enable
  EUSART0->CFG2_SET = EUSART_CFG2_FORCELOAD;
  EUSART0->EN_SET = EUSART_EN_EN;

  // Set the EUSART interrupt handlers
  sl_interrupt_manager_set_irq_handler(EUSART0_RX_IRQn, EUSART0_RX_Handler);
  sl_interrupt_manager_set_irq_handler(EUSART0_TX_IRQn, EUSART0_TX_Handler);

  // Enable NVIC EUSART source
  sl_interrupt_manager_clear_irq_pending(EUSART0_TX_IRQn);
  sl_interrupt_manager_enable_irq(EUSART0_TX_IRQn);
  sl_interrupt_manager_clear_irq_pending(EUSART0_RX_IRQn);
  sl_interrupt_manager_enable_irq(EUSART0_RX_IRQn);
}

/**************************************************************************//**
 * @brief
 *    EUSART0 transmit interrupt handler
 *****************************************************************************/
__attribute__((section("text_application_ram")))
void EUSART0_TX_Handler(void)
{
  // Fill TX buffer with output array data while there is still data left
  if(bufpos_tx < BUFLEN)
    EUSART0->TXDATA = outbuf[bufpos_tx++];
  else{
    // Disable TXFL interrupt after data transfer is done
    sl_hal_eusart_disable_interrupts(EUSART0, EUSART_IEN_TXFL);
    app_state = TX_COMPLETE;
  }

  // Clear the requesting interrupt before exiting the handler
  sl_hal_eusart_clear_interrupts(EUSART0, EUSART_IF_TXFL);
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

  // Receive is complete when the bufpos reaches the length of the transmitted data
  if(bufpos_rx == BUFLEN){
    // Disable the falling edge interrupt on the EUS0CS_PIN
    sl_gpio_disable_interrupts(1 << EUS0CS_PIN);

    // Disable RXFL interrupt after data transfer is done
    sl_hal_eusart_disable_interrupts(EUSART0, EUSART_IEN_RXFL);

    app_state = RX_COMPLETE;
  }

  // Clear the requesting interrupt before exiting the handler
  sl_hal_eusart_clear_interrupts(EUSART0, EUSART_IF_RXFL);
  sl_interrupt_manager_clear_irq_pending(EUSART0_RX_IRQn);

  // Drive the activity pin low to denote IRQ handler exit
  sl_gpio_clear_pin(&TIME);
}

/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  // Initialize GPIO and EUSART0
  gpio_init();
  eusart0_init();
  app_state = INIT;
}

/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{
  switch (app_state) {
    case TX_COMPLETE:
      // Drive the activity pin high to show prep for next data transfer
      sl_gpio_set_pin(&TIME);
      break;

    case INIT:
      uint32_t i;

      // Zero incoming buffer and populate outgoing data array
      for (i = 0; i < BUFLEN; i++){
        inbuf[i] = 0;
        outbuf[i] = (uint8_t)i;
      }

    case RX_COMPLETE:

      // Start at the beginning of the buffer
      bufpos_rx = 0;
      bufpos_tx = 0;

      sl_hal_gpio_clear_interrupts(1 << EUS0CS_PIN);
      sl_gpio_enable_interrupts(1 << EUS0CS_PIN);

      // Drive the activity pin low when ready for CS assertion
      sl_gpio_clear_pin(&TIME);
      break;

    case RECEIVE:
      break;

    default:
      break;
  }
}

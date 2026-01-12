/***************************************************************************//**
 * @file app.c
 *
 * @brief This project demonstrates DMA-driven use of the EUSART in synchronous
 * (SPI) main mode. The main loop starts the LDMA channels, which transmit the
 * specified number of bytes and receive the byte that is shifted in with each
 * outgoing one.
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
#include "dmadrv.h"
#include "sl_gpio.h"
#include "sl_hal_eusart.h"
#include "sl_hal_gpio.h"
#include "sl_hal_ldma.h"

#include "pin_config.h"

// Size of the data buffers
#define BUFLEN  10

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
  SEND
} AppState_t;

/*******************************************************************************
 ***************************   GLOBAL VARIABLES   ******************************
 ******************************************************************************/

volatile AppState_t app_state;

// LDMA descriptor and transfer configuration structures for TX and RX channels
sl_hal_ldma_descriptor_t ldma_tx_descriptor, ldma_rx_descriptor;
sl_hal_ldma_transfer_config_t ldma_tx_config, ldma_rx_config;
unsigned int tx_channel_id, rx_channel_id;

// Outgoing data
uint8_t outbuf[BUFLEN];

// Incoming data
uint8_t inbuf[BUFLEN];

/**************************************************************************//**
 * @brief  LDMA RX IRQ Callback
 *****************************************************************************/
void ldma_rx_callback(void)
{
  // End of receive
  app_state = RX_COMPLETE;
}

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
  adv.auto_inter_frame_time = 7; // 7 bit times of delay between frames
                              // to accommodate non-DMA secondaries

  // Default asynchronous initializer (main/master mode and 8-bit data)
  sl_hal_eusart_spi_config_t init = SL_HAL_EUSART_SPI_MASTER_INIT_DEFAULT_HF;

  // Calculate the clock divider from clock frequency for 1000000 baud rate
  uint32_t ref_freq;
  sl_clock_manager_get_clock_branch_frequency(SL_CLOCK_BRANCH_EUSART0CLK, &ref_freq);
  init.clock_div = sl_hal_eusart_spi_calculate_clock_div(ref_freq, 1000000);

  init.advanced_config = &adv;   // Advanced settings structure

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

  // Enable EUSART
  sl_hal_eusart_enable(EUSART0);
  sl_hal_eusart_enable_tx(EUSART0);
  sl_hal_eusart_enable_rx(EUSART0);
}

/**************************************************************************//**
 * @brief
 *    LDMA initialization
 *****************************************************************************/
void ldma_init(void)
{

  // Initialize DMADRV
  DMADRV_Init();

  // Halt on error if DMA channel cannot be allocated
  if (DMADRV_AllocateChannel(&tx_channel_id, NULL) != ECODE_EMDRV_DMADRV_OK) {
    __BKPT(0);
  }

  // Halt on error if DMA channel cannot be allocated
  if (DMADRV_AllocateChannel(&rx_channel_id, NULL) != ECODE_EMDRV_DMADRV_OK) {
    __BKPT(0);
  }

  // Source is outbuf, destination is EUSART0_TXDATA, and length if BUFLEN
  ldma_tx_descriptor = (sl_hal_ldma_descriptor_t)SL_HAL_LDMA_DESCRIPTOR_SINGLE_M2P(SL_HAL_LDMA_CTRL_SIZE_BYTE, outbuf, &(EUSART0->TXDATA), BUFLEN);

  // Transfer a byte on free space in the EUSART FIFO
  ldma_tx_config = (sl_hal_ldma_transfer_config_t)SL_HAL_LDMA_TRANSFER_CFG_PERIPHERAL(SL_HAL_LDMA_PERIPHERAL_SIGNAL_EUSART0_TXFL);

  // Source is EUSART0_RXDATA, destination is inbuf, and length if BUFLEN
  ldma_rx_descriptor = (sl_hal_ldma_descriptor_t)SL_HAL_LDMA_DESCRIPTOR_SINGLE_P2M(SL_HAL_LDMA_CTRL_SIZE_BYTE, &(EUSART0->RXDATA), inbuf, BUFLEN);

  // Transfer a byte on receive FIFO level event
  ldma_rx_config = (sl_hal_ldma_transfer_config_t)SL_HAL_LDMA_TRANSFER_CFG_PERIPHERAL(SL_HAL_LDMA_PERIPHERAL_SIGNAL_EUSART0_RXFL);
}

/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  // Initialize GPIO, EUSART0, and LDMA
  gpio_init();
  eusart0_init();
  ldma_init();

  app_state = IDLE;
}

/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{
  switch (app_state) {
    case IDLE:
      /*
       * This state allows the main loop to enter EM1 in order to
       * reduce current draw.
       */
      break;

    case RX_COMPLETE:
      // De-assert chip select upon final byte reception (drive high)
      sl_gpio_set_pin(&GPIO_EUS0CS);
      break;

    case SEND:
      uint32_t i;

      // Zero incoming buffer and populate outgoing data array
      for (i = 0; i < BUFLEN; i++)
      {
        inbuf[i] = 0;
        outbuf[i] = (uint8_t)i;
      }

      //Assert chip select (drive low).
      sl_gpio_clear_pin(&GPIO_EUS0CS);

      // Start both channels
      DMADRV_LdmaStartTransfer(rx_channel_id,
                               &ldma_rx_config,
                               &ldma_rx_descriptor,
                               (DMADRV_Callback_t)ldma_rx_callback,
                               NULL);

      DMADRV_LdmaStartTransfer(tx_channel_id,
                               &ldma_tx_config,
                               &ldma_tx_descriptor,
                               NULL,
                               NULL);
      app_state = IDLE;
      break;

    default:
      break;
  }
}

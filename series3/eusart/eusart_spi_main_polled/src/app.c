/***************************************************************************//**
 * @file app.c
 * @brief This project demonstrates synchronous (SPI) use of the EUSART in
 * polled main (formerly master) mode. The main loop transmits the specified
 * number of bytes and receives the byte that is shifted in with each outgoing
 * one.
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

/*******************************************************************************
 ***************************   GLOBAL VARIABLES   ******************************
 ******************************************************************************/
// State variable
volatile bool start_transfer = false;

// Outgoing data
uint8_t outbuf[BUFLEN];

// Incoming data
uint8_t inbuf[BUFLEN];

/* This function is called by Power Manager every time it tries to enter
 * lower energy modes.
 * The application should only enter EM1 before starting transmission
 * by a button push.
 */
bool app_is_ok_to_sleep(void)
{
  if (start_transfer) {
    return false;
  }
  return true;
}

/**************************************************************************//**
 * @brief GPIO IRQHandler
 *****************************************************************************/
void gpio_button0_callback(void)
{
  // Start transmission
  start_transfer = true;
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

  // Configure CS pin as an output and drive inactive high
  sl_gpio_set_pin_mode(&GPIO_EUS0CS, SL_GPIO_MODE_PUSH_PULL, 1);

  // Configure button 1 pin as an input
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

  // Enable EUSART
  sl_hal_eusart_enable(EUSART0);
  sl_hal_eusart_enable_tx(EUSART0);
  sl_hal_eusart_enable_rx(EUSART0);
}

/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  // Initialize GPIO and EUSART0
  gpio_init();
  eusart0_init();
}

/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{
  if (start_transfer){
    uint32_t i;
    // Zero incoming buffer and populate outgoing data array
    for (i = 0; i < BUFLEN; i++){
      inbuf[i] = 0;
      outbuf[i] = (uint8_t)i;
    }

    // Assert chip select (drive low)
    sl_gpio_clear_pin(&GPIO_EUS0CS);

    /*
     * Because this example is most likely going to be running with
     * another EFM32/EFR32 device on the secondary side, it must insert
     * a delay between chip select assertion and sending the first
     * byte.
     *
     * On Series 3 EFM32/EFR32 devices, this delay needs
     * to be between 7 and 10 us in order for the downstream firmware
     * to enable SPI reception and pre-load the first byte to be
     * transmitted.
     *
     * Similar delays are not uncommon for things like high-precision
     * delta-sigma A-to-D converters where the falling chip select
     * wakes the device from a low-power state, starts a conversion,
     * and can return data after some set delay.
     */
    sl_udelay_wait(15);

    /*
     * Repeatedly perform single byte SPI transfers (transmission and
     * reception) of the specified length. sl_hal_eusart_spi_tx_rx() polls
     * EUSART_STATUS_TXC for transmission complete, so this function ties
     * up the CPU until the last bit of the byte being transmitted is sent.
     */
    for (i = 0; i < BUFLEN; i++)
      inbuf[i] = sl_hal_eusart_spi_tx_rx(EUSART0, outbuf[i]);

    // De-assert chip select upon transfer completion (drive high)
    sl_gpio_set_pin(&GPIO_EUS0CS);

    /*
     * Insert a short delay after CS de-assertion. When this code is
     * running on a faster main (e.g. either higher clock rate or a
     * faster CPU such as a Cortex-M3), the secondary implementation
     * running on a slower device (e.g. either lower clock rate or a
     * slower CPU such as a Cortex-M0+) needs extra time to prepare
     * the input and output buffers for the next round of bytes to be
     * transferred.
     */
    sl_udelay_wait(15);

    // Stop transmission upon transfer completion
    start_transfer = false;
  }
}


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
#include "em_eusart.h"
#include "em_gpio.h"
#include "sl_udelay.h"

#include "pin_config.h"

// Size of the data buffers
#define BUFLEN  10

#ifndef BUTTON0_PORT
  #define BUTTON0_PORT LED0_BUTTON0_PORT
  #define BUTTON0_PIN LED0_BUTTON0_PIN
#endif

const sl_gpio_t GPIO_BUTTON0  = { .port = BUTTON0_PORT,  .pin = BUTTON0_PIN };
const sl_gpio_t GPIO_EUS0MOSI = { .port = EUS0MOSI_PORT, .pin = EUS0MOSI_PIN };
const sl_gpio_t GPIO_EUS0MISO = { .port = EUS0MISO_PORT, .pin = EUS0MISO_PIN };
const sl_gpio_t GPIO_EUS0CLK  = { .port = EUS0SCLK_PORT, .pin = EUS0SCLK_PIN };
const sl_gpio_t GPIO_EUS0CS   = { .port = EUS0CS_PORT,   .pin = EUS0CS_PIN };

/*******************************************************************************
 ***************************   GLOBAL VARIABLES   ******************************
 ******************************************************************************/

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

  // Configure MOSI (TX) pin as an output
  sl_gpio_set_pin_mode(&GPIO_EUS0MOSI, SL_GPIO_MODE_PUSH_PULL, 0);

  // Configure MISO (RX) pin as an input
  sl_gpio_set_pin_mode(&GPIO_EUS0MISO, SL_GPIO_MODE_INPUT, 0);

  // Configure SCLK pin as an output low (CPOL = 0)
  sl_gpio_set_pin_mode(&GPIO_EUS0CLK, SL_GPIO_MODE_PUSH_PULL, 0);

  // Configure CS pin as an output initially high
  sl_gpio_set_pin_mode(&GPIO_EUS0CS, SL_GPIO_MODE_PUSH_PULL, 1);

  // Configure button 0 pin as an input
  sl_gpio_set_pin_mode(&GPIO_BUTTON0, SL_GPIO_MODE_INPUT_PULL, 1);

  // Interrupt on button 0 rising edge to start transfers
  sl_gpio_configure_external_interrupt(&GPIO_BUTTON0, &int_no,
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
  EUSART_SpiAdvancedInit_TypeDef adv = EUSART_SPI_ADVANCED_INIT_DEFAULT;

  adv.msbFirst = true;        // SPI standard MSB first

  // Default asynchronous initializer (main/master mode and 8-bit data)
  EUSART_SpiInit_TypeDef init = EUSART_SPI_MASTER_INIT_DEFAULT_HF;

  init.bitRate = 1000000;         // 1 MHz shift clock
  init.advancedSettings = &adv;   // Advanced settings structure

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
  EUSART_SpiInit(EUSART0, &init);
}

/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  gpio_init();
  eusart0_init();
}

/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{
  if (start_transfer) {
    uint32_t i;

    // Zero incoming buffer and populate outgoing data array
    for (i = 0; i < BUFLEN; i++)
    {
      inbuf[i] = 0;
      outbuf[i] = (uint8_t)i;
    }

    // Assert chip select (drive low)
    sl_gpio_clear_pin(&GPIO_EUS0CS);

    /*
     * Because this example is most likely going to be running with
     * another EFM32/EFR32 device on the secondary side, it must
     * insert a delay between chip select assertion and sending the
     * first byte.
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
    sl_udelay_wait(15);

    /*
     * Repeatedly perform single byte SPI transfers (transmission and
     * reception) of the specified length.  EUSART_SpiTxRx() polls
     * EUSART_STATUS_TXC for transmission complete, so this function ties
     * up the CPU until the last bit of the byte being transmitted is sent.
     */
    for (i = 0; i < BUFLEN; i++) {
      inbuf[i] = EUSART_Spi_TxRx(EUSART0, outbuf[i]);
    }
    // De-assert chip select upon final byte reception (drive high)
    sl_gpio_set_pin(&GPIO_EUS0CS);

    /*
     * Insert a short delay after CS de-assertion.  When this code is
     * running on a faster main (e.g. either higher clock rate or a
     * faster CPU such as a Cortex-M3), the secondary implementation
     * running on a slower device (e.g. either lower clock rate or a
     * slower CPU such as a Cortex-M0+) needs extra time to prepare
     * the input and output buffers for the next round of bytes to be
     * transferred.
     */
    sl_udelay_wait(10);
  }
}

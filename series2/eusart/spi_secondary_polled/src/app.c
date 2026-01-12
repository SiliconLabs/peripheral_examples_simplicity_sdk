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
#include "em_eusart.h"
#include "em_gpio.h"

#include "pin_config.h"

// Size of the data buffers
#define BUFLEN  10

const sl_gpio_t GPIO_EUS0MOSI = { .port = EUS0MOSI_PORT, .pin = EUS0MOSI_PIN };
const sl_gpio_t GPIO_EUS0MISO = { .port = EUS0MISO_PORT, .pin = EUS0MISO_PIN };
const sl_gpio_t GPIO_EUS0CLK  = { .port = EUS0SCLK_PORT, .pin = EUS0SCLK_PIN };
const sl_gpio_t GPIO_EUS0CS   = { .port = EUS0CS_PORT,   .pin = EUS0CS_PIN };

/*******************************************************************************
 ***************************   GLOBAL VARIABLES   ******************************
 ******************************************************************************/

// Outgoing data
uint8_t buffer[BUFLEN];

/**************************************************************************//**
 * @brief
 *    GPIO initialization
 *****************************************************************************/
void gpio_init(void)
{
  int32_t int_no = GPIO_EUS0CS.pin;

  // Configure MOSI (TX) pin as an output
  sl_gpio_set_pin_mode(&GPIO_EUS0MOSI, SL_GPIO_MODE_INPUT, 0);

  // Configure MISO (RX) pin as an input
  sl_gpio_set_pin_mode(&GPIO_EUS0MISO, SL_GPIO_MODE_PUSH_PULL, 0);

  // Configure SCLK pin as an input
  sl_gpio_set_pin_mode(&GPIO_EUS0CLK, SL_GPIO_MODE_INPUT, 0);

  // Configure CS pin as an input pulled high
  sl_gpio_set_pin_mode(&GPIO_EUS0CS, SL_GPIO_MODE_INPUT_PULL, 1);

  // Generate an interrupt on a CS pin high-to-low transition
  sl_gpio_configure_external_interrupt(&GPIO_EUS0CS,
                                       &int_no,
                                       SL_GPIO_INTERRUPT_FALLING_EDGE,
                                       NULL,
                                       NULL);
  sl_gpio_disable_interrupts(1 << EUS0CS_PIN);
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
  EUSART_SpiInit_TypeDef init = EUSART_SPI_SLAVE_INIT_DEFAULT_HF;

  init.advancedSettings = &adv;   // Advanced settings structure
  init.enable = eusartDisable;    // Don't enable yet

  // Route EUSART0 RX, TX, CLK, and CS to the specified pins
  GPIO->EUSARTROUTE[0].TXROUTE   = (EUS0MOSI_PORT << _GPIO_EUSART_TXROUTE_PORT_SHIFT)
                                 | (EUS0MOSI_PIN << _GPIO_EUSART_TXROUTE_PIN_SHIFT);
  GPIO->EUSARTROUTE[0].RXROUTE   = (EUS0MISO_PORT << _GPIO_EUSART_RXROUTE_PORT_SHIFT)
                                 | (EUS0MISO_PIN << _GPIO_EUSART_RXROUTE_PIN_SHIFT);
  GPIO->EUSARTROUTE[0].SCLKROUTE = (EUS0SCLK_PORT << _GPIO_EUSART_SCLKROUTE_PORT_SHIFT)
                                 | (EUS0SCLK_PIN << _GPIO_EUSART_SCLKROUTE_PIN_SHIFT);
  GPIO->EUSARTROUTE[0].CSROUTE   = (EUS0CS_PORT << _GPIO_EUSART_CSROUTE_PORT_SHIFT)
                                 | (EUS0CS_PIN << _GPIO_EUSART_CSROUTE_PIN_SHIFT);

  // Enable EUSART interface pins
  GPIO->EUSARTROUTE[0].ROUTEEN = GPIO_EUSART_ROUTEEN_RXPEN |    // MISO
                                 GPIO_EUSART_ROUTEEN_TXPEN |    // MOSI
                                 GPIO_EUSART_ROUTEEN_SCLKPEN |
                                 GPIO_EUSART_ROUTEEN_CSPEN;

  // Configure and enable EUSART0
  EUSART_SpiInit(EUSART0, &init);

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
   * be set, but this requires disabling the EUSART because the emlib
   * advanced initializer for SPI mode does not include an option to
   * set this (even though it includes a way to set the default frame).
   */
  EUSART0->EN_CLR = EUSART_EN_EN;

  // Wait while the EUSART disables itself
  while ((EUSART0->EN & EUSART_EN_DISABLING));

  // Set FORCELOAD, then re-enable
  EUSART0->CFG2_SET = EUSART_CFG2_FORCELOAD;
  EUSART0->EN_SET = EUSART_EN_EN;
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

  gpio_init();
  eusart0_init();
}

/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{
  uint32_t i;
  // Clear the flag and wait for a falling edge on EUS0CS_PIN
  sl_hal_gpio_clear_interrupts(1<<EUS0CS_PIN);

  while ((sl_hal_gpio_get_pending_interrupts() & (1 << EUS0CS_PIN)) == 0);

  // Now enable the EUSART receiver and transmitter
  EUSART_Enable(EUSART0, eusartEnable);

  /*
    * Perform the specified number of single byte SPI transfers
    * (transmission and reception).
    *
    * Data to be queued for transmission is sent with EUSART_Tx(),
    * which checks that there is room in the transmit FIFO but does
    * not actually wait for transmit complete (EUSART_STATUS_TXC).
    *
    * Instead, incoming data is read from the FIFO with EUSART_Rx(),
    * which polls EUSART_STATUS_RXFL to make sure data has arrived.
    */
  for (i = 0; i < BUFLEN; i++) {
    EUSART_Tx(EUSART0, buffer[i]);
    buffer[i] = EUSART_Rx(EUSART0);
  }
}

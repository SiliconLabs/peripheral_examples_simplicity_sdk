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

#include "dmadrv.h"

#include "sl_gpio.h"
#include "em_eusart.h"
#include "em_gpio.h"

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

typedef enum {
  IDLE,
  RX_COMPLETE,
  SEND
} AppState_t;

/*******************************************************************************
 ***************************   GLOBAL VARIABLES   ******************************
 ******************************************************************************/

volatile AppState_t app_state;

// LDMA descriptors, configs and channels for TX and RX channel
LDMA_Descriptor_t ldma_tx_desc, ldma_rx_desc;
LDMA_TransferCfg_t ldma_tx_config, ldma_rx_config;
unsigned int tx_channel_id, rx_channel_id;

// Outgoing data
uint8_t outbuf[BUFLEN];

// Incoming data
uint8_t inbuf[BUFLEN];

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
  EUSART_SpiAdvancedInit_TypeDef adv = EUSART_SPI_ADVANCED_INIT_DEFAULT;

  adv.msbFirst = true;        // SPI standard MSB first
  adv.autoInterFrameTime = 7; // 7 bit times of delay between frames
                              // to accommodate non-DMA secondaries

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

/**************************************************************************//**
 * @brief
 *    LDMA initialization
 *****************************************************************************/
void ldma_init(void)
{
  // First, initialize the LDMA unit itself
  LDMA_Init_t ldmaInit = LDMA_INIT_DEFAULT;
  LDMA_Init(&ldmaInit);

  // Source is outbuf, destination is EUSART0_TXDATA, and length is BUFLEN
  ldma_tx_desc = (LDMA_Descriptor_t)LDMA_DESCRIPTOR_SINGLE_M2P_BYTE(outbuf, &(EUSART0->TXDATA), BUFLEN);

  // Transfer a byte on free space in the EUSART FIFO
  ldma_tx_config = (LDMA_TransferCfg_t)LDMA_TRANSFER_CFG_PERIPHERAL(ldmaPeripheralSignal_EUSART0_TXFL);

  // Source is EUSART0_RXDATA, destination is inbuf, and length is BUFLEN
  ldma_rx_desc = (LDMA_Descriptor_t)LDMA_DESCRIPTOR_SINGLE_P2M_BYTE(&(EUSART0->RXDATA), inbuf, BUFLEN);

  // Transfer a byte on receive FIFO level event
  ldma_rx_config = (LDMA_TransferCfg_t)LDMA_TRANSFER_CFG_PERIPHERAL(ldmaPeripheralSignal_EUSART0_RXFL);

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
}

/**************************************************************************//**
 * @brief  LDMA RX IRQ Callback
 *****************************************************************************/
void ldma_rx_callback(void)
{
  app_state = RX_COMPLETE;
}

/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
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

      /* Break is missing intentionally. Fall through in order
       * to start sending the next frame continuously.
       */

    case SEND:
      uint32_t i;

      // Zero incoming buffer and populate outgoing data array
      for (i = 0; i < BUFLEN; i++)
      {
        inbuf[i] = 0;
        outbuf[i] = (uint8_t)i;
      }

      /*
       * Assert chip select (drive low).  Note that the calls to
       * LDMA_StartTransfer() for the receive and transmit channels take
       * about 42 microseconds at 19 MHz, which the secondary device
       * needs in order to service the GPIO falling edge interrupt on
       * CS and to prepare to receive the incoming data.
       */
      sl_gpio_clear_pin(&GPIO_EUS0CS);

      // Start both channels
      DMADRV_LdmaStartTransfer(rx_channel_id,
                               &ldma_rx_config,
                               &ldma_rx_desc,
                               (DMADRV_Callback_t)ldma_rx_callback,
                               NULL);
      DMADRV_LdmaStartTransfer(tx_channel_id,
                               &ldma_tx_config,
                               &ldma_tx_desc,
                               NULL,
                               NULL);
      app_state = IDLE;
      break;

    default:
      break;
  }
}

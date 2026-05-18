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

#include "dmadrv.h"

#include "sl_gpio.h"
#include "em_eusart.h"

#include "pin_config.h"
#include "peripheral_config.h"

// Size of the buffer for received data
#define BUFLEN  10

// In low-frequency mode, the maximum EUART baud rate is 9600
#define BAUDRATE  9600

// LDMA channels for receive and transmit servicing
#define RX_LDMA_CHANNEL 0
#define TX_LDMA_CHANNEL 1

const sl_gpio_t GPIO_PB0   = { .port = BUTTON0_PORT, .pin = BUTTON0_PIN };
const sl_gpio_t GPIO_LED0  = { .port = LED0_PORT, .pin = LED0_PIN };
const sl_gpio_t EUSART0_TX = { .port = EUART0_TX_PORT, . pin = EUART0_TX_PIN };
const sl_gpio_t EUSART0_RX = { .port = EUART0_RX_PORT, . pin = EUART0_RX_PIN };

// LDMA descriptor and transfer configuration structures for TX channel
LDMA_Descriptor_t ldma_tx_descriptor;
LDMA_TransferCfg_t ldma_tx_config;

// LDMA descriptor and transfer configuration structures for RX channel
LDMA_Descriptor_t ldma_rx_descriptor;
LDMA_TransferCfg_t ldma_rx_config;

unsigned int rx_channel_id;
unsigned int tx_channel_id;

// Receive data buffer
uint8_t buffer[BUFLEN];

volatile bool receive = true;

/**************************************************************************//**
 * @brief escapeHatch()
 *
 * When developing or debugging code that enters EM2 or lower, it's a
 * good idea to have an "escape hatch" type mechanism, e.g. a way to
 * pause the device so that a debugger can connect in order to erase
 * flash, among other things.
 *
 * Before proceeding with this example, make sure PB0 is not pressed.
 * If the PB0 pin is low, turn on LED0 and execute the breakpoint (BKPT)
 * instruction to stop the processor in EM0 and allow a debug
 * connection to be made.
 *****************************************************************************/
void escape_hatch(void)
{
  bool button_in;
  // Configure PB0 pin as an input pulled high
  sl_gpio_set_pin_mode(&GPIO_PB0, SL_GPIO_MODE_INPUT_PULL_FILTER, true);
  sl_gpio_get_pin_input(&GPIO_PB0, &button_in);
  // Check for PB0 low; if so halt the CPU
  if (button_in == 0) {
    sl_gpio_set_pin_mode(&GPIO_LED0, SL_GPIO_MODE_PUSH_PULL, LED_ON);
    __BKPT(0);
  }
  // Pin not asserted; disable the PB0 digital input
  else {
    sl_gpio_set_pin_mode(&GPIO_PB0, SL_GPIO_MODE_DISABLED, false);
  }
}

/**************************************************************************//**
 * @brief
 *    GPIO initialization
 *****************************************************************************/
void gpio_init(void)
{
  // Configure the EUART TX pin to the board controller as an output
  sl_gpio_set_pin_mode(&EUSART0_TX, SL_GPIO_MODE_PUSH_PULL, 1);

  // Configure the EUART RX pin to the board controller as an input
  sl_gpio_set_pin_mode(&EUSART0_RX, SL_GPIO_MODE_INPUT, 0);

  // Route EUART0 TX and RX to the board controller TX and RX pins
  GPIO->EUARTROUTE->TXROUTE = (EUART0_TX_PORT << _GPIO_EUART_TXROUTE_PORT_SHIFT)
                            | (EUART0_TX_PIN << _GPIO_EUART_TXROUTE_PIN_SHIFT);
  GPIO->EUARTROUTE->RXROUTE = (EUART0_RX_PORT << _GPIO_EUART_TXROUTE_PORT_SHIFT)
                            | (EUART0_RX_PIN << _GPIO_EUART_TXROUTE_PIN_SHIFT);

  // Enable TX signal now that it is routed (RX is always enabled)
  GPIO->EUARTROUTE->ROUTEEN = GPIO_EUART_ROUTEEN_TXPEN;
}

/**************************************************************************//**
 * @brief
 *    EUART initialization
 *****************************************************************************/
void euart0_init(void)
{
  // Enable EUART0 Clock
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_EUART0);

  // Initialize the EUART0 module
  EUSART_UartInit_TypeDef init = EUSART_UART_INIT_DEFAULT_LF;
  EUSART_AdvancedInit_TypeDef advance_init = EUSART_ADVANCED_INIT_DEFAULT;

  init.baudrate = BAUDRATE;
  init.advancedSettings = &advance_init;
  init.advancedSettings->dmaWakeUpOnRx = true;
  init.advancedSettings->dmaWakeUpOnTx = false;
  init.advancedSettings->dmaHaltOnError = true;

  // Configure and enable EUART0 for low-frequency (EM2) operation
  EUSART_UartInitLf(EUART0, &init);
}

/**************************************************************************//**
 * @brief
 *    LDMA initialization
 *****************************************************************************/
void ldma_init(void)
{
  DMADRV_Init();

  // Source is buffer, destination is EUART0_TXDATA, and length if BUFLEN
  ldma_tx_descriptor =
      (LDMA_Descriptor_t)LDMA_DESCRIPTOR_SINGLE_M2P_BYTE(buffer,
                                                         &(EUART0->TXDATA),
                                                         BUFLEN);

  // Transfer a byte on free space in the EUART FIFO
  ldma_tx_config = (LDMA_TransferCfg_t)LDMA_TRANSFER_CFG_PERIPHERAL(ldmaPeripheralSignal_EUART0_TXFL);

  // Source is EUART0_RXDATA, destination is buffer, and length if BUFLEN
  ldma_rx_descriptor =
      (LDMA_Descriptor_t)LDMA_DESCRIPTOR_SINGLE_P2M_BYTE(&(EUART0->RXDATA),
                                                         buffer,
                                                         BUFLEN);

  // Transfer a byte on receive FIFO level event
  ldma_rx_config = (LDMA_TransferCfg_t)LDMA_TRANSFER_CFG_PERIPHERAL(ldmaPeripheralSignal_EUART0_RXFL);

  // Halt on error if DMA channel cannot be allocated
  if (DMADRV_AllocateChannel(&rx_channel_id, NULL) != ECODE_EMDRV_DMADRV_OK) {
    __BKPT(0);
  }

  // Halt on error if DMA channel cannot be allocated
  if (DMADRV_AllocateChannel(&tx_channel_id, NULL) != ECODE_EMDRV_DMADRV_OK) {
    __BKPT(0);
  }
}

/**************************************************************************//**
 * @brief LDMA RX IRQHandler
 *****************************************************************************/
void ldma_rx_callback(void)
{
  // RX done, start transmitting
  receive = false;
}

/**************************************************************************//**
 * @brief LDMA TX IRQHandler
 *****************************************************************************/
void ldma_tx_callback(void)
{
  // TX done, start receiving
  receive = true;
}

/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  escape_hatch();
  gpio_init();
  euart0_init();
  ldma_init();
}

/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{
  if (receive) {
    uint32_t i;

    // Zero out buffer
    for (i = 0; i < BUFLEN; i++) {
      buffer[i] = 0;
    }

    // Start the LDMA receive channel
    DMADRV_LdmaStartTransfer(rx_channel_id,
                             &ldma_rx_config,
                             &ldma_rx_descriptor,
                             (DMADRV_Callback_t)ldma_rx_callback,
                             NULL);
  } else {
    // Start the LDMA transmit channel
    DMADRV_LdmaStartTransfer(tx_channel_id,
                             &ldma_tx_config,
                             &ldma_tx_descriptor,
                             (DMADRV_Callback_t)ldma_tx_callback,
                             NULL);
  }
}

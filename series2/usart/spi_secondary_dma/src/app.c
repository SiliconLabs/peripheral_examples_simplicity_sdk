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
#include "pin_config.h"
#include "em_usart.h"
#include "em_ldma.h"
#include "sl_board_control_config.h"
#include "sl_power_manager.h"

// LDMA channel for receive and transmit servicing
#define RX_LDMA_CHANNEL 0
#define TX_LDMA_CHANNEL 1

typedef enum {
  INIT,
  RECEIVE_START,
  WAIT,
  RECEIVE_DONE,
} AppState_t;

AppState_t appState;

// LDMA descriptor and transfer configuration structures for USART TX channel
LDMA_Descriptor_t ldmaTXDescriptor;
LDMA_TransferCfg_t ldmaTXConfig;

// LDMA descriptor and transfer configuration structures for USART RX channel
LDMA_Descriptor_t ldmaRXDescriptor;
LDMA_TransferCfg_t ldmaRXConfig;

// Size of the data buffers
#define BUFLEN  10

// Outgoing data
uint8_t outbuf[BUFLEN];

// Incoming data
uint8_t inbuf[BUFLEN];

const sl_gpio_t US0COPI = { .port = EXP_SPI_COPI_PORT, .pin = EXP_SPI_COPI_PIN };
const sl_gpio_t US0CIPO = { .port = EXP_SPI_CIPO_PORT, .pin = EXP_SPI_CIPO_PIN };
const sl_gpio_t US0CLK = { .port = EXP_SPI_SCK_PORT, .pin = EXP_SPI_SCK_PIN };
const sl_gpio_t US0CS = { .port = EXP_SPI_CS_PORT, .pin = EXP_SPI_CS_PIN };

/**************************************************************************//**
 * @brief GPIO IRQHandler
 *****************************************************************************/
#if (EXP_SPI_CS_PIN & 1)
void gpio_odd_irq_callback(uint8_t irq_idx, void* context)
#else
void gpio_even_irq_callback(uint8_t irq_idx, void* context)
#endif
{
  appState = RECEIVE_START;
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
  int32_t interrupt_number = EXP_SPI_CS_PIN;

  // Configure COPI pin as an input
  sl_gpio_set_pin_mode(&US0COPI, SL_GPIO_MODE_INPUT, 0);

  // Configure CIPO pin as an output
  sl_gpio_set_pin_mode(&US0CIPO, SL_GPIO_MODE_PUSH_PULL, 0);

  // Configure CLK pin as an input
  sl_gpio_set_pin_mode(&US0CLK, SL_GPIO_MODE_INPUT, 0);

  // Configure CS pin as input
  sl_gpio_set_pin_mode(&US0CS, SL_GPIO_MODE_INPUT, 1);

#if (EXP_SPI_CS_PIN & 1)
  // Request an interrupt on a CS pin high-to-low transition.
  sl_gpio_configure_external_interrupt(&US0CS, &interrupt_number, SL_GPIO_INTERRUPT_FALLING_EDGE, &gpio_odd_irq_callback, (void*)0);

  // Enable NVIC GPIO interrupt
  sl_interrupt_manager_clear_irq_pending(GPIO_ODD_IRQn);
  sl_interrupt_manager_enable_irq(GPIO_ODD_IRQn);
#else
  // Request an interrupt on a CS pin high-to-low transition.
  sl_gpio_configure_external_interrupt(&US0CS, &interrupt_number, SL_GPIO_INTERRUPT_FALLING_EDGE, &gpio_even_irq_callback, (void*)0);

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
  // Enable peripheral clock
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_USART0);

  // Default asynchronous initializer (main mode, 1 Mbps, 8-bit data)
  USART_InitSync_TypeDef init = USART_INITSYNC_DEFAULT;

  init.master = false;  // Operate as a secondary
  init.msbf = true;     // MSB first transmission for SPI compatibility
  init.enable = usartDisable;  // Do not enable yet

  // Route USART0 RX, TX, CLK, and CS to the specified pins.
  GPIO->USARTROUTE[0].TXROUTE = (EXP_SPI_COPI_PORT << _GPIO_USART_TXROUTE_PORT_SHIFT)
      | (EXP_SPI_COPI_PIN << _GPIO_USART_TXROUTE_PIN_SHIFT);
  GPIO->USARTROUTE[0].RXROUTE = (EXP_SPI_CIPO_PORT << _GPIO_USART_RXROUTE_PORT_SHIFT)
      | (EXP_SPI_CIPO_PIN << _GPIO_USART_RXROUTE_PIN_SHIFT);
  GPIO->USARTROUTE[0].CLKROUTE = (EXP_SPI_SCK_PORT << _GPIO_USART_CLKROUTE_PORT_SHIFT)
      | (EXP_SPI_SCK_PIN << _GPIO_USART_CLKROUTE_PIN_SHIFT);
  GPIO->USARTROUTE[0].CSROUTE = (EXP_SPI_CS_PORT << _GPIO_USART_CSROUTE_PORT_SHIFT)
      | (EXP_SPI_CS_PIN << _GPIO_USART_CSROUTE_PIN_SHIFT);

  // Enable USART interface pins
  GPIO->USARTROUTE[0].ROUTEEN = GPIO_USART_ROUTEEN_RXPEN |    // MISO
                                GPIO_USART_ROUTEEN_TXPEN |    // MOSI
                                GPIO_USART_ROUTEEN_CLKPEN |
                                GPIO_USART_ROUTEEN_CSPEN;

  // Configure but do not enable USART0
  USART_InitSync(USART0, &init);
}

/**************************************************************************//**
 * @brief
 *    LDMA initialization
 *****************************************************************************/
void initLDMA(void)
{
  // First, initialize the LDMA unit itself
  LDMA_Init_t ldmaInit = LDMA_INIT_DEFAULT;
  LDMA_Init(&ldmaInit);

  // Source is outbuf, destination is USART0_TXDATA, and length is BUFLEN
  ldmaTXDescriptor = (LDMA_Descriptor_t)LDMA_DESCRIPTOR_SINGLE_M2P_BYTE(outbuf, &(USART0->TXDATA), BUFLEN);
  ldmaTXDescriptor.xfer.blockSize = ldmaCtrlBlockSizeUnit2;    // Transfers 2 units per arbitration
  ldmaTXDescriptor.xfer.ignoreSrec = 1;    // Ignores single requests

  // Transfer 2 bytes on free space in the USART buffer
  ldmaTXConfig = (LDMA_TransferCfg_t)LDMA_TRANSFER_CFG_PERIPHERAL(ldmaPeripheralSignal_USART0_TXBL);

  // Source is USART0_RXDATA, destination is inbuf, and length is BUFLEN
  ldmaRXDescriptor = (LDMA_Descriptor_t)LDMA_DESCRIPTOR_SINGLE_P2M_BYTE(&(USART0->RXDATA), inbuf, BUFLEN);
  ldmaRXDescriptor.xfer.blockSize = ldmaCtrlBlockSizeUnit2;    // Transfers 2 units per arbitration
  ldmaRXDescriptor.xfer.ignoreSrec = 1;    // Ignores single requests

  // Transfer 2 bytes on receive data valid
  ldmaRXConfig = (LDMA_TransferCfg_t)LDMA_TRANSFER_CFG_PERIPHERAL(ldmaPeripheralSignal_USART0_RXDATAV);
}

/**************************************************************************//**
 * @brief LDMA IRQHandler
 *****************************************************************************/
void LDMA_IRQHandler()
{
  uint32_t flags = LDMA_IntGet();

  // Clear the transmit channel's done flag if set
  if (flags & (1 << TX_LDMA_CHANNEL)) {
    LDMA_IntClear(1 << TX_LDMA_CHANNEL);
  }

  /*
   * Clear the receive channel's done flag if set and change receive
   * state to done.
   */
  if (flags & (1 << RX_LDMA_CHANNEL)) {
    LDMA_IntClear(1 << RX_LDMA_CHANNEL);
    appState = RECEIVE_DONE;
  }

  // Stop in case there was an error
  if (flags & LDMA_IF_ERROR) {
    __BKPT(0);
  }
}

/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  // Initialize GPIO, USART0 and LDMA
  initGPIO();
  initUSART0();
  initLDMA();
  appState = INIT;
}

/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{
  switch (appState)
  {
    case RECEIVE_DONE:
      // Disable the falling edge interrupt on the US0CS_PIN
      sl_gpio_disable_interrupts(1<<EXP_SPI_CS_PIN);

      // Disable USART receiver and transmitter until next chip select
      USART_Enable(USART0, usartDisable);
      /*
       * Fall through intentional in order to prepare the MCU
       * to receive the next CS interrupt
       */
    case INIT:
      uint32_t i;
      // Zero incoming buffer and populate outgoing data array
      for (i = 0; i < BUFLEN; i++){
        inbuf[i] = 0;
        outbuf[i] = (uint8_t)i;
      }

      // Now enable the falling edge interrupt on the US0CS_PIN
      sl_gpio_enable_interrupts(1<<EXP_SPI_CS_PIN);

      break;

    case RECEIVE_START:
      // Now enable the USART receiver and transmitter
      USART_Enable(USART0, usartEnable);

      // Start both channels
      LDMA_StartTransfer(RX_LDMA_CHANNEL, &ldmaRXConfig, &ldmaRXDescriptor);
      LDMA_StartTransfer(TX_LDMA_CHANNEL, &ldmaTXConfig, &ldmaTXDescriptor);
      appState = WAIT;
      break;
    case WAIT:
      /*
       * Allow the main while() loop to enter EM1 until interrupt occurs on
       * US0CS pin
       */
      break;
    default:
      break;
  }
}

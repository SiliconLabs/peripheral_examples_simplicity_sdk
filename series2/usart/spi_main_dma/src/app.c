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
#include "em_ldma.h"
#include "sl_power_manager.h"

// LDMA channel for receive and transmit servicing
#define RX_LDMA_CHANNEL 0
#define TX_LDMA_CHANNEL 1

#ifndef BUTTON0_PORT
  #define BUTTON0_PORT LED0_BUTTON0_PORT
  #define BUTTON0_PIN LED0_BUTTON0_PIN
#endif

typedef enum {
  INIT,
  RECEIVE,
  SEND
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

// Data reception complete
bool rx_done;

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
  // Start transmission
  appState = SEND;
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

  // Configure COPI pin as an output
  sl_gpio_set_pin_mode(&US0COPI, SL_GPIO_MODE_PUSH_PULL, 0);

  // Configure CIPO pin as an input
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
  init.autoCsEnable = true;   // Allow the USART to assert CS

  /*
   * The autoCsSetup and autoCsHold members allow chip select setup and
   * hold times delays to be insert before and after assertion.
   * However, the predefined options of 0, 1, 2, 3, and 7 bit times can
   * be too short if the secondary device needs time to setup reception.
   *
   * These delays can be increased by using the programmable USART
   * timer comparators.  In this case, setting autoCsSetup and
   * autoCsHold to 5 uses the count programmed into USART0->TIMECMP0
   * to do this.  USART0->USART_TIMECMP0_TCMPVAL is programmed to
   * insert 10 bit times (10 us @ 1 MHz) of setup and hold delay.
   */
  init.autoCsSetup = 5;
  init.autoCsHold = 5;

  USART0->TIMECMP0 = 10;
  USART0->TIMECMP0 |= USART_TIMECMP0_RESTARTEN;

  /*
   * Route USART0 RX, TX, and CLK to the specified pins.
   */
  GPIO->USARTROUTE[0].TXROUTE = (EXP_SPI_COPI_PORT << _GPIO_USART_TXROUTE_PORT_SHIFT)
      | (EXP_SPI_COPI_PIN << _GPIO_USART_TXROUTE_PIN_SHIFT);
  GPIO->USARTROUTE[0].RXROUTE = (EXP_SPI_CIPO_PORT << _GPIO_USART_RXROUTE_PORT_SHIFT)
      | (EXP_SPI_CIPO_PIN << _GPIO_USART_RXROUTE_PIN_SHIFT);
  GPIO->USARTROUTE[0].CLKROUTE = (EXP_SPI_SCK_PORT << _GPIO_USART_CLKROUTE_PORT_SHIFT)
      | (EXP_SPI_SCK_PIN << _GPIO_USART_CLKROUTE_PIN_SHIFT);
  GPIO->USARTROUTE[0].CSROUTE = (EXP_SPI_CS_PORT << _GPIO_USART_CSROUTE_PORT_SHIFT)
      | (EXP_SPI_CS_PIN << _GPIO_USART_CSROUTE_PIN_SHIFT);

  // Enable USART interface pins
  GPIO->USARTROUTE[0].ROUTEEN = GPIO_USART_ROUTEEN_RXPEN  |    // CIPO
                                GPIO_USART_ROUTEEN_TXPEN  |    // COPI
                                GPIO_USART_ROUTEEN_CLKPEN |
                                GPIO_USART_ROUTEEN_CSPEN;

  // Configure and enable USART0
  USART_InitSync(USART0, &init);

  /*
   * Add 1 bit time delay between characters to allow secondary sufficient
   * time to set up next character
   */
  USART0->TIMING_SET = USART_TIMING_ICS_ONE;
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
   * Clear the receive channel's done flag if set and go to
   * send state
   */
  if (flags & (1 << RX_LDMA_CHANNEL)) {
    LDMA_IntClear(1 << RX_LDMA_CHANNEL);
    appState = SEND;
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
  // Initialize GPIO and USART0
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
  switch (appState) {
    case INIT:
    case RECEIVE:
      /*
       * This state is used to allow the main.c while() loop to go into EM1
       * in order to reduce current draw while the peripheral is running.
       */
      break;
    case SEND:

      uint32_t i;
      // Zero incoming buffer and populate outgoing data array
      for (i = 0; i < BUFLEN; i++){
        inbuf[i] = 0;
        outbuf[i] = (uint8_t)i;
      }

      // Start both channels
      LDMA_StartTransfer(RX_LDMA_CHANNEL, &ldmaRXConfig, &ldmaRXDescriptor);
      LDMA_StartTransfer(TX_LDMA_CHANNEL, &ldmaTXConfig, &ldmaTXDescriptor);

      // Go to receive state
      appState = RECEIVE;
      break;
    default:
      break;
  }
}

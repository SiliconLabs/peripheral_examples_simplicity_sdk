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
#include "sl_interrupt_manager.h"

#include "em_eusart.h"
#include "sl_gpio.h"

#include "pin_config.h"

/*******************************************************************************
 ***************************   GLOBAL VARIABLES   ******************************
 ******************************************************************************/
// Size of the buffer for received data
#define BUFLEN   80

// Receive data buffer
uint8_t buffer[BUFLEN];

// Current position in buffer
uint32_t buffer_pos;

const sl_gpio_t EUSART0_TX = { .port = EUSART0_TX_PORT,
                               . pin = EUSART0_TX_PIN };

typedef enum {
  WAIT,
  RX_COMPLETE,
  TX_COMPLETE
}
App_State_t;

/*******************************************************************************
 *******************************   DEFINES   ***********************************
 ******************************************************************************/
// In low-frequency mode, the maximum EUSART baud rate is 9600
#define BAUDRATE 9600
/*
 * As explained in the readme.md file, this example is designed to
 * run with one device being the initial transmitter and the other
 * being the initial receiver before switching roles.  Use the #define
 * below to select the operating state.
 */
#define INITIAL_TRANSMITTER
#define INITIAL_RECEIVER

#if defined INITIAL_TRANSMITTER
  static uint8_t tx_success_msg[] = "Initial TX: Receive success and transmitting now\r\n";
  App_State_t app_state = RX_COMPLETE;
#elif defined INITIAL_RECEIVER
  static uint8_t tx_success_msg[] = "Initial RX: Receive success and transmitting now\r\n";
  App_State_t app_state = WAIT;
#endif

/**************************************************************************//**
 * @brief  Initialize GPIO
 *****************************************************************************/
void gpio_init(void)
{
  // Configure the EUSART TX pin to the board controller as an output
  sl_gpio_set_pin_mode(&EUSART0_TX, SL_GPIO_MODE_PUSH_PULL, 0);

  // Route EUSART0 TX to the board controller TX pins
  GPIO->EUSARTROUTE[0].TXROUTE = (EUSART0_TX_PORT << _GPIO_EUSART_TXROUTE_PORT_SHIFT)
                                  | (EUSART0_TX_PIN << _GPIO_EUSART_TXROUTE_PIN_SHIFT);

  // Enable RX and TX signals now that they have been routed
  GPIO->EUSARTROUTE[0].ROUTEEN = GPIO_EUSART_ROUTEEN_TXPEN;
}

/**************************************************************************//**
 * @brief  Initialize EUSART
 *****************************************************************************/
void eusart_init(void)
{
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_EUSART0);
  // Initialize the EUSART0 for low-frequency operation
  EUSART_UartInit_TypeDef init = EUSART_UART_INIT_DEFAULT_LF;
  EUSART_AdvancedInit_TypeDef advance_init = EUSART_ADVANCED_INIT_DEFAULT;

  init.baudrate = BAUDRATE;
  init.advancedSettings = &advance_init;
  init.advancedSettings->txAutoTristate = true;
  init.loopbackEnable = eusartLoopbackEnable;
  init.enable = eusartEnable;

  EUSART0->TIMINGCFG = EUSART_TIMINGCFG_TXDELAY_TRIPPLE;

  EUSART_UartInitLf(EUSART0, &init);
  
  // Enable USART Interrupts
  sl_interrupt_manager_clear_irq_pending(EUSART0_RX_IRQn);
  sl_interrupt_manager_enable_irq(EUSART0_RX_IRQn);

  sl_interrupt_manager_clear_irq_pending(EUSART0_TX_IRQn);
  sl_interrupt_manager_enable_irq(EUSART0_TX_IRQn);
}

/**************************************************************************//**
 * @brief EUSART Receive Interrupt Handler - Saves incoming characters
 *****************************************************************************/
void EUSART0_RX_IRQHandler(void)
{
  // Get the character just received
  buffer[buffer_pos] = EUSART_Rx(EUSART0);
  // Exit loop on new line or buffer full
  if ((buffer[buffer_pos] != '\n') && (buffer_pos < BUFLEN)) {
    buffer_pos++;
    app_state = WAIT;
  } else {
    // Reset buffer index
    buffer_pos = 0;
    app_state = RX_COMPLETE;
    EUSART_IntDisable(EUSART0, EUSART_IEN_RXFL);
  }
  /*
   * The EUSART differs from the USART in that explicit clearing of
   * RX interrupt flags is required even after emptying the RX FIFO.
   */
  uint32_t int_flags = EUSART_IntGet(EUSART0);
  EUSART_IntClear(EUSART0, int_flags);
}

/**************************************************************************//**
 * @brief EUSART Transmit Interrupt Handler - Outputs characters
 *****************************************************************************/
void EUSART0_TX_IRQHandler(void)
{
  EUSART_IntClear(EUSART0, EUSART_IF_TXC);

  // Stop sending when at string NULL terminator
  if (buffer[buffer_pos] != 0) {
    EUSART_Tx(EUSART0, buffer[buffer_pos++]);
    app_state = WAIT;
  } else {
  /*
   * Need to disable the transmit buffer level interrupt in this IRQ
   * handler when done or it will immediately trigger again upon exit
   * even though there is no data left to send.
   */
    EUSART_IntDisable(EUSART0, EUSART_IEN_TXC);
    app_state = TX_COMPLETE;
    // Reset buffer index
    buffer_pos = 0;
  }
}

/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  gpio_init();

  eusart_init();
  
  uint32_t i;
  // Zero out buffer
  for (i = 0; i < BUFLEN; i++) {
     buffer[i] = 0;
  }
  // Reset buffer index
  buffer_pos = 0;
  /*
   * Enable receiving.  The eusartEnableRx enumeration enables the
   * receiver only, thus disabling the transmitter.
   */
  EUSART_Enable(EUSART0, eusartEnableRx);
  EUSART_IntEnable(EUSART0, EUSART_IEN_RXFL);
}

/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{
  switch(app_state){
    case RX_COMPLETE:
      uint32_t i;
      // Copy the outbound message to the buffer
      for (i = 0 ; tx_success_msg[i] != 0; i++) {
       buffer[i] = tx_success_msg[i];
      }

      // Reset buffer index
      buffer_pos = 0;
      /*
      * Enable transmitting.  The eusartEnableTx enumeration enables the
      * transmitter only, thus disabling the receiver.  Setting the TXC
      * flag forces execution of the transmit of the transmit interrupt
      * handler, which sends the first outgoing byte.
      */
      EUSART_Enable(EUSART0, eusartEnableTx);
      EUSART_IntEnable(EUSART0, EUSART_IEN_TXC);
      EUSART_IntSet(EUSART0, EUSART_IF_TXC);
      break;

    case WAIT:
      break;

    case TX_COMPLETE:
      // Zero out buffer
      for (i = 0; i < BUFLEN; i++) {
        buffer[i] = 0;
      }
      // Reset buffer index
      buffer_pos = 0;
      /*
      * Enable receiving.  The eusartEnableRx enumeration enables the
      * receiver only, thus disabling the transmitter.
      */
      EUSART_Enable(EUSART0, eusartEnableRx);
      EUSART_IntEnable(EUSART0, EUSART_IEN_RXFL);
      break;

     default:
      break;
  }
}

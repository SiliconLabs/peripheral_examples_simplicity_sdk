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
#include "sl_interrupt_manager.h"

#include "sl_gpio.h"
#include "em_gpio.h"
#include "em_eusart.h"

#include "pin_config.h"

#define BAUDRATE 9600

// Size of the buffer for received data
#define BUFLEN  80

typedef enum {
  WAIT,
  RX_COMPLETE,
  TX_COMPLETE
}
App_State_t;

/*
 * As explained in the readme.md file, this example is designed to
 * run with one device being the initial transmitter and the other
 * being the initial receiver before switching roles.  Use the #define
 * below to select the operating state.
 */
#define INITIAL_TRANSMITTER
//#define INITIAL_RECEIVER

#if defined INITIAL_TRANSMITTER
  static char txSuccessMsg[] = "Initial TX: Receive success and transmitting now\r\n";
  volatile App_State_t app_state = RX_COMPLETE;
#elif defined INITIAL_RECEIVER
  static char txSuccessMsg[] = "Initial RX: Receive success and transmitting now\r\n";
  volatile App_State_t app_state = WAIT;
#endif


// Receive data buffer
uint8_t buffer[BUFLEN];

// Current position in buffers
volatile uint32_t inpos;
volatile uint32_t outpos;

const sl_gpio_t EUART0_TX  = { .port = EUART0_TX_PORT,
                                .pin = EUART0_TX_PIN };

/**************************************************************************//**
 * @brief  Initialize GPIO
 *****************************************************************************/
void gpio_init(void)
{
  // Configure the EUART TX pin
  sl_gpio_set_pin_mode(&EUART0_TX, SL_GPIO_MODE_PUSH_PULL, 1);

  // Route EUART0 TX to the board controller TX pins
  GPIO->EUARTROUTE->TXROUTE = (EUART0_TX_PORT << _GPIO_EUART_TXROUTE_PORT_SHIFT)
                            | (EUART0_TX_PIN << _GPIO_EUART_TXROUTE_PIN_SHIFT);
  // Enable TX signal
  GPIO->EUARTROUTE->ROUTEEN = GPIO_EUART_ROUTEEN_TXPEN;
}

/**************************************************************************//**
 * @brief
 *    EUART initialization
 *****************************************************************************/
void euart0_init(void)
{
  // Enable EUART0 Bus Clock
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_EUART0);

  // Initialize the EUART0 module
  EUSART_UartInit_TypeDef init = EUSART_UART_INIT_DEFAULT_HF;
  EUSART_AdvancedInit_TypeDef advance_init = EUSART_ADVANCED_INIT_DEFAULT;
  init.baudrate = BAUDRATE;
  init.advancedSettings = &advance_init;
  init.advancedSettings->txAutoTristate = true;
  init.loopbackEnable = eusartLoopbackEnable;
  init.enable = eusartEnable;
  EUART0->TIMINGCFG = EUSART_TIMINGCFG_TXDELAY_TRIPPLE;
  EUSART_UartInitHf(EUART0, &init);

  // Enable EUART0 IRQs
  sl_interrupt_manager_clear_irq_pending(EUART0_RX_IRQn);
  sl_interrupt_manager_enable_irq(EUART0_RX_IRQn);
  sl_interrupt_manager_clear_irq_pending(EUART0_TX_IRQn);
  sl_interrupt_manager_enable_irq(EUART0_TX_IRQn);
}

/**************************************************************************//**
 * @brief
 *    The EUART0 receive interrupt saves incoming characters.
 *****************************************************************************/
void EUART0_RX_IRQHandler(void)
{
  // Get the character just received
  buffer[inpos] = EUSART_Rx(EUART0);

  // Exit loop on new line or buffer full
  if ((buffer[inpos] != '\n') && (inpos < BUFLEN)) {
    inpos++;
    app_state = WAIT;
  }
  else {
    inpos = 0;
    app_state = RX_COMPLETE;  // Stop receiving on CR
    EUSART_IntDisable(EUART0, EUSART_IEN_RXFLIEN);
  }

  // Clear the requesting interrupt before exiting the handler
  EUSART_IntClear(EUART0, EUSART_IF_RXFLIF);
}

/**************************************************************************//**
 * @brief
 *    The EUART0 transmit interrupt outputs characters.
 *****************************************************************************/
void EUART0_TX_IRQHandler(void)
{
  // Send a previously received character
  //if (outpos<inpos) {
  if (buffer[outpos] != 0) {
    EUSART_Tx(EUART0,buffer[outpos++]);
    app_state = WAIT;
  }
  else
  /*
   * Need to disable the transmit buffer level interrupt in this IRQ
   * handler when done or it will immediately trigger again upon exit
   * even though there is no data left to send.
   */
  {
    outpos = 0;
    app_state = TX_COMPLETE;   // Go back into receive when all is sent
    EUSART_IntDisable(EUART0, EUSART_IEN_TXCIEN);
  }

  // Clear the requesting interrupt before exiting the handler
  EUSART_IntClear(EUART0, EUSART_IF_TXCIF);
}

/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  gpio_init();
  euart0_init();

  // Switch to RX
  EUSART_Enable(EUART0, eusartEnableRx);
  EUSART_IntEnable(EUART0, EUSART_IEN_RXFLIEN);
}

/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{
  switch (app_state) {
    case RX_COMPLETE:
      uint32_t i;
      for (uint32_t i = 0 ; txSuccessMsg[i]!=0; i++) {
        buffer[i] = txSuccessMsg[i];
      }

      // Switch to TX
      EUSART_Enable(EUART0, eusartEnableTx);
      EUSART_IntEnable(EUART0, EUSART_IEN_TXCIEN);
      EUSART_IntSet(EUART0, EUSART_IF_TXCIF);
      break;

    case TX_COMPLETE:
      for (i = 0; i < BUFLEN; i++) {
        buffer[i] = 0;
      }
      // Switch to RX
      EUSART_Enable(EUART0, eusartEnableRx);
      EUSART_IntEnable(EUART0, EUSART_IEN_RXFLIEN);

      break;
    case WAIT:
      break;
    default:
      break;

  }
}

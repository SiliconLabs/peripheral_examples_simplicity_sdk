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
#include "sl_board_control_config.h"
#include "sl_power_manager.h"

// Size of the data buffers
#define BUFLEN  10

typedef enum {
  INIT,
  RECEIVE_START,
  WAIT,
  RECEIVE_DONE,
} AppState_t;

AppState_t appState;

// Outgoing data
uint8_t outbuf[BUFLEN];

// Incoming data
uint8_t inbuf[BUFLEN];

// Position in the buffer
uint32_t bufpos;

const sl_gpio_t US0COPI = { .port = EXP_SPI_COPI_PORT, .pin = EXP_SPI_COPI_PIN };
const sl_gpio_t US0CIPO = { .port = EXP_SPI_CIPO_PORT, .pin = EXP_SPI_CIPO_PIN };
const sl_gpio_t US0CLK = { .port = EXP_SPI_SCK_PORT, .pin = EXP_SPI_SCK_PIN };
const sl_gpio_t US0CS = { .port = EXP_SPI_CS_PORT, .pin = EXP_SPI_CS_PIN };
/*
 * The TIMEPORT/TIMEPIN is not part of the SPI bus.  It shows when the
 * CPU responds to the main before, during, and after data transfer.
 * Use a logic analyzer to capture the activity on this pin along with
 * the bus traffic to understand the timing relationship between the
 * CPU and the SPI during interrupt-driven transfers.
 */
const sl_gpio_t TIME = { .port = SPI_TIME_PORT, .pin = SPI_TIME_PIN };

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

  // Configure TIME pin as an output
  sl_gpio_set_pin_mode(&TIME, SL_GPIO_MODE_PUSH_PULL, 0);

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

  // Configure but do not enable USART0
  USART_InitSync(USART0, &init);

  // Route USART0 RX, TX, CLK, and CS to the specified pins.
  GPIO->USARTROUTE[0].RXROUTE = (EXP_SPI_CIPO_PORT << _GPIO_USART_RXROUTE_PORT_SHIFT)
      | (EXP_SPI_CIPO_PIN << _GPIO_USART_RXROUTE_PIN_SHIFT);
  GPIO->USARTROUTE[0].TXROUTE = (EXP_SPI_COPI_PORT << _GPIO_USART_TXROUTE_PORT_SHIFT)
      | (EXP_SPI_COPI_PIN << _GPIO_USART_TXROUTE_PIN_SHIFT);
  GPIO->USARTROUTE[0].CLKROUTE = (EXP_SPI_SCK_PORT << _GPIO_USART_CLKROUTE_PORT_SHIFT)
      | (EXP_SPI_SCK_PIN << _GPIO_USART_CLKROUTE_PIN_SHIFT);
  GPIO->USARTROUTE[0].CSROUTE = (EXP_SPI_CS_PORT << _GPIO_USART_CSROUTE_PORT_SHIFT)
      | (EXP_SPI_CS_PIN << _GPIO_USART_CSROUTE_PIN_SHIFT);

  // Enable USART interface pins
  GPIO->USARTROUTE[0].ROUTEEN = GPIO_USART_ROUTEEN_RXPEN  |    // CIPO
                                GPIO_USART_ROUTEEN_TXPEN  |    // COPI
                                GPIO_USART_ROUTEEN_CLKPEN |
                                GPIO_USART_ROUTEEN_CSPEN;

  // Enable NVIC USART sources
  sl_interrupt_manager_clear_irq_pending(USART0_RX_IRQn);
  sl_interrupt_manager_enable_irq(USART0_RX_IRQn);
}

/**************************************************************************//**
 * @brief
 *    The USART0 receive interrupt saves incoming characters.
 *****************************************************************************/
void USART0_RX_IRQHandler(void)
{
  // Drive the activity pin high to denote IRQ handler entry
  sl_hal_gpio_set_pin(&TIME);

  /*
   * Save the byte received concurrent with the transmission of the
   * last bit of the previous outgoing byte, and increment the buffer
   * position to the next byte.  Note that this read clears the
   * USART_IF_RXDATAV interrupt flag.
   */
  inbuf[bufpos++] = USART0->RXDATA;

  // If there are still bytes left to send, transmit the next one
  if (bufpos < BUFLEN) {
      USART0->TXDATA = outbuf[bufpos];
  }
  else {
      appState = RECEIVE_DONE;
  }

  // Drive the activity pin low to denote IRQ handler exit
  sl_hal_gpio_clear_pin(&TIME);
}

/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  // Initialize GPIO and USART0
  initGPIO();
  initUSART0();
  // Drive the activity pin high to denote IRQ handler entry
  sl_hal_gpio_set_pin(&TIME);
}

/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{
  switch (appState){
    case RECEIVE_DONE:
      // Drive the activity pin high to show prep for next data transfer
      sl_hal_gpio_set_pin(&TIME);

      // Disable receive data interrupt
      USART_IntDisable(USART0, USART_IEN_RXDATAV);

      // Disable the falling edge interrupt on the US0CS_PIN
      sl_gpio_disable_interrupts(1 << EXP_SPI_CS_PIN);

      // Disable USART receiver and transmitter until next chip select
      USART_Enable(USART0, usartDisable);

      /*
       * Fall through intentional in order to prepare the MCU
       * to receive the next CS interrupt
       */
    case INIT:
      uint32_t i;
      // Zero incoming buffer and populate outgoing data array
      for (i = 0; i < BUFLEN; i++)
      {
        inbuf[i] = 0;
        outbuf[i] = (uint8_t)i;
      }

      // Start at the beginning of the buffer
      bufpos = 0;

      // Enable the falling edge interrupt on the US0CS_PIN
      sl_hal_gpio_clear_interrupts(1 << EXP_SPI_CS_PIN);
      sl_hal_gpio_enable_interrupts(1 << EXP_SPI_CS_PIN);

      // Drive the activity pin low when ready for CS assertion
      sl_hal_gpio_clear_pin(&TIME);

      break;
    case RECEIVE_START:
      /*
       * Drive the activity pin high on wake from EM1 immediately after
       * CS assertion
       */
      sl_hal_gpio_set_pin(&TIME);

      // Now enable the USART receiver and transmitter
      USART_Enable(USART0, usartEnable);

      // Enable receive data valid interrupt
      USART_IntEnable(USART0, USART_IEN_RXDATAV);

      /*
       * Transmit the first byte, then go into EM1.  The IRQ handler will
       * receive each incoming byte and transmit the next outgoing byte.
       */
      USART0->TXDATA = outbuf[bufpos];

      appState = WAIT;
      // Drive the activity pin low when ready to receive data
      sl_hal_gpio_clear_pin(&TIME);
      break;
    case WAIT:
      // Wait in EM1 until all data is received
      break;
  }
}

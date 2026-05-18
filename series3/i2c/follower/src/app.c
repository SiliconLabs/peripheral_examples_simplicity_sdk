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

#include "sl_gpio.h"
#include "sl_device_peripheral.h"
#include "sl_hal_i2c.h"

#include "pin_config.h"
#include "peripheral_config.h"

#define I2C_ADDRESS       0xE2
#define I2C_BUFFER_SIZE   10

const sl_gpio_t GPIO_LED0 = { .port = LED0_PORT, .pin = LED0_PIN };
const sl_gpio_t GPIO_LED1 = { .port = LED1_PORT, .pin = LED1_PIN };
const sl_gpio_t I2C_SCL   = { .port = EXP_I2C_SCL_PORT, .pin = EXP_I2C_SCL_PIN };
const sl_gpio_t I2C_SDA   = { .port = EXP_I2C_SDA_PORT, .pin = EXP_I2C_SDA_PIN };

/*******************************************************************************
 ***************************   GLOBAL VARIABLES   ******************************
 ******************************************************************************/

// Buffers
uint8_t i2c_Buffer[I2C_BUFFER_SIZE];
uint8_t i2c_BufferIndex;

// Transmission flags
volatile bool i2c_gotTargetAddress;
volatile bool i2c_rxInProgress;

/*******************************************************************************
 ***************************   LOCAL FUNCTIONS   ******************************
 ******************************************************************************/

void I2C0_Handler(void);

/***************************************************************************//**
 * @brief GPIO initialization
 ******************************************************************************/
void gpio_init(void)
{
  // Initialize GPIO driver
  sl_gpio_init();

  // Configure LED0 and LED1 as output
  sl_gpio_set_pin_mode(&GPIO_LED0, SL_GPIO_MODE_PUSH_PULL, LED_OFF);
  sl_gpio_set_pin_mode(&GPIO_LED1, SL_GPIO_MODE_PUSH_PULL, LED_OFF);
}

/***************************************************************************//**
 * @brief Setup I2C
 ******************************************************************************/
void i2c_init(void)
{
  uint32_t clock_branch_freq = 0;

  // Enable I2C clock tree
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_I2C0);

  // Enable SCL and SDA GPIO pins
  sl_gpio_set_pin_mode(&I2C_SCL, SL_GPIO_MODE_WIRED_AND_PULLUP_FILTER, 1);
  sl_gpio_set_pin_mode(&I2C_SDA, SL_GPIO_MODE_WIRED_AND_PULLUP_FILTER, 1);

  // Route I2C pins to GPIO
  GPIO->I2CROUTE[0].SDAROUTE = (GPIO->I2CROUTE[0].SDAROUTE & ~_GPIO_I2C_SDAROUTE_MASK)
                        | (I2C_SDA.port << _GPIO_I2C_SDAROUTE_PORT_SHIFT
                        | (I2C_SDA.pin << _GPIO_I2C_SDAROUTE_PIN_SHIFT));
  GPIO->I2CROUTE[0].SCLROUTE = (GPIO->I2CROUTE[0].SCLROUTE & ~_GPIO_I2C_SCLROUTE_MASK)
                        | (I2C_SCL.port << _GPIO_I2C_SCLROUTE_PORT_SHIFT
                        | (I2C_SCL.pin << _GPIO_I2C_SCLROUTE_PIN_SHIFT));
  GPIO->I2CROUTE[0].ROUTEEN = GPIO_I2C_ROUTEEN_SDAPEN | GPIO_I2C_ROUTEEN_SCLPEN;

  // Configure to be addressable as follower
  sl_hal_i2c_init(I2C0, SL_I2C_FOLLOWER_MODE);

  sl_clock_manager_get_clock_branch_frequency(SL_PERIPHERAL_I2C0->clk_branch, &clock_branch_freq);

  // Set the I2C Bus Frequency.
  sl_hal_i2c_set_clock_frequency(I2C0, clock_branch_freq, 100000, SL_I2C_CLK_HLR_STANDARD);

  // Enable I2C.
  sl_hal_i2c_enable(I2C0);

  // Initialize the buffer index
  i2c_BufferIndex = 0;

  // Set up to enable follower mode
  sl_hal_i2c_set_follower_address(I2C0, I2C_ADDRESS, false);
  sl_hal_i2c_set_follower_mask_address(I2C0, 0xFE); // must match exact address

  // Set the I2C interrupt handler
  sl_interrupt_manager_set_irq_handler(I2C0_IRQn, I2C0_Handler);

  // Enable I2C interrupts
  sl_hal_i2c_clear_interrupts(I2C0, _I2C_IF_MASK);
  sl_hal_i2c_enable_interrupts(I2C0, I2C_IEN_ADDR | I2C_IEN_RXDATAV | I2C_IEN_ACK | I2C_IEN_SSTOP | I2C_IEN_BUSERR | I2C_IEN_ARBLOST);
  sl_interrupt_manager_clear_irq_pending(I2C0_IRQn);
  sl_interrupt_manager_enable_irq(I2C0_IRQn);
}

/***************************************************************************//**
 * @brief I2C Interrupt Handler
 ******************************************************************************/
void I2C0_Handler(void)
{
  uint32_t pending;
  uint32_t rxData;

  pending = I2C0->IF;

  // If some sort of fault, abort transfer.
  if (pending & (I2C_IF_BUSERR | I2C_IF_ARBLOST)) {
    i2c_rxInProgress = false;
    sl_gpio_set_pin(&GPIO_LED1);
  } else {
    if (pending & I2C_IF_ADDR) {
      // Address Match, indicating that reception is started
      // Stay in EM1 until RX completes
      i2c_rxInProgress = true;

      rxData = I2C0->RXDATA;
      sl_hal_i2c_clear_interrupts(I2C0, I2C_IF_ADDR | I2C_IF_RXDATAV);

      if (rxData & 0x1) { // read bit set
        if (i2c_BufferIndex < I2C_BUFFER_SIZE) {
          // Transfer data
          I2C0->TXDATA     = i2c_Buffer[i2c_BufferIndex++];
        } else {
          // Invalid buffer index; transfer data as if follower non-responsive
          I2C0->TXDATA     = 0xFF;
        }
      } else {
        i2c_gotTargetAddress = false;
      }

      I2C0->CMD = I2C_CMD_ACK;

      // Turn on LED0
#if (LED_ON == 1)
      sl_gpio_set_pin(&GPIO_LED0);
#else
      sl_gpio_clear_pin(&GPIO_LED0);
#endif
    } else if (pending & I2C_IF_RXDATAV) {
      rxData = I2C0->RXDATA;
      sl_hal_i2c_clear_interrupts(I2C0, I2C_IF_RXDATAV);

      if (!i2c_gotTargetAddress) {
        /******************************************************/
        /* Read target address from leader                    */
        /******************************************************/
        // Verify that target address is valid
        if (rxData < I2C_BUFFER_SIZE) {
          // Store target address
          i2c_BufferIndex = rxData;
          i2c_gotTargetAddress = true;
          I2C0->CMD = I2C_CMD_ACK;
        } else {
          I2C0->CMD = I2C_CMD_NACK;
        }
      } else {
        /******************************************************/
        /* Read new data and write to target address          */
        /******************************************************/
        // Verify that target address is valid
        if (i2c_BufferIndex < I2C_BUFFER_SIZE) {
          // Write new data to target address; auto increment target address
          i2c_Buffer[i2c_BufferIndex++] = rxData;
          I2C0->CMD = I2C_CMD_ACK;
        } else {
          I2C0->CMD = I2C_CMD_NACK;
        }
      }
    }

    if (pending & I2C_IF_ACK) {
      /******************************************************/
      /* Leader ACK'ed, so requesting more data             */
      /******************************************************/
      if (i2c_BufferIndex < I2C_BUFFER_SIZE) {
        // Transfer data
        I2C0->TXDATA = i2c_Buffer[i2c_BufferIndex++];
      } else {
        // Invalid buffer index; transfer data as if follower non-responsive
        I2C0->TXDATA = 0xFF;
      }

      sl_hal_i2c_clear_interrupts(I2C0, I2C_IF_ACK);
    }

    if (pending & I2C_IF_SSTOP) {
      // RX complete
      i2c_rxInProgress = false;

      // Turn off LED0
#if (LED_ON == 1)
      sl_gpio_clear_pin(&GPIO_LED0);
#else
      sl_gpio_set_pin(&GPIO_LED0);
#endif
      sl_hal_i2c_clear_interrupts(I2C0, I2C_IF_SSTOP);
    }
  }
}

/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  // Initialize GPIO and I2C
  gpio_init();
  i2c_init();
}

/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{
}

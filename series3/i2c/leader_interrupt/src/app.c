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

#include "app.h"

/*******************************************************************************
 *******************************   DEFINES   ***********************************
 ******************************************************************************/

#define I2C_FOLLOWER_ADDRESS              0xE2
#define I2C_TXBUFFER_SIZE                 10
#define I2C_RXBUFFER_SIZE                 10

// Read/write bit/mask for the I2C follower device address
#define I2C_RNOTW_BIT           0x01
#define I2C_RNOTW_MASK          0xFE

/** Error flags indicating that the I2C transfer has failed. */
/* Notice that I2C_IF_TXOF (transmit overflow) is not really possible with */
/* the software-supporting master mode. Likewise, for I2C_IF_RXUF (receive underflow) */
/* RXUF is only likely to occur with the software if using a debugger peeking into */
/* the RXDATA register. Therefore, those types of faults are ignored. */
#define I2C_IF_ERRORS    (I2C_IF_BUSERR | I2C_IF_ARBLOST)
#define I2C_IEN_ERRORS   (I2C_IEN_BUSERR | I2C_IEN_ARBLOST)

#define I2C_ERROR_INTERRUPT             I2C_IEN_SDAERR | I2C_IEN_SCLERR \
  | I2C_IEN_CLERR | I2C_IEN_BUSERR

const sl_gpio_t GPIO_BUTTON0     = { .port = BUTTON0_PORT, .pin = BUTTON0_PIN };
const sl_gpio_t GPIO_LED0        = { .port = LED0_PORT,    .pin = LED0_PIN };
const sl_gpio_t GPIO_LED1        = { .port = LED1_PORT,    .pin = LED1_PIN };
const sl_gpio_t I2C_SCL          = { .port = EXP_I2C_SCL_PORT, .pin = EXP_I2C_SCL_PIN };
const sl_gpio_t I2C_SDA          = { .port = EXP_I2C_SDA_PORT, .pin = EXP_I2C_SDA_PIN };

// I2C State Machine enum
enum i2c_state_machine {
  I2C_IDLE_STATE = 0,
  I2C_START_STATE = 0x1,
  I2C_ACK_TRANSMISSION_STATE = 0x2,
  I2C_NACK_STATE = 0x6,
  I2C_ERR_STATE = 0x7
};

// I2C Transfer Type enum
enum i2c_transfer_type {
  I2C_TRANSFER_READ,
  I2C_TRANSFER_WRITE
};

// I2C Test State enum
enum i2c_test_state {
  I2C_TEST_IDLE_STATE,
  I2C_TEST_READ_STATE,
  I2C_TEST_WRITE_STATE,
  I2C_TEST_READ_AND_VERIFY_STATE
};

/*******************************************************************************
 ***************************   GLOBAL VARIABLES   ******************************
 ******************************************************************************/

// Buffers
uint8_t i2c_txBuffer[I2C_TXBUFFER_SIZE];
uint8_t i2c_rxBuffer[I2C_RXBUFFER_SIZE];

// Global I2C transfer type
uint32_t i2c_transferType;

// I2C state variables and flags
volatile uint8_t  i2c_testState;       // read/update/verify test state machine
volatile uint32_t i2c_state;           // i2c state machine
volatile bool  i2c_testInProgress;
volatile bool  i2c_transferInProgress;
volatile bool  i2c_targetAddressSent;  // flag check for read
volatile uint32_t rxBufferIndex;
volatile uint32_t txBufferIndex;

/***************************************************************************//**
 * @brief GPIO Interrupt handler
 ******************************************************************************/
static void button_0_change(void)
{
  // Enable i2c.
  sl_hal_i2c_enable(I2C0);
  i2c_testInProgress = true;
}

/***************************************************************************//**
 * @brief GPIO initialization
 ******************************************************************************/
void gpio_init(void)
{
  int32_t int_no = GPIO_BUTTON0.pin;

  // Initialize GPIO driver
  sl_gpio_init();

  // Configure LED0 and LED1 as output
  sl_gpio_set_pin_mode(&GPIO_LED0, SL_GPIO_MODE_PUSH_PULL, LED_OFF);
  sl_gpio_set_pin_mode(&GPIO_LED1, SL_GPIO_MODE_PUSH_PULL, LED_OFF);

  // Configure PB0 as input and interrupt
  sl_gpio_set_pin_mode(&GPIO_BUTTON0, SL_GPIO_MODE_INPUT_PULL, 1);
  sl_gpio_configure_external_interrupt(&GPIO_BUTTON0, &int_no,
                                       SL_GPIO_INTERRUPT_FALLING_EDGE,
                                       (sl_gpio_irq_callback_t)button_0_change,
                                       NULL);
}

/***************************************************************************//**
 * I2C0 interrupt handler
 ******************************************************************************/
void I2C0_Handler(void)
{
  uint32_t flags = sl_hal_i2c_get_pending_interrupts(I2C0);
  sl_hal_i2c_clear_interrupts(I2C0, flags);

  // if in the middle of a transfer transfer complete; not sure why we'd get here
  if (!i2c_transferInProgress) {
    return;
  }

  // stop condition reached
  if (flags & I2C_IF_MSTOP) {
    i2c_state = I2C_IDLE_STATE;
    i2c_transferInProgress = false;    // transfer done
    return;
  }

  if (flags & (I2C_ERROR_INTERRUPT)) {
    i2c_state = I2C_ERR_STATE;
    return;
  }

  if (flags & I2C_IF_NACK) {
    // transmission NACKed, stop transmission
    I2C0->CMD_SET = I2C_CMD_STOP;
    return;
  }

  // I2C state machine
  switch (i2c_state) {
    case I2C_IDLE_STATE:
      // All transmissions start with a write to set target address
      if (flags & I2C_IF_START) {
        // ACKed restart
        if ((i2c_transferType == I2C_TRANSFER_READ) & i2c_targetAddressSent) {
          // target address sent already, issue a read request
          I2C0->TXDATA = (I2C_FOLLOWER_ADDRESS | I2C_RNOTW_BIT);
        } else { // ACKed initial start
          // write target address and write request
          I2C0->TXDATA = I2C_FOLLOWER_ADDRESS;
        }
        i2c_state = I2C_START_STATE;
      }
      break;

    case I2C_START_STATE:
      // ACK or NACK from follower
      if (flags & I2C_IF_ACK) {
        i2c_state = I2C_ACK_TRANSMISSION_STATE;

        // Address recognized, check RXDATAV if read
        if ((i2c_transferType == I2C_TRANSFER_READ) &
            (flags & I2C_IF_RXDATAV) & i2c_targetAddressSent) {
          // data valid, read from RXDATA
          i2c_rxBuffer[rxBufferIndex++] = I2C0->RXDATA;
          sl_hal_i2c_clear_interrupts(I2C0, I2C_IF_RXDATAV);

          // If index has reached end of buffer, end transmission
          if (rxBufferIndex >= I2C_RXBUFFER_SIZE) {
            I2C0->CMD_SET = I2C_CMD_STOP | I2C_CMD_NACK;
          } else {
            // leader ACKs reception of follower data transmission
            I2C0->CMD_SET = I2C_CMD_ACK;
          }
        }
        else if ((i2c_transferType == I2C_TRANSFER_WRITE) | !i2c_targetAddressSent) {
          // first send the follower buffer offset (always read from beginning)
          I2C0->TXDATA = (uint8_t)0;
        }
      }
      break;

    case I2C_ACK_TRANSMISSION_STATE:
      // if reading, RXDATA should be valid
      // if writing, expecting an ACK from follower

      // Read transmission follower ACK for setting target address
      if (i2c_transferType == I2C_TRANSFER_READ) {
        if (flags & I2C_IF_ACK) {
          // This is an ACK for setting the target address
          i2c_targetAddressSent = true;

          // Send restart
          i2c_state = I2C_IDLE_STATE;
          I2C0->CMD_SET = I2C_CMD_START;
        }

        if (flags & I2C_IF_RXDATAV) {
          // Store received data
          i2c_rxBuffer[rxBufferIndex++] = (uint8_t)I2C0->RXDATA;

          // If index has reached end of buffer, end transmission
          if (rxBufferIndex >= I2C_RXBUFFER_SIZE) {
            I2C0->CMD_SET = I2C_CMD_STOP | I2C_CMD_NACK;
          } else {
            // leader ACKs reception of follower data transmission
            I2C0->CMD_SET = I2C_CMD_ACK;
          }

          sl_hal_i2c_clear_interrupts(I2C0, I2C_IF_RXDATAV);
        }
      } else {
        // Write transmission; expecting ACK from follower
        if (flags & I2C_IF_ACK) {
          // continue transmission
          I2C0->TXDATA = (uint8_t)i2c_txBuffer[txBufferIndex++];

          // no more data left to send, stop transfer after last byte
          if (txBufferIndex >= I2C_TXBUFFER_SIZE) {
            I2C0->CMD_SET = I2C_CMD_STOP;
          }
        }
      }
      break;

    default:
      break;
  }
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
  sl_hal_i2c_init(I2C0, SL_I2C_LEADER_MODE);

  sl_clock_manager_get_clock_branch_frequency(SL_PERIPHERAL_I2C0->clk_branch, &clock_branch_freq);

  // Set the I2C Bus Frequency.
  sl_hal_i2c_set_clock_frequency(I2C0, clock_branch_freq, 100000, SL_I2C_CLK_HLR_STANDARD);

  // Set the I2C interrupt handler
  sl_interrupt_manager_set_irq_handler(I2C0_IRQn, I2C0_Handler);

  // Enable the I2C Interrupts.
  sl_hal_i2c_enable_interrupts(I2C0, (I2C_IEN_NACK | I2C_IEN_ACK | I2C_IEN_MSTOP | I2C_IEN_START | I2C_IEN_RXDATAV | I2C_IEN_ERRORS));
  sl_interrupt_manager_clear_irq_pending(I2C0_IRQn);
  sl_interrupt_manager_enable_irq(I2C0_IRQn);

  // Enable I2C.
  sl_hal_i2c_enable(I2C0);
}

/***************************************************************************//**
 * Initialize buffers and state machine and start transfer
 ******************************************************************************/
void i2c_transfer(uint32_t transferType)
{
  // Ensure buffers are empty.
  sl_hal_i2c_flush_buffers(I2C0);

  // Clear all pending interrupts prior to starting a transfer.
  sl_hal_i2c_clear_interrupts(I2C0, _I2C_IF_MASK);

  // Reset buffer indices
  rxBufferIndex = 0;
  txBufferIndex = 0;

  // Reset target address flag
  i2c_targetAddressSent = false;

  // Set transfer type
  i2c_transferType = transferType;

  // Set transfer indicator flag
  i2c_transferInProgress = true;

  // Send start condition
  I2C0->CMD_SET = I2C_CMD_START;
}

/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  gpio_init();
  i2c_init();

  // Set initial test state
  i2c_testState = I2C_TEST_IDLE_STATE;
}

/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{
  int i;
  bool I2CWriteVerify;

  if (i2c_testInProgress) { // start I2C test
    if (!i2c_transferInProgress) {
      switch (i2c_testState) {
        case I2C_TEST_IDLE_STATE:
          // Initiate first read of follower buffer
          i2c_transfer(I2C_TRANSFER_READ);

          // Move to next test state
          i2c_testState = I2C_TEST_READ_STATE;
          break;

        case I2C_TEST_READ_STATE:
          // Read transmission complete; increment and write to follower buffer
          // Increment received values and prepare to write back to follower
          for (i = 0; i < I2C_RXBUFFER_SIZE; i++) {
            i2c_txBuffer[i] = i2c_rxBuffer[i] + 1;
          }

          // Write update values to follower buffer
          i2c_transfer(I2C_TRANSFER_WRITE);

          // Move to next test state
          i2c_testState = I2C_TEST_WRITE_STATE;
          break;

        case I2C_TEST_WRITE_STATE:
          // Write transmission complete; second read of follower buffer
          i2c_transfer(I2C_TRANSFER_READ);

          // Move to next test state
          i2c_testState = I2C_TEST_READ_AND_VERIFY_STATE;
          break;

        case I2C_TEST_READ_AND_VERIFY_STATE:
          // Second read complete; verify follower read matches write
          // Verify I2C transmission
          I2CWriteVerify = true;
          for (i = 0; i < I2C_RXBUFFER_SIZE; i++) {
            if (i2c_txBuffer[i] != i2c_rxBuffer[i]) {
              I2CWriteVerify = false;
              break;
            }
          }

          if (!I2CWriteVerify) {
            // Verification failed; indicate error with LED1
            sl_gpio_set_pin(&GPIO_LED1);

            // Stop at breakpoint to allow debugging
            __BKPT(2);
          } else {
            // Toggle LED0 with data verification
            sl_gpio_toggle_pin(&GPIO_LED0);

            // I2C test complete
            i2c_testInProgress = false;
          }

          // Reset test state machine
          i2c_testState = I2C_TEST_IDLE_STATE;
          break;

        default:
          break;
      }
    }
  }
}

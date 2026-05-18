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

const sl_gpio_t GPIO_BUTTON0     = { .port = BUTTON0_PORT, .pin = BUTTON0_PIN };
const sl_gpio_t GPIO_LED0        = { .port = LED0_PORT,    .pin = LED0_PIN };
const sl_gpio_t GPIO_LED1        = { .port = LED1_PORT,    .pin = LED1_PIN };
const sl_gpio_t I2C_SCL          = { .port = EXP_I2C_SCL_PORT, .pin = EXP_I2C_SCL_PIN };
const sl_gpio_t I2C_SDA          = { .port = EXP_I2C_SDA_PORT, .pin = EXP_I2C_SDA_PIN };

/*******************************************************************************
 ***************************   GLOBAL VARIABLES   ******************************
 ******************************************************************************/

// Buffers
uint8_t i2c_txBuffer[I2C_TXBUFFER_SIZE];
uint8_t i2c_rxBuffer[I2C_RXBUFFER_SIZE];

// Transmission flags
volatile bool i2c_startTx;

/***************************************************************************//**
 * @brief GPIO Interrupt handler
 ******************************************************************************/
static void button_0_change(void)
{
  // Enable i2c.
  sl_hal_i2c_enable(I2C0);
  i2c_startTx = true;
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

  // Set the status flags
  i2c_startTx = false;

  // Enable automatic STOP on NACK
  I2C0->CTRL = I2C_CTRL_AUTOSN;
}

/***************************************************************************//**
 * @brief I2C read numBytes from follower device starting at target address
 ******************************************************************************/
void I2C_LeaderRead(uint16_t followerAddress, uint8_t targetAddress, uint8_t *rxBuff, uint8_t numBytes)
{
  uint8_t i;

  sl_hal_i2c_clear_interrupts(I2C0, _I2C_IF_MASK);

  // Enable automatic ACK for all bytes received except the last one
  sl_hal_i2c_auto_ack(I2C0, true);

  // Stage 2-level transmit FIFO with I2C address and target array address
  sl_hal_i2c_tx(I2C0, (followerAddress & I2C_RNOTW_MASK));
  sl_hal_i2c_tx(I2C0, targetAddress);

  // Initiate transfer
  sl_hal_i2c_start_cmd(I2C0);

  // Wait for secondary device to ACK
  while (!(I2C0->IF & I2C_IF_ACK));

  // Load read address into transmit FIFO
  sl_hal_i2c_tx(I2C0, (followerAddress | I2C_RNOTW_BIT));

  // Wait for secondary device to ACK
  while (!(I2C0->IF & I2C_IF_ACK));

  // Send restart
  sl_hal_i2c_start_cmd(I2C0);

  // Retrieve numBytes-1 from secondary device (auto-ACK enabled)
  for (i = 0; i < (numBytes - 1); i++) {
      rxBuff[i] = sl_hal_i2c_rx(I2C0);
  }

  // disable auto-ACK
  sl_hal_i2c_auto_ack(I2C0, false);

  // Retrieve last byte
  rxBuff[i] = sl_hal_i2c_rx(I2C0);

  // NACK last received byte
  sl_hal_i2c_send_nack(I2C0);

  // Send stop
  sl_hal_i2c_stop_cmd(I2C0);

  while (!(I2C0->IF & I2C_IF_MSTOP));
}

/***************************************************************************//**
 * @brief I2C write numBytes to follower device starting at target address
 ******************************************************************************/
void I2C_LeaderWrite(uint16_t followerAddress, uint8_t targetAddress, uint8_t *txBuff, uint8_t numBytes)
{
  sl_hal_i2c_clear_interrupts(I2C0, _I2C_IF_MASK);

  sl_hal_i2c_tx(I2C0, (followerAddress & I2C_RNOTW_MASK));
  sl_hal_i2c_tx(I2C0, targetAddress);

  sl_hal_i2c_start_cmd(I2C0);

  // Wait for secondary device to ACK
  while (!(I2C0->IF & I2C_IF_ACK));

  for (uint8_t i = 0; i < numBytes; i++) {
      sl_hal_i2c_tx(I2C0, txBuff[i]);
  }

  // Wait to complete transmission
  while (!(I2C0->IF & I2C_IF_TXC));
  while (!(I2C0->STATUS & I2C_STATUS_TXC));

  // Send stop
  sl_hal_i2c_stop_cmd(I2C0);

  while (!(I2C0->IF & I2C_IF_MSTOP));
}

/***************************************************************************//**
 * @brief I2C Read/Increment/Write/Verify
 ******************************************************************************/
bool testI2C(void)
{
  int i;
  bool I2CWriteVerify;

  // Initial read of bytes from follower
  I2C_LeaderRead(I2C_FOLLOWER_ADDRESS, 0, i2c_rxBuffer, I2C_RXBUFFER_SIZE);

  // Increment received values and prepare to write back to follower
  for (i = 0; i < I2C_RXBUFFER_SIZE; i++) {
    i2c_txBuffer[i] = i2c_rxBuffer[i] + 1;
  }

  // Block write new values to follower
  I2C_LeaderWrite(I2C_FOLLOWER_ADDRESS, 0, i2c_txBuffer, I2C_TXBUFFER_SIZE);

  // Block read from follower
  I2C_LeaderRead(I2C_FOLLOWER_ADDRESS, 0, i2c_rxBuffer, I2C_RXBUFFER_SIZE);

  // Verify I2C transmission
  I2CWriteVerify = true;
  for (i = 0; i < I2C_RXBUFFER_SIZE; i++) {
    if (i2c_txBuffer[i] != i2c_rxBuffer[i]) {
      I2CWriteVerify = false;
      break;
    }
  }

  return I2CWriteVerify;
}


/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  gpio_init();
  i2c_init();
}

/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{
  if (i2c_startTx) {
    // Transmitting data
    if (testI2C() == false) {
      // Indicate error with LED1
      sl_gpio_set_pin(&GPIO_LED1);

      // Stop at breakpoint to allow debugging
      __BKPT(2);
    } else {
      // Toggle LED0 with each pass
      sl_gpio_toggle_pin(&GPIO_LED0);

      // Transmission complete
      i2c_startTx = false;
    }
  }
}

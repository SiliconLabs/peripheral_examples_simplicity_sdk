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
#include "em_i2c.h"

#include "pin_config.h"
#include "peripheral_config.h"

// Define GPIO mapping for boards where LED0 and BUTTON0 share the same pins
#ifndef BUTTON0_PORT
  #define BUTTON0_PORT LED0_BUTTON0_PORT
  #define BUTTON0_PIN  LED0_BUTTON0_PIN
#endif

// Define GPIO mapping for boards where LED0 and BUTTON0 share the same pins
#ifndef LED0_PORT
  #define LED0_PORT LED0_BUTTON0_PORT
  #define LED0_PIN  LED0_BUTTON0_PIN
#endif

// Define GPIO mapping for boards where LED1 and BUTTON1 share the same pins
#ifndef LED1_PORT
  #define LED1_PORT LED1_BUTTON1_PORT
  #define LED1_PIN  LED1_BUTTON1_PIN
#endif

// Defines
#define I2C_FOLLOWER_ADDRESS              0xE2
#define I2C_TXBUFFER_SIZE                 10
#define I2C_RXBUFFER_SIZE                 10

const sl_gpio_t GPIO_BUTTON0     = { .port = BUTTON0_PORT, .pin = BUTTON0_PIN };
const sl_gpio_t GPIO_LED0        = { .port = LED0_PORT,    .pin = LED0_PIN };
const sl_gpio_t GPIO_LED1        = { .port = LED1_PORT,    .pin = LED1_PIN };
const sl_gpio_t I2C_SCL          = { .port = I2C_LEADER_SCL_PORT, .pin = I2C_LEADER_SCL_PIN};
const sl_gpio_t I2C_SDA          = { .port = I2C_LEADER_SDA_PORT, .pin = I2C_LEADER_SDA_PIN};
#ifndef I2C_DISABLED_PULLUPS
  const sl_gpio_t I2C_DOMAIN_POWER = { .port = I2C_DOMAIN_POWER_PORT,
                                      .pin = I2C_DOMAIN_POWER_PIN};
#endif

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
  // Re-enable I2C
  I2C_Enable(I2C0, true);
  i2c_startTx = true;
}

/***************************************************************************//**
 * @brief GPIO initialization
 ******************************************************************************/
void gpio_init(void)
{
  int32_t int_no = GPIO_BUTTON0.pin;

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
  // Enable I2C clock tree
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_I2C0);
  
  // Use default settings
  I2C_Init_TypeDef i2cInit = I2C_INIT_DEFAULT;

  /*
   * Power up the Si7021 RHT sensor domain on the mainboard.  This
   * also powers up the sensor's local SCL and SDA pull-ups, which
   * eliminates the need to jumper resistors to the EXP header pins.
   * Not available on xG21 and xG25.
   */
#ifndef I2C_DISABLED_PULLUPS
  sl_gpio_set_pin_mode(&I2C_DOMAIN_POWER, SL_GPIO_MODE_PUSH_PULL, 1);
#endif

  // Enable SCL and SDA GPIO pins
  sl_gpio_set_pin_mode(&I2C_SCL, SL_GPIO_MODE_WIRED_AND_PULLUP_FILTER, 1);
  sl_gpio_set_pin_mode(&I2C_SDA, SL_GPIO_MODE_WIRED_AND_PULLUP_FILTER, 1);

  // Route I2C pins to GPIO
  GPIO->I2CROUTE[0].SDAROUTE = (GPIO->I2CROUTE[0].SDAROUTE & ~_GPIO_I2C_SDAROUTE_MASK)
                        | (I2C_LEADER_SDA_PORT << _GPIO_I2C_SDAROUTE_PORT_SHIFT
                        | (I2C_LEADER_SDA_PIN << _GPIO_I2C_SDAROUTE_PIN_SHIFT));
  GPIO->I2CROUTE[0].SCLROUTE = (GPIO->I2CROUTE[0].SCLROUTE & ~_GPIO_I2C_SCLROUTE_MASK)
                        | (I2C_LEADER_SCL_PORT << _GPIO_I2C_SCLROUTE_PORT_SHIFT
                        | (I2C_LEADER_SCL_PIN << _GPIO_I2C_SCLROUTE_PIN_SHIFT));
  GPIO->I2CROUTE[0].ROUTEEN = GPIO_I2C_ROUTEEN_SDAPEN | GPIO_I2C_ROUTEEN_SCLPEN;

  // Initialize the I2C
  I2C_Init(I2C0, &i2cInit);

  // Set the status flags and index
  i2c_startTx = false;

  // Enable automatic STOP on NACK
  I2C0->CTRL = I2C_CTRL_AUTOSN;
}

/***************************************************************************//**
 * @brief I2C read numBytes from follower device starting at target address
 ******************************************************************************/
void I2C_LeaderRead(uint16_t followerAddress, uint8_t targetAddress, uint8_t *rxBuff, uint8_t numBytes)
{
  // Transfer structure
  I2C_TransferSeq_TypeDef i2cTransfer;
  I2C_TransferReturn_TypeDef result;

  // Initialize I2C transfer
  i2cTransfer.addr          = followerAddress;
  i2cTransfer.flags         = I2C_FLAG_WRITE_READ; // must write target address before reading
  i2cTransfer.buf[0].data   = &targetAddress;
  i2cTransfer.buf[0].len    = 1;
  i2cTransfer.buf[1].data   = rxBuff;
  i2cTransfer.buf[1].len    = numBytes;

  result = I2C_TransferInit(I2C0, &i2cTransfer);

  // Send data
  while (result == i2cTransferInProgress) {
    result = I2C_Transfer(I2C0);
  }

  if (result != i2cTransferDone) {
    // LED0 ON and breakpoint to indicate I2C transmission problem
    sl_gpio_set_pin(&GPIO_LED0);
    __BKPT(0);
  }
}

/***************************************************************************//**
 * @brief I2C write numBytes to follower device starting at target address
 ******************************************************************************/
void I2C_LeaderWrite(uint16_t followerAddress, uint8_t targetAddress, uint8_t *txBuff, uint8_t numBytes)
{
  // Transfer structure
  I2C_TransferSeq_TypeDef i2cTransfer;
  I2C_TransferReturn_TypeDef result;
  uint8_t txBuffer[I2C_TXBUFFER_SIZE + 1];

  txBuffer[0] = targetAddress;
  for(int i = 0; i < numBytes; i++)
  {
      txBuffer[i + 1] = txBuff[i];
  }

  // Initialize I2C transfer
  i2cTransfer.addr          = followerAddress;
  i2cTransfer.flags         = I2C_FLAG_WRITE;
  i2cTransfer.buf[0].data   = txBuffer;
  i2cTransfer.buf[0].len    = numBytes + 1;
  i2cTransfer.buf[1].data   = NULL;
  i2cTransfer.buf[1].len    = 0;

  result = I2C_TransferInit(I2C0, &i2cTransfer);

  // Send data
  while (result == i2cTransferInProgress) {
    result = I2C_Transfer(I2C0);
  }

  if (result != i2cTransferDone) {
    // LED0 ON and breakpoint to indicate I2C transmission problem
    sl_gpio_set_pin(&GPIO_LED0);
    __BKPT(1);
  }
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
      // Indicate error with LED0
      sl_gpio_set_pin(&GPIO_LED0);

      // Stop at breakpoint to allow debugging
      __BKPT(2);
    } else {
      // Toggle LED1 with each pass
      sl_gpio_toggle_pin(&GPIO_LED1);

      // Transmission complete
      i2c_startTx = false;
    }
  }
}

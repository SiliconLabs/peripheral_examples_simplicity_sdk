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
#include "sl_power_manager.h"

#include "sl_gpio.h"
#include "em_i2c.h"
#include "em_emu.h"

#include "pin_config.h"

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

#define I2C_ADDRESS                     0xE2
#define I2C_BUFFER_SIZE                 10

const sl_gpio_t GPIO_LED0 = { .port = LED0_PORT,    .pin = LED0_PIN };
const sl_gpio_t GPIO_LED1 = { .port = LED1_PORT,    .pin = LED1_PIN };
const sl_gpio_t I2C_SCL   = { .port = I2C_FOLLOWER_SCL_PORT, .pin = I2C_FOLLOWER_SCL_PIN};
const sl_gpio_t I2C_SDA   = { .port = I2C_FOLLOWER_SDA_PORT, .pin = I2C_FOLLOWER_SDA_PIN};

/*******************************************************************************
 ***************************   GLOBAL VARIABLES   ******************************
 ******************************************************************************/

// Buffers
uint8_t i2c_Buffer[I2C_BUFFER_SIZE];
uint8_t i2c_BufferIndex;

// Transmission flags
volatile bool i2c_gotTargetAddress;
volatile bool i2c_rxInProgress;
volatile bool em1_req_set;

/***************************************************************************//**
 * @brief GPIO initialization
 ******************************************************************************/
void gpio_init(void)
{
  // Configure LED0 and LED1 as output
  sl_gpio_set_pin_mode(&GPIO_LED0, SL_GPIO_MODE_PUSH_PULL, LED_OFF);
  sl_gpio_set_pin_mode(&GPIO_LED1, SL_GPIO_MODE_PUSH_PULL, LED_OFF);
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

  // Configure to be addressable as follower
  i2cInit.master = false;

  // Enable SCL and SDA GPIO pins
  sl_gpio_set_pin_mode(&I2C_SCL, SL_GPIO_MODE_WIRED_AND_PULLUP_FILTER, 1);
  sl_gpio_set_pin_mode(&I2C_SDA, SL_GPIO_MODE_WIRED_AND_PULLUP_FILTER, 1);

  // Route I2C pins to GPIO
  GPIO->I2CROUTE[0].SDAROUTE = (GPIO->I2CROUTE[0].SDAROUTE & ~_GPIO_I2C_SDAROUTE_MASK)
                        | (I2C_FOLLOWER_SDA_PORT << _GPIO_I2C_SDAROUTE_PORT_SHIFT
                        | (I2C_FOLLOWER_SDA_PIN << _GPIO_I2C_SDAROUTE_PIN_SHIFT));
  GPIO->I2CROUTE[0].SCLROUTE = (GPIO->I2CROUTE[0].SCLROUTE & ~_GPIO_I2C_SCLROUTE_MASK)
                        | (I2C_FOLLOWER_SCL_PORT << _GPIO_I2C_SCLROUTE_PORT_SHIFT
                        | (I2C_FOLLOWER_SCL_PIN << _GPIO_I2C_SCLROUTE_PIN_SHIFT));
  GPIO->I2CROUTE[0].ROUTEEN = GPIO_I2C_ROUTEEN_SDAPEN | GPIO_I2C_ROUTEEN_SCLPEN;

  // Initialize the I2C
  I2C_Init(I2C0, &i2cInit);

  // Initialize the buffer index
  i2c_BufferIndex = 0;

  // Set up to enable follower mode
  I2C_SlaveAddressSet(I2C0, I2C_ADDRESS);
  I2C_SlaveAddressMaskSet(I2C0, 0xFE); // must match exact address

  I2C_IntClear(I2C0, _I2C_IF_MASK);
  I2C_IntEnable(I2C0, I2C_IEN_ADDR | I2C_IEN_RXDATAV | I2C_IEN_ACK | I2C_IEN_SSTOP | I2C_IEN_BUSERR | I2C_IEN_ARBLOST);
  sl_interrupt_manager_clear_irq_pending(I2C0_IRQn);
  sl_interrupt_manager_enable_irq(I2C0_IRQn);
}

/***************************************************************************//**
 * @brief I2C Interrupt Handler
 ******************************************************************************/
void I2C0_IRQHandler(void)
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
      I2C_IntClear(I2C0, I2C_IF_ADDR | I2C_IF_RXDATAV);

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
      I2C_IntClear(I2C0, I2C_IF_RXDATAV);

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

      I2C_IntClear(I2C0, I2C_IF_ACK);
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
      I2C_IntClear(I2C0, I2C_IF_SSTOP);
    }
  }
}

/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
#ifndef EFR32MG21A010F1024IM32
  // Enable voltage downscaling in energy modes EM2 and EM3
  EMU_EM23Init_TypeDef em23Init = EMU_EM23INIT_DEFAULT;
  em23Init.vScaleEM23Voltage = emuVScaleEM23_LowPower;
  EMU_EM23Init(&em23Init);
#endif

  // Initialize GPIO and I2C
  gpio_init();
  i2c_init();

}

/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{
  if (i2c_rxInProgress) {
    // Check if the requirement is already added
    if (!em1_req_set){
      sl_power_manager_add_em_requirement(SL_POWER_MANAGER_EM1);
      em1_req_set = true;
    }
  }
  if (!i2c_rxInProgress && em1_req_set) {
    // Remove EM1 requirement to let Power Manager enter EM2
    sl_power_manager_remove_em_requirement(SL_POWER_MANAGER_EM1);
    em1_req_set = false;
  }
}

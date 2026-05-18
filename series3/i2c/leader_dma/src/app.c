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

#include "dmadrv.h"

#include "sl_gpio.h"
#include "sl_device_peripheral.h"
#include "sl_hal_i2c.h"
#include "sl_hal_ldma.h"

#include "pin_config.h"
#include "peripheral_config.h"

// Address of the I2C follower device (left-shifted to bits [7:1])
#define I2C_FOLLOWER_ADDRESS    0xE2

// Read/write bit/mask for the I2C follower device address
#define I2C_RNOTW_BIT           0x01
#define I2C_RNOTW_MASK          0xFE

// Buffer size configuration
#define I2C_BUFFER_SIZE         10

// LDMA Channel Configuration
#define I2C_DMA_RXCHAN          0
#define I2C_DMA_TXCHAN          1

/*******************************************************************************
 ********************************   ENUMS   ************************************
 ******************************************************************************/

/** Return codes for single Controller mode transfer function. */
typedef enum {
  /* In progress code (>0) */
  i2cTransferInProgress = 1,    /**< Transfer in progress. */

  /* Complete code (=0) */
  i2cTransferDone       = 0,    /**< Transfer completed successfully. */

  /* Transfer error codes (<0). */
  i2cTransferNack       = -1,   /**< NACK received during transfer. */
  i2cTransferBusErr     = -2,   /**< Bus error during transfer (misplaced START/STOP). */
  i2cTransferArbLost    = -3,   /**< Arbitration lost during transfer. */
  i2cTransferUsageFault = -4,   /**< Usage fault. */
  i2cTransferSwFault    = -5    /**< SW fault. */
} I2C_TransferReturn_TypeDef;

typedef enum {
  I2C_IDLE,
  I2C_TEST_START,
  I2C_READ,
  I2C_WRITE,
  I2C_READ_AND_VERIFY
} I2CTestState_t;


/*******************************************************************************
 ***************************   GLOBAL VARIABLES   ******************************
 ******************************************************************************/

// Pin configurations
const sl_gpio_t GPIO_BUTTON0     = { .port = BUTTON0_PORT, .pin = BUTTON0_PIN };
const sl_gpio_t GPIO_LED0        = { .port = LED0_PORT,    .pin = LED0_PIN };
const sl_gpio_t GPIO_LED1        = { .port = LED1_PORT,    .pin = LED1_PIN };
const sl_gpio_t I2C_SCL          = { .port = EXP_I2C_SCL_PORT, .pin = EXP_I2C_SCL_PIN };
const sl_gpio_t I2C_SDA          = { .port = EXP_I2C_SDA_PORT, .pin = EXP_I2C_SDA_PIN };

// I2C buffers
uint8_t i2c_rxBuffer[I2C_BUFFER_SIZE];
uint8_t i2c_txBuffer[I2C_BUFFER_SIZE];

// LDMA descriptors
sl_hal_ldma_descriptor_t i2c_rxDesc[5];
sl_hal_ldma_descriptor_t i2c_txDesc[1];
unsigned int rxChannelId;
unsigned int txChannelId;

// I2C Test/Transmission status
volatile bool i2c_testInProgress = false;
volatile I2C_TransferReturn_TypeDef i2c_xferStatus;
volatile I2CTestState_t test_i2c_state;

void I2C_LeaderRead(void);
void I2C_LeaderWrite(void);

/**************************************************************************//**
 * @brief GPIO interrupt handler
 *****************************************************************************/
static void button_0_change(void)
{
  if (i2c_testInProgress == false) // Only start a new test if previous is done
  {
    // Button 0 pressed; start the read/write/verify test
    i2c_testInProgress = true;

    // Update I2C test current state
    test_i2c_state = I2C_TEST_START;
  }
}

/***************************************************************************//**
 * @brief GPIO initialization
 ******************************************************************************/
void gpio_init(void)
{
  int32_t int_no = GPIO_BUTTON0.pin;

  // Configure LED0 and LED1 as outputs
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

  // Configure for I2C leader operation
  sl_hal_i2c_init(I2C0, SL_I2C_LEADER_MODE);

  sl_clock_manager_get_clock_branch_frequency(SL_PERIPHERAL_I2C0->clk_branch, &clock_branch_freq);

  // Set the I2C Bus Frequency.
  sl_hal_i2c_set_clock_frequency(I2C0, clock_branch_freq, 100000, SL_I2C_CLK_HLR_STANDARD);

  // Enable I2C.
  sl_hal_i2c_enable(I2C0);

  test_i2c_state = I2C_IDLE;
}

/***************************************************************************//**
 * @brief Read I2C_BUFFER_SIZE bytes starting at address 0 from the follower
 ******************************************************************************/
void I2C_LeaderRead(void)
{
  // Enable automatic ACK for all bytes received except the last one
  I2C0->CTRL_SET = I2C_CTRL_AUTOACK;

  /*
   * This example sends the follower address and the 1-byte read
   * address (0x0) simply by loading them into the TX FIFO.  Because
   * only two bytes are transmitted before the repeated start, there
   * is no need to use DMA.  The START condition is set later.
   */
  I2C0->TXDATA = (I2C_FOLLOWER_ADDRESS & I2C_RNOTW_MASK);
  I2C0->TXDATA = 0;

  // Setup the receive DMA to trigger on data valid
  sl_hal_ldma_transfer_config_t transferConfig =
      SL_HAL_LDMA_TRANSFER_CFG_PERIPHERAL(SL_HAL_LDMA_PERIPHERAL_SIGNAL_I2C0_RXDATAV);
  /*
   * Mark transfer in progress, start the RX DMA channel, and send the
   * START condition to transmit the data already in the FIFO.
   */
  i2c_xferStatus = i2cTransferInProgress;
  DMADRV_LdmaStartTransfer(rxChannelId,
                           (void*)&transferConfig,
                           &i2c_rxDesc[0],
                           NULL,
                           NULL);
  I2C0->CMD = I2C_CMD_START;

  /*
   * Disable the LDMA RX channel interrupt.  It is not needed because
   * the relevant I2C interrupts are used on end of transfer or error
   * conditions.
   *
   * Receiving data takes several steps because of the need to NACK
   * the last byte received, send stop, and request an interrupt when
   * everything is done.  This is done with linked descriptors, which
   * is something the Simplicity SDK DMADRV component does not have a means
   * of handling short of simply creating linked descriptors as was
   * done here.
   *
   * DMADRV has no means of handling read errors, so it's necessary to
   * have the I2C error interrupts enabled anyway.
   */
  sl_hal_ldma_disable_interrupts(LDMA0, rxChannelId);

  /*
   * Enable the TXC interrupt in order to issue the repeated start
   * followed by the device bus address to begin reading data.  Error
   * conditions interrupts get enabled here, too.
   */
  sl_hal_i2c_clear_interrupts(I2C0, _I2C_IF_MASK);
  sl_hal_i2c_enable_interrupts(I2C0, (I2C_IEN_ARBLOST |
                               I2C_IEN_BUSERR |
                               I2C_IEN_NACK   |
                               I2C_IEN_ACK));
  sl_interrupt_manager_clear_irq_pending(I2C0_IRQn);
  sl_interrupt_manager_enable_irq(I2C0_IRQn);
}

/***************************************************************************//**
 * @brief Write I2C_BUFFER_SIZE bytes starting at address 0 to the follower
 ******************************************************************************/
void I2C_LeaderWrite(void)
{
  // Setup the transmit DMA to trigger on available buffer space
  sl_hal_ldma_transfer_config_t transferConfig =
      SL_HAL_LDMA_TRANSFER_CFG_PERIPHERAL(SL_HAL_LDMA_PERIPHERAL_SIGNAL_I2C0_TXBL);
  transferConfig.debug_halt_en = true;

  /*
   * As in the read function, some simplification of the code is
   * possible by writing the follower address and the starting address
   * of the write directly to the TX FIFO instead of having the DMA
   * send it.
   */
  i2c_xferStatus = i2cTransferInProgress;
  I2C0->TXDATA = (I2C_FOLLOWER_ADDRESS & I2C_RNOTW_MASK);
  I2C0->TXDATA = 0;

  // Start the TX DMA channel but don't send START yet
  DMADRV_LdmaStartTransfer(txChannelId,
                           (void*)&transferConfig,
                           &i2c_txDesc[0],
                           NULL,
                           NULL);

  /*
   * Disable the LDMA TX channel interrupt. Because DMADRV owns the
   * LDMA_IRQHandler(), it's not possible to put custom handling for
   * the I2C in there.
   *
   * Instead, the MSTOP interrupt denotes successful transfer
   * completion. Because the follower is receiving data, it ought to
   * ACK each byte as received unless their is an error, so these
   * interrupts are also enabled here.
   */
  sl_hal_ldma_disable_interrupts(LDMA0, txChannelId);

  sl_hal_i2c_clear_interrupts(I2C0, _I2C_IF_MASK);
  sl_hal_i2c_enable_interrupts(I2C0, (I2C_IEN_MSTOP   |
                               I2C_IEN_ARBLOST |
                               I2C_IEN_BUSERR  |
                               I2C_IEN_NACK));
  sl_interrupt_manager_clear_irq_pending(I2C0_IRQn);
  sl_interrupt_manager_enable_irq(I2C0_IRQn);

  // Everything is ready; send the START condition
  I2C0->CMD = I2C_CMD_START;

  // Automatically send STOP when there is no data left to transmit
  I2C0->CTRL_SET = I2C_CTRL_AUTOSE;
}

/*****************************************************************************
 * @brief  I2C event and error interrupt handling
 *
 * When the leader has successfully sent a STOP condition, the MSTOP
 * interrupt is triggered and the transfer is marked as complete.
 *
 * The ARBLOST, BUSERR, and NACK errors are also handled here.  In any
 * of these cases, the current transfer is aborted and the last transfer
 * status is set to an error condition.  NACK is only enabled during
 * transmit activity except for the repeat start condition that begins
 * clocking read data out of the device.
 *****************************************************************************/
__attribute__((section("text_application_ram")))
void I2C0_IRQHandler(void)
{
  uint32_t flags = sl_hal_i2c_get_enabled_pending_interrupts(I2C0);

  sl_hal_i2c_clear_interrupts(I2C0, _I2C_IF_MASK);
  sl_interrupt_manager_clear_irq_pending(I2C0_IRQn);

  if (flags & I2C_IF_ACK)
  {
    /*
     * Secondary device ACK'ed address and has started transmitting target
     * address for subsequent read. Don't need NACK/ACK anymore so disable them
     * Load device read address and initiate restart with read address transfer
     */
    sl_hal_i2c_disable_interrupts(I2C0, I2C_IEN_ACK);

    // Send the device bus address to start the read
    I2C0->TXDATA = (I2C_FOLLOWER_ADDRESS | I2C_RNOTW_BIT);

    // Send repeat start for read operation
    I2C0->CMD = I2C_CMD_START;
  }

  // MSTOP set on success; transfer is complete
  else if (flags & I2C_IF_MSTOP)
  {
    // Disable all interrupts
    sl_hal_i2c_disable_interrupts(I2C0, _I2C_IEN_MASK);

    // Reset control register
    I2C0->CTRL = _I2C_CTRL_RESETVALUE;

    // Transfer is complete on MSTOP interrupt
    i2c_xferStatus = i2cTransferDone;
  }

  // Need to abort on any of the following error conditions
  else {
    if (flags & I2C_IF_ARBLOST) {
      i2c_xferStatus = i2cTransferArbLost;
    } else {
      if (flags & I2C_IF_BUSERR) {
        i2c_xferStatus = i2cTransferBusErr;
      } else {
        if (flags & I2C_IF_NACK) {
          i2c_xferStatus = i2cTransferNack;
        }
      }
    }

    // Abort on error
    I2C0->CMD = I2C_CMD_ABORT;
  }
}

/**************************************************************************//**
 * @brief LDMA initialization
 *****************************************************************************/
void ldma_init(void)
{
  // Initialize DMADRV
  DMADRV_Init();

  /*
   * The first RX DMA descriptor receives all but the last byte of data
   * and then links to the next descriptor.
   */
  i2c_rxDesc[0] =
    (sl_hal_ldma_descriptor_t) SL_HAL_LDMA_DESCRIPTOR_LINKREL_P2M(
        SL_HAL_LDMA_CTRL_SIZE_BYTE,
        (uint32_t)(&I2C0->RXDATA),
        (uint32_t)(i2c_rxBuffer),
        (I2C_BUFFER_SIZE - 1),
        1);

  /*
   * The next descriptor disables automatic acknowledge because the
   * last byte has to be NACKed to end the read. Do this by having the
   * LDMA clear the AUTOACK bit and link to the next descriptor.
   */
  i2c_rxDesc[1] =
    (sl_hal_ldma_descriptor_t) SL_HAL_LDMA_DESCRIPTOR_LINKREL_WRITE(
        I2C_CTRL_AUTOACK,
        (uint32_t)(&I2C0->CTRL_CLR),
        1);

  /*
   * The next descriptor enables the MSTOP interrupt so that when the
   * last byte is received, the transfer can be marked as complete.
   * Do this by having the DMA set the MSTOP bit in the interrupt
   * enable register and link to the next descriptor.
   */
  i2c_rxDesc[2] =
    (sl_hal_ldma_descriptor_t) SL_HAL_LDMA_DESCRIPTOR_LINKREL_WRITE(
        I2C_IEN_MSTOP,
        (uint32_t)(&I2C0->IEN_SET),
        1);

  /*
   * This descriptor receives the last byte of data by itself so that
   * it can be NACKed and then links to the next descriptor.
   */
  i2c_rxDesc[3] =
    (sl_hal_ldma_descriptor_t) SL_HAL_LDMA_DESCRIPTOR_LINKREL_P2M(
        SL_HAL_LDMA_CTRL_SIZE_BYTE,
        (uint32_t)(&I2C0->RXDATA),
        (uint32_t)(i2c_rxBuffer + I2C_BUFFER_SIZE - 1),
        1,
        1);

  /*
   * The final descriptor NACKs the last byte and issues a STOP to
   * properly end the transfer. Do this by having the DMA set the
   * NACK and STOP bits in the command register.
   */
  i2c_rxDesc[4] =
    (sl_hal_ldma_descriptor_t) SL_HAL_LDMA_DESCRIPTOR_SINGLE_WRITE(
        (I2C_CMD_NACK | I2C_CMD_STOP),
        (uint32_t)(&I2C0->CMD));

  // This descriptor transmits the data in i2c_txBuffer[]
  i2c_txDesc[0] =
    (sl_hal_ldma_descriptor_t) SL_HAL_LDMA_DESCRIPTOR_SINGLE_M2P(
        SL_HAL_LDMA_CTRL_SIZE_BYTE,
        (uint32_t)(i2c_txBuffer),
        (uint32_t)(&I2C0->TXDATA),
        I2C_BUFFER_SIZE);

  // Halt on error if DMA channel cannot be allocated
  if (DMADRV_AllocateChannel(&rxChannelId, NULL) != ECODE_EMDRV_DMADRV_OK) {
    __BKPT(0);
  }

  // Halt on error if DMA channel cannot be allocated
  if (DMADRV_AllocateChannel(&txChannelId, NULL) != ECODE_EMDRV_DMADRV_OK) {
    __BKPT(0);
  }

}

/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  gpio_init();
  i2c_init();
  ldma_init();
}

/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{
  uint8_t i;
  bool I2CWriteVerify;

  if (i2c_testInProgress == true)
  {
    switch(i2c_xferStatus) {
      case i2cTransferInProgress: // Do nothing; Wait for xfer to complete
        break;

      case i2cTransferDone: // Progress to next test state
        switch (test_i2c_state) {
          case I2C_IDLE:
            break;

          case I2C_TEST_START:
            // Initial read of bytes from follower
            I2C_LeaderRead();

            // Update I2C test current state
            test_i2c_state = I2C_READ;
            break;

          case I2C_READ:
            // Increment received values for write-back to the follower
            for (i = 0; i < I2C_BUFFER_SIZE; i++) {
              i2c_txBuffer[i] = i2c_rxBuffer[i] + 1;
            }

            // Block write new values to follower
            I2C_LeaderWrite();

            // Update current I2C state for next interrupt
            test_i2c_state = I2C_WRITE;
            break;

          case I2C_WRITE:
            // Block read from follower
            I2C_LeaderRead();

            // Update current I2C state for next interrupt
            test_i2c_state = I2C_READ_AND_VERIFY;
            break;

          case I2C_READ_AND_VERIFY:
            // Verify I2C transmission
            I2CWriteVerify = true;

            for (i = 0; i < I2C_BUFFER_SIZE; i++) {
              if (i2c_txBuffer[i] != i2c_rxBuffer[i]) {
                I2CWriteVerify = false;
                break;
              }
            }

            // Run test and check for success
            if (I2CWriteVerify == false)
            {
              // Indicate error with LED1 and breakpoint on failure
              sl_gpio_set_pin(&GPIO_LED1);
              __BKPT(1);
            }
            else
            {
              // Toggle LED1 on each pass
              sl_gpio_toggle_pin(&GPIO_LED0);

              // I2C Test Complete
              i2c_testInProgress = false;
            }

            // I2C peripheral idles until next button press
            test_i2c_state = I2C_IDLE;
            break;

          default:
            break;
        }
        break;

      default: // I2C peripheral interrupted with a transmission error
        // Indicate error with LED1 and breakpoint on failure
        sl_gpio_set_pin(&GPIO_LED1);
        __BKPT(2);

        break;
    }
  }
}

/***************************************************************************//**
 * @file app.c
 * @brief Use the GPCRC to check data
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 *******************************************************************************
 * # Evaluation Quality
 * This code has been minimally tested to ensure that it builds and is suitable 
 * as a demonstration for evaluation purposes only. This code will be maintained
 * at the sole discretion of Silicon Labs.
 ******************************************************************************/
#include "dmadrv.h"

#include "sl_hal_gpcrc.h"
#include "sl_hal_ldma.h"
#include "sl_clock_manager.h"


#define PRESET      0xFFFFFFFFUL
/*
 * This data was obtained from this online CRC calculator - https://crccalc.com/
 * The CRC is calculated with the input data shown below -
 * 0x00000001, 0x00000001, 0x00000002, 0x00000003, 0x00000005, 0x00000008
 * 0x0000000D, 0x00000015, 0x00000022, 0x00000037, 0x00000059, 0x00000090,
 * 0x000000E9, 0x00000179, 0x00000262, 0x000003DB
 *
 * However, the data is stored least significant byte first in RAM, which will
 * change the input data to the GPCRC. Therefore, the input to the GPCRC is byte
 * reversed so that CRC is calculated for the input shown above instead of
 * 0x01000000, ...., 0xDB030000.
 *
 * As a part of post processing in the crc calculator shown above, the resulting
 * CRC is XOR'd with 0xFFFFFFFF. This is not done by the GPCRC module on the
 * device. Therefore, the CRC of the data is XOR'd with 0xFFFFFFFF before
 * comparing it with the result of the online calculator.
 */
// IEEE 802.3 CRC for the input Fibonacci data
#define FIBONACCI_CRC_16WORDS (0x5CBF42AAUL)

// Allocated DMADRV Channel ID
unsigned int channelId;

// Descriptor linked list for LDMA transfer
sl_hal_ldma_descriptor_t ldmaCrcDesc[2];

// store 16 words (16 x 4 = 64 bytes) in RAM
#define ARRAY_SIZE 16

// Arbitrary values to cycle through
uint32_t data[ARRAY_SIZE];

// LDMA GPCRC channel assignment
#define LDMA_GPCRC_CHAN  0

// Register bit clear address for LDMA_IEN
#define LDMA_IEN_CLEAR  (((uint32_t)&(LDMA->IEN)) + 0x04000000)

/**************************************************************************//**
 * @brief  Calculates the n-th Fibonacci number recursively.
 *****************************************************************************/
uint32_t fib(uint32_t n)
{
  if (n < 2)
    return 1;
  else
    return (fib(n-1) + fib(n-2));
}

/***************************************************************************//**
 * @brief
 *   LDMA IRQ handler.
 ******************************************************************************/
void ldma_callback( void )
{
  // Shouldn't ever get in here, so take a breakpoint
  __BKPT(0);
}

/***************************************************************************//**
 * @brief
 *   Initialize the LDMA controller for descriptor list with looping
 ******************************************************************************/
void ldma_init(void)
{
  // Initialize DMADRV
  DMADRV_Init();

  // Halt on error if DMA channel cannot be allocated
  if (DMADRV_AllocateChannel(&channelId, NULL) != ECODE_EMDRV_DMADRV_OK) {
    __BKPT(0);
  }
}

/**************************************************************************//**
 * @brief  GPCRC Initializer
 *****************************************************************************/
void gpcrc_init(void)
{
  // Enable clocks required
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_GPCRC0);

  // GPCRC module initialization for IEEE 802.3 polynomial
  sl_hal_gpcrc_init_t init = SL_HAL_GPCRC_INIT_DEFAULT;

  init.init_value = PRESET;  // Starting value in GPCRC_DATA
  init.auto_init = true;     // Reset GPCRC_DATA to 0xFFFF_FFFF after every read
  init.reverse_byte_order = true;  // Reverse all bits of the incoming message

  // Initialize GPCRC
  sl_hal_gpcrc_init(GPCRC0, &init);

  sl_hal_gpcrc_enable(GPCRC0);
}

/**************************************************************************//**
 * @brief  Uses LDMA to transfer the array data into GPCRC->INPUTDATA register
 *****************************************************************************/
void crc_check_start(uint32_t *inputData)
{
  bool active;

  // Prepare GPCRC_DATA for inputs
  sl_hal_gpcrc_start(GPCRC0);

  // Use the generic memory-to-memory transfer configuration; halt during debug
  sl_hal_ldma_transfer_config_t transferConfig = SL_HAL_LDMA_TRANSFER_CFG_MEMORY();
  transferConfig.debug_halt_en = true;

  /*
   * Even though the GPCRC is a peripheral, it's necessary to use the
   * M2M descriptor because there is no DMA request (such as RXDATAV
   * upon USART reception).  Instead, the LDMA simply needs to pump
   * the page of flash through the GPCRC, word by word, as fast as it
   * can manage to do so.
   *
   * One minor inconvenience here is that even when the descriptor's
   * doneIfs field is set to 0 in order to avoid an interrupt upon
   * channel completion, the LDMA always sets the corresponding
   * LDMA_IF flag after executing the last descriptor.
   *
   * Setting the LDMA_IF flag in and of itself isn't a problem save
   * for the fact that LDMA_StartTransfer() enables a channel's
   * interrupt no matter what.
   *
   * Because the CRC blank checking operation is intended to run in
   * this example until software checks to see if it is actually
   * done, a little bit of trickery is used to disable the interrupt
   * right after LDMA_StartTransfer() enables it.
   *
   * Instead of using a single descriptor to execute the GPCRC
   * operation, a two element linked list is used.  The first entry
   * in the list is a WRI descriptor that writes the bit mask for the
   * GPCRC LDMA channel to the LDMA_IEN register in the peripheral
   * bit clear aliasing region (register address + 0x04000000).  The
   * next descriptor is the M2M transfer that feeds the flash page
   * contents into the GPCRC_INPUTDATA register word by word.
   */

  ldmaCrcDesc[0] = (sl_hal_ldma_descriptor_t) SL_HAL_LDMA_DESCRIPTOR_LINKREL_WRITE((1 << LDMA_GPCRC_CHAN), &(LDMA0 -> IEN_CLR), 1);
  ldmaCrcDesc[1] = (sl_hal_ldma_descriptor_t) SL_HAL_LDMA_DESCRIPTOR_SINGLE_M2M(SL_HAL_LDMA_CTRL_SIZE_WORD, inputData, &(GPCRC0->INPUTDATA), ARRAY_SIZE);
  ldmaCrcDesc[1].xfer.dst_inc = SL_HAL_LDMA_CTRL_DST_INC_NONE;

  // Check that IADC->memory LDMA channel is not currently active; halt if error
  if (DMADRV_TransferActive(channelId, &active) != ECODE_EMDRV_DMADRV_OK) {
    __BKPT(0);
  }

  // Start LDMA transfer
  if (!active) {
    DMADRV_LdmaStartTransfer(channelId,
                             (void*)&transferConfig,
                             ldmaCrcDesc,
                             (DMADRV_Callback_t)ldma_callback,
                             NULL);
  }
}

/**************************************************************************//**
 * @brief  Check to wait till LDMA transfer is completed
 *****************************************************************************/
bool crc_check_transfer_busy(void)
{
  bool transfer_done;

  DMADRV_TransferDone(LDMA_GPCRC_CHAN, &transfer_done);

  if(transfer_done)
    return false;
  else
    return true;
}

/**************************************************************************//**
 * @brief  Reads GPCRC DATA register and compares with IEEE 802.3 result
 *****************************************************************************/
bool crc_check_result(void)
{
  // XOR the data register output with 0xFFFFFFFF to get the post-processed
  // IEEE 802.3 result
  uint32_t crcResult = sl_hal_gpcrc_read_data(GPCRC0) ^ PRESET;

  if (crcResult == FIBONACCI_CRC_16WORDS)
    return true;
  else
    return false;
}

/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  bool crcCheck = false;

  // Fill data array with Fibonacci values
  for (uint32_t i = 0; i < ARRAY_SIZE; i++)
    data[i] = (fib(i));

  // Initialize GPCRC
  gpcrc_init();

  // Initialize LDMA
  ldma_init();

  // Transfer the data using the LDMA to the GPCRC input
  crc_check_start(data);

  // Wait LDMA transfer is in progress
  while (crc_check_transfer_busy() == true);

  // Read the GPCRC output and compare with predetermined CRC
  crcCheck = crc_check_result();

  if (!crcCheck)
  {
    // Halt if crcCheck fails
    __BKPT(2);
  }

}

/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{

}

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
#include "dmadrv.h"

#include "sl_clock_manager.h"
#include "sl_interrupt_manager.h"

#include "em_ldma.h"
#include "em_pdm.h"
#include "sl_gpio.h"

#include "pin_config.h"

/*******************************************************************************
 *******************************   DEFINES   ***********************************
 ******************************************************************************/
// Left/right buffer size
#define BUFFER_SIZE         128

// Ping-pong buffer size
#define PP_BUFFER_SIZE      64

// DMA channel used for the example
#define LDMA_CHANNEL        0
#define LDMA_CH_MASK        (1 << LDMA_CHANNEL)

/*******************************************************************************
 ***************************   GLOBAL VARIABLES   ******************************
 ******************************************************************************/
// Buffers for ping-pong transfer
uint32_t ping_buffer[PP_BUFFER_SIZE];
uint32_t pong_buffer[PP_BUFFER_SIZE];

// Buffers for left/right PCM data
int16_t left[BUFFER_SIZE];
int16_t right[BUFFER_SIZE];

// Keeps track of previously written buffer
bool prev_buffer_ping;

// Descriptor linked list for LDMA transfer
LDMA_Descriptor_t descriptor_link[2];

unsigned int channel_id;

// Default slew rate setting
volatile uint32_t slew_rate = 7; 

const sl_gpio_t SLEW_RATE  =  { .port = SLEW_RATE_PORT,
                                .pin  = SLEW_RATE_PIN};

const sl_gpio_t PDM_CLOCK  =  { .port = PDM_CLK_PORT,
                                .pin  = PDM_CLK_PIN};

const sl_gpio_t PDM_DATA   =  { .port = PDM_DAT0_PORT,
                                .pin  = PDM_DAT0_PIN};

const sl_gpio_t MIC_ENABLE =  { .port = MIC_ENABLE_PORT,
                                .pin  = MIC_ENABLE_PIN};

/***************************************************************************//**
 * @brief
 *   Initialize GPIO
 ******************************************************************************/
void gpio_init(void)
{
  sl_gpio_init();

  // Configure GPIO
  sl_gpio_set_pin_mode(&MIC_ENABLE, SL_GPIO_MODE_PUSH_PULL, true);

  sl_gpio_set_pin_mode(&PDM_CLOCK, SL_GPIO_MODE_PUSH_PULL, false);

  sl_gpio_set_pin_mode(&PDM_DATA, SL_GPIO_MODE_INPUT, false);

  sl_gpio_set_slew_rate(&SLEW_RATE, slew_rate);
}
/***************************************************************************//**
 * @brief
 *   Initialize PDM microphones
 ******************************************************************************/
void pdm_init(void)
{
  PDM_Init_TypeDef init_pdm;

  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_PDM);

  // Route PDM to GPIO
  GPIO->PDMROUTE.ROUTEEN = GPIO_PDM_ROUTEEN_CLKPEN;
  GPIO->PDMROUTE.CLKROUTE = (PDM_CLK_PORT << _GPIO_PDM_CLKROUTE_PORT_SHIFT)
                            | (PDM_CLK_PIN << _GPIO_PDM_CLKROUTE_PIN_SHIFT);
  GPIO->PDMROUTE.DAT0ROUTE = (PDM_DAT0_PORT << _GPIO_PDM_DAT0ROUTE_PORT_SHIFT)
                            | (PDM_DAT0_PIN << _GPIO_PDM_DAT0ROUTE_PIN_SHIFT);
  GPIO->PDMROUTE.DAT1ROUTE = (PDM_DAT1_PORT << _GPIO_PDM_DAT1ROUTE_PORT_SHIFT)
                            | (PDM_DAT1_PIN << _GPIO_PDM_DAT1ROUTE_PIN_SHIFT);

  // Initialize PDM registers with reset values
  PDM_Reset(PDM);

  // Configure PDM
  init_pdm.start = true;
  init_pdm.dsr = 32;
  init_pdm.gain = 5;
  init_pdm.ch0ClkPolarity = pdmCh0ClkPolarityRisingEdge;
  init_pdm.ch1ClkPolarity = pdmCh1ClkPolarityFallingEdge;
  init_pdm.enableCh0Ch1Stereo = true;
  init_pdm.fifoValidWatermark = pdmFifoValidWatermarkFour;
  init_pdm.dataFormat = pdmDataFormatDouble16;
  init_pdm.numChannels = pdmNumberOfChannelsTwo;
  init_pdm.filterOrder = pdmFilterOrderFifth;
  init_pdm.prescaler = 5;

  // Initialize PDM peripheral
  PDM_Init(PDM, &init_pdm);
}
/***************************************************************************//**
 * @brief
 *   LDMA Callback Function
 ******************************************************************************/
void ldma_callback(void)
{
  // Keep track of previously written buffer
  prev_buffer_ping = !prev_buffer_ping;
}

/***************************************************************************//**
 * @brief
 *   Initialize the DMA controller for ping-pong transfer
 ******************************************************************************/
void dma_init(void)
{
  // LDMA transfers trigger on PDM Rx Data Valid
  LDMA_TransferCfg_t peri_transfer_tx =
    LDMA_TRANSFER_CFG_PERIPHERAL(ldmaPeripheralSignal_PDM_RXDATAV);

  // Link descriptors for ping-pong transfer
  descriptor_link[0] = (LDMA_Descriptor_t)
    LDMA_DESCRIPTOR_LINKREL_P2M_WORD(&PDM->RXDATA, ping_buffer,
                                     PP_BUFFER_SIZE, 1);
  descriptor_link[1] = (LDMA_Descriptor_t)
    LDMA_DESCRIPTOR_LINKREL_P2M_WORD(&PDM->RXDATA, pong_buffer,
                                     PP_BUFFER_SIZE, -1);

  // Next transfer writes to ping_buffer
  prev_buffer_ping = false;

  // Initialize DMADRV
  DMADRV_Init();

  // Halt on error if DMA channel cannot be allocated
  if (DMADRV_AllocateChannel(&channel_id, NULL) != ECODE_EMDRV_DMADRV_OK) {
    __BKPT(0);
  }

  DMADRV_LdmaStartTransfer(channel_id,
                           &peri_transfer_tx,
                           descriptor_link,
                           (DMADRV_Callback_t)ldma_callback,
                           NULL);
}

/***************************************************************************//**
 * @brief
 *   Initialize application
 ******************************************************************************/
void app_init(void)
{
  // Initialize GPIO
  gpio_init();
  // Initialize PDM
  pdm_init();
  // Initialize DMA
  dma_init();
}

void app_process_action(void)
{
  unsigned i;

  /* After LDMA transfer completes and wakes up device from EM1,
   * convert data from ping-pong buffers to left/right PCM data
   */
  if(prev_buffer_ping) {
        for(i=0; i<PP_BUFFER_SIZE; i++) {
          left[i] = ping_buffer[i] & 0x0000FFFF;
          right[i] = (ping_buffer[i] >> 16) & 0x0000FFFF;
        }
      } else {
        for(i=0; i<PP_BUFFER_SIZE; i++) {
          left[PP_BUFFER_SIZE + i] = pong_buffer[i] & 0x0000FFFF;
          right[PP_BUFFER_SIZE + i] = (pong_buffer[i] >> 16) & 0x0000FFFF;
        }
      }
 }


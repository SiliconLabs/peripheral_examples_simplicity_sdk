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

#include "em_pdm.h"
#include "sl_gpio.h"

#include "pin_config.h"

/*******************************************************************************
 *******************************   DEFINES   ***********************************
 ******************************************************************************/
#define BUFFER_SIZE 128

/*******************************************************************************
 ***************************   GLOBAL VARIABLES   ******************************
 ******************************************************************************/
// Buffer index
uint32_t buffer_ptr = 0;

// Buffer for raw PDM data
uint32_t buffer[BUFFER_SIZE];

// Buffers for left/right PCM data
int16_t left[BUFFER_SIZE];
int16_t right[BUFFER_SIZE];

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
  // Configure GPIO
  sl_gpio_set_pin_mode(&MIC_ENABLE, SL_GPIO_MODE_PUSH_PULL, true);

  sl_gpio_set_pin_mode(&PDM_CLOCK, SL_GPIO_MODE_PUSH_PULL, false);

  sl_gpio_set_pin_mode(&PDM_DATA, SL_GPIO_MODE_INPUT, false);

  sl_gpio_set_slew_rate(&SLEW_RATE, slew_rate);
}
/***************************************************************************//**
 * @brief
 *   Sets up PDM microphones
 ******************************************************************************/
void pdm_init(void)
{
  PDM_Init_TypeDef init_pdm;

  // Enable clock for PDM
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
  init_pdm.fifoValidWatermark = pdmFifoValidWatermarkOne;
  init_pdm.dataFormat = pdmDataFormatDouble16;
  init_pdm.numChannels = pdmNumberOfChannelsTwo;
  init_pdm.filterOrder = pdmFilterOrderFifth;
  init_pdm.prescaler = 5;

  // Initialize PDM peripheral
  PDM_Init(PDM, &init_pdm);

  // Enable Interrupts
  PDM->IEN = PDM_IEN_DVL;

  sl_interrupt_manager_enable_irq(PDM_IRQn);
  sl_interrupt_manager_clear_irq_pending(PDM_IRQn);
}

/***************************************************************************//**
 * @brief
 *   PDM Interrupt Handler
 ******************************************************************************/
void PDM_IRQHandler(void)
{
  uint32_t interruptFlags = PDM->IF;

  // Read data from FIFO
  if(interruptFlags & PDM_IF_DVL) {
    PDM->IF_CLR = PDM_IF_DVL;

    while(!(PDM->STATUS & PDM_STATUS_EMPTY)) {
      buffer[buffer_ptr] = PDM->RXDATA;
      left[buffer_ptr] = buffer[buffer_ptr] & 0x0000FFFF;
      right[buffer_ptr] = (buffer[buffer_ptr] >> 16) & 0x0000FFFF;
      buffer_ptr = (buffer_ptr + 1) % BUFFER_SIZE;
    }
  }
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
}

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

#include "dmadrv.h"

#include "sl_gpio.h"
#include "sl_hal_ldma.h"
#include "sl_hal_letimer.h"
#include "sl_hal_pixelrz.h"
#include "sl_hal_prs.h"

#include "pin_config.h"

// 24-bit Green-Red-Blue bits in 32-bit MSB data alignment
#define RED_GRB     0x00FF0000
#define YELLOW_GRB  0xFFFF0000
#define GREEN_GRB   0xFF000000
#define CYAN_GRB    0xFF00FF00
#define BLUE_GRB    0x0000FF00
#define PURPLE_GRB  0x00FFFF00

// PIXELRZ settings
#define PIXELRZ_PIXEL_COUNT  60
#define PIXELRZ_PIXEL_WIDTH  24
#define PIXELRZ_PIXEL_TRAIL  0

// WS2812B symbol timing
#define WS2812B_T0_HIGH    0.4f
#define WS2812B_T0_LOW     0.85f
#define WS2812B_T1_HIGH    0.8f
#define WS2812B_T1_LOW     0.6f
#define WS2812B_TRST_HIGH  0.0f
#define WS2812B_TRST_LOW   60.0f

// PIXELRZ PRS channel
#define PIXELRZ_ASYNC_PRS_CH 0

// Frequencies for LETIMER
#define LETIMER_FREQ 32768
#define RAINBOW_FREQ 500

// GRB pixel data array
uint32_t pixels[PIXELRZ_PIXEL_COUNT];

typedef enum color {
  RED,
  YELLOW,
  GREEN,
  CYAN,
  BLUE,
  PURPLE,
  RAINBOW,
  COLORS_TOTAL
} color_t;

uint32_t colors[COLORS_TOTAL] = {RED, YELLOW, GREEN, CYAN, BLUE, PURPLE, RAINBOW};
uint32_t colors_grb[COLORS_TOTAL-1] = {RED_GRB, YELLOW_GRB, GREEN_GRB, CYAN_GRB, BLUE_GRB, PURPLE_GRB};
color_t color = 0;
volatile bool color_change = false;

// GPIO
const sl_gpio_t GPIO_BUTTON0 = { .port = BUTTON0_PORT, .pin = BUTTON0_PIN };
const sl_gpio_t GPIO_PIXELRZ_SEQ = { .port = PIXELRZ_SEQ_PORT, .pin = PIXELRZ_SEQ_PIN };

// LDMA
unsigned int channelId;
sl_hal_ldma_transfer_config_t config = SL_HAL_LDMA_TRANSFER_CFG_PERIPHERAL(SL_HAL_LDMA_PERIPHERAL_SIGNAL_PIXELRZ0REQ_TXF);
sl_hal_ldma_descriptor_t desc_solid = SL_HAL_LDMA_DESCRIPTOR_SINGLE_M2P(SL_HAL_LDMA_CTRL_SIZE_WORD, pixels, (void*)&(PIXELRZ0->TXDATA), PIXELRZ_PIXEL_COUNT);
sl_hal_ldma_descriptor_t desc_rainbow = SL_HAL_LDMA_DESCRIPTOR_LINKREL_M2P(SL_HAL_LDMA_CTRL_SIZE_WORD, pixels, (void*)&(PIXELRZ0->TXDATA), PIXELRZ_PIXEL_COUNT, 0);

/***************************************************************************//**
 * GPIO callback function for pushbutton 0 press. Changes the LED color.
 ******************************************************************************/
void gpio_button_callback(void)
{
  color = (color + 1) % COLORS_TOTAL;

  color_change = true;
}

/***************************************************************************//**
 * Initialize GPIO.
 ******************************************************************************/
void gpio_init(void)
{
  int32_t int_no = BUTTON0_PIN;

  // Initialize GPIO driver
  sl_gpio_init();

  // Configure push button BTN0 input
  sl_gpio_set_pin_mode(&GPIO_BUTTON0, SL_GPIO_MODE_INPUT_PULL_FILTER, true);
  sl_gpio_configure_external_interrupt(&GPIO_BUTTON0, &int_no, SL_GPIO_INTERRUPT_FALLING_EDGE, (sl_gpio_irq_callback_t)gpio_button_callback, NULL);

  // Configure the PIXELRZ pin as push-pull output driving low
  sl_gpio_set_pin_mode(&GPIO_PIXELRZ_SEQ, SL_GPIO_MODE_PUSH_PULL, 0);

  // Route the PIXELRZ pin
  GPIO->PIXELRZROUTE[PIXELRZ_NUM(PIXELRZ0)].RZTXOUTROUTE = (GPIO_PIXELRZ_SEQ.port << _GPIO_PIXELRZ_RZTXOUTROUTE_PORT_SHIFT) | (GPIO_PIXELRZ_SEQ.pin << _GPIO_PIXELRZ_RZTXOUTROUTE_PIN_SHIFT);
  GPIO->PIXELRZROUTE[PIXELRZ_NUM(PIXELRZ0)].ROUTEEN = GPIO_PIXELRZ_ROUTEEN_RZTXOUTPEN;
}

/***************************************************************************//**
 * Initialize PIXELRZ.
 ******************************************************************************/
void pixelrz_init(sl_hal_pixelrz_trigger_t trigger)
{
  // Enable PIXELRZ bus clock and get frequency
  uint32_t freq;
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_PIXELRZ0);
  sl_clock_manager_get_clock_branch_frequency(SL_CLOCK_BRANCH_PIXELRZCLK, &freq);

  // Configure PIXELRZ to work with 60 WS2812B
  sl_hal_pixelrz_config_t init = SL_HAL_PIXELRZ_CONFIG_SEQUENTIAL_DEFAULT;

  init.pixel_number = PIXELRZ_PIXEL_COUNT;
  init.pixel_width = PIXELRZ_PIXEL_WIDTH;
  init.trail_data_width = PIXELRZ_PIXEL_TRAIL;

  init.zero_symbol = sl_hal_pixelrz_get_symbol_configuration(WS2812B_T0_HIGH, WS2812B_T0_LOW, freq);
  init.one_symbol = sl_hal_pixelrz_get_symbol_configuration(WS2812B_T1_HIGH, WS2812B_T1_LOW, freq);
  init.reset_symbol = sl_hal_pixelrz_get_symbol_configuration(WS2812B_TRST_HIGH, WS2812B_TRST_LOW, freq);

  init.memalign_32b_enable = 1;
  init.msb_first_enable = 1;
  init.trigger_mode = trigger;

  // Initialize and start PIXELRZ
  sl_hal_pixelrz_init(PIXELRZ0, &init);
  sl_hal_pixelrz_enable(PIXELRZ0);
}

/***************************************************************************//**
 * Initialize PRS.
 ******************************************************************************/
void prs_init(void)
{
  // Enable PRS peripheral bus clock
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_PRS);

  // Configure LETIMER0 as PRS producer to trigger PIXELRZ frame output
  sl_hal_prs_async_connect_channel_producer(PIXELRZ_ASYNC_PRS_CH, SL_HAL_PRS_ASYNC_LETIMER0_CH0);
  sl_hal_prs_connect_channel_consumer(PIXELRZ_ASYNC_PRS_CH, SL_HAL_PRS_TYPE_ASYNC, SL_HAL_PRS_CONSUMER_PIXELRZ0_INASYNC);
}

/***************************************************************************//**
 * LDMA callback function for PIXELRZ channel. This callback function is only
 * called for the RAINBOW descriptor. This function updates the source array
 * with the correct color to output in the next pixel frame.
 ******************************************************************************/
void ldma_pixelrz_callback(void)
{
  static uint32_t add = 0;
  static uint32_t sub = 0;
  int i;

  switch (pixels[0]) {
    case 0x00FF0000: add = 0x01000000; sub = 0x00000000; break; // RED:    increase G
    case 0xFFFF0000: add = 0x00000000; sub = 0x00010000; break; // YELLOW: decrease R
    case 0xFF000000: add = 0x00000100; sub = 0x00000000; break; // GREEN:  increase B
    case 0xFF00FF00: add = 0x00000000; sub = 0x01000000; break; // GREENB: decrease G
    case 0x0000FF00: add = 0x00010000; sub = 0x00000000; break; // BLUE:   increase R
    case 0x00FFFF00: add = 0x00000000; sub = 0x00000100; break; // PURPLE: decrease B
    default: break;
  }

  for (i=0; i<PIXELRZ_PIXEL_COUNT; i++) {
    pixels[i] = pixels[i] + add - sub;
  }
}

/***************************************************************************//**
 * Initialize LDMA.
 ******************************************************************************/
void ldma_init(void)
{
  bool active;

  // Initialize DMADRV
  DMADRV_Init();

  // Ignore single request for better utilization of DMA
  desc_solid.xfer.ignore_single_req = true;
  desc_rainbow.xfer.ignore_single_req = true;

  // Do not trigger an interrupt when solid descriptor is complete
  desc_solid.xfer.done_ifs = false;

  // Trigger an interrupt when rainbow descriptor is complete
  desc_rainbow.xfer.done_ifs = true;

  // Halt on error if DMA channel cannot be allocated
  if (DMADRV_AllocateChannel(&channelId, NULL) != ECODE_EMDRV_DMADRV_OK) {
    __BKPT(0);
  }

  // Check that memory->PIXELRZ LDMA channel is not currently active; halt if error
  if (DMADRV_TransferActive(channelId, &active) != ECODE_EMDRV_DMADRV_OK) {
    __BKPT(0);
  }

  // Start LDMA transfer
  if (!active) {
    DMADRV_LdmaStartTransfer(channelId, &config, &desc_solid, NULL, NULL);
  }
}

/***************************************************************************//**
 * Initialize LETIMER.
 ******************************************************************************/
void letimer_init(void)
{
  // Declare initialization structures
  sl_hal_letimer_config_t letimerInit = SL_HAL_LETIMER_CONFIG_DEFAULT;

  // Enable LETIMER0 peripheral bus clock
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_LETIMER0);

  // Calculate the top value (frequency) based on clock source
  uint32_t topValue = LETIMER_FREQ / RAINBOW_FREQ;

  // Reload top on underflow, toggle output, and run in free mode
  letimerInit.enable_top = true;
  letimerInit.underflow_output0_action = SL_HAL_LETIMER_UNDERFLOW_OUTPUT_ACTION_PULSE;
  letimerInit.repeat_mode = SL_HAL_LETIMER_REPEAT_MODE_FREE;

  // Initialize LETIMER0
  sl_hal_letimer_init(LETIMER0, &letimerInit);

  // Enable LETIMER0; must be enabled to set top value
  sl_hal_letimer_enable(LETIMER0);

  // Set LETIMER0 top value
  sl_hal_letimer_set_top(LETIMER0, topValue);
}

/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  int i;

  // Initialize pixel array with first color
  for (i=0; i<PIXELRZ_PIXEL_COUNT; i++) {
    pixels[i] = colors_grb[color];
  }

  // Initialize peripherals
  gpio_init();
  pixelrz_init(SL_HAL_PIXELRZ_TRIG_SW);
  prs_init();
  letimer_init();
  ldma_init();

  // Start PIXELRZ transmission
  sl_hal_pixelrz_enable_tx(PIXELRZ0);
  sl_hal_pixelrz_wait_sync(PIXELRZ0);
}

/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{
  int i;
  bool active;

  // Pushbutton was pressed to change mode
  if (color_change) {
    color_change = false;

    if (color != RAINBOW) {
      // Change PIXELRZ to be SW triggered if changing from rainbow to solid color
      if (color == 0) {
        // Stop LETIMER0
        sl_hal_letimer_stop(LETIMER0);

        // Wait for PIXELRZ to finish transmitting current frame
        while (!(sl_hal_pixelrz_get_status(PIXELRZ0) & _PIXELRZ_STATUS_FRAMEDONE_MASK));

        // Stop DMA transfer
        DMADRV_StopTransfer(channelId);

        // Re-enable PIXELRZ with Software Triggered configuration
        sl_hal_pixelrz_clear_tx(PIXELRZ0);
        sl_hal_pixelrz_disable(PIXELRZ0);
        pixelrz_init(SL_HAL_PIXELRZ_TRIG_SW);
      }

      // Update LDMA source array with next solid color
      for (i=0; i<PIXELRZ_PIXEL_COUNT; i++) {
        pixels[i] = colors_grb[color];
      }

      // Check that memory->PIXELRZ LDMA channel is not currently active; halt if error
      if (DMADRV_TransferActive(channelId, &active) != ECODE_EMDRV_DMADRV_OK) {
        __BKPT(0);
      }

      // Start LDMA transfer if LDMA channel is not currently active
      if (!active) {
        DMADRV_LdmaStartTransfer(channelId, &config, &desc_solid, NULL, NULL);
      }

      // SW trigger PIXELRZ to TX next frame
      sl_hal_pixelrz_enable_tx(PIXELRZ0);
      sl_hal_pixelrz_wait_sync(PIXELRZ0);
    }
    else {
      // Change PIXELRZ to be PRS triggered if changing from solid color to rainbow
      sl_hal_pixelrz_clear_tx(PIXELRZ0);
      sl_hal_pixelrz_disable(PIXELRZ0);
      pixelrz_init(SL_HAL_PIXELRZ_TRIG_PRS);

      // Initialize pixel array with first color of rainbow
      for (i=0; i<PIXELRZ_PIXEL_COUNT; i++) {
        pixels[i] = colors_grb[RED];
      }

      // Check that memory->PIXELRZ LDMA channel is not currently active; halt if error
      if (DMADRV_TransferActive(channelId, &active) != ECODE_EMDRV_DMADRV_OK) {
        __BKPT(0);
      }

      // Start LDMA transfer
      if (!active) {
        DMADRV_LdmaStartTransfer(channelId, &config, &desc_rainbow, (DMADRV_Callback_t)ldma_pixelrz_callback, NULL);
      }

      // Enable PRS triggered PIXELRZ
      sl_hal_pixelrz_enable_tx(PIXELRZ0);
      sl_hal_pixelrz_wait_sync(PIXELRZ0);

      // Start LETIMER0
      sl_hal_letimer_set_counter(LETIMER0, LETIMER_FREQ / RAINBOW_FREQ);
      sl_hal_letimer_start(LETIMER0);
    }
  }
}


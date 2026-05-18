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
#include "sl_gpio.h"
#include "pin_config.h"
#include "peripheral_config.h"
#include "sl_hal_pcnt.h"
#include "sl_hal_prs.h"

// PRS channel to use for GPIO/PCNT
#define PCNT_PRS_Ch0    0

// PCNT Top value used to trigger interrupt
#define PCNTTopValue    5    // Interrupt at every 6 BTN0 presses

const sl_gpio_t GPIO_LED0 = { .port = LED0_PORT, .pin = LED0_PIN };
const sl_gpio_t GPIO_BUTTON0 = { .port = BUTTON0_PORT, .pin = BUTTON0_PIN };

/***************************************************************************//**
 * @brief escape_hatch()
 * When developing or debugging code that enters EM2 or
 * lower, it's a good idea to have an "escape hatch" type
 * mechanism, e.g. a way to pause the device so that a debugger can
 * connect in order to erase flash, among other things.
 *
 * Before proceeding with this example, make sure PB0 is not pressed.
 * If the PB0 pin is low, turn on LED0 and execute the breakpoint
 * instruction to stop the processor in EM0 and allow a debug
 * connection to be made.
 ******************************************************************************/
void escape_hatch(void)
{
  bool value;
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_GPIO);

  sl_gpio_set_pin_mode(&GPIO_BUTTON0, SL_GPIO_MODE_INPUT_PULL_FILTER, true);
  sl_gpio_get_pin_input(&GPIO_BUTTON0,&value);

  if (value == 0) {
    sl_gpio_set_pin_mode(&GPIO_LED0, SL_GPIO_MODE_PUSH_PULL, LED_ON);
    __BKPT(0);
  } else { // Pin not asserted, so disable input
    sl_gpio_set_pin_mode(&GPIO_BUTTON0,SL_GPIO_MODE_DISABLED, false);
  }
}

/***************************************************************************//**
 * @brief PCNT0 interrupt handler
 *        This function acknowledges the interrupt and toggles LED0
 ******************************************************************************/
void PCNT0_IRQHandler(void)
{
  // Acknowledge interrupt
  sl_hal_pcnt_clear_interrupts(PCNT0, PCNT_IF_OF);

  // Toggle LED0
  sl_gpio_toggle_pin(&GPIO_LED0);
}

/***************************************************************************//**
 * @brief Initialize PCNT0
 *        This function sets up PCNT0 with external quadrature mode
 *        S0 PRS linked to PRSCH0
 ******************************************************************************/
static void init_pcnt(void)
{
  sl_hal_pcnt_init_t init = SL_HAL_PCNT_INIT_DEFAULT(SL_HAL_PCNT_MODE_OVS_SINGLE);
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_PCNT0);

  sl_hal_prs_connect_channel_consumer(PCNT_PRS_Ch0,
                                      SL_HAL_PRS_TYPE_ASYNC,
                                      SL_HAL_PRS_CONSUMER_PCNT0_S0IN);

  // PCNT0 Init
  init.s0_prs_enable = true;
  sl_hal_pcnt_init(PCNT0, &init);

  // Enable PCNT
  sl_hal_pcnt_enable(PCNT0);

  //sl_hal_pcnt_set_main_counter(PCNT0, PCNTTopValue);
  sl_hal_pcnt_set_top(PCNT0, PCNTTopValue);

  // Enable PCNT0 interrupt
  sl_hal_pcnt_enable_interrupts(PCNT0, PCNT_IEN_OF);

  // Clear PCNT0 pending interrupt
  NVIC_ClearPendingIRQ(PCNT0_IRQn);

  // Enable PCNT0 interrupt in the interrupt controller
  NVIC_EnableIRQ(PCNT0_IRQn);

  sl_hal_pcnt_start_main_counter(PCNT0);
}

/***************************************************************************//**
 * @brief Initialize PRS
 *        This function sets up GPIO PRS which links BTN0 to PCNT0 PRS0
 ******************************************************************************/
static void init_prs(void)
{
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_PRS);

  sl_hal_prs_async_connect_channel_producer(PCNT_PRS_Ch0,
                                            SL_HAL_PRS_ASYNC_GPIO_PIN1);
}

/***************************************************************************//**
 * @brief Initialize GPIO
 *        This function initializes push button PB0 and enables external
 *        interrupts for PRS functionality
 ******************************************************************************/
static void init_gpio(void)
{
  int32_t button0_int_no = 1;
  sl_gpio_init();

  // Initialize LED driver
  sl_gpio_set_pin_mode(&GPIO_LED0, SL_GPIO_MODE_PUSH_PULL, 0);

  // Configure pin I/O - BUTTON0
  sl_gpio_set_pin_mode(&GPIO_BUTTON0, SL_GPIO_MODE_INPUT_PULL_FILTER, true);

  // Configure BUTTON0 for external interrupt
  sl_gpio_configure_external_interrupt(&GPIO_BUTTON0,
                                       &button0_int_no,
                                       SL_GPIO_INTERRUPT_NO_EDGE,
                                       NULL,
                                       NULL);
}

/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  escape_hatch();

  // Initialize GPIO
  init_gpio();

  // PRS Initialization
  init_prs();

  // PCNT Initialization
  init_pcnt();
}

/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{
}

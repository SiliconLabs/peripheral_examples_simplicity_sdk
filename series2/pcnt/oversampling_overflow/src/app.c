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
#include "em_pcnt.h"
#include "em_prs.h"

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
  sl_gpio_get_pin_input(&GPIO_BUTTON0, &value);

  if (value == 0) {
    sl_gpio_set_pin_mode(&GPIO_LED0, SL_GPIO_MODE_PUSH_PULL, LED_ON);
    __BKPT(0);
  }
  // Pin not asserted, so disable input
  else {
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
  PCNT_IntClear(PCNT0, PCNT_IF_OF);

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
  PCNT_Init_TypeDef pcnt_init = PCNT_INIT_DEFAULT;
  PCNT_Filter_TypeDef pcnt_filter_init = PCNT_FILTER_DEFAULT;

  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_PCNT0);

  pcnt_init.mode     = pcntModeOvsSingle;  // Oversampling single mode
  pcnt_init.top      = PCNTTopValue;
  pcnt_init.s1CntDir = false;              // S1 does not affect counter direction,
                                           // using default init setting; count up
  pcnt_init.s0PRS    = PCNT_PRS_Ch0;
  pcnt_init.filter   = true;               // Filter GPIO to remove glitching LED behavior

  // Use max filter len for GPIO push button
  pcnt_filter_init.filtLen = _PCNT_OVSCTRL_FILTLEN_MASK;

  // Enable PRS0 for PCNT0
  PCNT_PRSInputEnable(PCNT0, pcntPRSInputS0, true);

  // Filter configuration
  PCNT_FilterConfiguration(PCNT0, &pcnt_filter_init, true);

  // Enable PCNT0
  PCNT_Init(PCNT0, &pcnt_init);

  /*
   * On-board push button is active low on WSTK and PCNT is configured to count
   * on rising edge. Upon enabling PCNT module, initial state of push button
   * will cause the counter to increment without a button press.
   *
   * Wait here until this count propagates through counter-clock domain.
   */
  while(PCNT0->CNT == 0);

  /*
   * Reset the count back to zero.
   *
   * Alternatively, modifying the PCNT initialization configuration to count on
   * falling edge (add "pcnt_init.negEdge = true;" before PCNT_Init() function
   * call) would also alleviate additional count on PCNT module start-up, or the
   * PRS logic could be modified to negate the discrete on-board logic (select
   * NOT A function logic in PRS channel's FNSEL field). For this example, the
   * desire is to increment counter on push button release. Note that negating
   * both (negEdge and PRS logic) results in the same problem and would require
   * the counter clear that has been implemented for accurate count of the first
   * overflow event.
   */
  PCNT0->CMD_SET = PCNT_CMD_CNTRST;

  // Enable overflow interrupt for PCNT0
  PCNT_IntEnable(PCNT0, PCNT_IEN_OF);

  // Clear PCNT0 pending interrupt
  NVIC_ClearPendingIRQ(PCNT0_IRQn);

  // Enable PCNT0 interrupt in the interrupt controller
  NVIC_EnableIRQ(PCNT0_IRQn);
}

/***************************************************************************//**
 * @brief Initialize PRS
 *        This function sets up GPIO PRS which links BTN0 to PCNT0 PRS0
 ******************************************************************************/
static void init_prs(void)
{
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_PRS);

  // Set up GPIO PRS
  PRS_SourceAsyncSignalSet(PCNT_PRS_Ch0, PRS_ASYNC_CH_CTRL_SOURCESEL_GPIO,
                           BUTTON0_PIN);
}

/***************************************************************************//**
 * @brief Initialize GPIO
 *        This function initializes push button PB0 and enables external
 *        interrupts for PRS functionality
 ******************************************************************************/
static void init_gpio(void)
{
  int32_t button0_int_no;

  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_GPIO);

  // Initialize LED driver
  sl_gpio_set_pin_mode(&GPIO_LED0, SL_GPIO_MODE_PUSH_PULL, 0);

  // Configure pin I/O - BTN0
  sl_gpio_set_pin_mode(&GPIO_BUTTON0, SL_GPIO_MODE_INPUT_PULL_FILTER, 1);

  // Configure BTN0 for external interrupt
  button0_int_no = BUTTON0_PIN;
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

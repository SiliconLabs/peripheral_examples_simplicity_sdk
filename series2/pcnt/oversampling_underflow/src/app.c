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
  PCNT_IntClear(PCNT0, PCNT_IF_UF);

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

  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_PCNT0);

  pcnt_init.mode     = pcntModeDisable;   // Initialize with PCNT disabled
  pcnt_init.top      = PCNTTopValue;
  pcnt_init.s1CntDir = false;             // S1 does not affect counter direction,
                                          // using default init setting; count up
  pcnt_init.countDown = true;             // Counter counting down
  pcnt_init.cntEvent  = pcntCntEventDown; // Event triggering when counting down
  pcnt_init.s0PRS     = PCNT_PRS_Ch0;

  // Enable PRS0 input
  PCNT_PRSInputEnable(PCNT0, pcntPRSInputS0, true);

  // PCNT0 Init
  PCNT_Init(PCNT0, &pcnt_init);

  // Set mode to externally clocked quadrature decoder
  PCNT_Enable(PCNT0, pcntModeExtSingle);

  // Change to the external clock
  CMU->PCNT0CLKCTRL = CMU_PCNT0CLKCTRL_CLKSEL_PCNTS0;

  // Preload top value into CNT register (counting down)
  PCNT0->CMD_SET = PCNT_CMD_LCNTIM;

  /*
   * When the PCNT operates in externally clocked mode and switching external
   * clock source, a few clock pulses are required on the external clock to
   * synchronize accesses to the externally clocked domain. This example uses
   * push button PB0 via GPIO/PRS/PCNT0_S0IN for the external clock, which would
   * require multiple button presses to sync the PCNT registers and clock
   * domain, during which button presses would not be counted.
   *
   * To get around this, such that each button push is recognized, firmware
   * can use the PRS software pulse triggering mechanism to generate
   * those first few pulses. This allows the first actual button press and
   * subsequent button presses to be properly counted.
   */
  while (PCNT0->SYNCBUSY) {
    PRS_PulseTrigger(1 << PCNT_PRS_Ch0);
  }

  // Enable PCNT0 interrupt
  PCNT_IntEnable(PCNT0, PCNT_IEN_UF);

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

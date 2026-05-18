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
 * @brief
 *   Wait for ongoing sync of register(s) to the low-frequency domain to complete.
 *
 * @param[in] pcnt
 *   Pointer to the PCNT peripheral register block.
 ******************************************************************************/
void pcnt_wait_sync(PCNT_TypeDef *pcnt)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_PCNT_REF_VALID(pcnt));

  while (pcnt->SYNCBUSY & _PCNT_SYNCBUSY_MASK) {
    // Pulse prs channel to synchronize PCNT
    sl_hal_prs_async_set_channel_swlevel(PCNT_PRS_Ch0,true);
    sl_hal_prs_async_set_channel_swlevel(PCNT_PRS_Ch0,false);
  }
}

/***************************************************************************//**
 * @brief
 *   Set the top value buffer.
 *
 * @details
 *   When top value buffer register is updated, value is loaded into
 *   top value register at the next wrap around. This feature is useful
 *   in order to update top value safely when PCNT is running. This won't
 *   happen if @ref SL_HAL_PCNT_COUNT_EVENT_BOTH is used.
 *
 * @param[in] pcnt
 *   Pointer to the PCNT peripheral register block.
 *
 * @param[in] value
 *   Value to set in top value buffer register.
 ******************************************************************************/
void pcnt_set_top_buffer(PCNT_TypeDef *pcnt,
                                         uint32_t value)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_PCNT_REF_VALID(pcnt));
  // Make sure counter value is valid.
  EFM_ASSERT(value <= SL_HAL_PCNT_MAX_COUNT(pcnt));
  // Make sure module is enabled.
  EFM_ASSERT(pcnt->EN & _PCNT_EN_EN_MASK);

  pcnt_wait_sync(pcnt);
  pcnt->TOPB = value;
}

/***************************************************************************//**
 * @brief
 *   Set the top value.
 *
 * @param[in] pcnt
 *   Pointer to the PCNT peripheral register block.
 *
 * @param[in] value
 *   Value to set in top value register.
 ******************************************************************************/
void pcnt_set_top(PCNT_TypeDef *pcnt,
                                  uint32_t value)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_PCNT_REF_VALID(pcnt));
  // Make sure counter value is valid.
  EFM_ASSERT(value <= SL_HAL_PCNT_MAX_COUNT(pcnt));
  // Make sure module is enabled.
  EFM_ASSERT(pcnt->EN & _PCNT_EN_EN_MASK);

  pcnt_wait_sync(pcnt);
  pcnt->TOP = value;
}

/***************************************************************************//**
 * @brief
 *   Set a counter value.
 ******************************************************************************/
void pcnt_set_main_counter(PCNT_TypeDef *pcnt,
                                  uint32_t value)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_PCNT_REF_VALID(pcnt));
  // Make sure counter value is valid.
  EFM_ASSERT(value <= SL_HAL_PCNT_MAX_COUNT(pcnt));
  // Make sure module is enabled.
  EFM_ASSERT(pcnt->EN & _PCNT_EN_EN_MASK);

  uint32_t top = pcnt->TOP;
  if (top != value) {
    pcnt_set_top(pcnt, value);
  }

  pcnt_wait_sync(pcnt);
  pcnt->CMD_SET = PCNT_CMD_LCNTIM;

  if (top != value) {
    pcnt_set_top(pcnt, top);
  }
}

/***************************************************************************//**
 * @brief
 *   Start PCNT main counter.
 *
 * @details
 *   This function will send a start command to the PCNT peripheral. The PCNT
 *   peripheral will use some LF clock ticks before the command is executed.
 *   The @ref pcnt_wait_sync() function can be used to wait for the start
 *   command to be executed.
 *
 * @param[in] pcnt
 *   Pointer to the PCNT peripheral register block.
 ******************************************************************************/
void pcnt_start_main_counter(PCNT_TypeDef *pcnt)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_PCNT_REF_VALID(pcnt));
  // Make sure module is enabled.
  EFM_ASSERT(pcnt->EN & _PCNT_EN_EN_MASK);

  pcnt_wait_sync(pcnt);
  pcnt->CMD_SET = PCNT_CMD_STARTCNT;
  pcnt_wait_sync(pcnt);
}

/***************************************************************************//**
 * @brief
 *   Initialize PCNT.  This function does not disable the PCNT after
 *   initialization, unlike the sl_hal_pcnt_init() function
 ******************************************************************************/
void pcnt_init(PCNT_TypeDef *pcnt,
                      const sl_hal_pcnt_init_t *init)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_PCNT_REF_VALID(pcnt));

  // Disable PCNT.
  sl_hal_pcnt_disable(pcnt);
  sl_hal_pcnt_wait_ready(pcnt);

  // Write the CFG register with the configurations.
  pcnt->CFG = (pcnt->CFG & ~(_PCNT_CFG_MODE_MASK | _PCNT_CFG_DEBUGHALT_MASK | _PCNT_CFG_FILTEN_MASK
                             | _PCNT_CFG_HYST_MASK | _PCNT_CFG_S0PRSEN_MASK | _PCNT_CFG_S1PRSEN_MASK))
              | (((uint32_t)init->mode) << _PCNT_CFG_MODE_SHIFT)
              | (((uint32_t)init->debug_halt) << _PCNT_CFG_DEBUGHALT_SHIFT)
              | (((uint32_t)init->filter_enable) << _PCNT_CFG_FILTEN_SHIFT)
              | (((uint32_t)init->hysteresis_enable) << _PCNT_CFG_HYST_SHIFT)
              | (((uint32_t)init->s0_prs_enable) << _PCNT_CFG_S0PRSEN_SHIFT)
              | (((uint32_t)init->s1_prs_enable) << _PCNT_CFG_S1PRSEN_SHIFT);

  sl_hal_pcnt_enable(pcnt);

  // Write the CTRL register with the configurations.
  pcnt->CTRL = (pcnt->CTRL & ~(_PCNT_CTRL_S1CDIR_MASK | _PCNT_CTRL_CNTDIR_MASK | _PCNT_CTRL_EDGE_MASK
                               | _PCNT_CTRL_CNTEV_MASK | _PCNT_CTRL_AUXCNTEV_MASK))
               | (((uint32_t)init->s1_count_direction) << _PCNT_CTRL_S1CDIR_SHIFT)
               | (((uint32_t)init->count_down) << _PCNT_CTRL_CNTDIR_SHIFT)
               | (((uint32_t)init->negative_edge) << _PCNT_CTRL_EDGE_SHIFT)
               | (((uint32_t)init->main_count_event) << _PCNT_CTRL_CNTEV_SHIFT)
               | (((uint32_t)init->aux_count_event) << _PCNT_CTRL_AUXCNTEV_SHIFT);

  // Write the OVSCTRL register with the configurations.
  pcnt->OVSCTRL = (pcnt->OVSCTRL & ~(_PCNT_OVSCTRL_FLUTTERRM_MASK | _PCNT_OVSCTRL_FILTLEN_MASK))
                  | (((uint32_t)init->flutter_remove) << _PCNT_OVSCTRL_FLUTTERRM_SHIFT)
                  | (((uint32_t)init->filter_lenght) << _PCNT_OVSCTRL_FILTLEN_SHIFT);

  if (init->mode == SL_HAL_PCNT_MODE_EXT_CLK_SINGLE || init->mode == SL_HAL_PCNT_MODE_EXT_CLK_QUAD) {
    // Enable PCNT Clock Domain Reset. The PCNT must be in reset before changing
    // the clock source to an external clock.
    pcnt->CMD_SET = PCNT_CMD_CORERST;
    CMU->PCNT0CLKCTRL = CMU_PCNT0CLKCTRL_CLKSEL_PCNTS0;
  } else {
    CMU->PCNT0CLKCTRL = CMU_PCNT0CLKCTRL_CLKSEL_EM23GRPACLK;
  }
}

/***************************************************************************//**
 * @brief PCNT0 interrupt handler
 *        This function acknowledges the interrupt and toggles LED0
 ******************************************************************************/
void PCNT0_IRQHandler(void)
{
  // Acknowledge interrupt
  sl_hal_pcnt_clear_interrupts(PCNT0, PCNT_IF_UF);

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
  sl_hal_pcnt_init_t init = SL_HAL_PCNT_INIT_DEFAULT(SL_HAL_PCNT_MODE_EXT_CLK_SINGLE);
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_PCNT0);

  sl_hal_prs_connect_channel_consumer(PCNT_PRS_Ch0,
                                      SL_HAL_PRS_TYPE_ASYNC,
                                      SL_HAL_PRS_CONSUMER_PCNT0_S0IN);

  // PCNT0 Init
  init.s0_prs_enable = true;
  init.count_down = true;
  pcnt_init(PCNT0, &init);

  // Enable PCNT0 interrupt
  sl_hal_pcnt_enable_interrupts(PCNT0, PCNT_IEN_UF);

  // Clear PCNT0 pending interrupt
  NVIC_ClearPendingIRQ(PCNT0_IRQn);

  // Enable PCNT0 interrupt in the interrupt controller
  NVIC_EnableIRQ(PCNT0_IRQn);

  pcnt_set_main_counter(PCNT0, PCNTTopValue);
  pcnt_start_main_counter(PCNT0);
  pcnt_set_top(PCNT0, PCNTTopValue);
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

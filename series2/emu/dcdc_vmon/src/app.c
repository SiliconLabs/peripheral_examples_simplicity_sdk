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

#include "em_emu.h"
#include "peripheral_config.h"
#include "sl_gpio.h"

#include "sl_interrupt_manager.h"
#include "sl_power_manager.h"
#include "sl_sleeptimer.h"

#include "pin_config.h"

// Test mode control signals
volatile bool low_voltage = false;
volatile bool dcdc_interrupt_flag = false;

#define VREGVDD_THRESHOLD_HI emuVreginCmpThreshold_2v3
#define VREGVDD_THRESHOLD_LOW emuVreginCmpThreshold_2v1

#ifndef BUTTON0_PORT
  #define BUTTON0_PORT LED0_BUTTON0_PORT
  #define BUTTON0_PIN LED0_BUTTON0_PIN
#endif

#ifndef LED0_PORT
  #define LED0_PORT LED0_BUTTON0_PORT
  #define LED0_PIN LED0_BUTTON0_PIN
#endif

#ifndef LED1_PORT
  #define LED1_PORT LED1_BUTTON1_PORT
  #define LED1_PIN LED1_BUTTON1_PIN
#endif

// Conditional compilation to configure GPIO mappings based on the target microcontroller
#if defined(EFR32MG21A010F1024IM32)
const sl_gpio_t GPIO_ESCAPE_HATCH = { .port = BUTTON1_PORT, .pin = BUTTON1_PIN };
#else
const sl_gpio_t GPIO_ESCAPE_HATCH = { .port = BUTTON0_PORT, .pin = BUTTON0_PIN };
#endif

const sl_gpio_t LED0 = { .port = LED0_PORT, .pin = LED0_PIN };
const sl_gpio_t LED1 = { .port = LED1_PORT, .pin = LED1_PIN };

sl_sleeptimer_timer_handle_t periodic_timer;

void periodic_timer_callback(sl_sleeptimer_timer_handle_t *handle, void *data)
{
  (void)handle;
  (void)data;
  
  // Code executed when the timer expires.
  sl_gpio_toggle_pin(&LED1);
}

int start_timer(void)
{
  sl_status_t status;
  uint32_t timer_timeout = 32768;

  // We assume the sleeptimer is initialized properly
  status = sl_sleeptimer_start_periodic_timer(&periodic_timer,
                                     timer_timeout,
                                     periodic_timer_callback,
                                     (void *)NULL,
                                     0,
                                     0);
  if(status != SL_STATUS_OK) {
    return -1;
  }
  return 1;
}

/**************************************************************************//**
 * When developing or debugging code that enters EM2 or lower, it's a
 * good idea to have an "escape hatch" type mechanism, e.g. a way to
 * pause the device so that a debugger can connect in order to erase
 * flash, among other things.
 *
 * Before proceeding with this example, make sure Escape Hatch button is not
 * pressed. If the Escape Hatch button pin is low, turn on LED0 and execute the
 * breakpoint (BKPT) instruction to stop the processor in EM0 and allow a debug
 * connection to be made.
 *****************************************************************************/
void escapeHatch(void)
{
  bool pin_value;

  // Initialize GPIO driver
  sl_gpio_init();

  // Configure Escape Hatch button as an input pulled high
  sl_gpio_set_pin_mode(&GPIO_ESCAPE_HATCH, SL_GPIO_MODE_INPUT_PULL_FILTER, true);

  // Check Escape Hatch button state: if pressed, enable LED0 and trigger breakpoint
  sl_gpio_get_pin_input(&GPIO_ESCAPE_HATCH, &pin_value);
  if (pin_value == 0)
  {
    sl_gpio_set_pin_mode(&LED1, SL_GPIO_MODE_PUSH_PULL, LED_ON);
    __BKPT(0);  // Halt execution for debugging
  }
  else
  {
    // Disable Escape Hatch button digital input when not in use
    sl_gpio_set_pin_mode(&GPIO_ESCAPE_HATCH, SL_GPIO_MODE_DISABLED, false);
  }
}

/**************************************************************************//**
 * @brief  GPIO Initializer
 *****************************************************************************/
void gpio_init(void)
{
  // Configure indicator pins as outputs
  sl_gpio_set_pin_mode(&LED0, SL_GPIO_MODE_PUSH_PULL, LED_OFF);
  sl_gpio_set_pin_mode(&LED1, SL_GPIO_MODE_PUSH_PULL, LED_OFF);
}

/*****************************************************************************/
/*@brief: DCDC interrupt handler
 *****************************************************************************/
void DCDC_IRQHandler(void)
{
  volatile uint32_t flags = DCDC->IF;

  // Clear all interrupt flags
  DCDC->IF_CLR = flags;

  // Check for interrupt flag
  if((flags & DCDC_IF_VREGINHIGH) && (DCDC->IEN & DCDC_IEN_VREGINHIGH)) {
    low_voltage = false;
    DCDC->IEN_CLR = DCDC_IEN_VREGINHIGH;
  } else if((flags & DCDC_IF_VREGINLOW) && (DCDC->IEN & DCDC_IEN_VREGINLOW)) {
    low_voltage = true;
    DCDC->IEN_CLR = DCDC_IEN_VREGINLOW;
  }

  /*
  *  Indicate threshold interrupt to app_process_action()
  *  EMU emlib APIs have lengthy timeouts when setting DCDC to bypass or 
  *  regulation modes that must not be handled within an ISR.
  */ 
  dcdc_interrupt_flag = true;
}

/*****************************************************************************/
/*@brief DCDC Initialization
 *****************************************************************************/
void dcdc_init(void)
{
  // DCDC component initialized in sl_main_init(); default threshold is 2.3V
  // Set VREGVDD comparator high threshold to project's #define
  EMU->VREGVDDCMPCTRL = (EMU->VREGVDDCMPCTRL && _EMU_VREGVDDCMPCTRL_VREGINCMPEN_MASK)
   | (VREGVDD_THRESHOLD_HI << _EMU_VREGVDDCMPCTRL_THRESSEL_SHIFT);

  // Clear all DCDC interrupt
  DCDC->IF_CLR = _DCDC_IF_MASK;

  // Enable VREGIN high detection interrupt
  DCDC->IEN_SET = DCDC_IEN_VREGINHIGH;

  // Enable DCDC interrupt vector.
  sl_interrupt_manager_clear_irq_pending(DCDC_IRQn);
  sl_interrupt_manager_enable_irq(DCDC_IRQn);
}

/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  // Halt execution if Escape Hatch button is pressed before entering EM2
  escapeHatch();

  // Initialize GPIO
  gpio_init();

  // Initialize DCDC
  dcdc_init();

  // start periodic timer
  start_timer();
}

/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{
  if (dcdc_interrupt_flag) {
    /* 
    *  If below threshold, switch to bypass mode (DECOUPLE only mode for 
    *  dual-output DCDC), turn on LED, and enable interrupt flags to detect 
    *  VREGVDD > VTHRESHOLD
    */
    if (low_voltage) {
      // Set VREGVDD comparator threshold to VREGVDD_THRESHOLD_HI (adds some hysteresis)
      EMU->VREGVDDCMPCTRL = (EMU->VREGVDDCMPCTRL && _EMU_VREGVDDCMPCTRL_VREGINCMPEN_MASK)
      | (VREGVDD_THRESHOLD_HI << _EMU_VREGVDDCMPCTRL_THRESSEL_SHIFT);

      EMU_DCDCModeSet(emuDcdcMode_Bypass);

      // Conditional compilation to configure dual output based on the target microcontroller
  #if defined(EFR32FG2DB010F512IM48)    
      // Change regulation type to DECOUPLE
      EMU_DCDCSetRegulationType(emuDcdcRegulationType_RegDEC);
      
      // Re-enable DCDC
      EMU_DCDCModeSet(emuDcdcMode_Regulation);
  #endif

  #if (LED_ON == 1)
      sl_gpio_set_pin(&LED0);
  #else 
      sl_gpio_clear_pin(&LED0);
  #endif

      DCDC->IEN_SET = DCDC_IEN_VREGINHIGH;
    } else {
      /*
      *  If above threshold, switch to regulator on mode (DVDD and DECOUPLE for
      *  dual-output DCDC), turn off LED, and enable interrupt flags to detect
      *  VREGVDD < VTHRESHOLD
      */
      // Set VREGVDD comparator threshold to VREGVDD_THRESHOLD_LOW (adds some hysteresis)
      EMU->VREGVDDCMPCTRL = (EMU->VREGVDDCMPCTRL && _EMU_VREGVDDCMPCTRL_VREGINCMPEN_MASK)
      | (VREGVDD_THRESHOLD_LOW << _EMU_VREGVDDCMPCTRL_THRESSEL_SHIFT);

      // Conditional compilation to configure dual output based on the target microcontroller
  #if defined(EFR32FG2DB010F512IM48)
      // Disable DCDC; DCDC must be disabled when changing regulation type
      EMU_DCDCModeSet(emuDcdcMode_Bypass);

      // Change regulation type to dual-output DVDD + DECOUPLE
      EMU_DCDCSetRegulationType(emuDcdcRegulationType_RegDVDDDEC);
  #endif     

      // Re-enable DCDC
      EMU_DCDCModeSet(emuDcdcMode_Regulation);

  #if (LED_ON == 1)
      sl_gpio_clear_pin(&LED0);
  #else 
      sl_gpio_set_pin(&LED0);
  #endif

      DCDC->IEN_SET = DCDC_IEN_VREGINLOW;
    }
  
  dcdc_interrupt_flag = false;
  }
}

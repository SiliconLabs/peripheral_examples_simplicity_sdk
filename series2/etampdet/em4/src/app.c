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
#include "sl_power_manager.h"

#include "em_emu.h"
#include "em_cmu.h"
#include "em_burtc.h"
#include "em_rmu.h"
#include "sl_gpio.h"

#include "pin_config.h"
#include "peripheral_config.h"

#define POWER_DOWN_RAM  (1)

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
/*
 * These defines can be used to enable/disable each of ETAMPDET peripherals
 * two tamper detection channels
 */
#define ETAMP_CH0_EN 1
#define ETAMP_CH1_EN 1

volatile uint8_t tamperDetectedCh0 = 0;
volatile uint8_t tamperDetectedCh1 = 0;

volatile bool sleep_ok;

const sl_gpio_t LED0 = { .port = LED0_PORT,
                         .pin  = LED0_PIN };

const sl_gpio_t LED1 = { .port = LED1_PORT, 
                         .pin  = LED1_PIN };

const sl_gpio_t ESCAPE_HATCH = { .port = BUTTON0_PORT, 
                                 .pin  = BUTTON0_PIN };

const sl_gpio_t ETAMPDET_CH0_IN  =  { .port = ETAMPDET_IN0_PORT, 
                                      .pin  = ETAMPDET_IN0_PIN };
                                      
const sl_gpio_t ETAMPDET_CH0_OUT =  { .port = ETAMPDET_OUT0_PORT, 
                                      .pin  = ETAMPDET_OUT0_PIN };

const sl_gpio_t ETAMPDET_CH1_IN  =  { .port = ETAMPDET_IN1_PORT, 
                                      .pin  = ETAMPDET_IN1_PIN };

const sl_gpio_t ETAMPDET_CH1_OUT =  { .port = ETAMPDET_OUT1_PORT, 
                                      .pin  = ETAMPDET_OUT1_PIN, };

/* This is a weak function which will be called by Power Manager
 * every time when it tries to go to sleep.
 * With this the application can control when to go to sleep.
 * Before entering EM4 sleep_ok should be set to true, and after 
 * waking up from EM4 sleep_ok should be set to false, so device will stay in EM0
 * and the LED will toggle indefinitely.
 */
bool app_is_ok_to_sleep(void){
  return sleep_ok;
}

/***************************************************************************//**
 *  User defined weak function for performing early application initialization.
 *  This is called from sl_main_init.
 ******************************************************************************/
void app_init_early(void)
{
  // Release pin state after EM4 exit (reset).
  sl_power_manager_em4_unlatch_pin_retention();
}

/**************************************************************************//**
 * @brief  Initialize GPIO 
 *****************************************************************************/
void gpio_init(void)
{
  // Configure LEDs as outputs; will toggle for tamper detection
  sl_gpio_set_pin_mode(&LED0, SL_GPIO_MODE_PUSH_PULL, LED_OFF);
  sl_gpio_set_pin_mode(&LED1, SL_GPIO_MODE_PUSH_PULL, LED_OFF);
}

/**************************************************************************//**
 * @brief  Configure BURTC to interrupt every BURTC_IRQ_PERIOD
 *****************************************************************************/
void burtc_init(void)
{
  // Setup BURTC parameters
  BURTC_Init_TypeDef burtcInit = BURTC_INIT_DEFAULT;

  // Initialize and enable BURTC
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_BURTC);
  BURTC_Reset();
  BURTC_Init(&burtcInit);
  BURTC_Enable(true);
}

/***************************************************************************//**
 * @brief Initialize ETAMPDET 
 ******************************************************************************/
void etampdet_init()
{
  uint8_t chnl0_en, chnl1_en;
  chnl0_en = 0;
  chnl1_en = 0;

#ifdef ETAMP_CH0_EN
#if (ETAMP_CH0_EN == 1)
  chnl0_en = 1;
#endif
#endif

#ifdef ETAMP_CH1_EN
#if (ETAMP_CH1_EN == 1)
  chnl1_en = 1;
#endif
#endif

  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_ETAMPDET);

  if (chnl0_en == 1) { // Configure GPIO for ETAMPDET channel 0
    // Disable GPIO signals associated with used ETAMPER channel 0
    GPIO_PinModeSet(ETAMPDET_IN0_PORT, ETAMPDET_IN0_PIN, gpioModeDisabled, 0);
    GPIO_PinModeSet(ETAMPDET_OUT0_PORT, ETAMPDET_OUT0_PIN, gpioModeDisabled, 0);
  }

  if (chnl1_en == 1) { // Configure GPIO for ETAMPDET channel 1
    // Disable GPIO signals associated with used ETAMPER channel 1
    GPIO_PinModeSet(ETAMPDET_IN1_PORT, ETAMPDET_IN1_PIN, gpioModeDisabled, 0);
    GPIO_PinModeSet(ETAMPDET_OUT1_PORT, ETAMPDET_OUT1_PIN, gpioModeDisabled, 0);
  }

  // Make sure module is disabled before configuring
  ETAMPDET->EN_CLR = ETAMPDET_EN_EN;

/*
 *   Must wait for peripheral to disable before modifying other registers; if
 *   register write is attempted before peripheral is disabled, hard fault will
 *   occur.
 */
#if defined(ETAMPDET_EN_DISABLING)
  while ((ETAMPDET->EN & ETAMPDET_EN_DISABLING) || (ETAMPDET->SYNCBUSY != 0U)) {
    // Wait for disabling to finish
  }
#else
  while (ETAMPDET->SYNCBUSY != 0U) {
    // Wait for all synchronizations to finish
  }
#endif

  // Configure upper and lower prescaler values
  ETAMPDET->CLKPRESCVAL = ETAMPDET_CLKPRESCVAL_LOWERPRESC_DivideBy64 |
                          ETAMPDET_CLKPRESCVAL_UPPERPRESC_Bypass;

  // Load a random seed value for channel 0/1
  ETAMPDET->CHNLSEEDVAL0 = 0x167DC55F;
  ETAMPDET->CHNLSEEDVAL1 = 0x5F04B84A;

  // Enable ETAMPDET as EM4 wakeup source
  ETAMPDET->EM4WUEN_SET = (chnl0_en << _ETAMPDET_EM4WUEN_CHNLEM4WUEN0_SHIFT) |
      (chnl1_en << _ETAMPDET_EM4WUEN_CHNLEM4WUEN1_SHIFT);

  /*
   *   Additional ETAMPDET configurable settings; configuring for both channels
   *   regardless of channel enablement
   */
  ETAMPDET->CFG_SET = ETAMPDET_CFG_CHNLTAMPDETFILTEN0 | ETAMPDET_CFG_CHNLTAMPDETFILTEN1;
  ETAMPDET->CNTMISMATCHMAX =
      ETAMPDET_CNTMISMATCHMAX_CHNLCNTMISMATCHMAX0_DetectFilterThreshold4 |
      ETAMPDET_CNTMISMATCHMAX_CHNLCNTMISMATCHMAX1_DetectFilterThreshold4;
  ETAMPDET->CHNLFILTWINSIZE =
      ETAMPDET_CHNLFILTWINSIZE_CHNLFILTWINSIZE0_DetectFilterMovingWinSize10 |
      ETAMPDET_CHNLFILTWINSIZE_CHNLFILTWINSIZE1_DetectFilterMovingWinSize10;

  // Channel 0/1 pad enable
  ETAMPDET->CFG_SET = (chnl0_en << _ETAMPDET_CFG_CHNLPADEN0_SHIFT) |
      (chnl1_en << _ETAMPDET_CFG_CHNLPADEN1_SHIFT);

  // Enable ETAMPDET module
  if (chnl0_en || chnl1_en)
    ETAMPDET->EN_SET = ETAMPDET_EN_EN;

  // Load channel 0/1 linear feedback shift register (LFSR) seeds
  ETAMPDET->CMD_SET = (chnl0_en << _ETAMPDET_CMD_CHNLLOAD0_SHIFT) |
      (chnl1_en << _ETAMPDET_CMD_CHNLLOAD1_SHIFT);

  /*
   *  Command register writes must synchronize with the LF clock
   *  Check the SYNCBUSY register for busy status; wait for register write
   */
  while (ETAMPDET->SYNCBUSY != 0);

  // Start channel 0/1 LFSR
  ETAMPDET->CMD_SET = (chnl0_en << _ETAMPDET_CMD_CHNLSTART0_SHIFT) |
      (chnl1_en << _ETAMPDET_CMD_CHNLSTART1_SHIFT);

  // Wait for register write
  while (ETAMPDET->SYNCBUSY != 0);
}

/**************************************************************************//**
 * @brief escapeHatch()
 * When developing or debugging code that enters EM2 or
 * lower, it's a good idea to have an "escape hatch" type
 * mechanism, e.g. a way to pause the device so that a debugger can
 * connect in order to erase flash, among other things.
 *
 * Before proceeding with this example, make sure PB0 is not pressed.
 * If the PB0 pin is low, turn on LED0 and execute the breakpoint
 * instruction to stop the processor in EM0 and allow a debug
 * connection to be made.
 *****************************************************************************/
void escape_hatch(void)
{
  bool pin_value;

  // Configure Escape Hatch button as an input pulled high
  sl_gpio_set_pin_mode(&ESCAPE_HATCH, SL_GPIO_MODE_INPUT_PULL_FILTER, true);

  // Check Escape Hatch button state: if pressed, enable LED0 and trigger breakpoint
  sl_gpio_get_pin_input(&ESCAPE_HATCH, &pin_value);
  if (pin_value == 0)
  {
    sl_gpio_set_pin_mode(&LED0, SL_GPIO_MODE_PUSH_PULL, LED_ON);
    __BKPT(0);  // Halt execution for debugging
  } else {
    // Disable Escape Hatch button digital input when not in use
    sl_gpio_set_pin_mode(&ESCAPE_HATCH, SL_GPIO_MODE_DISABLED, false);
  }
}

/**************************************************************************//**
 * @brief  Configure EM4 energy mode
 *****************************************************************************/
void em4_config(void)
{
  EMU_EM4Init_TypeDef em4Init = EMU_EM4INIT_DEFAULT;
  em4Init.retainLfxo = false;
  em4Init.pinRetentionMode = emuPinRetentionLatch;
  EMU_EM4Init(&em4Init);
}

/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  uint32_t reset_cause;
  uint32_t int_flags;
  EMU_UnlatchPinRetention();
  reset_cause = RMU_ResetCauseGet();
  RMU_ResetCauseClear();

  escape_hatch();

  if ((reset_cause & EMU_RSTCAUSE_EM4)){
    // Initialize GPIO for LEDs
    gpio_init();

    sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_ETAMPDET);
    int_flags = ETAMPDET->IF;

    // Set the appropriate channel flag for LED indicator
    if ((int_flags & _ETAMPDET_IF_TAMPDET0_MASK) == ETAMPDET_IF_TAMPDET0)
      tamperDetectedCh0 = 1;
    else if ((int_flags & _ETAMPDET_IF_TAMPDET1_MASK) == ETAMPDET_IF_TAMPDET1)
      tamperDetectedCh1 = 1;

    // Make sure module is disabled before configuring
    ETAMPDET->EN_CLR = ETAMPDET_EN_EN;

  /*
   *   Must wait for peripheral to disable before modifying other registers; if
   *   register write is attempted before peripheral is disabled, hard fault will
   *   occur.
   */
  #if defined(ETAMPDET_EN_DISABLING)
    while ((ETAMPDET->EN & ETAMPDET_EN_DISABLING) || (ETAMPDET->SYNCBUSY != 0U)) {
      // Wait for disabling to finish
    }
  #else
    while (ETAMPDET->SYNCBUSY != 0U) {
      // Wait for all synchronizations to finish
    }
  #endif

    ETAMPDET->IEN_CLR = _ETAMPDET_IEN_MASK;
    ETAMPDET->IF_CLR = _ETAMPDET_IF_MASK;

    sleep_ok = false;

  } else {
      etampdet_init();
      burtc_init();
      em4_config();

      sleep_ok = true;
      sl_power_manager_enter_em4();
  }
}

/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{
  if (tamperDetectedCh0 == 1) {
    sl_gpio_toggle_pin(&LED0);
  } else {
    if (tamperDetectedCh1 == 1) {
      sl_gpio_toggle_pin(&LED1);
    }
  }

  sl_sleeptimer_delay_millisecond(500);
}

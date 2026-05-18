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
#include "sl_interrupt_manager.h"

#include "sl_gpio.h"
#include "sl_hal_adc.h"
#include "sl_hal_gpio.h"
#include "sl_hal_letimer.h"

#include "pin_config.h"
#include "peripheral_config.h"

// Desired LETIMER frequency in Hz
#define LETIMER_FREQ   2

const sl_gpio_t GPIO_ADC_INPUT0 =  { .port = ADC_INPUT0_PORT,
                                     .pin  = ADC_INPUT0_PIN };
const sl_gpio_t GPIO_ADC_OUTPUT0 = { .port = ADC_OUTPUT0_PORT,
                                     .pin  = ADC_OUTPUT0_PIN };
const sl_gpio_t GPIO_LETIMER0   =  { .port = LETIMER_OUTPUT0_PORT,
                                     .pin  = LETIMER_OUTPUT0_PIN };

static volatile sl_hal_adc_result_t sample;
static volatile float singleResult;

static void ADC0_Handler(void);
static void LETIMER0_Handler(void);

/***************************************************************************//**
 * Initialize GPIO.
 ******************************************************************************/
void gpio_init(void)
{
  // Initialize GPIO driver
  sl_gpio_init();

  /*
   * Configure ADC input as disabled; this is the default configuration out
   * of reset, so API call is not required for this example
   */
  sl_gpio_set_pin_mode(&GPIO_ADC_INPUT0, SL_GPIO_MODE_DISABLED, false);

  // Configure LETIMER0 output to monitor sampling rate
  sl_gpio_set_pin_mode(&GPIO_LETIMER0, SL_GPIO_MODE_PUSH_PULL, false);

  // Configure ADC conversion complete via GPIO toggle in interrupt callback
  sl_gpio_set_pin_mode(&GPIO_ADC_OUTPUT0, SL_GPIO_MODE_PUSH_PULL, false);

  // Enable optional low noise mode for ADC input
  GPIO->P_SET[ADC_INPUT0_PORT].AMUXMODE = 1 << ADC_INPUT0_PIN;

  // Allocate the analog bus for ADC0 inputs
  GPIO->ADC_INPUT0_BUS |= ADC_INPUT0_BUSALLOC;
}

/***************************************************************************//**
 * Initialize ADC.
 ******************************************************************************/
void adc_init(void)
{
  // Declare initialization structures
  uint32_t branch_clock_freq;
  sl_hal_adc_init_t init = SL_HAL_ADC_INIT_DEFAULT;
  sl_hal_adc_config_t initConfig = SL_HAL_ADC_CONFIG_DEFAULT;
  sl_hal_adc_scan_entry_t initScanEntry = SL_HAL_ADC_SCAN_ENTRY_DEFAULT;

  // Enable ADC peripheral bus clock
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_ADC0);

  // Get ADCCLK frequency
  sl_clock_manager_get_clock_branch_frequency(SL_CLOCK_BRANCH_ADCCLK,
                                              &branch_clock_freq);

  // Configure scan table
  initScanEntry.pos_port = ADC_INPUT0_HAL_PORT;
  initScanEntry.pos_pin  = ADC_INPUT0_PIN;

  // Configure gain to adjust full-scale to 3.84 V (from internal reference)
  initConfig.gain = SL_HAL_ADC_ANALOG_GAIN_0_3125;

  // Configure and enable ADC
  init.debug_halt = true;
  init.scan_trigger_action = SL_HAL_ADC_TRIGGER_ACTION_ONCE;
  init.config[SL_HAL_ADC_CONFIG_ID_0] = initConfig;
  init.entries[SL_HAL_ADC_CHANNEL_ID_0] = initScanEntry;

  sl_hal_adc_init(ADC0, &init, branch_clock_freq);
  sl_hal_adc_enable(ADC0);

  // Configure scan channels
  sl_hal_adc_set_scan_mask(ADC0, (1 << SL_HAL_ADC_CHANNEL_ID_0));

  // Set the ADC interrupt handler
  sl_interrupt_manager_set_irq_handler(ADC0_IRQn, ADC0_Handler);

  // Enable ADC interrupt
  sl_hal_adc_enable_interrupts(ADC0, ADC_IF_SCANTABLEDONE);
  sl_interrupt_manager_clear_irq_pending(ADC0_IRQn);
  sl_interrupt_manager_enable_irq(ADC0_IRQn);
}

/***************************************************************************//**
 * @brief LETIMER initialization
 ******************************************************************************/
void letimer_init(void)
{
  // Declare initialization structures
  uint32_t branch_clock_freq;
  sl_hal_letimer_config_t letimerInit = SL_HAL_LETIMER_CONFIG_DEFAULT;

  // Enable LETIMER0 peripheral bus clock
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_LETIMER0);

  // Get ADCCLK frequency
  sl_clock_manager_get_clock_branch_frequency(SL_CLOCK_BRANCH_EM23GRPACLK,
                                              &branch_clock_freq);

  // Calculate the top value (frequency) based on clock source
  uint32_t topValue = branch_clock_freq / LETIMER_FREQ;

  // Reload top on underflow, toggle output, and run in free mode
  letimerInit.enable_top = true;
  letimerInit.underflow_output0_action = \
      SL_HAL_LETIMER_UNDERFLOW_OUTPUT_ACTION_TOGGLE;
  letimerInit.repeat_mode = SL_HAL_LETIMER_REPEAT_MODE_FREE;

  // Enable LETIMER0 output0
  GPIO->LETIMERROUTE_SET->ROUTEEN = GPIO_LETIMER_ROUTEEN_OUT0PEN;
  GPIO->LETIMERROUTE->OUT0ROUTE = \
      (LETIMER_OUTPUT0_PORT << _GPIO_LETIMER_OUT0ROUTE_PORT_SHIFT) \
      | (LETIMER_OUTPUT0_PIN << _GPIO_LETIMER_OUT0ROUTE_PIN_SHIFT);

  // Initialize LETIMER0
  sl_hal_letimer_init(LETIMER0, &letimerInit);

  // Enable LETIMER0; must be enabled to set top value
  sl_hal_letimer_enable(LETIMER0);

  // Set LETIMER0 top value
  sl_hal_letimer_set_top(LETIMER0, topValue);

  // Set the LETIMER interrupt handler
  sl_interrupt_manager_set_irq_handler(LETIMER0_IRQn, LETIMER0_Handler);

  // Enable LETIMER0 underflow interrupt
  sl_hal_letimer_clear_interrupts(LETIMER0, _LETIMER_IF_MASK);
  sl_hal_letimer_enable_interrupts(LETIMER0, LETIMER_IEN_UF);
  sl_interrupt_manager_clear_irq_pending(LETIMER0_IRQn);
  sl_interrupt_manager_enable_irq(LETIMER0_IRQn);
}

/***************************************************************************//**
 * LETIMER interrupt handler.
 ******************************************************************************/
void LETIMER0_Handler(void)
{
  uint32_t flags = sl_hal_letimer_get_pending_interrupts(LETIMER0);

  // Trigger an ADC scan
  sl_hal_adc_start(ADC0);

  // Clear LETIMER0 interrupt flags
  sl_hal_letimer_clear_interrupts(LETIMER0, flags);
  sl_interrupt_manager_clear_irq_pending(LETIMER0_IRQn);
}

/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  // Initialize GPIO, ADC and LETIMER
  gpio_init();
  adc_init();
  letimer_init();

  // Start the LETIMER
  sl_hal_letimer_start(LETIMER0);
}

/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{
}

/***************************************************************************//**
 * ADC interrupt handler.
 ******************************************************************************/
__attribute__((section("text_application_ram")))
void ADC0_Handler(void)
{
  uint32_t rawData;

  /*
   * Set GPIO on during IRQ to signal conversion result complete and start
   * of voltage conversion. Direct register writes are used to optimize
   * execution time.
   */
  GPIO->P_SET[ADC_OUTPUT0_PORT].DOUT = 1UL << ADC_OUTPUT0_PIN;

  /*
   * Clear the scan table done interrupt. This is done early in the interrupt
   * handler so the clear command can propagate while data is being processed.
   * Reading FIFO results does not automatically clear the interrupt flag.
   */
  ADC0->IF_CLR = ADC_IF_SCANTABLEDONE;

  // Pull a scan conversion result from the FIFO
  rawData = ADC0->SCANFIFODATA;
  sample.data = (rawData & (uint32_t)0x0000FFFF) >> 0;
  sample.id = (rawData & (uint32_t)0xFF000000) >> 24;

  /*
   * For single-ended input, the range is 0 V to (+Vref / 0.3125) = 3.84 V with
   * 12 bits for the conversion value.
   */
  singleResult = sample.data * 1.2f / 0.3125f / 0xFFF;

  /*
   * Clear GPIO at end of handler. Direct register writes are used to optimize
   * execution time.
   */
  GPIO->P_CLR[ADC_OUTPUT0_PORT].DOUT = 1UL << ADC_OUTPUT0_PIN;
}


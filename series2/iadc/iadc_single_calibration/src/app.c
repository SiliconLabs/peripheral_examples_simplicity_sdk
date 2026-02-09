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
#include "em_iadc.h"

#include "pin_config.h"

/*******************************************************************************
 *******************************   DEFINES   ***********************************
 ******************************************************************************/

// Define GPIO mapping for boards where LED0 and BUTTON0 share the same pins
#ifndef BUTTON0_PORT
#define BUTTON0_PORT LED0_BUTTON0_PORT
#define BUTTON0_PIN  LED0_BUTTON0_PIN
#endif

// Define GPIO mapping for boards where LED1 and BUTTON1 share the same pins
#ifndef LED1_PORT
#define LED1_PORT LED1_BUTTON1_PORT
#define LED1_PIN  LED1_BUTTON1_PIN
#endif

// Sampling configuration
#define NUM_SAMPLES                 1024
#define CLK_SRC_ADC_FREQ            20000000              // Source clock: 20 MHz
#define CLK_ADC_FREQ                10000000              // ADC clock: 10 MHz max in normal mode

#define IADC_SCALE_OFFSET_MAX_NEG   0x00020000UL          // 18-bit 2's complement max negative offset

#define IADC_GAIN13LSB_LSB          (1.0 / 32768.0)
#define IADC_SCALE_OFFSET_ZERO      0x00000000UL

#define IADC_SIGNED_OFFSET_MAX      ((1 << 17) - 1)
#define IADC_SIGNED_OFFSET_MIN      (-(1 << 17))

const sl_gpio_t GPIO_LED1 = { .port = LED1_PORT, 
                              .pin = LED1_PIN };
const sl_gpio_t GPIO_PB0  = { .port = BUTTON0_PORT, 
                              .pin = BUTTON0_PIN };

/*******************************************************************************
 ***************************   GLOBAL VARIABLES   ******************************
 ******************************************************************************/

static volatile uint32_t sample;
static volatile double singleResult;  // Result in volts

/**************************************************************************//**
 *  Initialize GPIOs for push button and LED control.
 *****************************************************************************/
void gpio_init (void)
{
  // Initialize GPIO driver
  sl_gpio_init();

  // Configure Button PB0 as an input
  sl_gpio_set_pin_mode(&GPIO_PB0, SL_GPIO_MODE_INPUT_PULL, true);

  // Configure LED1 as a push-pull output for LED control
  sl_gpio_set_pin_mode(&GPIO_LED1, SL_GPIO_MODE_PUSH_PULL, LED_OFF);
}

/**************************************************************************//**
 * Initialize IADC with differential input configuration
 *****************************************************************************/
void iadc_init (void)
{
  // Declare init structs
  IADC_Init_t init = IADC_INIT_DEFAULT;
  IADC_AllConfigs_t initAllConfigs = IADC_ALLCONFIGS_DEFAULT;
  IADC_InitSingle_t initSingle = IADC_INITSINGLE_DEFAULT;
  IADC_SingleInput_t initSingleInput = IADC_SINGLEINPUT_DEFAULT;

  // Enable IADC0 clock branch
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_IADC0);

  // Modify init structs and initialize
  init.warmup = iadcWarmupKeepWarm;

  // Set the HFSCLK prescale value
  init.srcClkPrescale = IADC_calcSrcClkPrescale(IADC0, CLK_SRC_ADC_FREQ, 0);

  /* Configuration 0 is used by both scan and single conversions by default
   * Use unbuffered AVDD (supply voltage in mV) as reference
   */
  initAllConfigs.configs[0].reference      = iadcCfgReferenceVddx;
  initAllConfigs.configs[0].vRef           = 3300;

  // Divides CLK_SRC_ADC to set the CLK_ADC frequency
  initAllConfigs.configs[0].adcClkPrescale = IADC_calcAdcClkPrescale(IADC0,
                                             CLK_ADC_FREQ,
                                             0,
                                             iadcCfgModeNormal,
                                             init.srcClkPrescale);

  // Force bipolar output words (result is signed positive or negative)
  initAllConfigs.configs[0].twosComplement = iadcCfgTwosCompBipolar; 

  // Assign pins to positive and negative inputs in differential mode
  initSingleInput.posInput = IADC_INPUT_0_PORT_PIN;
  initSingleInput.negInput = IADC_INPUT_1_PORT_PIN;

  // Initialize IADC and single conversion
  IADC_init(IADC0, &init, &initAllConfigs);
  IADC_initSingle(IADC0, &initSingle, &initSingleInput);

  // Allocate the analog bus for ADC0 inputs
  GPIO->IADC_INPUT_0_BUS |= IADC_INPUT_0_BUSALLOC;
  GPIO->IADC_INPUT_1_BUS |= IADC_INPUT_1_BUSALLOC;
}

void iadc_wait(void)
{
  /* Wait for the single conversion to complete. A conversion is done when the
   * IADC->STATUS register's SINGLEFIFODV and CONVERTING bits are 1 (conversion
   * data is in the output FIFO) and 0 (a conversion is not currently in
   * progress), respectively.
   */
  while ((IADC0->STATUS & (_IADC_STATUS_CONVERTING_MASK \
      | _IADC_STATUS_SINGLEFIFODV_MASK)) != IADC_STATUS_SINGLEFIFODV);
}

/**************************************************************************//**
 * Take several sequential samples and average the measurement
 *****************************************************************************/
double iadc_average_conversion(uint32_t numSamples)
{
  uint32_t i;
  double average = 0;
  IADC_Result_t sample;

  // Indicate to user IADC is busy by turning on LED1
  if(LED_ON)
    sl_gpio_set_pin(&GPIO_LED1);
  else
    sl_gpio_clear_pin(&GPIO_LED1);

  // Averaging loop
  for(i = 0; i < numSamples; i++) 
  {

    // Start IADC conversion
    IADC_command(IADC0, iadcCmdStartSingle);

    iadc_wait();

    // Get ADC result
    sample = IADC_pullSingleFifoResult(IADC0);
    average += (int32_t) sample.data;
  }

  // Turn off LED after IADC is done converting
  if(LED_OFF)
    sl_gpio_set_pin(&GPIO_LED1);
  else
    sl_gpio_clear_pin(&GPIO_LED1);

  return average / numSamples;
}

/**************************************************************************//**
 *  IADC must be disabled to change scale
 *****************************************************************************/
void iadc_rescale(uint32_t newScale)
{
  // Disable the IADC
  IADC0->EN_CLR = IADC_EN_EN;

#if defined(_IADC_EN_DISABLING_MASK)
  while (IADC0->EN & _IADC_EN_DISABLING_MASK);
#endif

  // configure new scale settings
  IADC0->CFG[0].SCALE = newScale;

  // Re-enable IADC
  IADC0->EN_SET = IADC_EN_EN;
}

/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  uint32_t scale, iadcCalibratedGain13Lsb;
  double calibrationGain13Lsb, gainCorrectionFactor, resultFullScale, resultZero, resultOffset;
  int32_t iadcCalibratedOffset;
  bool pinValue;

  gpio_init();
  iadc_init();

  // Set initial offset to maximum negative and initial gain to 1.0
  scale = IADC_SCALE_GAIN3MSB_GAIN100 | IADC_SCALE_GAIN13LSB_DEFAULT | IADC_SCALE_OFFSET_MAX_NEG;
  iadc_rescale(scale);

  /* Apply a full-scale positive input to the IADC
   * Wait until differential voltage is applied (user presses button AND releases)
   */ 
  do 
  { 
    sl_gpio_get_pin_input(&GPIO_PB0, &pinValue); 
  } while (pinValue == 1);
  do {
     sl_gpio_get_pin_input(&GPIO_PB0, &pinValue); 
    } while (pinValue == 0);

  // Take multiple conversions and average to reduce system-level noise
  resultFullScale = iadc_average_conversion(NUM_SAMPLES);

  /* Apply a zero differential input to the IADC
   * Wait until differential voltage is applied (user presses button AND releases)
   */
  do 
  { 
    sl_gpio_get_pin_input(&GPIO_PB0, &pinValue); 
  } while (pinValue == 1);
  do 
  { 
    sl_gpio_get_pin_input(&GPIO_PB0, &pinValue); 
  } while (pinValue == 0);

  resultZero = iadc_average_conversion(NUM_SAMPLES);

  /* Calculate gain correction factor
   * In bipolar mode, expected positive full-scale for IADC is (2^11) - 1 = 2047
   */
  gainCorrectionFactor = 2047 / (resultFullScale - resultZero);

  /* Set IADC gain correction factor and zero out the offset in order to insert
   * the calibrated values.  The 3 most significant bits of the gain are
   * expressed as 1 bit where 1 = 0b100 and represents 1.00x to 1.2499x and
   * 0 = 0b011 and represents 0.75x to 0.9999x.
   */
  if (gainCorrectionFactor >= 1.0) {
    calibrationGain13Lsb = (gainCorrectionFactor - 1.0) / IADC_GAIN13LSB_LSB;
    iadcCalibratedGain13Lsb = (uint32_t)(calibrationGain13Lsb + 0.5);
    scale = IADC_SCALE_GAIN3MSB_GAIN100 |
            (iadcCalibratedGain13Lsb << _IADC_SCALE_GAIN13LSB_SHIFT) |
            IADC_SCALE_OFFSET_ZERO;
  } else {
    calibrationGain13Lsb = (gainCorrectionFactor - 0.75) / IADC_GAIN13LSB_LSB;
    iadcCalibratedGain13Lsb = (uint32_t)(calibrationGain13Lsb + 0.5);
    scale = IADC_SCALE_GAIN3MSB_GAIN011 |
            (iadcCalibratedGain13Lsb << _IADC_SCALE_GAIN13LSB_SHIFT) |
            IADC_SCALE_OFFSET_ZERO;
  }

  iadc_rescale(scale);

  resultOffset = iadc_average_conversion(NUM_SAMPLES);

  /* Scale and negate the offset, which is encoded as a 2's complement,
   * 18-bit number with the LSB representing 1 / (2^20) of full scale.
   */
  iadcCalibratedOffset = (int32_t)(resultOffset * -256);

  /* Determine if the calculated offset is within the allowable offset limits.
   * If the magnitude of the offset is too large to be corrected, set it to
   * permitted minimum or maximum.
   */
  if (iadcCalibratedOffset > IADC_SIGNED_OFFSET_MAX)
      iadcCalibratedOffset = IADC_SIGNED_OFFSET_MAX;
  if (iadcCalibratedOffset < IADC_SIGNED_OFFSET_MIN)
      iadcCalibratedOffset = IADC_SIGNED_OFFSET_MIN;

  // Maintain previous gain correction; apply offset correction
  if (gainCorrectionFactor >= 1.0)
  {
    scale = IADC_SCALE_GAIN3MSB_GAIN100 | (iadcCalibratedGain13Lsb << _IADC_SCALE_GAIN13LSB_SHIFT)
              | (iadcCalibratedOffset & _IADC_SCALE_OFFSET_MASK);
  } else {
    scale = IADC_SCALE_GAIN3MSB_GAIN011 | (iadcCalibratedGain13Lsb << _IADC_SCALE_GAIN13LSB_SHIFT)
              | (iadcCalibratedOffset & _IADC_SCALE_OFFSET_MASK);
  }

  iadc_rescale(scale);
}

/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{
  // Start IADC conversion
  IADC_command(IADC0, iadcCmdStartSingle);

  iadc_wait();

  // Get ADC result
  sample = IADC_pullSingleFifoResult(IADC0).data;

  /* Calculate input voltage:
   * For differential inputs, the resultant range is from -Vref to +Vref, i.e.,
   * for Vref = AVDD = 3.30V, 12 bits represents 6.60V full scale IADC range.
   */
  singleResult = ((double)sample * 6.6) / 0xFFF;
}

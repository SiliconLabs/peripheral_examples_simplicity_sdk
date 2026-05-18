// LED active high
#define LED_ON                     1
#define LED_OFF                    0

// EM4 wake-up enable
#define EM4WUENx                   3

// Clock output select
#define CLKOUT_SEL                 0

// ACMP input
#define ACMP_INPUT_PORT_PIN        _ACMP_INPUTCTRL_POSSEL_PB1

// ACMP input bus allocation
#define ACMP_BUS_ALLOCATION()      (GPIO->BBUSALLOC = GPIO_BBUSALLOC_BODD0_ACMP0)

// RAM power-down end for EM2 and EM3
#define RAM_POWER_DOWN_END         (0)

// GPIO LETIMER route structure
#define GPIO_LETIMERROUTE          (GPIO->LETIMERROUTE)

// No pull-up resistors on the WSTK/WPK
#define I2C_DISABLED_PULLUPS

/*
 * IADC inputs
 *
 * Specify the IADC input using the IADC_PosInput_t typedef.
 * This must be paired with a corresponding macro definition
 * that allocates the corresponding ABUS to the IADC.
 *
 * Example bus allocations:
 * GPIO->ABUSALLOC  |= GPIO_ABUSALLOC_AEVEN0_ADC0
 * GPIO->ABUSALLOC  |= GPIO_ABUSALLOC_AODD0_ADC0
 * GPIO->BBUSALLOC  |= GPIO_BBUSALLOC_BEVEN0_ADC0
 * GPIO->BBUSALLOC  |= GPIO_BBUSALLOC_BODD0_ADC0
 * GPIO->CDBUSALLOC |= GPIO_CDBUSALLOC_CDEVEN0_ADC0
 * GPIO->CDBUSALLOC |= GPIO_CDBUSALLOC_CDODD0_ADC0
 *
 * Applies to port A, port B, and port C/D (even/odd pins).
 */

// IADC input selection
#define IADC_INPUT_0_PORT_PIN      iadcPosInputPortBPin2
#define IADC_INPUT_1_PORT_PIN      iadcNegInputPortBPin3
#define IADC_INPUT_2_PORT_PIN      iadcPosInputPortBPin3

// IADC bus allocation
#define IADC_INPUT_0_BUS           BBUSALLOC
#define IADC_INPUT_0_BUSALLOC      GPIO_BBUSALLOC_BEVEN0_ADC0

#define IADC_INPUT_1_BUS           BBUSALLOC
#define IADC_INPUT_1_BUSALLOC      GPIO_BBUSALLOC_BODD0_ADC0

#define IADC_INPUT_2_BUS           BBUSALLOC
#define IADC_INPUT_2_BUSALLOC      GPIO_BBUSALLOC_BODD0_ADC0

// IADC scan configuration
#define IADC_SCAN_NUM_INPUTS       8
#define IADC_SCANFIFO_DVL          iadcFifoCfgDvl8

// LESENSE IADC inputs
#define LESENSE_IADC_INPUT_0_PORT_PIN      iadcPosInputPortBPin0
#define LESENSE_IADC_INPUT_1_PORT_PIN      iadcPosInputPortBPin1
#define LESENSE_IADC_INPUT_2_PORT_PIN      iadcPosInputPortAPin6
#define LESENSE_IADC_INPUT_3_PORT_PIN      iadcPosInputPortAPin7

#define LESENSE_IADC_INPUT_0_BUS           BBUSALLOC
#define LESENSE_IADC_INPUT_0_BUSALLOC      GPIO_BBUSALLOC_BEVEN0_ADC0

#define LESENSE_IADC_INPUT_1_BUS           BBUSALLOC
#define LESENSE_IADC_INPUT_1_BUSALLOC      GPIO_BBUSALLOC_BODD0_ADC0

#define LESENSE_IADC_INPUT_2_BUS           ABUSALLOC
#define LESENSE_IADC_INPUT_2_BUSALLOC      GPIO_ABUSALLOC_AEVEN0_ADC0

#define LESENSE_IADC_INPUT_3_BUS           ABUSALLOC
#define LESENSE_IADC_INPUT_3_BUSALLOC      GPIO_ABUSALLOC_AODD0_ADC0

// Lock word for the last flash memory page
#define PAGELOCKn                  PAGELOCK7

// Bit mask to lock the last page of main flash
#define LASTLOCK                   0x00008000

// PRS input channels
#define PRS_INPUT_CH_PB0           5
#define PRS_INPUT_CH_PB1           6
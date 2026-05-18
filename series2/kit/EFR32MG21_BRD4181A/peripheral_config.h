// LED active high
#define LED_ON                     1
#define LED_OFF                    0

// EM4 Wake-up enable
#define EM4WUENx                   9

// Clock output select
#define CLKOUT_SEL                 0

// ACMP input
#define ACMP_INPUT_PORT_PIN        _ACMP_INPUTCTRL_POSSEL_PD3

// ACMP input bus allocation
#define ACMP_BUS_ALLOCATION()      (GPIO->CDBUSALLOC = GPIO_CDBUSALLOC_CDODD0_ACMP0)

// GPIO LETIMER route structure
#define GPIO_LETIMERROUTE          (GPIO->LETIMERROUTE[0])

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
 */

// IADC input selection
#define IADC_INPUT_0_PORT_PIN      iadcPosInputPortAPin5
#define IADC_INPUT_1_PORT_PIN      iadcNegInputPortAPin6
#define IADC_INPUT_2_PORT_PIN      iadcPosInputPortBPin0

// IADC bus allocation
#define IADC_INPUT_0_BUS           ABUSALLOC
#define IADC_INPUT_0_BUSALLOC      GPIO_ABUSALLOC_AODD0_ADC0

#define IADC_INPUT_1_BUS           ABUSALLOC
#define IADC_INPUT_1_BUSALLOC      GPIO_ABUSALLOC_AEVEN0_ADC0

#define IADC_INPUT_2_BUS           BBUSALLOC
#define IADC_INPUT_2_BUSALLOC      GPIO_BBUSALLOC_BEVEN0_ADC0

// IADC scan configuration
#define IADC_SCAN_NUM_INPUTS       4
#define IADC_SCANFIFO_DVL          iadcFifoCfgDvl4

// Lock word for the last flash memory page
#define PAGELOCKn                  PAGELOCK3

// Bit mask to lock the last page of main flash
#define LASTLOCK                   0x80000000

// PRS input channels
#define PRS_INPUT_CH_PB0           0
#define PRS_INPUT_CH_PB1           1
// LED active high
#define LED_ON                     1
#define LED_OFF                    0

// ACMP input analog bus allocation
#define ACMP_INPUT_BUS             BBUSALLOC
#define ACMP_INPUT_BUSALLOC        GPIO_BBUSALLOC_BODD0_ACMP0

/*
 * ADC inputs
 *
 * Specify the ADC positive and negative input port/pin.
 * These macros must be paired with a corresponding macro
 * definition that allocates the associated ABUS to the ADC.
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

// ADC input analog bus allocation
#define ADC_INPUT0_BUS             ABUSALLOC
#define ADC_INPUT0_BUSALLOC        GPIO_ABUSALLOC_AEVEN0_ADC0

#define ADC_INPUT1_BUS             ABUSALLOC
#define ADC_INPUT1_BUSALLOC        GPIO_ABUSALLOC_AODD0_ADC0

// ADC input HAL configuration
#define ADC_INPUT0_HAL_PORT        SL_HAL_ADC_PORT_POS_PORTA
#define ADC_INPUT1_HAL_PORT        SL_HAL_ADC_PORT_POS_PORTA

// ADC asynchronous PRS channel (0–5 valid for Port A)
#define ADC_ASYNC_PRS_CH           0
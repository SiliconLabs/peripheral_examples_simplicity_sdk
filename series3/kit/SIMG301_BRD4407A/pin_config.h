// LED active high
#define LED_ON  1
#define LED_OFF 0

// ACMP input Analog Bus allocation
#define ACMP_INPUT_BUS      BBUSALLOC
#define ACMP_INPUT_BUSALLOC GPIO_BBUSALLOC_BODD0_ACMP0

/*
 * Specify the ADC positive input and negative input port and pin. These
 * macros must be paired with a corresponding macro definition that allocates
 * the associated ABUS to the ADC. These are...
 *
 * GPIO->ABUSALLOC |= GPIO_ABUSALLOC_AEVEN0_ADC0
 * GPIO->ABUSALLOC |= GPIO_ABUSALLOC_AODD0_ADC0
 * GPIO->BBUSALLOC |= GPIO_BBUSALLOC_BEVEN0_ADC0
 * GPIO->BBUSALLOC |= GPIO_BBUSALLOC_BODD0_ADC0
 * GPIO->CDBUSALLOC |= GPIO_CDBUSALLOC_CDEVEN0_ADC0
 * GPIO->CDBUSALLOC |= GPIO_CDBUSALLOC_CDODD0_ADC0
 *
 * ...for port A, port B, and port C/D pins, even and odd, respectively.
 */

// ADC Input GPIO configuration
#define ADC_INPUT0_PORT      SL_GPIO_PORT_A
#define ADC_INPUT0_PIN       6

// ADC Input GPIO configuration
#define ADC_INPUT1_PORT      SL_GPIO_PORT_A
#define ADC_INPUT1_PIN       7

// ADC Input Analog Bus allocation
#define ADC_INPUT0_BUS       ABUSALLOC
#define ADC_INPUT0_BUSALLOC  GPIO_ABUSALLOC_AEVEN0_ADC0
#define ADC_INPUT1_BUS       ABUSALLOC
#define ADC_INPUT1_BUSALLOC  GPIO_ABUSALLOC_AODD0_ADC0

// ADC Input HAL driver configuration
#define ADC_INPUT0_HAL_PORT  SL_HAL_ADC_PORT_POS_PORTA
#define ADC_INPUT1_HAL_PORT  SL_HAL_ADC_PORT_POS_PORTA

// ADC Output GPIO configuration
#define ADC_OUTPUT0_PORT     SL_GPIO_PORT_A
#define ADC_OUTPUT0_PIN      0
#define ADC_ASYNC_PRS_CH     0 // PRS channels 0-5 can be used with port A; see data sheet Digital Peripheral Connectivity for details

// EUSART TX and RX configuration
#define EUSART0_TX_PORT SL_GPIO_PORT_B
#define EUSART0_TX_PIN  2
#define EUSART0_RX_PORT SL_GPIO_PORT_B
#define EUSART0_RX_PIN  0

// LETIMER Output GPIO configuration
#define LETIMER_OUTPUT0_PORT SL_GPIO_PORT_A
#define LETIMER_OUTPUT0_PIN  5

// Timer output for slew rate demo
#define SLEW_RATE_OUTPUT_PORT SL_GPIO_PORT_C
#define SLEW_RATE_OUTPUT_PIN  0

// EUSART SPI ports and pins
#define EUS0MOSI_PORT   SL_GPIO_PORT_B
#define EUS0MOSI_PIN    2
#define EUS0MISO_PORT   SL_GPIO_PORT_B
#define EUS0MISO_PIN    0
#define EUS0SCLK_PORT   SL_GPIO_PORT_A
#define EUS0SCLK_PIN    0
#define EUS0CS_PORT     SL_GPIO_PORT_B
#define EUS0CS_PIN      3

#define SPI_TIME_PORT   SL_GPIO_PORT_C
#define SPI_TIME_PIN    3

// <<< sl:start pin_tool >>>

// <gpio> LED0
// $[GPIO_LED0]
#define LED0_PORT SL_GPIO_PORT_D
#define LED0_PIN  3
// [GPIO_LED0]$

// <gpio> LED1
// $[GPIO_LED1]
#define LED1_PORT SL_GPIO_PORT_D
#define LED1_PIN  4
// [GPIO_LED1]$

// <gpio> BUTTON0
// $[GPIO_BUTTON0]
#define BUTTON0_PORT SL_GPIO_PORT_B
#define BUTTON0_PIN  1
// [GPIO_BUTTON0]$

// <gpio> BUTTON1
// $[GPIO_BUTTON1]
#define BUTTON1_PORT SL_GPIO_PORT_D
#define BUTTON1_PIN  2
// [GPIO_BUTTON1]$

// <gpio> EXP_I2C_SCL
// $[GPIO_EXP_I2C_SCL]
#define EXP_I2C_SCL_PORT SL_GPIO_PORT_D
#define EXP_I2C_SCL_PIN  0
// [GPIO_EXP_I2C_SCL]$

// <gpio> EXP_I2C_SDA
// $[GPIO_EXP_I2C_SDA]
#define EXP_I2C_SDA_PORT SL_GPIO_PORT_D
#define EXP_I2C_SDA_PIN  1
// [GPIO_EXP_I2C_SDA]$

// <gpio> EXP_LET0_O0
// $[GPIO_EXP_LET0_O0]
#define EXP_LET0_O0_PORT SL_GPIO_PORT_A
#define EXP_LET0_O0_PIN  6
// [GPIO_EXP_LET0_O0]$

// <gpio> EXP_UART_TX
// $[GPIO_EXP_UART_TX]
#define EXP_UART_TX_PORT SL_GPIO_PORT_B
#define EXP_UART_TX_PIN  2
// [GPIO_EXP_UART_TX]$

// <gpio> EXP_UART_RX
// $[GPIO_EXP_UART_RX]
#define EXP_UART_RX_PORT SL_GPIO_PORT_B
#define EXP_UART_RX_PIN  0
// [GPIO_EXP_UART_RX]$

// <gpio> PIXELRZ_SEQ
// $[GPIO_PIXELRZ_SEQ]
#define PIXELRZ_SEQ_PORT SL_GPIO_PORT_C
#define PIXELRZ_SEQ_PIN  1
// [GPIO_PIXELRZ_SEQ]$

// <gpio> WDOG_PRS
// $[GPIO_WDOG_PRS]
#define WDOG_PRS_PORT SL_GPIO_PORT_C
#define WDOG_PRS_PIN  1
// [GPIO_WDOG_PRS]$

// <<< sl:end pin_tool >>>

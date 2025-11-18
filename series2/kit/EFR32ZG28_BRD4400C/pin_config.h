// LED active high
#define LED_ON  1
#define LED_OFF 0

// EM4 Wake-up enable
#define EM4WUENx 4

// Clock output select
#define CLKOUT_SEL  1

// ACMP Input
#define ACMP_INPUT_PORT_PIN  _ACMP_INPUTCTRL_POSSEL_PB3

// ACMP Input bus allocation
#define ACMP_BUS_ALLOCATION() (GPIO->BBUSALLOC = GPIO_BBUSALLOC_BODD0_ACMP0)

// RAM power down end for EM2 and EM3
#define RAM_POWER_DOWN_END (0)

// EUSART0 Transmit
#define EUSART0_TX_PORT   gpioPortA
#define EUSART0_TX_PIN    8

// EUSART0 Receive
#define EUSART0_RX_PORT   gpioPortA
#define EUSART0_RX_PIN    9

// EUSART SPI ports and pins
#define EUS0MOSI_PORT   SL_GPIO_PORT_A
#define EUS0MOSI_PIN    11
#define EUS0MISO_PORT  	SL_GPIO_PORT_A
#define EUS0MISO_PIN   	12
#define EUS0SCLK_PORT  	SL_GPIO_PORT_A
#define EUS0SCLK_PIN   	13
#define EUS0CS_PORT    	SL_GPIO_PORT_A
#define EUS0CS_PIN     	14

// GPIO LETIMER route structure
#define GPIO_LETIMERROUTE (GPIO->LETIMERROUTE)

// I2C Follower SCL and SDA pins
#define I2C_FOLLOWER_SCL_PORT  gpioPortA
#define I2C_FOLLOWER_SCL_PIN   12
#define I2C_FOLLOWER_SDA_PORT  gpioPortA
#define I2C_FOLLOWER_SDA_PIN   11

// I2C Leader SCL and SDA pins
#define I2C_LEADER_SCL_PORT  gpioPortC
#define I2C_LEADER_SCL_PIN   5
#define I2C_LEADER_SDA_PORT  gpioPortC
#define I2C_LEADER_SDA_PIN   7

// I2C power domain control
#define I2C_DOMAIN_POWER_PORT  gpioPortC
#define I2C_DOMAIN_POWER_PIN   11

// LETIMER output
#define LETIMER_OUTPUT_0_PORT     gpioPortA
#define LETIMER_OUTPUT_0_PIN      11

// GPIO output to indicate IADC conversion complete
#define GPIO_IADC0_EOC_PORT      gpioPortA
#define GPIO_IADC0_EOC_PIN       12

// GPIO output to indicate LDMA transfer complete
#define GPIO_LDMA_COMPLETE_PORT   gpioPortA
#define GPIO_LDMA_COMPLETE_PIN    12

/*
 * IADC inputs.
 *
 * Specify the IADC input using the IADC_PosInput_t typedef.  This
 * must be paired with a corresponding macro definition that allocates
 * the corresponding ABUS to the IADC.  These are...
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
#define IADC_INPUT_0_PORT_PIN     iadcPosInputPortBPin4;
#define IADC_INPUT_1_PORT_PIN     iadcNegInputPortBPin5;
#define IADC_INPUT_2_PORT_PIN     iadcPosInputPortBPin5;

#define IADC_INPUT_0_BUS          BBUSALLOC
#define IADC_INPUT_0_BUSALLOC     GPIO_BBUSALLOC_BEVEN0_ADC0
#define IADC_INPUT_1_BUS          BBUSALLOC
#define IADC_INPUT_1_BUSALLOC     GPIO_BBUSALLOC_BODD0_ADC0
#define IADC_INPUT_2_BUS          BBUSALLOC
#define IADC_INPUT_2_BUSALLOC     GPIO_BBUSALLOC_BODD0_ADC0

// IADC scan channel number and data valid level
#define IADC_SCAN_NUM_INPUTS      8
#define IADC_SCANFIFO_DVL         iadcFifoCfgDvl8

// TIMER external input/output
#define TIMER_EXTIO_PORT     SL_GPIO_PORT_B
#define TIMER_EXTIO_PIN      4

// Lock word for the last flash memory page
#define PAGELOCKn PAGELOCK3

// Bit mask to lock the last page of main flash
#define LASTLOCK  0x80000000
// <<< sl:start pin_tool >>>

// <gpio> LED0
// $[GPIO_LED0]
#define LED0_PORT SL_GPIO_PORT_B
#define LED0_PIN  2
// [GPIO_LED0]$

// <gpio> LED1
// $[GPIO_LED1]
#define LED1_PORT SL_GPIO_PORT_D
#define LED1_PIN  3
// [GPIO_LED1]$

// <gpio> BUTTON0
// $[GPIO_BUTTON0]
#define BUTTON0_PORT SL_GPIO_PORT_B
#define BUTTON0_PIN  1
// [GPIO_BUTTON0]$

// <gpio> BUTTON1
// $[GPIO_BUTTON1]
#define BUTTON1_PORT SL_GPIO_PORT_B
#define BUTTON1_PIN  3
// [GPIO_BUTTON1]$

// <gpio> ACMP_OUTPUT0
// $[ACMP_OUTPUT0]
#define ACMP_OUTPUT0_PORT LED0_PORT
#define ACMP_OUTPUT0_PIN  LED0_PIN
// [ACMP_OUTPUT0]$

// <gpio> ACMP_OUTPUT1
// $[ACMP_OUTPUT1]
#define ACMP_OUTPUT1_PORT LED1_PORT
#define ACMP_OUTPUT1_PIN  LED1_PIN
// [ACMP_OUTPUT1]$

// <gpio> EXP_LET0_O0
// $[GPIO_EXP_LET0_O0]
#define EXP_LET0_O0_PORT SL_GPIO_PORT_A
#define EXP_LET0_O0_PIN  12
// [GPIO_EXP_LET0_O0]$

// <gpio> EXP_UART_TX
// $[GPIO_EXP_UART_TX]
#define EXP_UART_TX_PORT SL_GPIO_PORT_A
#define EXP_UART_TX_PIN  8
// [GPIO_EXP_UART_TX]$

// <gpio> EXP_UART_RX
// $[GPIO_EXP_UART_RX]
#define EXP_UART_RX_PORT SL_GPIO_PORT_A
#define EXP_UART_RX_PIN  9
// [GPIO_EXP_UART_RX]$

// <gpio> WDOG_PRS
// $[GPIO_WDOG_PRS]
#define WDOG_PRS_PORT SL_GPIO_PORT_B
#define WDOG_PRS_PIN  4
// [GPIO_WDOG_PRS]$

// <gpio> EXP_SPI_COPI
// $[GPIO_EXP_SPI_COPI]
#define EXP_SPI_COPI_PORT SL_GPIO_PORT_D
#define EXP_SPI_COPI_PIN  7
// [GPIO_EXP_SPI_COPI]$

// <gpio> EXP_SPI_CIPO
// $[GPIO_EXP_SPI_CIPO]
#define EXP_SPI_CIPO_PORT SL_GPIO_PORT_D
#define EXP_SPI_CIPO_PIN  8
// [GPIO_EXP_SPI_CIPO]$

// <gpio> EXP_SPI_SCK
// $[GPIO_EXP_SPI_SCK]
#define EXP_SPI_SCK_PORT SL_GPIO_PORT_D
#define EXP_SPI_SCK_PIN  9
// [GPIO_EXP_SPI_SCK]$

// <gpio> EXP_SPI_CS
// $[GPIO_EXP_SPI_CS]
#define EXP_SPI_CS_PORT SL_GPIO_PORT_D
#define EXP_SPI_CS_PIN  10
// [GPIO_EXP_SPI_CS]$

// <gpio> SPI_TIME
// $[GPIO_SPI_TIME]
#define SPI_TIME_PORT SL_GPIO_PORT_D
#define SPI_TIME_PIN  11
// [GPIO_EXP_SPI_CS]$

// <gpio> SQUARE_WAVE
// $[GPIO_SQUARE_WAVE]
#define SQUARE_WAVE_PORT SL_GPIO_PORT_D
#define SQUARE_WAVE_PIN  10
// [GPIO_SQUARE_WAVE]$

// <gpio> CLKOUT
// $[GPIO_CLKOUT]
#define CLKOUT_PORT SL_GPIO_PORT_D
#define CLKOUT_PIN  9
// [GPIO_CLKOUT]$

// <<< sl:end pin_tool >>>

// PRS input
#define PRS_INPUT_CH_PB0 5
#define PRS_INPUT_CH_PB1 6

// PRS output
#define PRS_OUTPUT_PORT LED1_PORT
#define PRS_OUTPUT_PIN LED1_PIN

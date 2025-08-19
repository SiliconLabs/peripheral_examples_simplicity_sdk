// LED active high
#define LED_ON  1
#define LED_OFF 0

// EM4 Wake-up enable
#define EM4WUENx 3

// Clock output select
#define CLKOUT_SEL  0

// GPIO LETIMER route structure
#define GPIO_LETIMERROUTE (GPIO->LETIMERROUTE[0])

// Lock word for the last flash memory page
#define PAGELOCKn PAGELOCK1

// Bit mask to lock the last page of main flash
#define LASTLOCK  0x80000000

// <<< sl:start pin_tool >>>

// <gpio> LED0
// $[GPIO_LED0]
#define LED0_PORT SL_GPIO_PORT_D
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
#define BUTTON0_PIN  0
// [GPIO_BUTTON0]$

// <gpio> BUTTON1
// $[GPIO_BUTTON1]
#define BUTTON1_PORT SL_GPIO_PORT_B
#define BUTTON1_PIN  1
// [GPIO_BUTTON1]$

// <gpio> EXP_LET0_O0
// $[GPIO_EXP_LET0_O0]
#define EXP_LET0_O0_PORT SL_GPIO_PORT_A
#define EXP_LET0_O0_PIN  6
// [GPIO_EXP_LET0_O0]$

// <gpio> EXP_UART_TX
// $[GPIO_EXP_UART_TX]
#define EXP_UART_TX_PORT SL_GPIO_PORT_A
#define EXP_UART_TX_PIN  5
// [GPIO_EXP_UART_TX]$

// <gpio> EXP_UART_RX
// $[GPIO_EXP_UART_RX]
#define EXP_UART_RX_PORT SL_GPIO_PORT_A
#define EXP_UART_RX_PIN  6
// [GPIO_EXP_UART_RX]$

// <gpio> WDOG_PRS
// $[GPIO_WDOG_PRS]
#define WDOG_PRS_PORT SL_GPIO_PORT_A
#define WDOG_PRS_PIN  5
// [GPIO_WDOG_PRS]$

// <gpio> EXP_SPI_COPI
// $[GPIO_EXP_SPI_COPI]
#define EXP_SPI_COPI_PORT SL_GPIO_PORT_C
#define EXP_SPI_COPI_PIN  0
// [GPIO_EXP_SPI_COPI]$

// <gpio> EXP_SPI_CIPO
// $[GPIO_EXP_SPI_CIPO]
#define EXP_SPI_CIPO_PORT SL_GPIO_PORT_C
#define EXP_SPI_CIPO_PIN  1
// [GPIO_EXP_SPI_CIPO]$

// <gpio> EXP_SPI_SCK
// $[GPIO_EXP_SPI_SCK]
#define EXP_SPI_SCK_PORT SL_GPIO_PORT_C
#define EXP_SPI_SCK_PIN  2
// [GPIO_EXP_SPI_SCK]$

// <gpio> EXP_SPI_CS
// $[GPIO_EXP_SPI_CS]
#define EXP_SPI_CS_PORT SL_GPIO_PORT_C
#define EXP_SPI_CS_PIN  3
// [GPIO_EXP_SPI_CS]$

// <gpio> SPI_TIME
// $[GPIO_SPI_TIME]
#define SPI_TIME_PORT SL_GPIO_PORT_D
#define SPI_TIME_PIN  2
// [GPIO_EXP_SPI_CS]$

// <gpio> SQUARE_WAVE
// $[GPIO_SQUARE_WAVE]
#define SQUARE_WAVE_PORT SL_GPIO_PORT_C
#define SQUARE_WAVE_PIN  0
// [GPIO_SQUARE_WAVE]$

// <gpio> CLKOUT
// $[GPIO_CLKOUT]
#define CLKOUT_PORT SL_GPIO_PORT_C
#define CLKOUT_PIN  3
// [GPIO_CLKOUT]$

// <<< sl:end pin_tool >>>

// PRS input
#define PRS_INPUT_CH_PB0 5
#define PRS_INPUT_CH_PB1 6

// PRS output
#define PRS_OUTPUT_PORT LED1_PORT
#define PRS_OUTPUT_PIN LED1_PIN

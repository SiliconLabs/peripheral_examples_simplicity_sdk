// LED active high
#define LED_ON  1
#define LED_OFF 0

// EM4 Wake-up enable
#define EM4WUENx 3
 
// Clock output select
#define CLKOUT_SEL  0

// <<< sl:start pin_tool >>>

// <gpio> LED0
// $[GPIO_LED0]
#define LED0_PORT SL_GPIO_PORT_C
#define LED0_PIN  6
// [GPIO_LED0]$

// <gpio> LED1
// $[GPIO_LED1]
#define LED1_PORT SL_GPIO_PORT_C
#define LED1_PIN  7
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
#define WDOG_PRS_PORT SL_GPIO_PORT_A
#define WDOG_PRS_PIN  5
// [GPIO_WDOG_PRS]$

// <gpio> SQUARE_WAVE
// $[GPIO_SQUARE_WAVE]
#define SQUARE_WAVE_PORT SL_GPIO_PORT_C
#define SQUARE_WAVE_PIN  0
// [GPIO_SQUARE_WAVE]$

// <gpio> CLKOUT
// $[GPIO_CLKOUT]
#define CLKOUT_PORT SL_GPIO_PORT_C
#define CLKOUT_PIN  2
// [GPIO_CLKOUT]$

// <<< sl:end pin_tool >>>

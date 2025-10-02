// LED active low
#define LED_ON  0
#define LED_OFF 1

/* RAM power down end for EM2 and EM3
 * NOTE: The top 8 KB of RAM (BLK1) **MUST** remain powered in EM2/3
 * (SYSCFG_DMEM0RETNCTRL_RAMRETNCTRL != 2) or the device is liable to
 * hard fault on wake-up depending on what data may have been saved
 * on the stack, e.g. the return address from the EM3
 */
#define RAM_POWER_DOWN_END (SRAM_BASE + SRAM_SIZE - 0x2000)

// <<< sl:start pin_tool >>>

// <gpio> LED0_BUTTON0
// $[GPIO_LED0_BUTTON0]
#define LED0_BUTTON0_PORT SL_GPIO_PORT_C
#define LED0_BUTTON0_PIN  5
// [GPIO_LED0_BUTTON0]$

// <gpio> LED1_BUTTON1
// $[GPIO_LED1_BUTTON1]
#define LED1_BUTTON1_PORT SL_GPIO_PORT_C
#define LED1_BUTTON1_PIN  4
// [GPIO_LED1_BUTTON1]$

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

// <<< sl:end pin_tool >>>

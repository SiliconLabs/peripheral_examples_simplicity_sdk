// <<< sl:start pin_tool >>>

// =========================================================
// LEDs & BUTTONS
// =========================================================

// <gpio> LED0
// $[GPIO_LED0]
#define LED0_PORT                   SL_GPIO_PORT_D
#define LED0_PIN                    3
// [GPIO_LED0]$

// <gpio> LED1
// $[GPIO_LED1]
#define LED1_PORT                   SL_GPIO_PORT_D
#define LED1_PIN                    4
// [GPIO_LED1]$

// <gpio> BUTTON0
// $[GPIO_BUTTON0]
#define BUTTON0_PORT                SL_GPIO_PORT_B
#define BUTTON0_PIN                 1
// [GPIO_BUTTON0]$

// <gpio> BUTTON1
// $[GPIO_BUTTON1]
#define BUTTON1_PORT                SL_GPIO_PORT_D
#define BUTTON1_PIN                 2
// [GPIO_BUTTON1]$


// =========================================================
// EXP HEADER
// =========================================================

// <gpio> EXP_UART_TX
// $[GPIO_EXP_UART_TX]
#define EXP_UART_TX_PORT            SL_GPIO_PORT_B
#define EXP_UART_TX_PIN             2
// [GPIO_EXP_UART_TX]$

// <gpio> EXP_UART_RX
// $[GPIO_EXP_UART_RX]
#define EXP_UART_RX_PORT            SL_GPIO_PORT_B
#define EXP_UART_RX_PIN             0
// [GPIO_EXP_UART_RX]$


// =========================================================
// ADC GPIOs
// =========================================================

// <gpio> ADC_INPUT0
// $[GPIO_ADC_INPUT0]
#define ADC_INPUT0_PORT             SL_GPIO_PORT_A
#define ADC_INPUT0_PIN              6
// [GPIO_ADC_INPUT0]$

// <gpio> ADC_INPUT1
// $[GPIO_ADC_INPUT1]
#define ADC_INPUT1_PORT             SL_GPIO_PORT_A
#define ADC_INPUT1_PIN              7
// [GPIO_ADC_INPUT1]$


// =========================================================
// SLEW / OUTPUT
// =========================================================

// <gpio> SLEW_RATE_OUTPUT
// $[GPIO_SLEW_RATE_OUTPUT]
#define SLEW_RATE_OUTPUT_PORT       SL_GPIO_PORT_C
#define SLEW_RATE_OUTPUT_PIN        0
// [GPIO_SLEW_RATE_OUTPUT]$


// =========================================================
// SPI GPIOs
// =========================================================

// <gpio> EUS0SCLK
// $[GPIO_EUS0SCLK]
#define EUS0SCLK_PORT               SL_GPIO_PORT_A
#define EUS0SCLK_PIN                0
// [GPIO_EUS0SCLK]$

// <gpio> EUS0CS
// $[GPIO_EUS0CS]
#define EUS0CS_PORT                 SL_GPIO_PORT_B
#define EUS0CS_PIN                  3
// [GPIO_EUS0CS]$

// <gpio> SPI_TIME
// $[GPIO_SPI_TIME]
#define SPI_TIME_PORT               SL_GPIO_PORT_C
#define SPI_TIME_PIN                3
// [GPIO_SPI_TIME]$


// =========================================================
// MISC
// =========================================================

// <gpio> EXP_I2C_SCL
// $[GPIO_EXP_I2C_SCL]
#define EXP_I2C_SCL_PORT            SL_GPIO_PORT_D
#define EXP_I2C_SCL_PIN             0
// [GPIO_EXP_I2C_SCL]$

// <gpio> EXP_I2C_SDA
// $[GPIO_EXP_I2C_SDA]
#define EXP_I2C_SDA_PORT            SL_GPIO_PORT_D
#define EXP_I2C_SDA_PIN             1
// [GPIO_EXP_I2C_SDA]$

// <gpio> PIXELRZ_SEQ
// $[GPIO_PIXELRZ_SEQ]
#define PIXELRZ_SEQ_PORT            SL_GPIO_PORT_C
#define PIXELRZ_SEQ_PIN             1
// [GPIO_PIXELRZ_SEQ]$

// <gpio> LETIMER_OUTPUT0
// $[GPIO_LETIMER_OUTPUT0]
#define LETIMER_OUTPUT0_PORT        SL_GPIO_PORT_A
#define LETIMER_OUTPUT0_PIN         5
// [GPIO_LETIMER_OUTPUT0]$


// <<< sl:end pin_tool >>>


// NOTE:
// Each physical GPIO is defined exactly once in pin_tool.
// All other signals must reuse these definitions.
// Multiple aliases may map to the same physical GPIO pin.
// For a given GPIO only one alias/function is used at runtime


// =========================================================
// EUSART (UART)
// =========================================================

#define EUSART0_TX_PORT             EXP_UART_TX_PORT
#define EUSART0_TX_PIN              EXP_UART_TX_PIN

#define EUSART0_RX_PORT             EXP_UART_RX_PORT
#define EUSART0_RX_PIN              EXP_UART_RX_PIN


// =========================================================
// EUSART (SPI)
// =========================================================

#define EUS0MOSI_PORT               EXP_UART_TX_PORT
#define EUS0MOSI_PIN                EXP_UART_TX_PIN

#define EUS0MISO_PORT               EXP_UART_RX_PORT
#define EUS0MISO_PIN                EXP_UART_RX_PIN


// =========================================================
// TIMER / GPIO SIGNALS
// =========================================================

#define EXP_LET0_O0_PORT            ADC_INPUT0_PORT
#define EXP_LET0_O0_PIN             ADC_INPUT0_PIN

#define EXP_TIMER_CC0_PORT          ADC_INPUT0_PORT
#define EXP_TIMER_CC0_PIN           ADC_INPUT0_PIN

#define ADC_OUTPUT0_PORT            EUS0SCLK_PORT
#define ADC_OUTPUT0_PIN             EUS0SCLK_PIN

#define WDOG_PRS_PORT               PIXELRZ_SEQ_PORT
#define WDOG_PRS_PIN                PIXELRZ_SEQ_PIN
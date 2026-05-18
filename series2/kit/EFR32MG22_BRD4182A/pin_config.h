// <<< sl:start pin_tool >>>

// =========================================================
// LEDs & BUTTONs
// =========================================================

// <gpio> LED0
// $[GPIO_LED0]
#define LED0_PORT                   SL_GPIO_PORT_D
#define LED0_PIN                    2
// [GPIO_LED0]$

// <gpio> LED1
// $[GPIO_LED1]
#define LED1_PORT                   SL_GPIO_PORT_D
#define LED1_PIN                    3
// [GPIO_LED1]$

// <gpio> BUTTON0
// $[GPIO_BUTTON0]
#define BUTTON0_PORT                SL_GPIO_PORT_B
#define BUTTON0_PIN                 0
// [GPIO_BUTTON0]$

// <gpio> BUTTON1
// $[GPIO_BUTTON1]
#define BUTTON1_PORT                SL_GPIO_PORT_B
#define BUTTON1_PIN                 1
// [GPIO_BUTTON1]$

// =========================================================
// EXP HEADER
// =========================================================

// <gpio> EXP_UART_TX
// $[GPIO_EXP_UART_TX]
#define EXP_UART_TX_PORT            SL_GPIO_PORT_A
#define EXP_UART_TX_PIN             5
// [GPIO_EXP_UART_TX]$

// <gpio> EXP_UART_RX
// $[GPIO_EXP_UART_RX]
#define EXP_UART_RX_PORT            SL_GPIO_PORT_A
#define EXP_UART_RX_PIN             6
// [GPIO_EXP_UART_RX]$

// <gpio> EXP_SPI_COPI
// $[GPIO_EXP_SPI_COPI]
#define EXP_SPI_COPI_PORT           SL_GPIO_PORT_C
#define EXP_SPI_COPI_PIN            0
// [GPIO_EXP_SPI_COPI]$

// <gpio> EXP_SPI_CIPO
// $[GPIO_EXP_SPI_CIPO]
#define EXP_SPI_CIPO_PORT           SL_GPIO_PORT_C
#define EXP_SPI_CIPO_PIN            1
// [GPIO_EXP_SPI_CIPO]$

// <gpio> EXP_SPI_SCK
// $[GPIO_EXP_SPI_SCK]
#define EXP_SPI_SCK_PORT            SL_GPIO_PORT_C
#define EXP_SPI_SCK_PIN             2
// [GPIO_EXP_SPI_SCK]$

// <gpio> EXP_SPI_CS
// $[GPIO_EXP_SPI_CS]
#define EXP_SPI_CS_PORT             SL_GPIO_PORT_C
#define EXP_SPI_CS_PIN              3
// [GPIO_EXP_SPI_CS]$

// <gpio> I2C_LEADER_SCL
// $[GPIO_I2C_LEADER_SCL]
#define I2C_LEADER_SCL_PORT         SL_GPIO_PORT_B
#define I2C_LEADER_SCL_PIN          2
// [GPIO_I2C_LEADER_SCL]$

// <gpio> I2C_LEADER_SDA
// $[GPIO_I2C_LEADER_SDA]
#define I2C_LEADER_SDA_PORT         SL_GPIO_PORT_B
#define I2C_LEADER_SDA_PIN          3
// [GPIO_I2C_LEADER_SDA]$

// <gpio> I2C_DOMAIN_POWER
// $[GPIO_I2C_DOMAIN_POWER]
#define I2C_DOMAIN_POWER_PORT       SL_GPIO_PORT_C
#define I2C_DOMAIN_POWER_PIN        7
// [GPIO_I2C_DOMAIN_POWER]$

// <<< sl:end pin_tool >>>

// NOTE:
// Each physical GPIO is defined exactly once in pin_tool.
// All other signals reuse these definitions.
// Multiple aliases may map to the same GPIO.
// Only one function uses a GPIO at runtime.


// =========================================================
// EUART
// =========================================================

#define EUART0_TX_PORT              EXP_UART_TX_PORT
#define EUART0_TX_PIN               EXP_UART_TX_PIN

#define EUART0_RX_PORT              EXP_UART_RX_PORT
#define EUART0_RX_PIN               EXP_UART_RX_PIN


// =========================================================
// I2C CONFIGURATION
// =========================================================

#define I2C_FOLLOWER_SDA_PORT       EXP_UART_TX_PORT
#define I2C_FOLLOWER_SDA_PIN        EXP_UART_TX_PIN

#define I2C_FOLLOWER_SCL_PORT       EXP_UART_RX_PORT
#define I2C_FOLLOWER_SCL_PIN        EXP_UART_RX_PIN


// =========================================================
// TIMER / GPIO SIGNALS
// =========================================================

#define LETIMER_OUTPUT_0_PORT       BUTTON1_PORT
#define LETIMER_OUTPUT_0_PIN        BUTTON1_PIN

#define TIMER_EXTIO_PORT            EXP_UART_RX_PORT
#define TIMER_EXTIO_PIN             EXP_UART_RX_PIN

#define EXP_LET0_O0_PORT            EXP_UART_RX_PORT
#define EXP_LET0_O0_PIN             EXP_UART_RX_PIN

#define GPIO_IADC0_EOC_PORT         EXP_UART_TX_PORT
#define GPIO_IADC0_EOC_PIN          EXP_UART_TX_PIN

#define GPIO_LDMA_COMPLETE_PORT     EXP_UART_TX_PORT
#define GPIO_LDMA_COMPLETE_PIN      EXP_UART_TX_PIN


// =========================================================
// WATCHDOG
// =========================================================

#define WDOG_PRS_PORT               EXP_UART_TX_PORT
#define WDOG_PRS_PIN                EXP_UART_TX_PIN


// =========================================================
// GENERIC SIGNALS / OUTPUTS
// =========================================================

#define SPI_TIME_PORT               LED0_PORT
#define SPI_TIME_PIN                LED0_PIN

#define SQUARE_WAVE_PORT            EXP_SPI_COPI_PORT
#define SQUARE_WAVE_PIN             EXP_SPI_COPI_PIN

#define CLKOUT_PORT                 EXP_SPI_CS_PORT
#define CLKOUT_PIN                  EXP_SPI_CS_PIN

// =========================================================
// PRS
// =========================================================

#define PRS_OUTPUT_PORT             LED1_PORT
#define PRS_OUTPUT_PIN              LED1_PIN
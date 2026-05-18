// <<< sl:start pin_tool >>>

// =========================================================
// LEDs / BUTTONs (shared)
// =========================================================

// <gpio> LED0_BUTTON0
// $[GPIO_LED0_BUTTON0]
#define LED0_BUTTON0_PORT           SL_GPIO_PORT_B
#define LED0_BUTTON0_PIN            0
// [GPIO_LED0_BUTTON0]$

// <gpio> LED1_BUTTON1
// $[GPIO_LED1_BUTTON1]
#define LED1_BUTTON1_PORT           SL_GPIO_PORT_B
#define LED1_BUTTON1_PIN            1
// [GPIO_LED1_BUTTON1]$


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


// =========================================================
// SPI GPIOs
// =========================================================

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


// =========================================================
// I2C / POWER
// =========================================================

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


// =========================================================
// MISC GPIOs
// =========================================================

// <gpio> LETIMER_OUTPUT_0
// $[GPIO_LETIMER_OUTPUT_0]
#define LETIMER_OUTPUT_0_PORT       SL_GPIO_PORT_A
#define LETIMER_OUTPUT_0_PIN        8
// [GPIO_LETIMER_OUTPUT_0]$


// <<< sl:end pin_tool >>>


// NOTE:
// Each physical GPIO is defined exactly once in pin_tool.
// All other signals reuse these definitions.
// Multiple aliases may map to the same GPIO.
// Only one function uses a GPIO at runtime.


// =========================================================
// EUSART (UART mode)
// =========================================================

#define EUSART0_TX_PORT             EXP_UART_TX_PORT
#define EUSART0_TX_PIN              EXP_UART_TX_PIN

#define EUSART0_RX_PORT             EXP_UART_RX_PORT
#define EUSART0_RX_PIN              EXP_UART_RX_PIN


// =========================================================
// EUSART (SPI mode)
// =========================================================

#define EUS0MOSI_PORT               EXP_SPI_COPI_PORT
#define EUS0MOSI_PIN                EXP_SPI_COPI_PIN

#define EUS0MISO_PORT               EXP_SPI_CIPO_PORT
#define EUS0MISO_PIN                EXP_SPI_CIPO_PIN

#define EUS0SCLK_PORT               EXP_SPI_SCK_PORT
#define EUS0SCLK_PIN                EXP_SPI_SCK_PIN

#define EUS0CS_PORT                 EXP_SPI_CS_PORT
#define EUS0CS_PIN                  EXP_SPI_CS_PIN


// =========================================================
// I2C CONFIGURATION
// =========================================================

#define I2C_FOLLOWER_SDA_PORT       EXP_UART_TX_PORT
#define I2C_FOLLOWER_SDA_PIN        EXP_UART_TX_PIN

#define I2C_FOLLOWER_SCL_PORT       EXP_UART_RX_PORT
#define I2C_FOLLOWER_SCL_PIN        EXP_UART_RX_PIN


// =========================================================
// GPIO SIGNALS / DEBUG
// =========================================================

#define ETAMPDET_IN0_PORT           LED1_BUTTON1_PORT
#define ETAMPDET_IN0_PIN            LED1_BUTTON1_PIN

#define ETAMPDET_OUT0_PORT          EXP_SPI_CIPO_PORT
#define ETAMPDET_OUT0_PIN           EXP_SPI_CIPO_PIN

#define ETAMPDET_IN1_PORT           EXP_SPI_COPI_PORT
#define ETAMPDET_IN1_PIN            EXP_SPI_COPI_PIN

#define ETAMPDET_OUT1_PORT          EXP_SPI_SCK_PORT
#define ETAMPDET_OUT1_PIN           EXP_SPI_SCK_PIN

#define GPIO_IADC0_EOC_PORT         EXP_UART_TX_PORT
#define GPIO_IADC0_EOC_PIN          EXP_UART_TX_PIN

#define GPIO_LDMA_COMPLETE_PORT     EXP_UART_TX_PORT
#define GPIO_LDMA_COMPLETE_PIN      EXP_UART_TX_PIN

#define WDOG_PRS_PORT               EXP_UART_TX_PORT
#define WDOG_PRS_PIN                EXP_UART_TX_PIN

#define EXP_LET0_O0_PORT            EXP_UART_RX_PORT
#define EXP_LET0_O0_PIN             EXP_UART_RX_PIN

#define SQUARE_WAVE_PORT            EXP_SPI_CS_PORT
#define SQUARE_WAVE_PIN             EXP_SPI_CS_PIN

#define CLKOUT_PORT                 EXP_SPI_CS_PORT
#define CLKOUT_PIN                  EXP_SPI_CS_PIN

#define TIMER_EXTIO_PORT            EXP_UART_RX_PORT
#define TIMER_EXTIO_PIN             EXP_UART_RX_PIN

#define SPI_TIME_PORT               LETIMER_OUTPUT_0_PORT
#define SPI_TIME_PIN                LETIMER_OUTPUT_0_PIN


// =========================================================
// ANALOG COMPARATOR (ACMP)
// =========================================================

#define ACMP_OUTPUT0_PORT           LED0_BUTTON0_PORT
#define ACMP_OUTPUT0_PIN            LED0_BUTTON0_PIN

#define ACMP_OUTPUT1_PORT           LED1_BUTTON1_PORT
#define ACMP_OUTPUT1_PIN            LED1_BUTTON1_PIN


// =========================================================
// PRS
// =========================================================

#define PRS_OUTPUT_PORT             EXP_SPI_CS_PORT
#define PRS_OUTPUT_PIN              EXP_SPI_CS_PIN
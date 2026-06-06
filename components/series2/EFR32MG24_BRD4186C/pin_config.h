// <<< sl:start pin_tool >>>

// =========================================================
// LEDs & BUTTONs
// =========================================================

// <gpio> LED0
// $[GPIO_LED0]
#define LED0_PORT                   SL_GPIO_PORT_B
#define LED0_PIN                    2
// [GPIO_LED0]$

// <gpio> LED1
// $[GPIO_LED1]
#define LED1_PORT                   SL_GPIO_PORT_B
#define LED1_PIN                    4
// [GPIO_LED1]$

// <gpio> BUTTON0
// $[GPIO_BUTTON0]
#define BUTTON0_PORT                SL_GPIO_PORT_B
#define BUTTON0_PIN                 1
// [GPIO_BUTTON0]$

// <gpio> BUTTON1
// $[GPIO_BUTTON1]
#define BUTTON1_PORT                SL_GPIO_PORT_B
#define BUTTON1_PIN                 3
// [GPIO_BUTTON1]$


// =========================================================
// EXP HEADER
// =========================================================

// <gpio> WDOG_PRS
// $[GPIO_WDOG_PRS]
#define WDOG_PRS_PORT               SL_GPIO_PORT_A
#define WDOG_PRS_PIN                5
// [GPIO_WDOG_PRS]$

// <gpio> TIMER_EXTIO
// $[GPIO_TIMER_EXTIO]
#define TIMER_EXTIO_PORT            SL_GPIO_PORT_A
#define TIMER_EXTIO_PIN             6
// [GPIO_TIMER_EXTIO]$

// <gpio> LETIMER_OUTPUT_0
// $[GPIO_LETIMER_OUTPUT_0]
#define LETIMER_OUTPUT_0_PORT       SL_GPIO_PORT_A
#define LETIMER_OUTPUT_0_PIN        0
// [GPIO_LETIMER_OUTPUT_0]$

// <gpio> EXP_UART_TX
// $[GPIO_EXP_UART_TX]
#define EXP_UART_TX_PORT            SL_GPIO_PORT_A
#define EXP_UART_TX_PIN             8
// [GPIO_EXP_UART_TX]$

// <gpio> EXP_UART_RX
// $[GPIO_EXP_UART_RX]
#define EXP_UART_RX_PORT            SL_GPIO_PORT_A
#define EXP_UART_RX_PIN             9
// [GPIO_EXP_UART_RX]$

// <gpio> EXP_SPI_CS
// $[GPIO_EXP_SPI_CS]
#define EXP_SPI_CS_PORT             SL_GPIO_PORT_C
#define EXP_SPI_CS_PIN              0
// [GPIO_EXP_SPI_CS]$

// <gpio> EXP_SPI_COPI
// $[GPIO_EXP_SPI_COPI]
#define EXP_SPI_COPI_PORT           SL_GPIO_PORT_C
#define EXP_SPI_COPI_PIN            1
// [GPIO_EXP_SPI_COPI]$

// <gpio> EXP_SPI_CIPO
// $[GPIO_EXP_SPI_CIPO]
#define EXP_SPI_CIPO_PORT           SL_GPIO_PORT_C
#define EXP_SPI_CIPO_PIN            2
// [GPIO_EXP_SPI_CIPO]$

// <gpio> EXP_SPI_SCK
// $[GPIO_EXP_SPI_SCK]
#define EXP_SPI_SCK_PORT            SL_GPIO_PORT_C
#define EXP_SPI_SCK_PIN             3
// [GPIO_EXP_SPI_SCK]$

// <gpio> I2C_LEADER_SCL
// $[GPIO_I2C_LEADER_SCL]
#define I2C_LEADER_SCL_PORT         SL_GPIO_PORT_C
#define I2C_LEADER_SCL_PIN          5
// [GPIO_I2C_LEADER_SCL]$

// <gpio> I2C_LEADER_SDA
// $[GPIO_I2C_LEADER_SDA]
#define I2C_LEADER_SDA_PORT         SL_GPIO_PORT_C
#define I2C_LEADER_SDA_PIN          7
// [GPIO_I2C_LEADER_SDA]$

// <gpio> SPI_TIME
// $[GPIO_SPI_TIME]
#define SPI_TIME_PORT               SL_GPIO_PORT_D
#define SPI_TIME_PIN                2
// [GPIO_SPI_TIME]$

// <gpio> I2C_DOMAIN_POWER
// $[GPIO_I2C_DOMAIN_POWER]
#define I2C_DOMAIN_POWER_PORT       SL_GPIO_PORT_D
#define I2C_DOMAIN_POWER_PIN        3
// [GPIO_I2C_DOMAIN_POWER]$

// <gpio> VDAC0_CH0_MAINOUT
// $[GPIO_VDAC0_CH0_MAINOUT]
#define VDAC0_CH0_MAINOUT_PORT      SL_GPIO_PORT_B
#define VDAC0_CH0_MAINOUT_PIN       0
// [GPIO_VDAC0_CH0_MAINOUT]$


// <<< sl:end pin_tool >>>

// NOTE:
// Each physical GPIO is defined exactly once in pin_tool.
// All other signals reuse these definitions.
// Multiple aliases may map to the same GPIO.
// Only one function uses a GPIO at runtime.


// =========================================================
// EUSART0 (UART mode)
// =========================================================

#define EUSART0_TX_PORT             EXP_UART_TX_PORT
#define EUSART0_TX_PIN              EXP_UART_TX_PIN

#define EUSART0_RX_PORT             EXP_UART_RX_PORT
#define EUSART0_RX_PIN              EXP_UART_RX_PIN


// =========================================================
// EUSART0 (SPI mode)
// =========================================================

#define EUS0MOSI_PORT               EXP_UART_TX_PORT
#define EUS0MOSI_PIN                EXP_UART_TX_PIN

#define EUS0MISO_PORT               EXP_UART_RX_PORT
#define EUS0MISO_PIN                EXP_UART_RX_PIN

#define EUS0SCLK_PORT               TIMER_EXTIO_PORT
#define EUS0SCLK_PIN                TIMER_EXTIO_PIN

#define EUS0CS_PORT                 EXP_LET0_O0_PORT
#define EUS0CS_PIN                  EXP_LET0_O0_PIN


// =========================================================
// TIMER SIGNALS
// =========================================================

#define EXP_LET0_O0_PORT            LETIMER_OUTPUT_0_PORT
#define EXP_LET0_O0_PIN             LETIMER_OUTPUT_0_PIN

// =========================================================
// I2C FOLLOWER
// =========================================================

#define I2C_FOLLOWER_SDA_PORT       WDOG_PRS_PORT
#define I2C_FOLLOWER_SDA_PIN        WDOG_PRS_PIN

#define I2C_FOLLOWER_SCL_PORT       TIMER_EXTIO_PORT
#define I2C_FOLLOWER_SCL_PIN        TIMER_EXTIO_PIN


// =========================================================
// ADC / LDMA / Debug Signals
// =========================================================

#define GPIO_IADC0_EOC_PORT         EXP_UART_TX_PORT
#define GPIO_IADC0_EOC_PIN          EXP_UART_TX_PIN

#define GPIO_LDMA_COMPLETE_PORT     EXP_UART_TX_PORT
#define GPIO_LDMA_COMPLETE_PIN      EXP_UART_TX_PIN


// =========================================================
// ANALOG COMPARATOR
// =========================================================

#define ACMP_OUTPUT0_PORT           LED0_PORT
#define ACMP_OUTPUT0_PIN            LED0_PIN

#define ACMP_OUTPUT1_PORT           LED1_PORT
#define ACMP_OUTPUT1_PIN            LED1_PIN


// =========================================================
// PRS
// =========================================================

#define PRS_OUTPUT_PORT             LED1_PORT
#define PRS_OUTPUT_PIN              LED1_PIN


// =========================================================
// GENERIC SIGNALS / OUTPUTS
// =========================================================

#define SQUARE_WAVE_PORT            EXP_SPI_CS_PORT
#define SQUARE_WAVE_PIN             EXP_SPI_CS_PIN

#define CLKOUT_PORT                 EXP_SPI_SCK_PORT
#define CLKOUT_PIN                  EXP_SPI_SCK_PIN
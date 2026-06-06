// <<< sl:start pin_tool >>>

// =========================================================
// LEDs & BUTTONs
// =========================================================

// <gpio> LED0
// $[GPIO_LED0]
#define LED0_PORT                   SL_GPIO_PORT_C
#define LED0_PIN                    6
// [GPIO_LED0]$

// <gpio> LED1
// $[GPIO_LED1]
#define LED1_PORT                   SL_GPIO_PORT_C
#define LED1_PIN                    7
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
// UART
// =========================================================

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

// =========================================================
// ETAMPDET
// =========================================================

// <gpio> ETAMPDET_IN0
// $[GPIO_ETAMPDET_IN0]
#define ETAMPDET_IN0_PORT           SL_GPIO_PORT_A
#define ETAMPDET_IN0_PIN            5
// [GPIO_ETAMPDET_IN0]$

// <gpio> ETAMPDET_OUT0
// $[GPIO_ETAMPDET_OUT0]
#define ETAMPDET_OUT0_PORT          SL_GPIO_PORT_A
#define ETAMPDET_OUT0_PIN           6
// [GPIO_ETAMPDET_OUT0]$

// <gpio> ETAMPDET_IN1
// $[GPIO_ETAMPDET_IN1]
#define ETAMPDET_IN1_PORT           SL_GPIO_PORT_D
#define ETAMPDET_IN1_PIN            5
// [GPIO_ETAMPDET_IN1]$

// <gpio> ETAMPDET_OUT1
// $[GPIO_ETAMPDET_OUT1]
#define ETAMPDET_OUT1_PORT          SL_GPIO_PORT_D
#define ETAMPDET_OUT1_PIN           4
// [GPIO_ETAMPDET_OUT1]$

// =========================================================
// I2C LEADER
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

// =========================================================
// MISC
// =========================================================
// <gpio> LETIMER_OUTPUT_0
// $[GPIO_LETIMER_OUTPUT_0]
#define LETIMER_OUTPUT_0_PORT       SL_GPIO_PORT_A
#define LETIMER_OUTPUT_0_PIN        7
// [GPIO_LETIMER_OUTPUT_0]$

// <gpio> EUS0SCLK
// $[GPIO_EUS0SCLK]
#define EUS0SCLK_PORT               SL_GPIO_PORT_A
#define EUS0SCLK_PIN                10
// [GPIO_EUS0SCLK]$

// <gpio> EXP_LET0_O0
// $[GPIO_EXP_LET0_O0]
#define EXP_LET0_O0_PORT            SL_GPIO_PORT_A
#define EXP_LET0_O0_PIN             0
// [GPIO_EXP_LET0_O0]$

// <gpio> SQUARE_WAVE
// $[GPIO_SQUARE_WAVE]
#define SQUARE_WAVE_PORT            SL_GPIO_PORT_C
#define SQUARE_WAVE_PIN             0
// [GPIO_SQUARE_WAVE]$

// <gpio> CLKOUT
// $[GPIO_CLKOUT]
#define CLKOUT_PORT                 SL_GPIO_PORT_C
#define CLKOUT_PIN                  2
// [GPIO_CLKOUT]$

// <<< sl:end pin_tool >>>

// NOTE:
// Each physical GPIO is defined exactly once in pin_tool.
// All other signals reuse these definitions.
// Multiple aliases may map to the same physical GPIO pin.
// For a given GPIO only one alias/function is used at runtime.


// =========================================================
// EUSART / SPI
// =========================================================

#define EUSART0_TX_PORT             EXP_UART_TX_PORT
#define EUSART0_TX_PIN              EXP_UART_TX_PIN

#define EUSART0_RX_PORT             EXP_UART_RX_PORT
#define EUSART0_RX_PIN              EXP_UART_RX_PIN

#define EUS0MOSI_PORT               EXP_UART_TX_PORT
#define EUS0MOSI_PIN                EXP_UART_TX_PIN

#define EUS0MISO_PORT               EXP_UART_RX_PORT
#define EUS0MISO_PIN                EXP_UART_RX_PIN

#define EUS0CS_PORT                 LETIMER_OUTPUT_0_PORT
#define EUS0CS_PIN                  LETIMER_OUTPUT_0_PIN


// =========================================================
// I2C FOLLOWER
// =========================================================

#define I2C_FOLLOWER_SDA_PORT       ETAMPDET_IN0_PORT
#define I2C_FOLLOWER_SDA_PIN        ETAMPDET_IN0_PIN

#define I2C_FOLLOWER_SCL_PORT       ETAMPDET_OUT0_PORT
#define I2C_FOLLOWER_SCL_PIN        ETAMPDET_OUT0_PIN


// =========================================================
// TIMER / GPIO SIGNALS
// =========================================================

#define TIMER_EXTIO_PORT            ETAMPDET_OUT0_PORT
#define TIMER_EXTIO_PIN             ETAMPDET_OUT0_PIN

#define GPIO_IADC0_EOC_PORT         ETAMPDET_IN0_PORT
#define GPIO_IADC0_EOC_PIN          ETAMPDET_IN0_PIN

#define GPIO_LDMA_COMPLETE_PORT     ETAMPDET_IN0_PORT
#define GPIO_LDMA_COMPLETE_PIN      ETAMPDET_IN0_PIN

#define SPI_TIME_PORT               ETAMPDET_IN0_PORT
#define SPI_TIME_PIN                ETAMPDET_IN0_PIN


// =========================================================
// ANALOG COMPARATOR
// =========================================================

#define ACMP_OUTPUT0_PORT           EXP_UART_TX_PORT
#define ACMP_OUTPUT0_PIN            EXP_UART_TX_PIN

#define ACMP_OUTPUT1_PORT           EXP_UART_RX_PORT
#define ACMP_OUTPUT1_PIN            EXP_UART_RX_PIN


// =========================================================
// PRS
// =========================================================

#define PRS_OUTPUT_PORT             LED1_PORT
#define PRS_OUTPUT_PIN              LED1_PIN


// =========================================================
// VDAC
// =========================================================

#define VDAC0_CH0_MAINOUT_PORT      BUTTON0_PORT
#define VDAC0_CH0_MAINOUT_PIN       BUTTON0_PIN

#define VDAC_CH0_AUXOUT_PORT        ETAMPDET_OUT0_PORT
#define VDAC_CH0_AUXOUT_PIN         ETAMPDET_OUT0_PIN


// =========================================================
// WDOG
// =========================================================

#define WDOG_PRS_PORT               ETAMPDET_IN0_PORT
#define WDOG_PRS_PIN                ETAMPDET_IN0_PIN
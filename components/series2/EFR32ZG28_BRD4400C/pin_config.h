// <<< sl:start pin_tool >>>

// =========================================================
// LEDs & BUTTONS
// =========================================================

// <gpio> LED0
// $[GPIO_LED0]
#define LED0_PORT                   SL_GPIO_PORT_B
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
#define BUTTON0_PIN                 1
// [GPIO_BUTTON0]$

// <gpio> BUTTON1
// $[GPIO_BUTTON1]
#define BUTTON1_PORT                SL_GPIO_PORT_B
#define BUTTON1_PIN                 3
// [GPIO_BUTTON1]$


// =========================================================
// EXP / UART
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
// SPI 
// =========================================================

// <gpio> EXP_SPI_COPI
// $[GPIO_EXP_SPI_COPI]
#define EXP_SPI_COPI_PORT           SL_GPIO_PORT_D
#define EXP_SPI_COPI_PIN            7
// [GPIO_EXP_SPI_COPI]$

// <gpio> EXP_SPI_CIPO
// $[GPIO_EXP_SPI_CIPO]
#define EXP_SPI_CIPO_PORT           SL_GPIO_PORT_D
#define EXP_SPI_CIPO_PIN            8
// [GPIO_EXP_SPI_CIPO]$

// <gpio> EXP_SPI_SCK
// $[GPIO_EXP_SPI_SCK]
#define EXP_SPI_SCK_PORT            SL_GPIO_PORT_D
#define EXP_SPI_SCK_PIN             9
// [GPIO_EXP_SPI_SCK]$

// <gpio> EXP_SPI_CS
// $[GPIO_EXP_SPI_CS]
#define EXP_SPI_CS_PORT             SL_GPIO_PORT_D
#define EXP_SPI_CS_PIN              10
// [GPIO_EXP_SPI_CS]$

// <gpio> EUS0MOSI
#define EUS0MOSI_PORT               SL_GPIO_PORT_A
#define EUS0MOSI_PIN                11

// <gpio> EUS0MISO
#define EUS0MISO_PORT               SL_GPIO_PORT_A
#define EUS0MISO_PIN                12

// <gpio> EUS0SCLK
#define EUS0SCLK_PORT               SL_GPIO_PORT_A
#define EUS0SCLK_PIN                13

// <gpio> EUS0CS
#define EUS0CS_PORT                 SL_GPIO_PORT_A
#define EUS0CS_PIN                  14


// =========================================================
// MISC
// =========================================================

// <gpio> WDOG_PRS
// $[GPIO_WDOG_PRS]
#define WDOG_PRS_PORT               SL_GPIO_PORT_B
#define WDOG_PRS_PIN                4
// [GPIO_WDOG_PRS]$

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

// <gpio> I2C_DOMAIN_POWER
// $[GPIO_I2C_DOMAIN_POWER]
#define I2C_DOMAIN_POWER_PORT       SL_GPIO_PORT_C
#define I2C_DOMAIN_POWER_PIN        11
// [GPIO_I2C_DOMAIN_POWER]$

// <gpio> SPI_TIME
// $[GPIO_SPI_TIME]
#define SPI_TIME_PORT               SL_GPIO_PORT_D
#define SPI_TIME_PIN                11
// [GPIO_SPI_TIME]$

// <gpio> SQUARE_WAVE
// $[GPIO_SQUARE_WAVE]
#define SQUARE_WAVE_PORT            SL_GPIO_PORT_D
#define SQUARE_WAVE_PIN             10
// [GPIO_SQUARE_WAVE]$

// <gpio> CLKOUT
// $[GPIO_CLKOUT]
#define CLKOUT_PORT                 SL_GPIO_PORT_D
#define CLKOUT_PIN                  9
// [GPIO_CLKOUT]$

// <gpio> VDAC0_CH0_MAINOUT
// $[GPIO_VDAC0_CH0_MAINOUT]
#define VDAC0_CH0_MAINOUT_PORT      SL_GPIO_PORT_B
#define VDAC0_CH0_MAINOUT_PIN       0
// [GPIO_VDAC0_CH0_MAINOUT]$


// <<< sl:end pin_tool >>>


// =========================================================
// EUSART (UART)
// =========================================================

#define EUSART0_TX_PORT             EXP_UART_TX_PORT
#define EUSART0_TX_PIN              EXP_UART_TX_PIN

#define EUSART0_RX_PORT             EXP_UART_RX_PORT
#define EUSART0_RX_PIN              EXP_UART_RX_PIN


// =========================================================
// I2C
// =========================================================

#define I2C_FOLLOWER_SDA_PORT       EUS0MOSI_PORT
#define I2C_FOLLOWER_SDA_PIN        EUS0MOSI_PIN

#define I2C_FOLLOWER_SCL_PORT       EUS0MISO_PORT
#define I2C_FOLLOWER_SCL_PIN        EUS0MISO_PIN


// =========================================================
// GPIO SIGNALS
// =========================================================

#define LETIMER_OUTPUT_0_PORT       EUS0MOSI_PORT
#define LETIMER_OUTPUT_0_PIN        EUS0MOSI_PIN

#define GPIO_IADC0_EOC_PORT         EUS0MISO_PORT
#define GPIO_IADC0_EOC_PIN          EUS0MISO_PIN

#define GPIO_LDMA_COMPLETE_PORT     EUS0MISO_PORT
#define GPIO_LDMA_COMPLETE_PIN      EUS0MISO_PIN

#define EXP_LET0_O0_PORT            EUS0MISO_PORT
#define EXP_LET0_O0_PIN             EUS0MISO_PIN

#define TIMER_EXTIO_PORT            WDOG_PRS_PORT
#define TIMER_EXTIO_PIN             WDOG_PRS_PIN


// =========================================================
// ACMP / PRS
// =========================================================

#define ACMP_OUTPUT0_PORT           LED0_PORT
#define ACMP_OUTPUT0_PIN            LED0_PIN

#define ACMP_OUTPUT1_PORT           LED1_PORT
#define ACMP_OUTPUT1_PIN            LED1_PIN

#define PRS_OUTPUT_PORT             LED1_PORT
#define PRS_OUTPUT_PIN              LED1_PIN
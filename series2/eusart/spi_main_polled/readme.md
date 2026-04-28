# Peripheral Examples - EUSART SPI Main Polled

## Summary

This project demonstrates polled operation of the EUSART in synchronous
main (formerly master) mode.  EUSART0 is configured for SPI-compatible
operation at 1 Mbps.  Compatibility with the Synchronous Peripheral
Interface standard implies a word size of 8 data bits transmitted and
received MSB-first.

An output buffer is filled with 10 bytes of data (which can be changed by
modifying BUFLEN).  The EUSART chip select pin (controlled as a GPIO), is
asserted (driven low) to notify a secondary (formerly slave) device that
data will soon be on the way.  This example inserts a delay of 15 us
after chip select assertion in order to allow the secondary device (expected
to be one of the equivalent EFM32/EFR32 interrupt-driven examples) to exit
its low-power state and pre-load its transmit buffer with the first byte of
data to be returned.

The emlib `EUSART_Spi_TxRx()` function is used to transmit each byte and
save each byte that is simultaneously received.  `EUSART_Spi_TxRx()` polls
the FIFO level flag (EUSART_STATUS_TXFL), and, if space is available,
writes a byte to the transmit data register to be sent.  It then polls
the receive FIFO level flag (EUSART_STATUS_RXFL) to determine when all
bits of the incoming frame have been received, which indicates that a
complete byte is available to be read from the receive data register.

After BUFLEN characters have been transmitted and received, the chip
select is de-asserted (driven high), and the process repeats.  A
breakpoint can be set at the call to GPIO_PinOutSet() when the chip
select is de-asserted to inspect the received data.

Before repeating the process, another delay (10 us) is inserted
because the secondary implementations running on slower devices (e.g.
either lower clock rate or lower IPC CPU such as the Cortex-M0+) need
extra time to prepare the input and output buffers for the next round of
bytes to be transferred.  Without this delay, these devices effectively
skip receiving data on every other chip select assertion.

> Note: This example uses inclusive lexicon wherever possible. For more
information, visit https://www.silabs.com/about-us/inclusive-lexicon-project

## Peripherals Used

* GPIO
* EUSART0

The CMU is used indirectly via the `EUSART_SpiInit()` function to calculate
the divisor necessary to derive the desired bit rate.

## How To Test

1. This example requires two Starter Kits/Pro Kits for any two Series 2 EFM32
   or EFR32 devices.  Build the `spi_secondary_polled` example and download it
   to the first board.

2. Build this project and download it to the second board.

3. Connect the Expansion Header pins from the main to the secondary board
   as follows:

    | Main Kit |     | Secondary Kit |
    | -------- | --- | ------------- |
    | GND      | <-> | GND           |
    | MOSI     | <-> | MOSI          |
    | MISO     | <-> | MISO          |
    | SCLK     | <-> | SCLK          |
    | CSn      | <-> | CSn           |

4. Before running the example, set a breakpoint on the `sl_gpio_set_pin`
   function call that de-asserts the chip select.  Examine the `inbuf[]`
   array to see the received data from the secondary.  This example
   transfers data continuously but will not start until PB0 is pressed.

## Hardware & Connections

* Board:  Silicon Labs EFR32ZG23 868-915 MHz 14 dBm Radio Board (BRD4204D)
        + Wireless Starter Kit Mainboard
  * Device: EFR32ZG23B010F512IM48
    * PA08 -  EUSART0_TX (MOSI)  - Expansion Header Pin 12, WSTK Pin 9
    * PA09 -  EUSART0_RX (MISO)  - Expansion Header Pin 14, WSTK Pin 11
    * PA10 -  EUSART0_CLK (SCLK) - Expansion Header Pin 3, WSTK Pin 0
    * PA07 -  EUSART0_CS (CSn)   - Expansion Header Pin 13, WSTK Pin 10

* Board:  Silicon Labs EFR32xG24 2.4 GHz 10 dBm Radio Board (BRD4186C)
        + Wireless Starter Kit Mainboard
  * Device: EFR32MG24B210F1536IM48
    * PA08 -  EUSART0_TX (MOSI)  - Expansion Header Pin 12, WSTK Pin 9
    * PA09 -  EUSART0_RX (MISO)  - Expansion Header Pin 14, WSTK Pin 11
    * PA06 -  EUSART0_CLK (SCLK) - Expansion Header Pin 11, WSTK Pin 8
    * PA07 -  EUSART0_CS (CSn)   - Expansion Header Pin 13, WSTK Pin 10

* Board:  Silicon Labs EFR32FG25 902-928 MHz 14 dBm Radio Board (BRD4270B)
        + Wireless Starter Kit Mainboard
  * Device: EFR32FG25B222F1920IM56
    * PA08 -  EUSART0_TX (MOSI)  - Expansion Header Pin 12, WSTK Pin 9
    * PA09 -  EUSART0_RX (MISO)  - Expansion Header Pin 14, WSTK Pin 11
    * PA10 -  EUSART0_CLK (SCLK) - Expansion Header Pin 3, WSTK Pin 0
    * PA07 -  EUSART0_CS (CSn)   - Expansion Header Pin 13, WSTK Pin 10

* Board:  Silicon Labs EFR32xG26 2.4 GHz 20 dBm Radio Board (BRD4117A)
        + Wireless Starter Kit Mainboard
  * Device: EFR32MG26B420F3200IM48
    * PA08 -  EUSART0_TX (MOSI)  - Expansion Header Pin 12, WSTK Pin 9
    * PA09 -  EUSART0_RX (MISO)  - Expansion Header Pin 14, WSTK Pin 11
    * PA06 -  EUSART0_CLK (SCLK) - Expansion Header Pin 11, WSTK Pin 8
    * PA07 -  EUSART0_CS (CSn)   - Expansion Header Pin 13, WSTK Pin 10

* Board:  Silicon Labs EFR32xG27 Buck Radio Board (BRD4194A)
          + Wireless Starter Kit Mainboard
  * Device: EFR32MG27C140F768IM40
    * PC00 -  EUSART0_TX (MOSI)  - Expansion Header Pin 4, WSTK Pin 1
    * PC01 -  EUSART0_RX (MISO)  - Expansion Header Pin 6, WSTK Pin 3
    * PC02 -  EUSART0_CLK (SCLK) - Expansion Header Pin 8, WSTK Pin 5
    * PC03 -  EUSART0_CS (CSn)   - Expansion Header Pin 10, WSTK Pin 7

* Board:  Silicon Labs EFR32xG28 868/915 MHz +14 dBm + 2.4 GHz +10 dBm
          Radio Board (BRD4400C) + Wireless Starter Kit Mainboard
  * Device: EFR32ZG28B312F1024IM68
    * PA11 -  EUSART0_TX (MOSI)  - Expansion Header Pin 3, WSTK Pin 0
    * PA12 -  EUSART0_RX (MISO)  - Expansion Header Pin 5, WSTK Pin 2
    * PA13 -  EUSART0_CLK (SCLK) - Expansion Header Pin 7, WSTK Pin 4
    * PA14 -  EUSART0_CS (CSn)   - Expansion Header Pin 9, WSTK Pin 6

* Board:  Silicon Labs EFR32xG29 Wireless 2.4 GHz 8 dBm Buck Radio Board
          (BRD4412A) + Wireless Starter Kit Mainboard
  * Device: EFR32MG29B140F1024IM40
    * PA05 -  EUSART0_TX (MOSI)  - Expansion Header Pin 12, WSTK Pin 9
    * PA06 -  EUSART0_RX (MISO)  - Expansion Header Pin 14, WSTK Pin 11
    * PB02 -  EUSART0_CLK (SCLK) - Expansion Header Pin 15, WSTK Pin 12
    * PB03 -  EUSART0_CS (CSn)   - Expansion Header Pin 16, WSTK Pin 13

* Board:  Silicon Labs EFR32FG2D 868-915 MHz 14 dBm Radio Board (BRD4277A)
        + Wireless Starter Kit Mainboard
  * Device: EFR32FG2DB010F512IM48
    * PA08 -  EUSART0_TX (MOSI)  - Expansion Header Pin 12, WSTK Pin 9
    * PA09 -  EUSART0_RX (MISO)  - Expansion Header Pin 14, WSTK Pin 11
    * PA10 -  EUSART0_CLK (SCLK) - Expansion Header Pin 3, WSTK Pin 0
    * PA07 -  EUSART0_CS (CSn)   - Expansion Header Pin 13, WSTK Pin 10

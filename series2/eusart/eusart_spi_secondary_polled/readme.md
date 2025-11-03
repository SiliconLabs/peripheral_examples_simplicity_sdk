# Peripheral Examples - EUSART SPI Secondary Polled

## Summary

This project demonstrates polled operation of the EUSART in synchronous
secondary (formerly slave mode).  EUSART0 is configured for SPI-compatible
operation at 1 Mbps.  Compatibility with the Synchronous Peripheral Interface
standard implies a word size of 8 data bits transmitted and received MSB-first.

It is important to note that because this example runs in secondary mode,
the GPIO configuration of the EUSART receive and transmit pins is reversed.
This is reflected in their SPI signal names: Main In/Secondary Out
(MISO) and Main Out/Secondary In (MOSI).  Thus, in main mode the EUSART_TX and
EUSART_RX pins are output and input, respectively, while in secondary mode 
they operate as input (MOSI) and output (MISO), respectively.

Likewise, the clock and chip select pins are also secondary mode inputs.
Because a secondary device can be expected to operate primarily in a
low-power mode in order to reduce energy use, the chip select input
is configured to request a GPIO interrupt on a falling edge.  The main
device must add setup time after chip select assertion in order for the
secondary device to exit its low-power state and pre-load the transmit
buffer if it is expected to shift out a byte of data concurrent with the
first incoming byte fromt the main (even if this is otherwise ignored).
The provided EUSART main mode examples do this with simple software delay
loops.

A single output buffer is populated with 10 bytes (set by BUFLEN) of
outbound data that will be overwritten by the incoming data.  The device
polls for the falling edge of the chip select and then clears the receive
FIFO.  This step is not necessary in a well-behaved SPI system
configuration, but because this example is intended to run with one of the 
main mode examples, which constantly transmit data, it is required so that
data is received in the expected order when the main asserts the chip
select.

The specified number of data bytes are transmitted and received in a
for-loop.  The emlib `EUSART_Tx()` function checks for space in the transmit
FIFO space (EUSART_STATUS_TXFL != 0) before loading it with each byte.
`EUSART_Rx()` checks for data in the receive FIFO (EUSART_STATUS_RXFL == 1)
before reading a byte.  This provides the desired polling mechanism, which
is predicated on the main driving eight clock edges for each byte transferred
to and from the secondary.

> Note: This example uses inclusive lexicon wherever possible. For more
information, visit https://www.silabs.com/about-us/inclusive-lexicon-project

## Peripherals Used

* GPIO
* EUSART0

## How To Test

1. This example requires two Starter Kits/Pro Kits for any two Series 2 EFM32
   or EFR32 devices.  Build the `spi_main_polled` example and download it to
   the first board.

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

4. Before running the example, set a breakpoint at the 
   `sl_hal_gpio_clear_interrupts` call in the main loop.  The `buffer[]` array
   contain 0s on initial execution, after which it should hold the values 
   sent by the main device (0 to 9 inclusive).  The secondary example transfers 
   data continuously but will not start until PB0 is pressed on the board 
   running the main code.

## Hardware & Connections

* Board:  Silicon Labs EFR32ZG23 868-915 MHz 14 dBm Radio Board (BRD4204D)
        + Wireless Starter Kit Mainboard (BRD4001A)
  * Device: EFR32ZG23B010F512IM48
    * PA08 -  EUSART0_TX (MOSI)  - Expansion Header Pin 12, WSTK Pin 9
    * PA09 -  EUSART0_RX (MISO)  - Expansion Header Pin 14, WSTK Pin 11
    * PA10 -  EUSART0_CLK (SCLK) - Expansion Header Pin 3, WSTK Pin 0
    * PA07 -  EUSART0_CS (CSn)   - Expansion Header Pin 13, WSTK Pin 10

* Board:  Silicon Labs EFR32xG24 2.4 GHz 10 dBm Radio Board (BRD4186C)
        + Wireless Starter Kit Mainboard (BRD4001A)
  * Device: EFR32MG24B210F1536IM48
    * PA08 -  EUSART0_TX (MOSI)  - Expansion Header Pin 12, WSTK Pin 9
    * PA09 -  EUSART0_RX (MISO)  - Expansion Header Pin 14, WSTK Pin 11
    * PA06 -  EUSART0_CLK (SCLK) - Expansion Header Pin 11, WSTK Pin 8
    * PA07 -  EUSART0_CS (CSn)   - Expansion Header Pin 13, WSTK Pin 10

* Board:  Silicon Labs EFR32FG25 902-928 MHz 14 dBm Radio Board (BRD4270B)
        + Wireless Starter Kit Mainboard (BRD4001A)
  * Device: EFR32FG25B222F1920IM56
    * PA08 -  EUSART0_TX (MOSI)  - Expansion Header Pin 12, WSTK Pin 9
    * PA09 -  EUSART0_RX (MISO)  - Expansion Header Pin 14, WSTK Pin 11
    * PA10 -  EUSART0_CLK (SCLK) - Expansion Header Pin 3, WSTK Pin 0
    * PA07 -  EUSART0_CS (CSn)   - Expansion Header Pin 13, WSTK Pin 10

* Board:  Silicon Labs EFR32xG26 2.4 GHz 20 dBm Radio Board (BRD4117A)
        + Wireless Starter Kit Mainboard (BRD4001A)
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

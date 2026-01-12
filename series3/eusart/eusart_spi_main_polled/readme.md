# Peripheral Examples - EUSART SPI Main Polled

## Summary

This project demonstrates polled operation of the EUSART in synchronous
main (formerly master) mode. EUSART0 is configured for SPI-compatible
operation at 1 Mbps. Compatibility with the Synchronous Peripheral
Interface standard implies a word size of 8 data bits transmitted and
received MSB-first.

An output buffer is filled with 10 bytes of data (which can be changed by
modifying BUFLEN). The EUSART chip select pin (controlled as a GPIO), is
asserted (driven low) to notify a secondary (formerly slave) device that
data will soon be on the way. This example inserts a delay of 15 us
after chip select assertion in order to allow the secondary device (expected
to be one of the equivalent Series 3 polled examples) to exit its low-power 
state and pre-load its transmit buffer with the first byte of data to be 
returned.

The `sl_hal_eusart_spi_tx_rx()` function is used to transmit each byte and
save each byte that is simultaneously received. `sl_hal_eusart_spi_tx_rx()` polls
the FIFO level flag (EUSART_STATUS_TXFL), and if space is available,
writes a byte to the transmit data register to be sent. It then polls
the receive FIFO level flag (EUSART_STATUS_RXFL) to determine when all
bits of the incoming frame have been received, which, in turn, will read 
all values into the receive data register.

After BUFLEN characters have been transmitted and received, the chip
select is de-asserted (driven high), and the process repeats. A
breakpoint can be set at the call to sl_gpio_set_pin() when the chip
select is de-asserted to inspect the received data.

Before repeating the process, another delay (10 us) is inserted
because the secondary implementations running on slower devices (e.g.
either lower clock rate or lower IPC CPU such as the Cortex-M0+) need
extra time to prepare the input and output buffers for the next round of
bytes to be transferred. Without this delay, these devices effectively
skip receiving data on every other chip select assertion.

> Note: This example uses inclusive lexicon wherever possible. For more
information, visit https://www.silabs.com/about-us/inclusive-lexicon-project

## Peripherals Used

* GPIO
* EUSART0

The CMU is used indirectly via the `sl_clock_manager_get_clock_branch_frequency()` 
function in `eusart0_init()` to calculate the divisor necessary to derive the 
desired bit rate.

## How To Test

1. This example requires two Starter Kits/Pro Kits for two Series 3 devices. Build 
   the `spi_secondary_polled` example and download it to the first board.

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
   function call that de-asserts the chip select. Examine the `inbuf[]`
   array to see the received data from the secondary. This example
   transfers data only when PB0 is pressed. 
   
5. When debugging, make sure to reset *both* the main and secondary board.

## Hardware & Connections

* Board: Silicon Labs SixG301 Radio Board (BRD4407A) + Wireless Pro Kit Mainboard
  * Device: SIMG301M104LIL
    * PB02 -  EUSART0_TX (MOSI)  - Expansion Header Pin 12, WSTK Pin F6
    * PB00 -  EUSART0_RX (MISO)  - Expansion Header Pin 14, WSTK Pin F7
    * PA00 -  EUSART0_CLK (SCLK) - Expansion Header Pin 5, WSTK Pin F9
    * PB03 -  EUSART0_CS (CSn)   - Expansion Header Pin 3, WSTK Pin F8
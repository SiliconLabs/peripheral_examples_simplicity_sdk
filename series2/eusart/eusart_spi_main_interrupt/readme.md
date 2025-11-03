# Peripheral Examples - EUSART SPI Main Interrupt

## Summary

This project demonstrates interrupt-driven operation of the EUSART in
synchronous main (formerly master) mode.  EUSART0 is configured for
SPI-compatible operation at 1 Mbps.  Compatibility with the Synchronous
Peripheral Interface standard implies a word size of 8 data bits transmitted
and received MSB-first.

An output buffer is filled with 10 bytes of data (which can be changed by
modifying BUFLEN).  The EUSART chip select pin (controlled as a GPIO), is
asserted (driven low) to notify a secondary (formerly slave) device that data
will soon be on the way.  This example inserts a delay of 15 us after
chip select assertion in order to allow the secondary device (expected to
be one of the equivalent EFM32/EFR32 interrupt-driven examples) to exit its
low-power state and pre-load its transmit buffer with the first byte of data
to be returned.

The program flow is controlled by a simple state machine with three states: 
**IDLE**, **RX_COMPLETE**, and **SEND**. The switch statement 
in the `app_process_action` loop includes an intentional fall-through from 
**RX_COMPLETE** to **SEND**, allowing the next transfer to begin immediately 
for uninterrupted operation.  The first byte of data to be transmitted 
(`outbuf[0]`) is written to the EUSARTn_TXDATA register, after which the 
device enters the EM1 low-energy mode.  While the CPU is halted, the EUSART
drives the CLK pin such that each pulse causes the byte written to TXDATA to be
shifted out while an incoming byte is shifted into RXDATA one bit at a time.

Once the last CLK pulse has been driven, the EUSART asserts its transmit
complete flag (EUSARTn_STATUS_TXC) and its associated interrupt request
(EUSARTn_IF_TXC).  This causes the processor to exit the EM1 low-energy
mode and enter the `EUSARTn_TX_IRQHandler()` where the newly received byte
is written to the `inbuf[]` array.  If characters remain to be sent, the
next byte is written to TXDATA, and the TXC interrupt flag is cleared
before exiting the IRQ handler.

Until BUFLEN characters have been transferred, the device repeatedly
enters EM1, waiting for the transmit complete interrupt to wake it up in
order to save the most recently received character.

After BUFLEN characters have been transmitted and received, the chip
select is de-asserted (driven high).  A breakpoint can be set at the
start of case **SEND** to inspect the received data.

Before repeating the process, another delay (around 8 us) is inserted
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
   or EFR32 devices.  Build the `spi_secondary_interrupt` example and download 
   it to the first board.

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

4. Before running the example, set a breakpoint at the start of case **SEND**
   in ``app_process_action``.  Examine the ``inbuf[]`` array to see the
   received data from the secondary.  This example transfers data
   continuously but will not start until PB0 is pressed.

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

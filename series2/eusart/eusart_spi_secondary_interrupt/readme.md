# Peripheral Examples - EUSART SPI Secondary Interrupt

## Summary

This project demonstrates interrupt-driven operation of the EUSART in
synchronous secondary (formerly slave) mode.  EUSART0 is configured for
SPI-compatible operation at 1 Mbps.  Compatibility with the Synchronous
Peripheral Interface standard implies a word size of 8 data bits
transmitted and received MSB-first.

It is important to note that because this example runs in secondary mode,
the GPIO configuration of the EUSART receive and transmit pins is reversed.
This is reflected in their SPI signal names: Main In/Secondary Out
(MISO) and Main Out/Secondary In (MOSI).  Thus, in main mode the EUSART_TX
and EUSART_RX pins are output and input, respectively, while in secondary
mode they operate as input (MOSI) and output (MISO), respectively.

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

Buffers `inbuf[]` and `outbuf[]` are, respectively, zeroed out and populated
with 10 bytes of data (which can be changed by modifying BUFLEN).  The
falling edge interrupt is enabled for the chip select pin, and the device
waits in the EM1 low-energy mode.  Upon exit from EM1, the EUSART receiver
and transmitter are enabled in the event that the main was in mid-transfer
when the secondary started running.  This step is not necessary in a
well-behaved SPI system, but because this example is intended to run with
the matching main mode example, which constantly transmits data, it is
required so that data is received in the expected order when the main
asserts the chip select.

The program flow is controlled by a simple state machine with four states: 
**INIT**, **RECEIVE**, and **RX_COMPLETE**. The switch statement in the 
`app_process_action` loop includes an intentional fall-through from
**RX_COMPLETE** to **SEND**, allowing the next transfer to begin immediately 
for uninterrupted operation.  The first byte of data to be transmitted 
(`outbuf[0]`) is written to the EUSART_TXDATA register, after which the device 
enters the EM1 low-energy mode.  The EUSART_CLK pin is driven by the main so 
that each pulse causes the byte written to TXDATA to be shifted out while an
incoming byte is shifted into RXDATA one bit at a time.

Once the eighth CLK pulse of a byte has been received, the EUSART asserts
its receive FIFO level flag (EUSART_STATUS_RXFL) and its associated interrupt
request (EUSART_IF_RXFL).  This causes the processor to exit the EM1
low-energy mode and enter the `EUSART_RX_IRQHandler()` where the newly received
byte is written to the `inbuf[]` array.  If characters remain to be sent, the
next byte is written to TXDATA.  The RXFL interrupt flag is cleared before
exiting the IRQ handler.

Until BUFLEN characters have been transferred, the device repeatedly
enters EM1, waiting for the receive FIFO level interrupt to wake it up in
order to save the most recently received character.  After this, the EUSART
receive FIFO level and GPIO falling edge interrupts are disabled, and the
entire process repeats. A breakpoint can be set at the start of case **SEND**
to inspect the received data.

> Note: This example uses inclusive lexicon wherever possible. For more
information, visit https://www.silabs.com/about-us/inclusive-lexicon-project

## Peripherals Used

* GPIO
* EUSART0

## How To Test

1. This example requires two Starter Kits/Pro Kits for any two Series 2 EFM32
   or EFR32 devices.  Build the `spi_main_interrupt` example and download 
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
   received data from the main.  This example transfers data continuously 
   but will not start until PB0 is pressed on the board running the main code.

================================================================================

## Activity Indicator

This example toggles a GPIO pin at critical points during execution so
that the timing of when the secondary is ready to transmit data or has
entered the receive data interrupt handler can be seen relative to the
start and transfer of data from the main.  The length of these pulses
is mostly dependent on the frequency of the HFCLK (and therefore the
number of wait states required to access flash) but is also impacted by
the device's CPU core (Cortex M0+ vs. M3/4 vs. M33) and the delays
through the pin-to-peripheral routing.

The sequence of pulses on the activity pin is documented in the code
comments and is as follows:

1. One pulse following chip select assertion by the main.  The rising
   edge of this pulse denotes exit from the GPIO IRQ handler and is the
   point at which the secondary enables the EUSART and RXDATAV interrupt
   and pre-loads the first byte to be sent to the main.  The falling edge
   of this pulse indicates that the secondary is ready to receive data.

2. The next BUFLEN pulses indicate entry into and exit from the EUSART RX
   IRQ handler.  In order to prevent drops, some amount of spacing is
   necessary between consecutive bytes when interrupt-driven.  The width
   of these pulses provides an idea of how much inter-byte spacing is
   necessary at a given clock rate.

3. After all data is sent, there will be a final, longer pulse during
   which the secondary prepares for the next transfer of BUFLEN bytes by
   pre-loading the `outbuf[]` array and zeroing out the `inbuf[]` array.

================================================================================

## Hardware & Connections

* Board:  Silicon Labs EFR32ZG23 868-915 MHz 14 dBm Radio Board (BRD4204D)
        + Wireless Starter Kit Mainboard (BRD4001A)
  * Device: EFR32ZG23B010F512IM48
    * PA08 -  EUSART0_TX (MOSI)  - Expansion Header Pin 12, WSTK Pin 9
    * PA09 -  EUSART0_RX (MISO)  - Expansion Header Pin 14, WSTK Pin 11
    * PA10 -  EUSART0_CLK (SCLK) - Expansion Header Pin 3, WSTK Pin 0
    * PA07 -  EUSART0_CS (CSn)   - Expansion Header Pin 13, WSTK Pin 10
    * PD02 -  Activity Indicator Pin - Expansion Header Pin 9, WSTK Pin 6

* Board:  Silicon Labs EFR32xG24 2.4 GHz 10 dBm Radio Board (BRD4186C)
        + Wireless Starter Kit Mainboard (BRD4001A)
  * Device: EFR32MG24B210F1536IM48
    * PA08 -  EUSART0_TX (MOSI)  - Expansion Header Pin 12, WSTK Pin 9
    * PA09 -  EUSART0_RX (MISO)  - Expansion Header Pin 14, WSTK Pin 11
    * PA06 -  EUSART0_CLK (SCLK) - Expansion Header Pin 11, WSTK Pin 8
    * PA07 -  EUSART0_CS (CSn)   - Expansion Header Pin 13, WSTK Pin 10
    * PD02 -  Activity Indicator Pin - Expansion Header Pin 9, WSTK Pin 6

* Board:  Silicon Labs EFR32FG25 902-928 MHz 14 dBm Radio Board (BRD4270B)
        + Wireless Starter Kit Mainboard (BRD4001A)
  * Device: EFR32FG25B222F1920IM56
    * PA08 -  EUSART0_TX (MOSI)  - Expansion Header Pin 12, WSTK Pin 9
    * PA09 -  EUSART0_RX (MISO)  - Expansion Header Pin 14, WSTK Pin 11
    * PA10 -  EUSART0_CLK (SCLK) - Expansion Header Pin 3, WSTK Pin 0
    * PA07 -  EUSART0_CS (CSn)   - Expansion Header Pin 13, WSTK Pin 10
    * PA05 -  Activity Indicator Pin - Expansion Header Pin 7, WSTK Pin 4

* Board:  Silicon Labs EFR32xG26 2.4 GHz 20 dBm Radio Board (BRD4117A)
        + Wireless Starter Kit Mainboard (BRD4001A)
  * Device: EFR32MG26B420F3200IM48
    * PA08 -  EUSART0_TX (MOSI)  - Expansion Header Pin 12, WSTK Pin 9
    * PA09 -  EUSART0_RX (MISO)  - Expansion Header Pin 14, WSTK Pin 11
    * PA06 -  EUSART0_CLK (SCLK) - Expansion Header Pin 11, WSTK Pin 8
    * PA07 -  EUSART0_CS (CSn)   - Expansion Header Pin 13, WSTK Pin 10
    * PD02 -  Activity Indicator Pin - Expansion Header Pin 9, WSTK Pin 6

* Board:  Silicon Labs EFR32xG27 Buck Radio Board (BRD4194A)
          + Wireless Starter Kit Mainboard
  * Device: EFR32MG27C140F768IM40
    * PC00 -  EUSART0_TX (MOSI)  - Expansion Header Pin 4, WSTK Pin 1
    * PC01 -  EUSART0_RX (MISO)  - Expansion Header Pin 6, WSTK Pin 3
    * PC02 -  EUSART0_CLK (SCLK) - Expansion Header Pin 8, WSTK Pin 5
    * PC03 -  EUSART0_CS (CSn)   - Expansion Header Pin 10, WSTK Pin 7
    * PA08 -  Activity Indicator Pin - Expansion Header Pin 13, WSTK Pin 10

* Board:  Silicon Labs EFR32xG28 868/915 MHz +14 dBm + 2.4 GHz +10 dBm 
          Radio Board (BRD4400C) + Wireless Starter Kit Mainboard
  * Device: EFR32ZG28B312F1024IM68
    * PA11 -  EUSART0_TX (MOSI)  - Expansion Header Pin 3, WSTK Pin 0
    * PA12 -  EUSART0_RX (MISO)  - Expansion Header Pin 5, WSTK Pin 2
    * PA13 -  EUSART0_CLK (SCLK) - Expansion Header Pin 7, WSTK Pin 4
    * PA14 -  EUSART0_CS (CSn)   - Expansion Header Pin 9, WSTK Pin 6
    * PD11 -  Activity Indicator Pin - Expansion Header Pin 12, WSTK Pin 11

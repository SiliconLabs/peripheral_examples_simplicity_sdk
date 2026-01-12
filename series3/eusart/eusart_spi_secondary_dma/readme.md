# Peripheral Examples - EUSART SPI Secondary DMA

## Summary

This project demonstrates DMA-driven operation of the EUSART in
synchronous secondary (formerly slave) mode. EUSART0 is configured
for SPI-compatible operation at 1 Mbps. Compatibility with the
Synchronous Peripheral Interface standard implies a word size of 8
data bits transmitted and received MSB-first.

It is important to note that because this example runs in secondary mode,
the GPIO configuration of the EUSART receive and transmit pins is reversed.
This is reflected in their SPI signal names: Main In/Secondary Out
(MISO) and Main Out/Secondary In (MOSI). Thus, in main mode the EUSART_TX
and EUSART_RX pins are output and input, respectively, while in secondary
mode they operate as input (MOSI) and output (MISO), respectively.

Likewise, the clock and chip select pins are also secondary mode inputs.
Because a secondary device can be expected to operate primarily in a
low-power mode in order to reduce energy use, the chip select input
is configured to request a GPIO interrupt on a falling edge. The main
device must add setup time after chip select assertion in order for the
secondary device to exit its low-power state and pre-load the transmit
buffer if it is expected to shift out a byte of data concurrent with the
first incoming byte from the main (even if this is otherwise ignored).
The provided EUSART main mode examples do this with simple software delay
loops.

Buffers `inbuf[]` and `outbuf[]` are, respectively, zeroed out and populated
with 10 bytes of data (which can be changed by modifying BUFLEN). The
falling edge interrupt is enabled for the chip select pin, and the device
waits in the EM1 low-energy mode. Upon exit from EM1 the receive FIFO is
cleared. This step is not necessary in a well-behaved SPI system, but
because this example is intended to run with one of the main mode
examples, which constantly transmit data, it is required so that data is
received in the expected order when the main asserts the chip select.

The program flow is controlled by a simple state machine with three states: 
**INIT**, **RECEIVE**, and **RX_COMPLETE**. The switch statement in 
the `app_process_action` loop includes an intentional fall-through from 
**INIT** to **RX_COMPLETE**, allowing the next transfer interrupt to be set up.  
The LDMA has been configured so that (a) one channel moves each byte in `outbuf[]` 
to the EUSARTn_TXDATA register when there is space in the transmit FIFO 
(EUSARTn_STATUS_TXFL is asserted), and (b) another channel moves the matching 
received byte from EUSARTn_RXDATA to `inbuf[]` (upon assertion of 
EUSARTn_STATUS_RXFL). Transfers are started for these channels, and the device 
enters the EM1 low-energy mode.

While the CPU is halted, the LDMA writes each byte from `outbuf[]` to
the transmit FIFO as space is available. The EUSARTn_CLK pin is driven by
the main so that each pulse causes the byte written to TXDATA to be
shifted out while an incoming byte is shifted into RXDATA one bit at a
time. Once all 8 bits of the incoming byte have been received, the LDMA
writes each byte from the receive FIFO to `inbuf[]`.

This process occurs autonomously, and only the LDMA interrupt request
(assertion of one or more of the LDMA_IF_DONEx flags or LDMA_IF_ERROR in
the event of some kind of problem) causes the device to exit EM1.
Entering EM1 is handled by the Power Manager when exiting 
`app_process_action()`. The `ldma_rx_callback()` sets the state machine 
to RX_COMPLETE, and the chip select is de-asserted once the last byte 
is received.

> Note: This example uses inclusive lexicon wherever possible. For more
information, visit https://www.silabs.com/about-us/inclusive-lexicon-project

## Peripherals Used

* GPIO
* LDMA
* EUSART0

## How To Test

1. This example requires two Starter Kits/Pro Kits for two Series 3 devices. 
   Build the `spi_main_dma` example and download it to the first board.

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
   in ``app_process_action`` in the main example. Examine the ``inbuf[]`` array 
   to see the received data from the secondary. This example will not transfer 
   data until PB0 is pressed on the board running the main code.
   
5. When debugging, make sure to reset *both* the main and secondary board.   

## Hardware & Connections

* Board: Silicon Labs SixG301 Radio Board (BRD4407A) + Wireless Pro Kit Mainboard
  * Device: SIMG301M104LIL
    * PB02 -  EUSART0_TX (MOSI)  - Expansion Header Pin 12, WSTK Pin F6
    * PB00 -  EUSART0_RX (MISO)  - Expansion Header Pin 14, WSTK Pin F7
    * PA00 -  EUSART0_CLK (SCLK) - Expansion Header Pin 5, WSTK Pin F9
    * PB03 -  EUSART0_CS (CSn)   - Expansion Header Pin 3, WSTK Pin F8
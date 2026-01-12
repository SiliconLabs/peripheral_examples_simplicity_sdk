# Peripheral Examples - EUSART SPI Main DMA

## Summary

This project demonstrates DMA-driven operation of the EUSART in
synchronous main (formerly master) mode. EUSART0 is configured for
SPI-compatible operation at 1 Mbps. Compatibility with the Synchronous
Peripheral Interface standard implies a word size of 8 data bits
transmitted and received MSB-first.

In systems where the secondary (formerly slave) device is a processor,
some delay is generally required to detect the chip select falling edge
and to prepare to receive the incoming data. It is incumbent on the main
processor to insert this delay through some means, e.g. via a software
delay or otheriwse (alternatively, a pin-based handshaking mechanism can
be used). Because this example must start the receive and transmit LDMA
channels, the pair of calls to LDMA_StartTransfer() do this.

Buffers `inbuf[]` and `outbuf[]` are, respectively, zeroed out and populated
with 10 bytes of data (which can be changed by modifying BUFLEN).

The program flow is controlled by a simple state machine with three states: 
**IDLE**, **RX_COMPLETE**, and **SEND**. The LDMA is configured so that 
(a) one channel moves each byte in `outbuf[]` to the EUSARTn_TXDATA register 
when there is space in the transmit FIFO (EUSARTn_STATUS_TXFL is asserted), 
and (b) another channel moves the matching received byte from EUSARTn_RXDATA 
to `inbuf[]` (upon assertion of EUSARTn_STATUS_RXFL).  Chip select is 
asserted, the transfers are started for the LDMA channels, and the device 
enters the EM1 low-energy mode.

While the CPU is halted, the LDMA writes each byte from `outbuf[]` to
the transmit FIFO as space is available. EUSART0 drives the CLK pin
so that each pulse causes the byte written to TXDATA to be shifted out
while an incoming byte is shifted into RXDATA one bit at a time. Once
all 8 bits of the incoming byte have been received, the LDMA writes each
byte from the receive FIFO to `inbuf[]`.

This process occurs upon a button press, and only the LDMA interrupt request
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

The CMU is used indirectly via the `sl_clock_manager_get_clock_branch_frequency()` 
function in `eusart0_init()` to calculate the divisor necessary to derive the 
desired bit rate.

## How To Test

1. This example requires two Starter Kits/Pro Kits for two Series 3 devices. 
   Build the `spi_secondary_dma` example and download it to the first board.

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
   in ``app_process_action``. Examine the ``inbuf[]`` array to see the
   received data from the secondary. This example will not transfer data 
   until PB0 is pressed on the board running the main code.
   
5. When debugging, make sure to reset *both* the main and secondary board.   

## Hardware & Connections

* Board: Silicon Labs SixG301 Radio Board (BRD4407A) + Wireless Pro Kit Mainboard
  * Device: SIMG301M104LIL
    * PB02 -  EUSART0_TX (MOSI)  - Expansion Header Pin 12, WSTK Pin F6
    * PB00 -  EUSART0_RX (MISO)  - Expansion Header Pin 14, WSTK Pin F7
    * PA00 -  EUSART0_CLK (SCLK) - Expansion Header Pin 5, WSTK Pin F9
    * PB03 -  EUSART0_CS (CSn)   - Expansion Header Pin 3, WSTK Pin F8
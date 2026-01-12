# Peripheral Examples - EUSART SPI Secondary Interrupt

## Summary

This project demonstrates interrupt-driven operation of the EUSART in
synchronous secondary (formerly slave) mode. EUSART0 is configured for
SPI-compatible operation at 1 Mbps. Compatibility with the Synchronous
Peripheral Interface standard implies a word size of 8 data bits
transmitted and received MSB-first.

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
first incoming byte fromt the main (even if this is otherwise ignored).
The provided EUSART main mode examples do this with simple software delay
loops.

Buffers `inbuf[]` and `outbuf[]` are, respectively, zeroed out and populated
with 64 bytes of data (which can be changed by modifying BUFLEN). The
falling edge interrupt is enabled for the chip select pin, and the device
waits in the EM1 low-energy mode. Upon exit from EM1, the EUSART receiver
and transmitter are enabled in the event that the main was in mid-transfer
when the secondary started running. This step is not necessary in a
well-behaved SPI system, but because this example is intended to run with
the matching main mode example, it is required so that data is received 
in the expected order when the main asserts the chip select.

The program flow is controlled by a simple state machine with four states: 
**INIT**, **RECEIVE**, **TX_COMPLETE**, and **RX_COMPLETE**. The switch 
statement in the `app_process_action` loop includes an intentional fall-through 
from **INIT** to **RX_COMPLETE**, preparing for the next button press.
The EUSART SCLK pin is driven by the main so that each pulse causes the byte 
written to TXDATA to be shifted out while an incoming byte is shifted into 
RXDATA one bit at a time.

Interrupts are based on the RXFL and TXFL flags, which are both set high when
there is a certain amount of values in the RX buffer or certain amount of space
left in the TX buffer, respectively. This threshold is set in the EUSART0's CFG1 
register, which can be any value between 0-32 and can be updated in the 
RXFIW_VAL and TXFIW_VAL defines. For this example, this value is 16 to reduce
the number of interrupts to allow the device to enter EM1.

Once the SCLK pulse of a byte has been received, the EUSART asserts
its receive FIFO level flag (EUSART_STATUS_RXFL) and its associated interrupt
request (EUSART_IF_RXFL). This causes the processor to exit the EM1
low-energy mode and enter the `EUSART_RX_Handler()` where the newly received
byte is written to the `inbuf[]` array. The RXFL interrupt flag is cleared before
exiting the IRQ handler.

After data has been received, the EUSART asserts its transmit FIFO level flag 
(EUSARTn_STATUS_TXFL) and its associated interrupt request (EUSARTn_IF_TXFL). 
This causes the processor to exit the EM1 low-energy mode and enter the 
`EUSARTn_TX_Handler()` where the next byte from the `outbuf[]` array is written
to TXDATA, if characters remain to be sent. The TXFL interrupt flag is cleared 
before exiting the IRQ handler.

Until all BUFLEN characters have been transferred, the device repeatedly
enters EM1, waiting for the receive FIFO level interrupt to wake it up in
order to save the most recently received character. After this, the EUSART
receive FIFO level and GPIO falling edge interrupts are disabled, and the
entire process repeats. A breakpoint can be set at the start of case **SEND**
to inspect the received data.

> Note: This example uses inclusive lexicon wherever possible. For more
information, visit https://www.silabs.com/about-us/inclusive-lexicon-project

## Peripherals Used

* GPIO
* EUSART0

## How To Test

1. This example requires two Starter Kits/Pro Kits for two Series 3 devices. 
   Build the `spi_main_interrupt` example and download it to the first board.

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
   
5. When debugging, make sure to reset *both* the main and secondary board.
   
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

1. One pulse following chip select assertion by the main. The rising
   edge of this pulse denotes exit from the GPIO IRQ handler and is the
   point at which the secondary enables the EUSART and RXDATAV interrupt
   and pre-loads the first byte to be sent to the main. The falling edge
   of this pulse indicates that the secondary is ready to receive data.

2. The next BUFLEN pulses indicate entry into and exit from the EUSART RX
   IRQ handler. In order to prevent drops, some amount of spacing is
   necessary between consecutive bytes when interrupt-driven. The width
   of these pulses provides an idea of how much inter-byte spacing is
   necessary at a given clock rate.

3. After all data is sent, there will be a final, longer pulse during
   which the secondary prepares for the next transfer of BUFLEN bytes by
   pre-loading the `outbuf[]` array and zeroing out the `inbuf[]` array.

## Hardware & Connections

* Board: Silicon Labs SixG301 Radio Board (BRD4407A) + Wireless Pro Kit Mainboard
  * Device: SIMG301M104LIL
    * PB02 -  EUSART0_TX (MOSI)  - Expansion Header Pin 12, WSTK Pin F6
    * PB00 -  EUSART0_RX (MISO)  - Expansion Header Pin 14, WSTK Pin F7
    * PA00 -  EUSART0_CLK (SCLK) - Expansion Header Pin 5, WSTK Pin F9
    * PB03 -  EUSART0_CS (CSn)   - Expansion Header Pin 3, WSTK Pin F8
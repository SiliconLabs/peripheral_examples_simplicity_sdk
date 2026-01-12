# Peripheral Examples - EUSART SPI Main Interrupt

## Summary

This project demonstrates interrupt-driven operation of the EUSART in
synchronous main (formerly master) mode. EUSART0 is configured for
SPI-compatible operation at 1 Mbps. Compatibility with the Synchronous
Peripheral Interface standard implies a word size of 8 data bits transmitted
and received MSB-first.

An output buffer is filled with 64 bytes of data (which can be changed by
modifying BUFLEN). The EUSART chip select pin (controlled as a GPIO), is
asserted (driven low) to notify a secondary (formerly slave) device that data
will soon be on the way.  This example inserts a delay of 15 us after
chip select assertion in order to allow the secondary device (expected to
be one of the equivalent EFM32/EFR32 interrupt-driven examples) to exit its
low-power state and pre-load its transmit buffer with the first byte of data
to be returned.

The program flow is controlled by a simple state machine with four states: 
**IDLE**, **RX_COMPLETE**, **TX_COMPLETE**, and **SEND**. The switch statement 
in the `app_process_action` loop includes an intentional fall-through from 
**RX_COMPLETE** to **TX_COMPLETE**, as both are meant to be helpful debugging
tools to see the current state. During **SEND** while the CPU is halted, the EUSART
drives the SCLK pin such that each pulse causes the byte written to TXDATA to be
shifted out while an incoming byte is shifted into RXDATA one bit at a time.

Interrupts are based on the RXFL and TXFL flags, which are both set high when
there is a certain amount of values in the RX buffer or certain amount of space
left in the TX buffer, respectively. This threshold is set in the EUSART0's CFG1 
register, which can be any value between 0-32 and can be updated in the 
RXFIW_VAL and TXFIW_VAL defines. For this example, this value is 16 to reduce
the number of interrupts to allow the device to enter EM1.

Once the final SCLK pulse has been driven, the EUSART asserts its transmit
FIFO level flag (EUSARTn_STATUS_TXFL) and its associated interrupt request
(EUSARTn_IF_TXFL). This causes the processor to exit the EM1 low-energy
mode and enter the `EUSARTn_TX_Handler()` where the next byte from the `outbuf[]` 
array is written to TXDATA, if characters remain to be sent. The TXFL interrupt
flag is cleared before exiting the IRQ handler.

Initially, TX interrupts are based on the TXFL flag until the final byte is
transmitted, then the interrupt is changed to TXC. This is to ensure the final
transmission of all bytes in the TX FIFO.

When data is received from the secondary device, the EUSART asserts
its receive FIFO level flag (EUSART_STATUS_RXFL) and its associated interrupt
request (EUSART_IF_RXFL). This causes the processor to exit the EM1
low-energy mode and enter the `EUSART_RX_Handler()` where the newly received
byte is written to the `inbuf[]` array. If characters remain to be sent, the
next byte is written to TXDATA. The RXFL interrupt flag is cleared before
exiting the IRQ handler.

Until all BUFLEN characters have been transferred, the device repeatedly
enters EM1, waiting for the transmit complete interrupt to wake it up in
order to save the most recently received character.

After BUFLEN characters have been transmitted and received, the chip
select is de-asserted (driven high). A breakpoint can be set at the
start of case **SEND** to inspect the received data.

Before repeating the process, another delay (around 15 us) is inserted
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
   the `spi_secondary_interrupt` example and download it to the first board.

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
# Peripheral Examples - EUSART Asynchronous Half Duplex #

## Summary ##

This project demonstrates operation of the EUSART in a half-duplex, single
link configuration. One wire is used to communicate between the EUSARTs
on two different devices such that only one device can be transmitting at
any given time. The receiving device switches to transmit mode after it receives either
`BUFLEN` characters or the `'\n'` character (new line).

> Note: Because this example runs on two boards with one initially being
the transmitter and the other the receiver, it is necessary to specify
which one is which by building the example with the `INITIAL_TRANSMITTER`
macro #defined for one board and `INITIAL_RECEIVER` #defined for the other.
The source file defaults to #defining `INITIAL_TRANSMITTER`, so this must
be changed to `INITIAL_RECEIVER` for one of the two boards.

## Peripherals used ##

* EUSART0
    * 9600 baudrate
    * 8-N-1 - 8 data bits, no parity, one stop bit
* GPIO

## How to Test ##

1. Build the project with `INITIAL_RECEIVER` #defined and download it to
   one Starter Kit.  This kit needs to be powered to run the example, but
   it does not require a debug connection.
2. Build the project again but this time with `INITIAL_TRANSMITTER` #defined
   and download it to a second Starter Kit by launching the debugger.  Do
   not run the example at this time. 
3. Connect the two boards' EUSART TX pins together.  Also connect the
   grounds on both boards (e.g. the EXP header ground on pin 1).
4. Open a terminal program and configure it for 9600N81 operation on the
   "JLink CDC UART Port" that is provided by the board controller on the
   Starter Kit mainboard.
5. Now run the example on the device configured as the `INITIAL_TRANSMITTER`
   (the device with the active connection in the debugger).
6. If successful, the terminal program window should show:
   Initial RX: Receive success and transmitting now
   Initial TX: Receive success and transmitting now
   Initial RX: Receive success and transmitting now
   Initial TX: Receive success and transmitting now
   Initial RX: Receive success and transmitting now
   Initial TX: Receive success and transmitting now
> Note: If the messages do not appear as expected, reset the setup by
   (a) stopping execution on the transmitter board, (b) pressing the reset
   button on the receiver board, (c) resetting the transmitter board 
   (d) restarting execution on the transmitter
   board.
    
## Hardware & Connections ##

* Board: Silicon Labs SixG301 Radio Board (BRD4407A) + Wireless Pro Kit Mainboard
    * Device: SIMG301M104LIL
        * PB00 - VCOM RX, Lower Breakout Header F7
        * PB02 - VCOM TX, Lower Breakout Header F6

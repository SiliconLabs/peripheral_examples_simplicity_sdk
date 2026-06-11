# Peripheral Examples - EUSART Asynchronous Half Duplex #

## Summary ##

This project demonstrates operation of the EUSART in a half-duplex, single
link configuration.  One wire is used to communicate between the EUSARTs
on two different devices such that only one device can be transmitting at
any given time.  This entails the EUSART on one device starting with its
transmitter enabled and receiver disabled while the EUSART on the second
device starts with its receiver enabled and transmitter disabled.  The
receiving device switches to transmit mode after it receives either
BUFLEN characters or the '\n' character (new line).

> Note: Because this example runs on two boards with one initially being
the transmitter and the other the receiver, it is necessary to specify
which one is which by building the example with the INITIAL_TRANSMITTER
macro #defined for one board and INITIAL_RECEIVER #defined for the other.
The source file defaults to #defining INITIAL_TRANSMITTER, so this must
be changed to INITIAL_RECEIVER for one of the two boards.

> Note: When EFR32xG22 and later devices enter EM2/3, the clock to the CPU debug
logic is stopped, effectively dropping the host debugger connection.
This example includes an "escape hatch" mechanism to pause the device so
that a debugger can connect in order to erase flash.  To use this, press
the PB0 push button and, while holding the button, press the RESET button
in the lower right corner of the WSTK.  Upon releasing the RESET button,
the device runs code that sees that PB0 is depressed, turns on LED0, and
issues a breakpoint instruction to halt the CPU.  At this point, the
debugger connection can be resumed to erase flash, etc.
one pin per port.

## Peripherals used ##

* EUSART0
    * 9600 baudrate
    * 8-N-1 - 8 data bits, no parity, one stop bit
* GPIO

## How to Test ##

1. Build the project with INITIAL_RECEIVER #defined and download it to
   one Starter Kit.  This kit needs to be powered to run the example, but
   it does not require a debug connection.
2. Build the project again but this time with INITIAL_TRANSMITTER #defined
   and download it to a second Starter Kit by launching the debugger.  Do
   not run the example at this time. 
3. Connect the two boards' EUSART TX pins together.  Also connect the
   grounds on both boards (e.g. the EXP header ground on pin 1).
4. Open a terminal program and configure it for 9600N81 operation on the
   "JLink CDC UART Port" that is provided by the board controller on the
   Starter Kit mainboard.
5. Now run the example on the device configured as the INITIAL_TRANSMITTER
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

* Board:  Silicon Labs EFR32ZG23 868-915 MHz 14 dBm Radio Board (BRD4204D) + Wireless Starter Kit Mainboard
    * Device: EFR32ZG23B010F512IM48
        * PA08 -  EUSART0 transmit, Expansion Header Pin 12, WSTK P9

* Board:  Silicon Labs EFR32MG24 2.4 GHz 10 dBm Radio Board (BRD4186C) + Wireless Starter Kit Mainboard
    * Device: EFR32MG24B210F1536IM48
        * PA08 -  EUSART0 transmit, Expansion Header Pin 12, WSTK P9

* Board:  Silicon Labs EFR32FG25 902-928 MHz 14 dBm Radio Board (BRD4270B) + Wireless Starter Kit Mainboard
    * Device: EFR32FG25B222F1920IM56
        * PA08 -  EUSART0 transmit, Expansion Header Pin 12, WSTK P9

* Board:  Silicon Labs EFR32MG26 2.4 GHz 20 dBm Radio Board (BRD4117A) + Wireless Starter Kit Mainboard
    * Device: EFR32MG26B420F3200IM48
        * PA08 -  EUSART0 transmit, Expansion Header Pin 12, WSTK P9

* Board:  Silicon Labs EFR32xG27 2.4 GHz 8 dBm Buck DCDC Radio Board (BRD4194A) + Wireless Starter Kit Mainboard
    * Device: EFR32MG27C140F768IM40
        * PA05 -  EUSART0 transmit, Expansion Header Pin 12, WSTK P1

* Board:  Silicon Labs EFR32xG28 868/915 MHz +14 dBm + 2.4 GHz +10 dBm Radio Board (BRD4400C) + Wireless Starter Kit Mainboard
    * Device: EFR32ZG28B312F1024IM68
        * PA08 -  EUSART0 transmit, WSTK P28

* Board:  Silicon Labs EFR32xG29 Wireless 2.4 GHz 8 dBm Buck Radio Board (BRD4412A) + Wireless Starter Kit Mainboard
    * Device: EFR32MG29B140F1024IM40
        * PA05 -  EUSART0 transmit, Expansion Header Pin 12, WSTK P9
# Peripheral Examples - EUSART Asynchronous Polled #

## Summary ##

This project demonstrates polled operation of the EUSART in asynchronous
mode. EUSART0 is configured for asynchronous operation at 115200 baud
with 8 data bits, no parity, and one stop bit `115200N81`.  The main loop
waits until 80 characters or a carriage return are received and then echos
these back to the user.

## Peripherals used ##

* EUSART0
* GPIO

## How to Test ##

1. Build the project and download to the Starter Kit.
2. Open a terminal program and configure it for 115200N81 operation on the
   "JLink CDC UART Port" that is provided by the board controller on the
   Starter Kit mainboard.
3. Type some characters in the terminal program (they will not show) and
   press Enter to have the MCU echo them.

Alternatively, the example may be tested with a USB-to-serial converter,
such as the Silicon Labs CP2102N-EK.  Refer to the list below for the
mapping of EUSART signals to Expansion (EXP) Header pins.
    
## Hardware & Connections ##

* Board: Silicon Labs SixG301 Radio Board (BRD4407A) + Wireless Pro Kit Mainboard
    * Device: SIMG301M104LIL
        * PB00 - VCOM RX, Lower Breakout Header F7
        * PB02 - VCOM TX, Lower Breakout Header F6
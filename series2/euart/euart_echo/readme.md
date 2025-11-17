# Peripheral Examples - EUART Echo

## Summary

This project demonstrates interrupt-driven operation of the EUART.  EUART0 is 
configured for asynchronous operation at 115200 baud with 8 data bits,  no 
parity, and one stop bit (115200N81). The main loop waits until 80 characters 
or a carriage return are received and then echos these back to the user.

## Peripherals Used

* GPIO
* EUART0 - 115200 baud, 8-N-1 (8 data bits, no parity, one stop bit)

The CMU is used indirectly via the `EUSART_InitHf()` function to calculate the
divisor necessary to derive the desired baud rate.

## How To Test

1. Build the project and download to the Starter Kit.
2. Open a terminal program and configure it for 115200N81 operation on the
   serial port assigned on the "JLink CDC UART Port" that is provided by the
   board controller on the Starter Kit mainboard.
3. Type some characters in the terminal program (characters may/may not display 
   depending on terminal settings) and press enter to have the MCU echo them.

Alternatively, the example may be tested with a USB-to-serial converter,
such as the Silicon Labs CP2102N-EK.  Refer to the list below for the
mapping of EUSART signals to Expansion (EXP) Header pins.

## Hardware & Connections

* Board:  Silicon Labs EFR32xG22 Radio Board (BRD4182A) + 
        Wireless Starter Kit Mainboard
  * Device: EFR32MG22A224F512IM40
    * PA05 - EUART0_TX - Expansion Header Pin 12, WSTK P9
    * PA06 - EUART0_RX - Expansion Header Pin 14, WSTK P11

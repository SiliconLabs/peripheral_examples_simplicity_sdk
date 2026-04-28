# Peripheral Examples - EUSART Asynchronous Polled #

## Summary ##

This project demonstrates polled operation of the EUSART in asynchronous
mode.  EUSART0 is configured for asynchronous operation at 115200 baud
with 8 data bits, no parity, and one stop bit (115200N81).  The main loop
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

* Board:  Silicon Labs EFR32ZG23 868-915 MHz 14 dBm Radio Board (BRD4204D) + Wireless Starter Kit Mainboard
    * Device: EFR32ZG23B010F512IM48
        * PA08 -  EUSART0 transmit, Expansion Header Pin 12, WSTK P9
        * PA09 -  EUSART0 receive,  Expansion Header Pin 14, WSTK P11

* Board:  Silicon Labs EFR32MG24 2.4 GHz 10 dBm Radio Board (BRD4186C) + Wireless Starter Kit Mainboard
    * Device: EFR32MG24B210F1536IM48
        * PA08 -  EUSART0 transmit, Expansion Header Pin 12, WSTK P9
        * PA09 -  EUSART0 receive,  Expansion Header Pin 14, WSTK P11

* Board:  Silicon Labs EFR32FG25 902-928 MHz 14 dBm Radio Board (BRD4270B) + Wireless Starter Kit Mainboard
    * Device: EFR32FG25B222F1920IM56
        * PA08 -  EUSART0 transmit, Expansion Header Pin 12, WSTK P9
        * PA09 -  EUSART0 receive,  Expansion Header Pin 14, WSTK P11

* Board:  Silicon Labs EFR32MG26 2.4 GHz 20 dBm Radio Board (BRD4117A) + Wireless Starter Kit Mainboard
    * Device: EFR32MG26B420F3200IM48
        * PA08 -  EUSART0 transmit, Expansion Header Pin 12, WSTK P9
        * PA09 -  EUSART0 receive,  Expansion Header Pin 14, WSTK P11

* Board:  Silicon Labs EFR32xG27 2.4 GHz 8 dBm Buck DCDC Radio Board (BRD4194A) + Wireless Starter Kit Mainboard
    * Device: EFR32MG27C140F768IM40
        * PA05 -  EUSART0 transmit, Expansion Header Pin 12, WSTK P1
        * PA06 -  EUSART0 receive, Expansion Header Pin 14, WSTK P3

* Board:  Silicon Labs EFR32xG28 868/915 MHz +14 dBm + 2.4 GHz +10 dBm Radio Board (BRD4400C) + Wireless Starter Kit Mainboard
    * Device: EFR32ZG28B312F1024IM68
        * PA08 -  EUSART0 transmit, WSTK P28
        * PA09 -  EUSART0 receive,  WSTK P30

* Board:  Silicon Labs EFR32xG29 Wireless 2.4 GHz 8 dBm Buck Radio Board (BRD4412A) + Wireless Starter Kit Mainboard
    * Device: EFR32MG29B140F1024IM40
        * PA05 -  EUSART0 transmit, Expansion Header Pin 12, WSTK P9
        * PA06 -  EUSART0 receive,  Expansion Header Pin 14, WSTK P11

* Board:  Silicon Labs EFR32FG2D 868-915 MHz 14 dBm Radio Board (BRD4277A) + Wireless Starter Kit Mainboard
    * Device: EFR32FG2DB010F512IM48
        * PA08 -  EUSART0 transmit, Expansion Header Pin 12, WSTK P9
        * PA09 -  EUSART0 receive,  Expansion Header Pin 14, WSTK P11

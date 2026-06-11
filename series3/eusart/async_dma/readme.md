# Peripheral Examples - EUSART Asynchronous DMA #

## Summary ##

This project demonstrates low-frequency operation of EUSART0 using LDMA to
receive inbound data and transmit outbound data.

This example effectively demonstrates half-duplex operation by
first receiving a set number of characters and then echoing these back,
there is nothing that prevents the LDMA from simultaneously servicing
transmit and receive data.

## Peripherals used ##

* EUSART0
    * 9600 baudrate
    * 8-N-1 - 8 data bits, no parity, one stop bit
* LDMA
* LFRCO

## How to Test ##

1. Build the project and download to the Starter Kit.
2. Open a terminal program and configure it for `9600N81` operation on the
   "JLink CDC UART Port" that is provided by the board controller on the
   Starter Kit mainboard.
3. Type 10 characters (will not show if it is less than 10) in the terminal
   program and watch the characters echo back from the terminal.

> NOTE: The WSTK board controller defaults to `115200` baud for the virtual
COM port (JLink CDC UART Port).  Follow these steps to switch to `9600`
baud:
1. While in the Simplicity Studio IDE, navigate to **Tools → Device Manager Tool**.
2. Select your board, click **Configure**, then open **Terminal → Admin**.
   The WSTK> prompt will appear.
3. To change the virtual COM (VCOM) port baud rate, enter the following
   command:
   
   `serial vcom config speed 9600`

Alternatively, the example may be tested with a USB-to-serial converter,
such as the Silicon Labs CP2102N-EK.  Refer to the list below for the
mapping of USART signals to Expansion (EXP) Header pins.
    
## Hardware & Connections ##

* Board: Silicon Labs SixG301 Radio Board (BRD4407A) + Wireless Pro Kit Mainboard
    * Device: SIMG301M104LIL
        * PB00 - VCOM RX, Lower Breakout Header F7
        * PB02 - VCOM TX, Lower Breakout Header F6
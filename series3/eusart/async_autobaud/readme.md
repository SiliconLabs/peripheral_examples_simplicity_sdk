# Peripheral Examples - EUSART Asynchronous Automatic Baud Detection #

## Summary ##

This project demonstrates interrupt-driven EUSART operation in
asynchronous mode with automatic baud rate detection. EUSART0 is
configured for 8 data bits, no parity, and one stop bit. Setting
`EUSART0->CFG0 |= EUSART_CFG0_AUTOBAUDEN` to enable the auto baud feature.

The main loop waits for reception of a frame containing the SYNC byte
(`0x55, which is the ASCII character 'U'`) in order to set the baud rate.
After this, it waits until 80 characters or a carriage return are
received and echos these back to the user.

> Note: The auto baud detection logic must receive a frame that includes
a proper start bit followed by the SYNC character and then the correct
number of stop bits in order to function correctly.  It has no ability
to discard false SYNC characters nor any kind of timeout and restart
mechanism.

Consequently, the auto baud logic will simply measure the baud times for
whatever sequence of high-to-low and low-to-high transitions it receives
and set the EUSART clock divider accordingly, even if this is not the
actual baud rate of the data being transmitted.

In this example, if the first character typed in the terminal window is
not the letter `'U'` (ASCII character 0x55), the auto baud logic will
calculate some random baud rate based on the average length of the RX
line transitions and set the EUSART clock divider accordingly.  Because
this will not be the same as the baud rate for which the terminal
program is configured, the example will not function correctly.

Proper auto baud operation relies upon a well-defined serial protocol
that governs the sequence of frames received.  The LIN protocol, for
example, makes use of a SYNC frame that is preceded by an extended
duration break character.  This break sequence is used to alert nodes
on the network that a packet has been started and that a SYNC character
will arrive next so that all receivers can auto baud.  If a receiver
were to somehow lose network synchronization, the break character is
what allows it to resync. 

Apart from using a protocol like LIN, that provides inherent
protection for proper auto baud operation, another option would be to
include a timeout mechanism that is triggered by the falling edge
associated with the SYNC frame's start bit.  On EFM32/EFR32 devices, this
could consist of using a TIMER input capture to start a counter in
response to the start bit falling edge.  If the counter were to overflow
or otherwise reach some predetermined value before the auto baud done
interrupt is requested, firmware could then wait for the EUSART receiver
to return to the idle state and re-enable auto baud detection. 

## Peripherals used ##

* EUSART0
* GPIO

## How to Test ##

1. Build the project and download it to the Starter Kit.
2. Open a terminal program and configure it for `115200N81` operation on the
   "JLink CDC UART Port" that is provided by the board controller on the
   Starter Kit mainboard.
3. Set a breakpoint at the end of the `EUSARTn_RX_IRQHandler` and run the
   example project.
4. Pause the program and observe the `EUSARTn_CLKDIV` value after
   initalization,  which will be 0.  Resume execution and type the `'U'`
   character in the terminal program.
5. At the breakpoint, notice that the `EUSARTn_CLKDIV` value has been
   updated with a value that reflects the baud rate set in the terminal.
   Remove the breakpoint and resume execution.
5. Type some characters in the terminal program (they will not show) and
   press Enter to have the MCU echo them.

> NOTE: The WSTK board controller defaults to 115200 baud for the virtual
COM port (JLink CDC UART Port).  Follow these steps to use a different
baud rate:
1. While in the Simplicity Studio IDE, navigate to **Tools → Device Manager Tool**.
2. Select your board, click **Configure**, then open **Terminal → Admin**.
   The WSTK> prompt will appear.
3. To change the virtual COM (VCOM) port baud rate, enter the following
   command:

   `serial vcom config speed <desired baud rate>`

Alternatively, the example may be tested with a USB-to-serial converter,
such as the Silicon Labs CP2102N-EK.  Refer to the list below for the
mapping of USART signals to Expansion (EXP) Header pins.
    
## Hardware & Connections ##

* Board: Silicon Labs SixG301 Radio Board (BRD4407A) + Wireless Pro Kit Mainboard
    * Device: SIMG301M104LIL
        * PB00 - VCOM RX, Lower Breakout Header F7
        * PB02 - VCOM TX, Lower Breakout Header F6
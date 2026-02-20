# Peripheral Examples - LDMA Ping Pong #

## Summary ##

This example is based on the EFR32 Series 3 Reference Manual:
Chapter: LDMA
Section: Examples
Subsection: Example #7

Software requests LDMA ping-pong transfers. A universal source array is
transfered first to the "ping_buffer" array, then to the "pong_buffer", then back
to ping, etc. After each transfer, there is an interrupt that increments the
elements of the source buffer, then requests the next transfer.  
In this way, you should see "11111111" transfered to ping, then "22222222" 
transfered to pong, etc.

## Peripherals used ##

* CMU
  * HFRCODPLL @ 38 MHz
* LDMA
  * Channel 0
  
## How to Test ##

1. Update the kit's firmware from the Simplicity Launcher (if necessary)
2. Build the project and download to the Pro Kit
3. Open Simplicity Debugger and add "ping_buffer" and "pong_buffer" to the
   Expressions window
4. Add a breakpoint at the beginning of ldma_callback()
5. Run the debugger. It should halt inside the callback subroutine with the
   first descriptor complete (this can be seen in the Expressions window)
   and LED0 will toggle
6. Resume the program. The debugger should halt inside the callback subroutine
   again, after the next descriptor has completed.

## Hardware & Connections ##

* Board: Silicon Labs SixG301 Radio Board (BRD4407A) + Wireless Pro Kit Mainboard
    * Device: SIMG301M104LIL
        * PD03 - LED0

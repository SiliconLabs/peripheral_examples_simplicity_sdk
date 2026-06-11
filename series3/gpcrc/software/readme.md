# Peripheral Examples - GPCRC Software #

## Summary ##

This project demonstrates the GPCRC used to check an array of 32-bit data
using the IEEE 802.3 polynomial standard. The GPCRC conversions are initiated
by software.

Functionality is included to show how one could perform this conversion 
without the GPCRC. To note, the pure software method requires more memory and
can take 3-5 times longer to compute for single words, and the gap grows the more
consecutive transfers you do.

## Peripherals used ##

* GPCRC - IEEE 802.3 poly standard

## How to Test ##

1. Open Simplicity Studio and update the kit's firmware using **Device Manager Tool**(if necessary)
2. Build the project and download to the Starter Kit
3. Start a debug session in the IDE and add "results" to the Watch/Variables window
4. Run the debugger, then pause it. You should notice that "results" is
   filled with "checked" values

## Hardware & Connections ##

* Board: Silicon Labs SixG301 Radio Board (BRD4407A) + Wireless Pro Kit Mainboard
    * Device: SIMG301M104LIL


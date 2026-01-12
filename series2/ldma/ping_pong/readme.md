# Peripheral Examples - LDMA Ping Pong #

## Summary ##

This example is based on the EFR32 Series 2 Reference Manual:
Chapter: LDMA
Section: Examples
Subsection: Example #7

Software requests LDMA ping-pong transfers. A universal source array is
transfered first to the "pingBuffer" array, then to the "pongBuffer", then back
to ping, etc. After each transfer, there is an interrupt that increments the
elements of the source buffer, then requests the next transfer.  
In this way, you should see "11111111" transfered to ping, then "22222222" 
transfered to pong, etc.

## Peripherals used ##

* CMU
  * HFRCODPLL @ 19 MHz
* LDMA
  * Channel 0
  
## How to Test ##

1. Update the kit's firmware from the Simplicity Launcher (if necessary)
2. Build the project and download to the Starter Kit
3. Open Simplicity Debugger and add "pingBuffer" and "pongBuffer" to the
   Expressions window
4. Add a breakpoint at the beginning of ldmaCallback()
5. Run the debugger. It should halt inside the callback subroutine with the
   first descriptor complete (this can be seen in the Expressions window)
   and LED0 will toggle
6. Resume the program. The debugger should halt inside the callback subroutine
   again, after the next descriptor has completed.

## Hardware & Connections ##

* Board: Silicon Labs EFR32xG21 2.4 GHz 10 dBm Board (BRD4181A) + Wireless Starter Kit Mainboard
    * Device: EFR32MG21A010F1024IM32
        * PB00 - LED0, WSTK EXP Header 11

* Board: Silicon Labs EFR32xG22 2.4 GHz 6 dBm Board (BRD4182A) + Wireless Starter Kit Mainboard
    * Device: EFR32MG22A224F512IM40
        * PD02 - LED0, WSTK EXP Header 11
        
* Board: Silicon Labs EFR32xG23 Radio Board (BRD4204D) + Wireless Starter Kit Mainboard
    * Device: EFR32ZG23B010F512IM48
        * PB02 - LED0, WSTK P19

* Board: Silicon Labs EFR32xG24 Radio Board (BRD4186C) + Wireless Starter Kit Mainboard
    * Device: EFR32MG24B210F1536IM48
        * PB02 - LED0, WSTK P19

* Board: Silicon Labs EFR32xG25 Radio Board (BRD4270B) + Wireless Starter Kit Mainboard
    * Device: EFR32FG25B222F1920IM56
        * PC06 - LED0, WSTK P27

* Board: Silicon Labs EFR32xG26 2.4 GHz 20 dBm Radio Board (BRD4117A) + Wireless Starter Kit Mainboard
    * Device: EFR32MG26B420F3200IM48
        * PB02 - LED0, WSTK P19

* Board: Silicon Labs EFR32xG27 Buck Radio Board (BRD4194A) + Wireless Starter Kit Mainboard
    * Device: EFR32MG27C140F768IM40
        * PB00 - LED0, WSTK EXP Header 7

* Board: Silicon Labs EFR32xG28 Radio Board (BRD4400C) + Wireless Starter Kit Mainboard
    * Device: EFR32ZG28B312F1024IM68
        * PB02 - LED0, WSTK P19

* Board: Silicon Labs EFR32xG29 Wireless 2.4 GHz 8 dBm Buck Radio Board (BRD4412A) + Wireless Starter Kit Mainboard
    * Device: EFR32MG29B140F1024IM40
        * PB00 - LED0, WSTK EXP Header 7
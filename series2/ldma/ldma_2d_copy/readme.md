# Peripheral Examples - LDMA 2D Copy #

## Summary ##

This example is based on the EFR32 Series 2 Reference Manual:
Chapter: LDMA
Section: Examples
Subsection: Example #6

In this example, the LDMA transfers a submatrix from one software matrix to
another. The source buffer is numbered from 0 to 
(BUFFER_2D_WIDTH * BUFFER_2D_HEIGHT - 1) in row major order. Row major order was
chosen so that the debugger will show the matrix as expected.

The submatrix moved will start at (SRC_COL_INDEX, SRC_ROW_INDEX) in the source
matrix, and be moved to (DST_COL_INDEX, DST_ROW_INDEX) in the destination
matrix. The width and height of this submatrix is defined by TRANSFER_HEIGHT
and TRANSFER_WIDTH.

With the sample values given, the LDMA will transfer
10 11 12
20 21 22
30 31 32
40 41 42
from the source matrix to index (2, 1) of the destination matrix.

## Peripherals used ##

* CMU
  * HFRCODPLL @ 19 MHz
* LDMA
  * Channel 0

## How to Test ##

1. Update the kit's firmware from the Simplicity Launcher (if necessary)
2. Build the project and download to the Starter Kit
3. Open Simplicity Debugger and add "dst2d" to the Expressions window
4. Add a breakpoint to the beginning of ldmaCallback()
5. Run the debugger. It should halt inside the callback subroutine
6. You can expand the "dst2d" variable in the Expressions window to see that the
   submatrix has been successfully transferred. LED0 will toggle

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
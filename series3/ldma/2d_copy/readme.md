# Peripheral Examples - LDMA 2D Copy #

## Summary ##

This example is based on the EFR32 Series 3 Reference Manual:
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
  * HFRCODPLL @ 38 MHz
* LDMA
  * Channel 0

## How to Test ##

1. Open Simplicity Studio and update the kit's firmware using **Device Manager Tool**(if necessary)
2. Build the project and download to the Pro Kit
3. Start a debug session in the IDE and add "dst2d" to the Watch/Variables window
4. Add a breakpoint to the beginning of ldma_callback()
5. Run the debugger. It should halt inside the callback subroutine
6. You can expand the "dst2d" variable in the Watch/Variables window to see that the
   submatrix has been successfully transferred. LED0 will toggle

## Hardware & Connections ##

* Board: Silicon Labs SixG301 Radio Board (BRD4407A) + Wireless Pro Kit Mainboard
    * Device: SIMG301M104LIL
        * PD03 - LED0

# Peripheral Examples - LDMA Linked List #

## Summary ##

This example is based on the EFR32 Series 3 Reference Manual:
Chapter: LDMA
Section: Examples
Subsection: Example #2

Software requests a LDMA transfer on a linked list of 4 descriptors, 
which each move 32 halfwords of memory into a destination buffer.  
There are interrupts triggered after the second and fourth descriptors.

## Peripherals used ##

* CMU
  * HFRCODPLL @ 38 MHz
* LDMA
  * Channel 0

## How to Test ##

1. Open Simplicity Studio and update the kit's firmware using **Device Manager Tool**(if necessary)
2. Build the project and download to the Pro Kit
3. Start a debug session in the IDE and add "dest_buffer" to the Watch/Variables window
4. Add a breakpoint at the beginning of ldma_callback()
5. Run the debugger. It should halt inside the callback subroutine with the
   first two descriptors complete (this can be seen in the Watch/Variables window).
   LED0 will toggle
6. Resume the debugger. It should halt inside the callback subroutine again
   after all the descriptors have completed.

## Hardware & Connections ##

* Board: Silicon Labs SixG301 Radio Board (BRD4407A) + Wireless Pro Kit Mainboard
    * Device: SIMG301M104LIL
        * PD03 - LED0

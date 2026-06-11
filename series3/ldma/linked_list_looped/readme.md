# Peripheral Examples - LDMA Linked List Looped #

## Summary ##

This example is based on the EFR32 Series 3 Reference Manual:
Chapter: LDMA
Section: Examples
Subsection: Example #4

In this example, the LDMA transfers 4 character strings from one software
array to another. It switches between A and B four times, then transfers C,
like so:
A, B, A, B, A, B, A, B, C

The transfer is requested by software at the end of the LDMA initialization.

If you wanted to have the transfers run automatically without interrupts, 
you can remove from init_ldma() the following lines:

    // Enable interrupts
    descLink[0].xfer.done_ifs = true;
    descLink[1].xfer.done_ifs = true;
    descLink[2].xfer.done_ifs = true;
    
    // Disable automatic triggers
    descLink[0].xfer.struct_req = false;
    descLink[1].xfer.struct_req = false;
    descLink[2].xfer.struct_req = false;
    
    ... (Do not remove the lines between these)
    
    // Request first transfer
    LDMA0->SWREQ |= LDMA_CH_MASK;
  
Also remove from ldma_callback() the following lines:

    // Request next transfer
    LDMA0->SWREQ |= LDMA_CH_MASK;

## Peripherals used ##

* CMU
  * HFRCODPLL @ 38 MHz
* LDMA
  * Channel 0
  
## How to Test ##

1. Open Simplicity Studio and update the kit's firmware using **Device Manager Tool**(if necessary)
2. Build the project and download to the Pro Kit
3. Start a debug session in the IDE and add "dest_buffer" to the Watch/Variables window
4. Add a breakpoint at the beginning of LDMA_IRQHandler()
5. Run the debugger. It should halt inside the interrupt subroutine with the
   first descriptor complete (this can be seen in the Watch/Variables window)
6. Resume the program. The debugger should halt inside the interrupt subroutine
   again, after the next descriptor has completed. You can do this 7 more times,
   then the LDMA tranfer will be completed. LED0 will toggle with every 
   transfer

## Hardware & Connections ##

* Board: Silicon Labs SixG301 Radio Board (BRD4407A) + Wireless Pro Kit Mainboard
    * Device: SIMG301M104LIL
        * PD03 - LED0

# Peripheral Examples - LDMA Single Looped #

## Summary ##

This example is based on the EFR32 Series 3 Reference Manual:
Chapter: LDMA
Section: Examples
Subsection: Example #3

In this example, the LDMA transfers 16 words from one software array to another
in 4 sets of 4 words. The transfer is requested by software at the end of the
LDMA initialization.

If you wanted to have the transfers run automatically without interrupts,
you can remove from init_ldma() the following lines:

    descLink.xfer.done_ifs     = true;                     // Enable interrupts
    descLink.xfer.struct_req   = false;                    // Disable auto-requests
    
    ... (Do not remove the lines between these)
    
    // Send software request
    LDMA0->SWREQ |= LDMA_CH_MASK;
  
Also remove from ldma_callback() the following lines:

    // Start next Transfer
    LDMA0->SWREQ |= LDMA_CH_MASK;

## Peripherals used ##

* CMU
  * HFRCODPLL @ 38 MHz
* LDMA
  * Channel 0

## How to Test ##

1. Update the kit's firmware from the Simplicity Launcher (if necessary)
2. Build the project and download to the Pro Kit
3. Open Simplicity Debugger and add "dest_buffer" to the Expressions window
4. Add a breakpoint at the beginning of ldma_callback()
5. Run the debugger. It should halt inside the callback subroutine with the
   first descriptor complete (this can be seen in the Expressions window).
6. Resume the program. The debugger should halt inside the callback subroutine
   again, after the next descriptor has completed. You can do this 2 more times,
   then the LDMA transfer will be completed. LED0 will toggle with every 
   transfer

## Hardware & Connections ##

* Board: Silicon Labs SixG301 Radio Board (BRD4407A) + Wireless Pro Kit Mainboard
    * Device: SIMG301M104LIL
        * PD03 - LED0

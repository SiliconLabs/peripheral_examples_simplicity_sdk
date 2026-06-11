# Peripheral Examples - LDMA Scatter Gather #

## Summary ##

This example is based on the EFR32 Series 3 Reference Manual:
Chapter: LDMA
Section: Examples
Subsection: Example #8

Software requests a LDMA transfer on a linked list of 4 descriptors to "scatter"
one large array into 4 smaller arrays. Each descriptor moves 8 halfwords of
memory into a destination buffer. An interrupt triggers after the completion
of the "scatter".

The scattering is done out of order so that the functional difference between 
this and a single descriptor are more clear. The source array (source_buffer)
contains values from 0 to (BUFFER_SIZE * 4 - 1), and they get transfered to 
arrays of size BUFFER_SIZE starting with dest_buffer[3], then dest_buffer[2], etc.  

Therefore, the result dest_buffer matrix will look like:
dest_buffer[0] = 24, 25, 26, 27, 28, 29, 30, 31
dest_buffer[1] = 16, 17, 18, 19, 20, 21, 22, 23
dest_buffer[2] =  8,  9, 10, 11, 12, 13, 14, 15
dest_buffer[3] =  0,  1,  2,  3,  4,  5,  6,  7

Then software requests a LDMA transfer on a linked list of 4 descriptors to
"gather" 4 smaller arrays into one large array. Each descriptor moves 8
halfwords of memory into one destination buffer. An interrupt triggers after
the completion of the "gather".

The gathering is done out of order so that the functional difference between 
this and a single descriptor are more clear.  The dest_buffer[n] becomes the 
source arrays and they get transfered to source_buffer starting with dest_buffer[3],
then dest_buffer[2], etc.

Therefore, the result source_buffer will look like:
source_buffer =  0,  1,  2,  3,  4,  5,  6,  7,
          =  8,  9, 10, 11, 12, 13, 14, 15,
          = 16, 17, 18, 19, 20, 21, 22, 23,
          = 24, 25, 26, 27, 28, 29, 30, 31

## Peripherals used ##

* CMU
  * HFRCODPLL @ 38 MHz
* LDMA
  * Channel 0
  
## How to Test ##

1. Open Simplicity Studio and update the kit's firmware using **Device Manager Tool**(if necessary)
2. Build the project and download to the Pro Kit
3. Start a debug session in the IDE and add "source_buffer" and "dest_buffer" to the
   Watch/Variables window
4. Add a breakpoint inside ldma_callback()
5. Run the debugger. It should halt inside the callback subroutine with values in
   dest_buffer that match the values described above. LED0 will toggle
6. Add a breakpoint below inside init_ldma_gather()
   LDMA_TransferCfg_t memTransferTx = LDMA_TRANSFER_CFG_MEMORY();
7. Run the debugger. It should halt inside init_ldma_gather() and source_buffer is
   clear to 0
8. Run the debugger. It should halt inside the callback subroutine again with
   values in source_buffer that match the values described above

## Hardware & Connections ##

* Board: Silicon Labs SixG301 Radio Board (BRD4407A) + Wireless Pro Kit Mainboard
    * Device: SIMG301M104LIL
        * PD03 - LED0

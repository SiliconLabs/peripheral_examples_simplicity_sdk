# Peripheral Examples - LDMA Scatter Gather #

## Summary ##

This example is based on the EFR32 Series 2 Reference Manual:
Chapter: LDMA
Section: Examples
Subsection: Example #8

Software requests a LDMA transfer on a linked list of 4 descriptors to "scatter"
one large array into 4 smaller arrays. Each descriptor moves 8 halfwords of
memory into a destination buffer. An interrupt triggers after the completion
of the "scatter".

The scattering is done out of order so that the functional difference between 
this and a single descriptor are more clear. The source array (srcBuffer)
contains values from 0 to (BUFFER_SIZE * 4 - 1), and they get transfered to 
arrays of size BUFFER_SIZE starting with dstBuffer[3], then dstBuffer[2], etc.  

Therefore, the result dstBuffer matrix will look like:
dstBuffer[0] = 24, 25, 26, 27, 28, 29, 30, 31
dstBuffer[1] = 16, 17, 18, 19, 20, 21, 22, 23
dstBuffer[2] =  8,  9, 10, 11, 12, 13, 14, 15
dstBuffer[3] =  0,  1,  2,  3,  4,  5,  6,  7

Then software requests a LDMA transfer on a linked list of 4 descriptors to
"gather" 4 smaller arrays into one large array. Each descriptor moves 8
halfwords of memory into one destination buffer. An interrupt triggers after
the completion of the "gather".

The gathering is done out of order so that the functional difference between 
this and a single descriptor are more clear.  The dstBuffer[n] becomes the 
source arrays and they get transfered to srcBuffer starting with dstBuffer[3],
then dstBuffer[2], etc.

Therefore, the result srcBuffer will look like:
srcBuffer =  0,  1,  2,  3,  4,  5,  6,  7,
          =  8,  9, 10, 11, 12, 13, 14, 15,
          = 16, 17, 18, 19, 20, 21, 22, 23,
          = 24, 25, 26, 27, 28, 29, 30, 31
		  
## Peripherals used ##

* CMU
  * HFRCODPLL @ 19 MHz
* LDMA
  * Channel 0
  
## How to Test ##

1. Open Simplicity Studio and update the kit's firmware using **Device Manager Tool**(if necessary)
2. Build the project and download to the Starter Kit
3. Start a debug session in the IDE and add "srcBuffer" and "dstBuffer" to the
   Watch/Variables window
4. Add a breakpoint inside ldmaCallback()
5. Run the debugger. It should halt inside the callback subroutine with values in
   dstBuffer that match the values described above. LED0 will toggle
6. Add a breakpoint below inside initLdmaGather()
   LDMA_TransferCfg_t memTransferTx = LDMA_TRANSFER_CFG_MEMORY();
7. Run the debugger. It should halt inside initLdmaGather() and srcBuffer is
   clear to 0
8. Run the debugger. It should halt inside the callback subroutine again with
   values in srcBuffer that match the values described above

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

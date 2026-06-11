# Peripheral Examples - LDMA Single Button #

## Summary ##

This example is based on the EFR32 Series 3 Reference Manual:
Chapter: LDMA
Section: Examples
Subsection: Example #1

In this example, the LDMA transfers 128 halfwords (256 bytes) from one software
array to another. The transfer is requested by software (#define USE_GPIO_PRS 0)
or a button press on Push Button 1 (#define USE_GPIO_PRS 1).

## Peripherals used ##

* CMU
  * HFRCODPLL @ 38 MHz
* LDMA
  * Channel 0
* PRS
  * Channel 2, Push Button PB1
* GPIO

## How to Test ##

1. Open Simplicity Studio and update the kit's firmware using **Device Manager Tool**(if necessary)
2. Build the project and download to the Pro Kit
3. Start a debug session in the IDE and add "dest_buffer" to the Watch/Variables window
4. Add a breakpoint at the beginning of ldma_callback()
5. The transfer is requested by software (#define USE_GPIO_PRS 0):
   Run the debugger. It should halt inside the callback subroutine with the
   transfer complete (this can be seen in the Watch/Variables window)
6. The transfer is requested by a button press on PB1 (#define USE_GPIO_PRS 1):
   Run the debugger. After you press the PB1, the debugger should halt inside
   the callback subroutine with the transfer complete (this can be seen in the
   Watch/Variables window) and LED0 will toggle

## Hardware & Connections ##

* Board: Silicon Labs SixG301 Radio Board (BRD4407A) + Wireless Pro Kit Mainboard
    * Device: SIMG301M104LIL
        * PB01 - Push Button 0
        * PD02 - Push Button 1
        * PD03 - LED0

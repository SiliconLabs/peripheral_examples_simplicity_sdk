# Peripheral Examples - LDMA Interchannel Synchronization #

## Summary ##

This example is based on the EFR32 Series 3 Reference Manual:
Chapter: LDMA
Section: Examples
Subsection: Example #5

In this example, the LDMA synchronizes across 2 Channels. Each channel starts
off on a different button press, and transfers 4-character strings from one 
software array to another.

If you press PB0, "AAaa" will be transfered to dest_buffer.
If you press PB1, "YYyy" will be transfered to dest_buffer.
After you press the second button (regardless of the order), 
"CCcc" will be transferred to dest_buffer.
  
## Peripherals used ##

* CMU
  * HFRCODPLL @ 38 MHz
* LDMA
  * Channel 0
* PRS
  * Channel 1, Push Button PB0
  * Channel 2, Push Button PB1
* GPIO

## How to Test ##

1. Update the kit's firmware from the Simplicity Launcher (if necessary)
2. Build the project and download to the Pro Kit
3. Open Simplicity Debugger and add "dest_buffer" to the Expressions window
4. Run the debugger
5. Press either PB0 or PB1, and observe the value in dest_buffer before and after
   each button press change according to the behavior described above. LED0 will
   toggle
   > Note: You will have to pause the debugger to see the value in dest_buffer.

## Hardware & Connections ##

* Board: Silicon Labs SixG301 Radio Board (BRD4407A) + Wireless Pro Kit Mainboard
    * Device: SIMG301M104LIL
        * PB01 - Push Button 0
        * PD02 - Push Button 1
        * PD03 - LED0

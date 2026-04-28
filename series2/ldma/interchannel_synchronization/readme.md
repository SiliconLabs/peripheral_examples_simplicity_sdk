# Peripheral Examples - LDMA Interchannel Synchronization #

## Summary ##

This example is based on the EFR32 Series 2 Reference Manual:
Chapter: LDMA
Section: Examples
Subsection: Example #5

In this example, the LDMA synchronizes across 2 Channels. Each channel starts
off on a different button press, and transfers 4-character strings from one 
software array to another.

If you press PB0, "AAaa" will be transfered to dstBuffer.
If you press PB1, "YYyy" will be transfered to dstBuffer.
After you press the second button (regardless of the order), 
"CCcc" will be transferred to dstBuffer.
  
## Peripherals used ##

* CMU
  * HFRCODPLL @ 19 MHz
* LDMA
  * Channel 0
* PRS
  * Channel 1, Push Button PB0
  * Channel 2, Push Button PB1
* GPIO

## How to Test ##

1. Open Simplicity Studio and update the kit's firmware using **Device Manager Tool**(if necessary)
2. Build the project and download to the Starter Kit
3. Start a debug session in the IDE and add "dstBuffer" to the Watch/Variables window
4. Run the debugger
5. Press either PB0 or PB1, and observe the value in dstBuffer before and after
   each button press change according to the behavior described above. LED0 will
   toggle
   > Note: You will have to pause the debugger to see the value in dstBuffer.

## Hardware & Connections ##

* Board: Silicon Labs EFR32xG21 2.4 GHz 10 dBm Board (BRD4181A) + Wireless Starter Kit Mainboard
    * Device: EFR32MG21A010F1024IM32
        * PD02 - Push Button PB0
		* PD03 - Push Button PB1
		* PB00 - LED0, WSTK EXP Header 11
		
* Board: Silicon Labs EFR32xG22 2.4 GHz 6 dBm Board (BRD4182A) + Wireless Starter Kit Mainboard
    * Device: EFR32MG22A224F512IM40
        * PB00 - Push Button PB0
        * PB01 - Push Button PB1
        * PD02 - LED0, WSTK EXP Header 11
		
* Board: Silicon Labs EFR32xG23 Radio Board (BRD4204D) + Wireless Starter Kit Mainboard
    * Device: EFR32ZG23B010F512IM48
        * PB01 - Push Button PB0, WSTK Pin 17
        * PB03 - Push Button PB1, WSTK Pin 21
        * PB02 - LED0, WSTK P19
		
* Board: Silicon Labs EFR32xG24 Radio Board (BRD4186C) + Wireless Starter Kit Mainboard
    * Device: EFR32MG24B210F1536IM48
        * PB01 - Push Button PB0, WSTK Pin 17
        * PB03 - Push Button PB1, WSTK Pin 21
        * PB02 - LED0, WSTK P19
		
* Board: Silicon Labs EFR32xG25 Radio Board (BRD4270B) + Wireless Starter Kit Mainboard
    * Device: EFR32FG25B222F1920IM56
        * PB00 - Push Button PB0, WSTK Pin 17
        * PB01 - Push Button PB1, WSTK Pin 21
        * PC06 - LED0, WSTK P27
		
* Board: Silicon Labs EFR32xG26 2.4 GHz 20 dBm Radio Board (BRD4117A) + Wireless Starter Kit Mainboard
    * Device: EFR32MG26B420F3200IM48
        * PB01 - Push Button PB0, WSTK Pin 17
        * PB03 - Push Button PB1, WSTK Pin 21
        * PB02 - LED0, WSTK P19
		
* Board: Silicon Labs EFR32xG27 Buck Radio Board (BRD4194A) + Wireless Starter Kit Mainboard
    * Device: EFR32MG27C140F768IM40
        * PB00 - Push Button PB0, WSTK EXP Header Pin 7
        * PB01 - Push Button PB1, WSTK EXP Header Pin 9
        * PB02 - WSTK EXP Header 15
		
* Board: Silicon Labs EFR32xG28 Radio Board (BRD4400C) + Wireless Starter Kit Mainboard
    * Device: EFR32ZG28B312F1024IM68
        * PB01 - Push Button PB0, WSTK Pin 17
        * PB03 - Push Button PB1, WSTK Pin 21
        * PB02 - LED0, WSTK P19
        
* Board: Silicon Labs EFR32xG29 Wireless 2.4 GHz 8 dBm Buck Radio Board (BRD4412A) + Wireless Starter Kit Mainboard
    * Device: EFR32MG29B140F1024IM40
        * PB00 - Push Button PB0, WSTK EXP Header Pin 7
        * PB01 - Push Button PB1, WSTK EXP Header Pin 9
        * PB02 - WSTK EXP Header 15

* Board:  Silicon Labs EFR32FG2D 868-915 MHz 14 dBm Radio Board (BRD4277A) + Wireless Starter Kit Mainboard
    * Device: EFR32FG2DB010F512IM48
        * PB01 - Push Button PB0, WSTK Pin 17
        * PB03 - Push Button PB1, WSTK Pin 21
        * PB02 - LED0, WSTK P19
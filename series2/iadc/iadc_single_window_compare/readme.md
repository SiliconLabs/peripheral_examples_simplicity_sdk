# Peripheral Examples - IADC Single Window Compare #

## Summary ##

This project demonstrates using the IADC peripheral as a window comparator on a
single-ended input. The window comparator is configured to trigger on 
output results which are inside the specified window. WSTK LED1 is toggled on 
each conversion with results within the specified window. The most recent
sample within the window comparison is also stored globally.

> Note: On EFR32xG21 devices, oscillators and clock branches are automatically 
enabled/disabled based on peripheral demand. Therefore, manually enabling 
clocks has no effect on xG21 devices.

## Peripherals Used ##

* CMU
  * HFXO  
* GPIO  
* IADC  
  * 12-bit resolution  
  * Automatic 2's Complement (single-ended = unipolar)  
  * Internal VBGR reference with 0.5x analog gain (1.21V / 0.5 = 2.42V) 
  * IADC and reference kept in warmup mode  
  * Conversions initiated by firmware and triggered continuously  

## How to Test ##

How To Test:
1. Update the kit's firmware from the Simplicity Launcher (if necessary)
2. Build the project and download to the Starter Kit
3. Open the Simplicity Debugger and add "sample" and "singleResult" to the 
   Expressions window
4. Observe GPIO output (WSTK LED1) using an oscilloscope while varying the input
   voltage; GPIO will toggle while the input is within the specified window, and
   hold steady to the last state when the input voltage moves outside the window 
5. Suspend the debugger, observe the measured voltage in the Expressions Window
   of the most recent conversion within the specified window.
  
## Hardware & Connections ##

* Board:  Silicon Labs EFR32xG21 2.4 GHz 10 dBm Board (BRD4181A) + Wireless Starter Kit Mainboard
    * Device: EFR32MG21A010F1024IM32
        * PA05 - IADC positive differential input, WSTK EXP Header Pin 12
        * PB01 - LED1, WSTK EXP Header Pin 13

* Board: Silicon Labs EFR32xG22 2.4 GHz 6 dBm Radio Board (BRD4182A) + Wireless Starter Kit Mainboard
    * Device: EFR32MG22C224F512IM40
		* PB02 - IADC positive differential input, WSTK EXP Header Pin 15
        * PD03 - LED1, WSTK EXP Header Pin 13

* Board:  Silicon Labs EFR32ZG23 868-915 MHz 14 dBm Radio Board (BRD4204D) + Wireless Starter Kit Mainboard
    * Device: EFR32ZG23B010F512IM48
		* PA05 - IADC positive differential input, WSTK EXP Header Pin 7
        * PD03 - LED1, WSTK P26

* Board:  Silicon Labs EFR32MG24 2.4 GHz 10 dBm Radio Board (BRD4186C) + Wireless Starter Kit Mainboard
    * Device: EFR32MG24B210F1536IM48
		* PA05 - IADC positive differential input, WSTK EXP Header Pin 7
        * PB04 - LED1, WSTK P26

* Board:  Silicon Labs EFR32FG25 902-928 MHz 14 dBm Radio Board (BRD4270B) + Wireless Starter Kit Mainboard
    * Device: EFR32FG25B222F1920IM56
		* PB02 - IADC positive differential input, WSTK EXP Header Pin 15
        * PC07 - LED1, WSTK P26

* Board:  Silicon Labs EFR32MG26 2.4 GHz 20 dBm Radio Board (BRD4117A) + Wireless Starter Kit Mainboard
    * Device: EFR32MG26B420F3200IM48
		* PA08 - IADC positive differential input, WSTK EXP Header Pin 12
        * PB04 - LED1, WSTK P26

* Board:  Silicon Labs EFR32xG27 8 dBm Buck DCDC Radio Board (BRD4194A) + Wireless Starter Kit Mainboard
    * Device: EFR32MG27C140F768IM40
		* PB02 - IADC positive differential input, WSTK EXP Header Pin 15
        * PB01 - LED1, WSTK EXP Header Pin 9

* Board:  Silicon Labs EFR32xG28 868/915 MHz +14 dBm + 2.4 GHz +10 dBm Radio Board (BRD4400C) + Wireless Starter Kit Mainboard
    * Device: EFR32ZG28B312F1024IM68
		* PB04 - IADC positive differential input, WSTK EXP Header Pin 11
        * PD03 - LED1, WSTK P23
        
* Board:  Silicon Labs EFR32xG29 Wireless 2.4 GHz 8 dBm Buck Radio Board (BRD4412A) + Wireless Starter Kit Mainboard
    * Device: EFR32MG29B140F1024IM40
		* PB02 - IADC input, single-ended, WSTK EXP Header Pin 15
		* PB01 - LED1, WSTK EXP Header Pin 9
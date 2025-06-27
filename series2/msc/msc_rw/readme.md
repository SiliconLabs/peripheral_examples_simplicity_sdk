# Peripheral Examples - MSC Read-Write #

## Summary ##

This project demonstrates how to use the user data page of flash memory. The 
value `32` is written to the 4th word in the page, and then the same memory 
location is read back into a variable. This allows users to observe and verify 
read/write flash behavior using a debugger.

> Note: On EFR32xG21 devices, oscillators and clock branches are automatically 
enabled/disabled based on peripheral demand. Therefore, manually enabling 
clocks has no effect on xG21 devices.

## Peripherals used ##

* CMU    - HFRCODPLL @ 19 MHz
* MSC    - Flash memory controller
* SE     - Secure Element (EFR32xG21 only)

## How to Test ##

1. Build the project and download it to the Wireless Starter Kit.
2. Run the program in debug mode.
3. Inspect the value of `Cleared_value` in the debugger.  
   It should read: 4294967295 (0xFFFFFFFF).
4. Inspect the value of `Set_value` in the debugger.  
   It should read: 32.

## Hardware & Connections ##

* Board:  Silicon Labs EFR32xG21 2.4 GHz 10 dBm Board (BRD4181A) + Wireless Starter Kit Mainboard
    * Device: EFR32MG21A010F1024IM32

* Board: Silicon Labs EFR32xG22 2.4 GHz 6 dBm Radio Board (BRD4182A) + Wireless Starter Kit Mainboard
    * Device: EFR32MG22C224F512IM40

* Board:  Silicon Labs EFR32ZG23 868-915 MHz 14 dBm Radio Board (BRD4204D) + Wireless Starter Kit Mainboard
    * Device: EFR32ZG23B010F512IM48

* Board:  Silicon Labs EFR32MG24 2.4 GHz 10 dBm Radio Board (BRD4186C) + Wireless Starter Kit Mainboard
    * Device: EFR32MG24B210F1536IM48

* Board:  Silicon Labs EFR32FG25 902-928 MHz 14 dBm Radio Board (BRD4270B) + Wireless Starter Kit Mainboard
    * Device: EFR32FG25B222F1920IM56

* Board:  Silicon Labs EFR32MG26 2.4 GHz 20 dBm Radio Board (BRD4117A) + Wireless Starter Kit Mainboard
    * Device: EFR32MG26B420F3200IM48

* Board:  Silicon Labs EFR32xG27 8 dBm Buck DCDC Radio Board (BRD4194A) + Wireless Starter Kit Mainboard
    * Device: EFR32MG27C140F768IM40

* Board:  Silicon Labs EFR32xG28 868/915 MHz +14 dBm + 2.4 GHz +10 dBm Radio Board (BRD4400C) + Wireless Starter Kit Mainboard
    * Device: EFR32ZG28B312F1024IM68

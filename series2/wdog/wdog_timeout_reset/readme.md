# Peripheral Examples - WDOG Timeout Reset #

## Summary ##

This project demonstrates the functionality of the WDOG timer. The WDOG is configured to trigger a reset if not fed within 2049 ms. The WDOG is fed every 100 ms and LED0 toggles each time the WDOG is fed. Pressing Push Button 1 (PB1) blocks the application from feeding the WDOG. If PB1 is pressed and held through the WDOG timeout period, the WDOG triggers a reset. The reset source is checked in the beginning of the application. If the latest reset was caused by the WDOG, LED0 turns on and the application stops running.

Note: For the EFR32xG27 Radio Board, PB1 is connected to LED1 via hardware, so LED1 will be on while PB1 is pressed and off while PB1 is not pressed.

## Peripherals used ##

* WDOG - 2049 ms period
* GPIO

## How to Test ##

1. Build the project and download to the Starter Kit
2. LED0 should be blinking
3. Press and hold PB1 for 3 seconds
4. LED0 is now on and not blinking anymore, indicating a WDOG reset

## Hardware & Connections ##

* Board: Silicon Labs EFR32xG21 2.4 GHz 10 dBm Board (BRD4181A) + Wireless Starter Kit Mainboard
    * Device: EFR32MG21A010F1024IM32
        * PB00 - LED0
        * PD03 - Push Button PB1
* Board: Silicon Labs EFR32xG22 2.4 GHz 6 dBm Board (BRD4182A) + Wireless Starter Kit Mainboard
    * Device: EFR32MG22A224F512IM40
        * PD02 - LED0
        * PB01 - Push Button PB1
* Board: Silicon Labs EFR32xG23 Radio Board (BRD4204D) + Wireless Starter Kit Mainboard
    * Device: EFR32ZG23B010F512IM48
        * PB02 - LED0
        * PB03 - Push Button PB1
* Board: Silicon Labs EFR32xG24 Radio Board (BRD4186C) + Wireless Starter Kit Mainboard
    * Device: EFR32MG24B210F1536IM48
        * PB02 - LED0
        * PB03 - Push Button PB1
* Board: Silicon Labs EFR32xG25 Radio Board (BRD4270B) + Wireless Starter Kit Mainboard
    * Device: EFR32FG25B222F1920IM56
        * PC06 - LED0
        * PB01 - Push Button PB1
* Board: Silicon Labs EFR32xG26 2.4 GHz 20 dBm Radio Board (BRD4117A) + Wireless Starter Kit Mainboard
    * Device: EFR32MG26B420F3200IM48
        * PB02 - LED0
        * PB03 - Push Button PB1
* Board: Silicon Labs EFR32xG27 Buck Radio Board (BRD4194A) + Wireless Starter Kit Mainboard
    * Device: EFR32MG27C140F768IM40
        * PB00 - LED0
        * PB01 - Push Button PB1
* Board: Silicon Labs EFR32xG28 Radio Board (BRD4400C) + Wireless Starter Kit Mainboard
    * Device: EFR32ZG28B312F1024IM68
        * PB02 - LED0
        * PB03 - Push Button PB1

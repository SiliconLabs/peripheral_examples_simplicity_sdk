# Peripheral Examples - WDOG PRS Clear #

## Summary ##

This project demonstrates the PRS clear functionality of the WDOG timer. The WDOG is configured to trigger a reset if not fed within 2049 ms. The WDOG is also configured to be fed by a PRS channel. The PRS channel is connected to a GPIO input. The GPIO must receive a periodic signal with a period < 2049 ms. If the period is > 2049 ms, the WDOG will not be fed in time and triggers a reset. The reset source is checked in the beginning of the application. If the latest reset was caused by the WDOG, LED0 turns on and the application stops running.

## Peripherals used ##

* WDOG - 2049 ms period
* PRS
* GPIO

## How to Test ##

1. Connect the periodic external signal to the GPIO input pin
2. Build the project and download to the Starter Kit
3. LED0 should be off
4. Disconnect the periodic external signal
5. LED0 is now on, indicating a WDOG reset

## Hardware & Connections ##

* Board: Silicon Labs EFR32xG21 2.4 GHz 10 dBm Board (BRD4181A) + Wireless Starter Kit Mainboard
    * Device: EFR32MG21A010F1024IM32
        * PD02 - LED0
        * PA05 - GPIO PRS WDOG input (Expansion Header Pin 12)

* Board: Silicon Labs EFR32xG22 2.4 GHz 6 dBm Board (BRD4182A) + Wireless Starter Kit Mainboard
    * Device: EFR32MG22A224F512IM40
        * PD02 - LED0
        * PA05 - GPIO PRS WDOG input (Expansion Header Pin 12)

* Board: Silicon Labs EFR32xG23 Radio Board (BRD4204D) + Wireless Starter Kit Mainboard
    * Device: EFR32ZG23B010F512IM48
        * PB02 - LED0
        * PA05 - GPIO PRS WDOG input (Expansion Header Pin 7)

* Board: Silicon Labs EFR32xG24 Radio Board (BRD4186C) + Wireless Starter Kit Mainboard
    * Device: EFR32MG24B210F1536IM48
        * PB02 - LED0
        * PA05 - GPIO PRS WDOG input (Expansion Header Pin 7)

* Board: Silicon Labs EFR32xG25 Radio Board (BRD4270B) + Wireless Starter Kit Mainboard
    * Device: EFR32FG25B222F1920IM56
        * PC06 - LED0
        * PA05 - GPIO PRS WDOG input (Expansion Header Pin 7)

* Board: Silicon Labs EFR32xG26 2.4 GHz 20 dBm Radio Board (BRD4117A) + Wireless Starter Kit Mainboard
    * Device: EFR32MG26B420F3200IM48
        * PB02 - LED0
        * PA05 - GPIO PRS WDOG input (Expansion Header Pin 7)

* Board: Silicon Labs EFR32xG27 Buck Radio Board (BRD4194A) + Wireless Starter Kit Mainboard
    * Device: EFR32MG27C140F768IM40
        * PB00 - LED0
        * PA05 - GPIO PRS WDOG input (Expansion Header Pin 12)

* Board: Silicon Labs EFR32xG28 Radio Board (BRD4400C) + Wireless Starter Kit Mainboard
    * Device: EFR32ZG28B312F1024IM68
        * PB02 - LED0
        * PB04 - GPIO PRS WDOG input (Expansion Header Pin 11)

* Board: Silicon Labs EFR32xG29 Buck Radio Board (BRD4412A) + Wireless Starter Kit Mainboard
    * Device: EFR32MG29B140F1024IM40
        * PB00 - LED0
        * PA05 - GPIO PRS WDOG input (Expansion Header Pin 12)

* Board:  Silicon Labs EFR32FG2D 868-915 MHz 14 dBm Radio Board (BRD4277A) + Wireless Starter Kit Mainboard
    * Device: EFR32FG2DB010F512IM48
        * PB02 - LED0
        * PA05 - GPIO PRS WDOG input (Expansion Header Pin 7)

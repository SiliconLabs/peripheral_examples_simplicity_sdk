# Peripheral Examples - WDOG Timeout Reset #

## Summary ##

This project demonstrates the functionality of the WDOG timer. The WDOG is configured to trigger a reset if not fed within 2049 ms. The WDOG is fed every 100 ms and LED0 toggles each time the WDOG is fed. Pressing Push Button 1 (PB1) blocks the application from feeding the WDOG. If PB1 is pressed and held through the WDOG timeout period, the WDOG triggers a reset. The reset source is checked in the beginning of the application. If the latest reset was caused by the WDOG, LED0 turns on and the application sits in an infinite while() loop.

## Peripherals used ##

* WDOG - 2049 ms period
* GPIO

## How to Test ##

1. Build the project and download to the Starter Kit
2. LED0 should be blinking
3. Press and hold PB1 for 3 seconds
4. LED0 is now on and not blinking anymore, indicating a WDOG reset

## Hardware & Connections ##

* Board: Silicon Labs SixG301 Radio Board (BRD4407A) + Wireless Pro Kit Mainboard
    * Device: SIMG301M104LIL
        * PD03 - LED0
        * PD02 - Push Button PB1

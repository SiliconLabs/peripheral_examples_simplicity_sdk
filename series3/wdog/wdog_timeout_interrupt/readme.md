# Peripheral Examples - WDOG Timeout Interrupt #

## Summary ##

This project demonstrates the functionality of the WDOG timer interrupt. The WDOG is configured to trigger an interrupt if not fed within 2049 ms. The WDOG is fed every 100 ms and LED0 toggles each time the WDOG is fed. Pressing Push Button 1 (PB1) blocks the application from feeding the WDOG. If PB1 is pressed and held through the WDOG timeout period, the WDOG triggers an interrupt. LED0 turns on and the application sits in an infinite while() loop.

## Peripherals used ##

* WDOG - 2049 ms period
* GPIO

## How to Test ##

1. Build the project and download to the Starter Kit
2. LED0 should be blinking
3. Press and hold PB1 for 3 seconds
4. LED0 is now on and not blinking anymore, indicating a WDOG timeout

## Hardware & Connections ##

* Board: Silicon Labs SixG301 Radio Board (BRD4407A) + Wireless Pro Kit Mainboard
    * Device: SIMG301M104LIL
        * PD03 - LED0
        * PD02 - Push Button PB1

# Peripheral Examples - WDOG Window Interrupt #

## Summary ##

This project demonstrates the window interrupt functionality of the WDOG timer. The WDOG is configured to trigger an interrupt when the WDOG is fed within 50% of the timer period. The timer period is set to 2049 ms, so the WDOG must be fed after 1024 ms otherwise the interrupt will trigger. The application feeds the WDOG every 1100 ms. When Push Button 1 (PB1) is pressed, the application feeds the WDOG after 800 ms, triggering the WDOG warning interrupt. LED0 turns on and the application stalls in an infinite while() loop, causing the WDOG timer to overflow and trigger a reset. The reset source is checked in the beginning of the application. If the latest reset was caused by the WDOG, LED0 turns on again and the application sits in an infinite while() loop.

## Peripherals used ##

* WDOG - 2049 ms period
* GPIO

## How to Test ##

1. Build the project and download to the Starter Kit
2. LED0 should be blinking
3. Press PB1
4. LED0 is now on and not blinking anymore, indicating a WDOG window interrupt and a WDOG reset

## Hardware & Connections ##

* Board: Silicon Labs SixG301 Radio Board (BRD4407A) + Wireless Pro Kit Mainboard
    * Device: SIMG301M104LIL
        * PD03 - LED0
        * PD02 - Push Button PB1

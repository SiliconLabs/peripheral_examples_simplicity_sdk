# Peripheral Examples - WDOG PRS Monitor #

## Summary ##

This project demonstrates the PRS monitoring functionality of the WDOG timer. The WDOG is configured to trigger a reset if it is not fed within 2049 ms or if the connected PRS channel does not see an event by the time the WDOG is fed. The application feeds the WDOG every 1000 ms and toggles LED0. The PRS channel is connected to a GPIO input. The GPIO must receive a periodic signal with a period < 1000 ms. If the period is > 1000 ms, a PRS event will not occur before the WDOG is fed and the WDOG will trigger a reset. The reset source is checked in the beginning of the application. If the latest reset was caused by the WDOG, LED0 turns on and the application sits in an infinite while() loop.

## Peripherals used ##

* WDOG - 2049 ms period
* PRS
* GPIO

## How to Test ##

1. Connect the periodic external signal to the GPIO input pin
2. Build the project and download to the Starter Kit
3. LED0 should be blinking
4. Disconnect the periodic external signal
5. LED0 is now on and not blinking anymore, indicating a WDOG reset

## Hardware & Connections ##

* Board: Silicon Labs SixG301 Radio Board (BRD4407A) + Wireless Pro Kit Mainboard
    * Device: SIMG301M104LIL
        * PD03 - LED0
        * PC01 - GPIO PRS WDOG input (Expansion Header Pin 6)

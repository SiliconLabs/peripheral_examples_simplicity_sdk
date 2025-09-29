# Peripheral Examples - GPIO EM4 Wakeup

## Summary ##

This project uses a GPIO pin configured as an input to wake from the EM4
energy mode, thus causing a reset. Pressing the specified button (PB0)
on the mainboard wakes the system from EM4 and toggles the LED0 indefinitely.

This demo delays a few seconds before entering EM4 to allow the debugger to
connect to the device.

## Peripherals used ##

* GPIO

## How to Test ##

How To Test:
1. Build the project and download it to the Starter Kit.
2. Terminate the debug session and return to the IDE or Launcher in
   Simplicity Studio.
3. Observe the current consumption of the device to verify that it is in
   EM4 using Studio's Energy Profiler.
4. Press PB0 to exit EM4.
5. Observe the LED0 blinking, indicating that the last reset cause was
   exit from EM4. Observing current consumption in Energy Profiler
   verifies that the device is in EM0.

## Hardware & Connections ##

* Board: Silicon Labs SixG301 Radio Board (BRD4407A) + Wireless Pro Kit Mainboard
    * Device: SIMG301M104LIL
        * PB01 - Push Button PB0
        * PD03 - LED0


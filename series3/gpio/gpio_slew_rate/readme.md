# Peripheral Examples - GPIO Slew Rate

## Summary ##

This project demonstrates how to set the slew rate for GPIO pins. A 1 MHz
square is driven on a pin and by connecting a capacitor, the effect of
slew rate changes on pin rise and fall times can be observed.  In this
example, pressing button PB0 increments the slew rate setting such
pressing the button 8 times cycles the through all slew rate settings.
The value of the slew rate setting varies from 0-7, and starts with a 
mid-level setting of 4.

## Peripherals used ##

* GPIO

## How to Test ##

1. Place a 50uF capacitor between the output pin and GND.
2. Build the example project and download it to the target system.
3. Click the Play/Resume (F8) button in the debugger to run the program.
4. While observing the rise and fall times of the waveform on the output pin, 
   press PB0 to change the slew rate.

## Hardware & Connections ##

* Board: Silicon Labs SixG301 Radio Board (BRD4407A) + Wireless Pro Kit Mainboard
    * Device: SIMG301M104LIL
        * PB01 - Push Button PB0
        * PC00 - 1 MHz output (WPK P1; Expansion Header pin 4)


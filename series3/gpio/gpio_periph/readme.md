# Peripheral Examples - GPIO Periph

## Summary ##

This project shows how to drive a pin from the output of an on-chip oscillator
(the LFRCO, in this case) and how to set the default and alternate slew
rates for the selected GPIO port.


## Peripherals used ##

* GPIO
* CMU

## How to Test ##

How To Test:
1. Build the project and download to the Starter Kit
2. Connect an oscilloscope to pin PC00 and observe the LFRCO signal

## Hardware & Connections ##

* Board: Silicon Labs SixG301 Radio Board (BRD4407A) + Wireless Pro Kit Mainboard
    * Device: SIMG301M104LIL
        * PC00 - LFRCO Output (WPK P1; Expansion Header pin 4)


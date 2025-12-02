# Peripheral Examples - LETIMER Pulse Width Modulation #

## Summary ##

This project demonstrates pulse width modulation using the LETIMER.
The project initializes the LETIMER for PWM with a set 30 percent 
duty cycle at 1 Hz frequency. The waveform is output on an 
Expansion Header Pin on the board.

## Peripherals used ##
LETIMER0 - PWM mode

## How to Test ##

1. Build the project and download to the Pro Kit.
2. Measure output on the appriopriate pin listed below.

## Hardware & Connections ##

* Board: Silicon Labs SixG301 Radio Board (BRD4407A) + Wireless Pro Kit Mainboard
    * Device: SIMG301M104LIL
        * PA06 - LET0_O0 (Expansion Header Pin 11, P08)

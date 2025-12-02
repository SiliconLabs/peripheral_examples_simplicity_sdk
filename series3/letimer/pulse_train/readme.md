# Peripheral Examples - LETIMER Pulse Train #

## Summary ##

This project demonstrates pulse train generation using the LETIMER.
The project initializes the LETIMER in free mode, and creates a one 
LFCLK length pulse on each underflow event. The underflow occurs 
at 1 kHz frequency.

## Peripherals used ##
LETIMER0 - free mode

## How to Test ##

1. Build the project and download to the Pro Kit.
2. Measure waveform on the appropriate pin listed below.

## Hardware & Connections ##

* Board: Silicon Labs SixG301 Radio Board (BRD4407A) + Wireless Pro Kit Mainboard
    * Device: SIMG301M104LIL$
        * PA06 - LET0_O0 (Expansion Header Pin 11, P08)

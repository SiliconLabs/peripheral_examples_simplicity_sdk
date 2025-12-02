# Peripheral Examples - LETIMER one shot output compare #

## Summary ##

This project demonstrates output compare using the LETIMER.
The project initializes the LETIMER in one-shot mode, and expansion 
header pin toggles REPEAT_COUNT times while staying in a low energy mode.

## Peripherals used ##
LETIMER0 - one-shot mode

## How to Test ##

1. Build the project and download to the Pro Kit.
2. Measure output on the appropriate pin listed below.

## Hardware & Connections ##

* Board: Silicon Labs SixG301 Radio Board (BRD4407A) + Wireless Pro Kit Mainboard
    * Device: SIMG301M104LIL
        * PA06 - LET0_O0 (Expansion Header Pin 11, P08)

# Peripheral Examples - TIMER Single Pulse Generation with DMA #

## Summary ##

This project demonstrates the generation of a single pulse using output compare and the LDMA. TIMER0 is initialized for output compare on Compare/Capture channel 0 which is routed to GPIO pin specified below. The timer is configured in one-shot mode and an interrupt triggers after the first compare to load the second, falling edge to the OC. The LDMA is configured to transfer a single word to the OC register once the first compare event occurs. The values loaded into the OC are such that a 1 ms pulse is generated after a 3 second delay.

## Peripherals used ##

* EM01GRPACLK - Sourced from HFRCODPLL @ 38 MHz
* TIMER0 - Output Compare Mode
* LDMA - CC0 Memory to Peripheral Transfer

## How to Test ##

1. Build the project and download it to the Starter Kit.
2. Use an oscilloscope to measure the GPIO pin specified below.
3. A 1 ms pulse should be generated after 3 second.

## Hardware & Connections ##

* Board: Silicon Labs SixG301 Radio Board (BRD4407A) + Wireless Pro Kit Mainboard
    * Device: SIMG301M104LIL
        * PA06 - TIM0_CC0 (Expansion Header Pin 11, P08)

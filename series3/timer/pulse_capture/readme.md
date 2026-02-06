# Peripheral Examples - TIMER Pulse Capture #

## Summary ##

This project demonstrates single pulse capture using the TIMER peripheral. The HFXO is configured and selected as the EM01GRPACLK source, which is the clock branch for high-frequency TIMERs on all Series 2 EFM32/EFR32 devices. The TIMER is then configured for input capture on channel CC0 for all edges but with interrupts on every other edge in order to reduce the CPU overhead.  The TIMER input capture logic is double-buffered and can capture two subsequent edges before requesting an interrupt.

The GPIO pin specified below must be connected to a periodic signal or pulse generator. The two edges captured (one falling and one rising) are read from the CCV register.

> Note: The range of frequencies this program can measure accurately is limited by the selected frequency of the EM01GRPACLK, prescaling of the local TIMER clock, and the counter width of the selected TIMER, e.g. TIMER0 is 32 bits wide while other TIMERs are generally 16 bits wide. The minimum measurable period is around 700 ns (about 1.43 MHz).

## Peripherals used ##

* EM01GRPACLK - Sourced from HFXO
* TIMER0 - Input Capture Mode

## How to Test ##

1. Build the project and download it to the Starter Kit.
2. Connect a periodic signal to GPIO pin specified below.
3. Go into debug mode and click run.
4. View the first_edge and second_edge global variables in the watch window.

## Hardware & Connections ##

* Board: Silicon Labs SixG301 Radio Board (BRD4407A) + Wireless Pro Kit Mainboard
    * Device: SIMG301M104LIL
        * PA06 - TIM0_CC0 (Expansion Header Pin 11, P08)

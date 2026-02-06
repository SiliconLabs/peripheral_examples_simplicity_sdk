# Peripheral Examples - TIMER Period Measurement Polled #

## Summary ##

This project demonstrates period measurement using TIMER0. The main loop polls the CC0 flag, which is set after capturing a pair of falling edges, thus eliminating the need to poll for both edges associated with one waveform period.

Once the flag is set, the two captured edge times are saved, and the overflow flag is checked in order to account for cases in which two edges span the time during which the counter rolls over from 0xFFFFFFFF (TIMER0 is 32 bits wide) to 0. These values are passed to the calculatePeriod() function, which returns the measuredPeriod as an integer value in microseconds, such that the measuredPeriod value shows 1000 for an input signal with a period of 1 kHz.

> Note: The range of frequencies this program can measure accurately is limited by the selected frequency of the EM01GRPACLK, prescaling of the local TIMER clock, and the counter width of the selected TIMER, e.g. TIMER0 is 32 bits wide while other TIMERs are generally 16 bits wide.

## Peripherals used ##

* EM01GRPACLK - Sourced from HFRCODPLL @ 38 MHz
* TIMER0 - Input Capture Mode

## How to Test ##

1. Build the project and download it to the Starter Kit.
2. Connect a periodic signal to the GPIO pin specified below.
3. Go into debug mode and click run.
4. View the measured_period (time period in us) global variable in the debugger.

## Hardware & Connections ##

* Board: Silicon Labs SixG301 Radio Board (BRD4407A) + Wireless Pro Kit Mainboard
    * Device: SIMG301M104LIL
        * PA06 - TIM0_CC0 (Expansion Header Pin 11, P08)

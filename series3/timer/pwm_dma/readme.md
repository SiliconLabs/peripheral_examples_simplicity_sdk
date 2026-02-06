# Peripheral Examples - TIMER Pulse Width Modulation (PWM) with DMA #

## Summary ##

This project demonstrates DMA-driven pulse width modulation using the TIMER periperhal. TIMER0 is initialized for PWM on capture/compare channel 0 and routed to a GPIO pin.

In PWM mode, compare events set the output pin and overflow events clear it such that the values in the TIMER_CC_OC and TIMER_TOP registers specify the PWM duty cycle and frequency, respectively.

Each time a compare event occurs, the LDMA responds by writing a new value to the TIMER_CC_OCB register such that, in this example, the output duty cycle increases from 0 to 100% by an increment of 10%, then rolls over back to 0% and repeats.

## Peripherals used ##

* EM01GRPACLK - Sourced from HFRCODPLL @ 38 MHz
* TIMER0 - Pulse Width Modulation Mode
* LDMA - CC0 Memory to Peripheral Transfer

## How to Test ##

1. Build the project and download it to the Starter Kit.
2. Use an oscilloscope to view the 1 kHz signal with continuously varying duty cycle on the expansion header (EXP) pin specified below.

## Hardware & Connections ##

* Board: Silicon Labs SixG301 Radio Board (BRD4407A) + Wireless Pro Kit Mainboard
    * Device: SIMG301M104LIL
        * PA06 - TIM0_CC0 (Expansion Header Pin 11, P08)

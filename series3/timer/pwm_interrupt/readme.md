# Peripheral Examples - TIMER Pulse Width Modulation (PWM) with Interrupts #

## Summary ##

This project demonstrates pulse width modulation using the TIMER module. TIMER is initialized for PWM on Compare/Capture channel 0 which is routed to the GPIO Pin specified below. In PWM mode, overflow events set the output pin, while compare events clear the pin. Thus the overflow value is set to output the desired signal frequency, while the CCV is set to control the duty cycle. The duty cycle is altered by writing to the CCVB register, which safely updates the compare value on the next overflow event.

## Peripherals used ##

* EM01GRPACLK - Sourced from HFRCODPLL @ 38 MHz
* TIMER0 - Pulse Width Modulation Mode

## How to Test ##

1. Build the project and download it to the Starter Kit.
2. Use an oscilloscope to view the 1 KHz signal with 30% duty cycle on the GPIO pin specified below.

## Hardware & Connections ##

* Board: Silicon Labs SixG301 Radio Board (BRD4407A) + Wireless Pro Kit Mainboard
    * Device: SIMG301M104LIL
        * PA06 - TIM0_CC0 (Expansion Header Pin 11, P08)

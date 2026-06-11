# Peripheral Examples - TIMER Input Capture Interrupt #

## Summary ##

This project demonstrates use of the TIMER module for interrupt-driven input capture. Button 0 presses trigger a TIMER interrupt, and TIMER interrupt handler captures the CC0 count and stores value to a circular buffer. 

> Note: This example captures falling edge because one side of each STK push button switch is grounded while the other is intended to be pulled high by the GPIO pin to which it is connected.

Comments are provided in the example that explain how to convert the code from interrupt-driven to polled operation.

## Peripherals used ##

* EM01GRPACLK - Sourced from HFRCODPLL @ 38 MHz
* TIMER0 - Input Capture Mode

## How to Test ##

1. Build the project and download it to the Wireless Starter Kit.
2. Go into debug mode and click Run.
3. Press button 0 to trigger the input capture and have the value recorded.
4. Pause the debugger, add the buffer[] variable to the Watch/Variables pane, and expand the array to see each the value of the counter for each edge (button press) captured.

## Hardware & Connections ##

* Board: Silicon Labs SixG301 Radio Board (BRD4407A) + Wireless Pro Kit Mainboard
    * Device: SIMG301M104LIL
        * PB01 - TIM0_CC0, Push Button 0 


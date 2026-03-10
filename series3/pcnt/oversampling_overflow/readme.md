# Peripheral Examples - PCNT Oversampling Overflow #

## Summary ##

This project demonstrates the single input oversampling mode of the pulse 
counter with interrupts. EM23GRPACLK clock is used as the pulse counter clock in
this example. The program generates an interrupt whenever the pulse counter goes 
above the configured top value. In this example, each press of Push Button PB0 
will increase the counter value by 1. The initial value of the counter is zero 
(default) and reload value for the top register is set by PCNTTopValue define.

When testing, pause the debugger and observe how the value of the counter 
register is counting up whenever BTN0 is pressed.

Use Simplicity Studio's Energy Profiler to observe current consumption while
example runs in low energy mode.

## How to Test ##

1. Build the project and download to the Starter Kit
2. Press Button 0 six times
3. Observe LED0 toggle
4. Repeat steps 2 thru 4

## Peripherals Used ##

* CMU  - LFRCO @ 32768 Hz
* PRS
* PCNT

## Hardware & Connections ##

* Board: Silicon Labs SixG301 Radio Board (BRD4407A) + Wireless Pro Kit Mainboard
    * Device: SIMG301M104LIL
        * PB01 -  Push Button PB0
        * PD03 -  LED0

# Peripheral Examples - PRS Logic Unit #

## Summary ##

This project demonstrates the built-in PRS logic functions between channels.
Onboard push-buttons (PB0 and PB1) are set as PRS source. Onboard LED (LED1)
is set as PRS output.

Output uses the following truth table for OR logic function:
PB1   PB0   |  OUTPUT 
------------------------
1     1     |     1     
1     0     |     1    
0     1     |     1     
0     0     |     0  

When the push button (PB0 or PB1) is pressed, a logical low is sampled by the
GPIO and sent to the corresponding PRS channel

When the push button (PB0 or PB1) is released, a logical high is sampled by the
GPIO and sent to the corresponding PRS channel

A logical low output will have the LED off and a logical high output will have
the LED on.


## Peripherals used ##

* PRS
* GPIO

## How to Test ##

1. Open Simplicity Studio and update the kit's firmware using **Device Manager Tool**(if necessary)
2. Build the project and download to the Starter Kit
3. Press the Push Buttons and notice how the LED1 responds.

Note: Various other logic functions like AND, NAND, NOR, XOR, NOT etc are
available. You can switch between these logic functions by changing the
parameters passed when calling PRS_Combine() function in main.c.

## Hardware & Connections ##

* Board: Silicon Labs SixG301 Radio Board (BRD4407A) + Wireless Pro Kit Mainboard
    * Device: SIMG301M104LIL
        * PB01 - Button 0
        * PD02 - Button 1
        * PD03 - LED0

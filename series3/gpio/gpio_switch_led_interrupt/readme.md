# Peripheral Examples - Swith LED Interrupt

## Summary ##

This project demonstrates how to use GPIO pins to trigger external
interrupts. If button 0 is pressed, LED0 will toggle; if button 1 is
pressed, LED1 will toggle. There are two sources for GPIO interrupts.
By default, one is assigned to even-numbered pins and the other to
odd-numbered pins. (The GPIO_EXTIPINSELH and GPIO_EXTIPINSELL registers
permit some offsetting from the even and odd defaults that is beyond
the scope of this example. See the Standard Interrupt Generation section
of the reference manual GPIO chapter for details).


## Peripherals used ##

* GPIO

## How to Test ##

How To Test:
1. Build the example project and download it to the target system.
2. Click the Play/Resume (F8) button in the debugger to run the program.
3. Press PB0 to toggle LED0.
4. Press PB1 to toggle LED1.


## Hardware & Connections ##

* Board: Silicon Labs SixG301 Radio Board (BRD4407A) + Wireless Pro Kit Mainboard
    * Device: SIMG301M104LIL
        * PB01 - Push Button PB0
        * PD02 - Push Button PB1
        * PD03 - LED0
        * PD04 - LED1


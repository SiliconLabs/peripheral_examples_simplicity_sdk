# Peripheral Examples - PCNT EXTCLK Quadrature #

## Summary ##

This project demonstrates the external quadrature mode of the pulse counter
peripheral. The program will request an interrupt whenever the quadrature
decoder detects a change in direction.

When testing, pause the debugger and observe how the value of the counter 
register is counting up whenever using the BTN0 -> BTN1 sequence, and counting
down with the BTN1 -> BTN0 sequence

Use Simplicity Studio's Energy Profiler to observe current consumption while
example runs in low energy mode.

This example replaces some of the sl_hal functions with a special version that
toggles the PRS signal until the PCNT is synchronized.  Without this version it
would take about 16 button presses before the PCNT is able to detect pulses.  The
special functions start with pcnt instead of sl_hal_pcnt.

## How to Test ##

1. Build the project and download to the Starter Kit
2. Press BTN1, hold it
3. Press BTN0, hold it
4. Release BTN1, then release BTN0
5. Keep doing the steps above until LED0 turns on, indicating
   the direction change. LED0 turns on to indicate change in Status register's
   direction bit to 1 (down counter; PCNT initialized counting up)
6. If you continue repeating steps 2-4 above, LED0 will remain on
7. Now do the reverse button sequence: press BTN0, hold; then BTN1, hold;
   release BTN0 then BTN1, press BTN0 then BTN1. When BTN0 is released
   LED1 will turn on since Status register's direction bit changed to 0 (up 
   counter)
8. If you reverse the pattern again, LED0 will turn off (toggle); note you
   need to repeat the pattern twice before noticing the change because the push
   button behavior is reverse logic (pressed is logic low)
9. You may also route a signal generator to the PCNT0 instead of using the push
   buttons.  This will require modifying which pins are configured as PRS inputs.
   Check the board schematic for available pins.

## Peripherals Used ##

* External Clock - Push Button 0
* PRS
* PCNT

## Hardware & Connections ##

* Board: Silicon Labs SixG301 Radio Board (BRD4407A) + Wireless Pro Kit Mainboard
    * Device: SIMG301M104LIL
        * PB01 -  Push Button PB0
        * PD03 -  LED0
        * PD02 -  Push Button PB1
        * PD04 -  LED1

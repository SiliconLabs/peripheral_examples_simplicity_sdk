# Peripheral Examples - MSC Page Lock #

## Summary ##

This project demonstrates how to lock flash memory pages using the 
`MSC_PAGELOCKn` and `MSC_MISCLOCKWORD` registers. While setting lock bits is 
straightforward, deciding where in your application to place these writes 
requires care.

This example demonstrates early-stage flash page locking via `app_init_early()`,
followed by verification attempts in `app_init()`. A GPIO pin connected to LED0 
is toggled to mark timing, and locked pages are confirmed by checking for expected 
erase failures.

> Note: On EFR32xG21 devices, oscillators and clock branches are automatically 
turned on/off based on demand from the peripherals.  As such, writes to clock 
enable bits are unnecessary and have no effect on xG21 devices. 

> Future Work: The flash page locking currently occurs in `app_init_early()` to 
ensure it executes before most application logic. However, once the generalized 
`SystemInit2()` hook becomes available, the plan is to move this logic there.
`SystemInit2()` runs even earlier than `app_init_early()` in the boot 
sequence. 

## Peripherals used ##

* CMU    - HFRCODPLL @ 19 MHz
* MSC    - Flash page locking and erase operations

## How to Test ##

1. Open Simplicity Studio and update the kit's firmware using **Device Manager Tool**(if necessary).
2. Build the project and download it to the Starter Kit.
3. Run the program.
4. Press the pause button. The program should halt in a `app_process_action()` loop.
5. To observe how much elapses between issuing critical register writes in 
   `app_init_early()` vs. `app_init()` set an oscilloscope to trigger once on 
   a rising edge with a time scale of 25 or 50 us/division.
6. Observe the LED0 GPIO pin by probing the anode of LED0 on the WSTK
   mainboard (this is the solder pad just below the letter 'E' of the 'LED0'
   component label), or the breakout/expansion header pin indicated below.
7. Run the demo again and observe the pulse on the oscilloscope. 

## Hardware & Connections ##

* Board:  Silicon Labs EFR32xG21 2.4 GHz 10 dBm Board (BRD4181A) + Wireless Starter Kit Mainboard
    * Device: EFR32MG21A010F1024IM32
        * PB00 - LED0 (WSTK Breakout Header P8, Expansion Header Pin 11)

* Board: Silicon Labs EFR32xG22 2.4 GHz 6 dBm Radio Board (BRD4182A) + Wireless Starter Kit Mainboard
    * Device: EFR32MG22C224F512IM40
        * PD02 - LED0 (WSTK Breakout Header P8, Expansion Header Pin 11)

* Board:  Silicon Labs EFR32ZG23 868-915 MHz 14 dBm Radio Board (BRD4204D) + Wireless Starter Kit Mainboard
    * Device: EFR32ZG23B010F512IM48
        * PB02 - LED0 (WSTK Breakout Header P19)

* Board:  Silicon Labs EFR32MG24 2.4 GHz 10 dBm Radio Board (BRD4186C) + Wireless Starter Kit Mainboard
    * Device: EFR32MG24B210F1536IM48
        * PB02 - LED0 (WSTK Breakout Header P19)

* Board:  Silicon Labs EFR32FG25 902-928 MHz 14 dBm Radio Board (BRD4270B) + Wireless Starter Kit Mainboard
    * Device: EFR32FG25B222F1920IM56
        * PC06 - LED0 (WSTK Breakout Header P27)

* Board:  Silicon Labs EFR32MG26 2.4 GHz 20 dBm Radio Board (BRD4117A) + Wireless Starter Kit Mainboard
    * Device: EFR32MG26B420F3200IM48
        * PB02 - LED0 (WSTK Breakout Header P19)

* Board:  Silicon Labs EFR32xG27 8 dBm Buck DCDC Radio Board (BRD4194A) + Wireless Starter Kit Mainboard
    * Device: EFR32MG27C140F768IM40
        * PB00 - LED0 (WSTK Breakout Header P4, Expansion Header Pin 7)

* Board:  Silicon Labs EFR32xG28 868/915 MHz +14 dBm + 2.4 GHz +10 dBm Radio Board (BRD4400C) + Wireless Starter Kit Mainboard
    * Device: EFR32ZG28B312F1024IM68
        * PB02 - LED0 (WSTK Breakout Header P19)

* Board: Silicon Labs EFR32xG29 Buck Radio Board (BRD4412A) + Wireless Starter Kit Mainboard
    * Device: EFR32MG29B140F1024IM40
        * PB00 - LED1 (WSTK Breakout Header P4, Expansion Header Pin 7)
# Peripheral Examples - LESENSE Single Channel #

## Summary ##

This project demonstrates the usage of LESENSE to sample a single channel
using ACMP. The LESENSE uses the ACMP peripheral to sample the input state of 
push button 0, and will trigger an interrupt to toggle LED0 when push button 0
is pressed.

> Note: In project where the device enters EM2 or lower, an escape_hatch
>      routine is usually recommended to prevent device lock-up. This example has
>      implemented a escape_hatch, where if the user holds down push-button 1 when
>      resetting the device, the escape_hatch will hold the device in EM0 state
>      to allow debug connection

## Peripheral Used: ##

* CMU
  * LFRCO @ 32768 Hz
* ACMP
  * Used to sample push-button 0 input state
* GPIO
  * LED0 configured as output
* LESENSE
  * Controls ACMP to sample push-button 0 on selected LESENSE channel

## How to test: ##

1. Build the project and download it to the starter kit.
2. The LED0 should be off right now with lower current consumption (~4.7 for xG23,
    ~4.7uA for xG25 and ~5.0uA for xG28).
3. Press push button 0, LED0 should turn on.
4. Release push button 0 and press it again, LED0 should turn off.
5. Enter debug mode and place a break point inside the `LESENSE_IRQHandler()`.
6. Repeat step 3 and observe the interrupt handler firing each time.

## Hardware & Connections ##

* Board:  Silicon Labs EFR32xG23 Radio Board (BRD4204D) + 
        Wireless Starter Kit Mainboard
    * Device: EFR32ZG23B010F512IM48
        * PB01 -  Push Button PB0
        * PB02 -  LED0

* Board:  Silicon Labs EFR32xG25 Radio Board (BRD4270B) + 
        Wireless Starter Kit Mainboard
    * Device: EFR32FG25B222F1920IM56
        * PB00 -  Push Button PB0
        * PC06 -  LED0

* Board:  Silicon Labs EFR32xG28 Radio Board (BRD4400C) +
        Wireless Starter Kit Mainboard
    * Device: EFR32ZG28B312F1024IM68
        * PB01 -  Push Button PB0
        * PB02 -  LED0

* Board:  Silicon Labs EFR32FG2D 868-915 MHz 14 dBm Radio Board (BRD4277A) + Wireless Starter Kit Mainboard
    * Device: EFR32FG2DB010F512IM48
        * PB01 - Push Button PB0
        * PB02 - LED0

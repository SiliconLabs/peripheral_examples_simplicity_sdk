# Peripheral Examples - GPIO Switch LED Polled #

## Summary ##

This project demonstrates how to set up simple digital inputs and
outputs. Button PB0 on the WSTK is the input; LED1 is the output.
While PB0 is pressed, LED1 is on.

Note: On the EFR32xG27 and EFR32xG29, PB0 and LED0 share the same pin. so LED0 will 
also be on while PB0 is pressed. Additionally, the polarity is reversed, 
pressing PB0 turns LED1 off, and releasing it turns LED1 on.

## Peripherals used ##

* CMU    - HFRCODPLL @ 19 MHz

## How to Test ##

1. Build the project and download to the Starter Kit
2. Click the Play/Resume (F8) button in the debugger to run the program
3. Press PB0 to turn the LED1 on (except on xG27/xG29 devices, where LED1 will turn off)
4. Release PB0 to turn the LED1 off (except on xG27/xG29 devices, where LED1 will turn on)

## Hardware & Connections ##

* Board: Silicon Labs EFR32xG21 2.4 GHz 10 dBm Radio Board (BRD4181A) + Wireless Starter Kit Mainboard
    * Device: EFR32MG21A010F1024IM32
        * PD2 - Push Button PB0
        * PB1 - LED1

* Board: Silicon Labs EFR32xG22 2.4 GHz 6 dBm Radio Board (BRD4182A) + Wireless Starter Kit Mainboard
    * Device: EFR32MG22C224F512IM40
        * PB00 -  Push Button PB0
        * PD03 -  LED1

* Board:  Silicon Labs EFR32ZG23 868-915 MHz 14 dBm Radio Board (BRD4204D) + Wireless Starter Kit Mainboard
    * Device: EFR32ZG23B010F512IM48
        * PB01 -  Push Button PB0
        * PD03 -  LED1

* Board:  Silicon Labs EFR32MG24 2.4 GHz 10 dBm Radio Board (BRD4186C) + Wireless Starter Kit Mainboard
    * Device: EFR32MG24B210F1536IM48
        * PB01 -  Push Button PB0
        * PB04 -  LED1

* Board:  Silicon Labs EFR32FG25 902-928 MHz 14 dBm Radio Board (BRD4270B) + Wireless Starter Kit Mainboard
    * Device: EFR32FG25B222F1920IM56
        * PB00 -  Push Button PB0
        * PC07 -  LED1

* Board:  Silicon Labs EFR32MG26 2.4 GHz 20 dBm Radio Board (BRD4117A) + Wireless Starter Kit Mainboard
    * Device: EFR32MG26B420F3200IM48
        * PB01 -  Push Button PB0
        * PB04 -  LED1

* Board:  Silicon Labs EFR32xG27 2.4 GHz 8 dBm Buck DCDC Radio Board (BRD4194A) + Wireless Starter Kit Mainboard
    * Device: EFR32MG27C140F768IM40
        * PB00 - Push Button PB0
        * PB01 - LED1

* Board:  Silicon Labs EFR32xG28 868/915 MHz +14 dBm + 2.4 GHz +10 dBm Radio Board (BRD4400C) + Wireless Starter Kit Mainboard
    * Device: EFR32ZG28B312F1024IM68
        * PB01 -  Push Button PB0
        * PD03 -  LED1
		
* Board: Silicon Labs EFR32xG29 Buck Radio Board (BRD4412A) + Wireless Starter Kit Mainboard
    * Device: EFR32MG29B140F1024IM40
        * PB00 - Push Button PB0
        * PB01 - LED1
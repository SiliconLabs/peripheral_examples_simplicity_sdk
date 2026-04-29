# Peripheral Examples - BURTC EM4 BURAM #

## Summary ##

This project uses the BURTC (Backup Real Time Counter) to wake the device from
EM4 mode and thus trigger a reset. This project also shows how to use the BURAM
retention registers to have data persist between resets. The number of resets 
triggered by the BURTC will printed via the device's USART and the mainboard's 
JLink CDC UART Port for view in a PC terminal program.

## Peripherals used ##

* BURTC   - Interrupt every ~3 seconds
* ULFRCO  - 1000 Hz, BURTC clock source
* USART0  - 115200 baud, 8-N-1

## How to Test ##

1. Build the project and download it to the Starter Kit
2. Close debug session in IDE
3. Open a terminal program and connect to the COM port associated with Starter Kit's
   Jlink CDC UART Port (see Windows Device Manager) using 115200 baud, 8-N-1
4. Press the reset button the mainboard
5. Follow instructions in the terminal program to enter EM4
6. Observe the number of EM4 wakeups should increase after each EM4 wakeup

## Hardware & Connections ##

* Board:  Silicon Labs EFR32xG21 2.4 GHz 10 dBm Board (BRD4181A) + Wireless Starter Kit Mainboard
    * Device: EFR32MG21A010F1024IM32
        * PD02 - Push Button PB0, WSTK EXP Header Pin 7
        * PA06 - WSTK EXP Header Pin 14
        * PB01 - LED1

* Board: Silicon Labs EFR32xG22 2.4 GHz 6 dBm Radio Board (BRD4182A) + Wireless Starter Kit Mainboard
    * Device: EFR32MG22C224F512IM40
        * PB00 - Push Button PB0
        * PD03 - LED1

* Board:  Silicon Labs EFR32ZG23 868-915 MHz 14 dBm Radio Board (BRD4204D) + Wireless Starter Kit Mainboard
    * Device: EFR32ZG23B010F512IM48
        * PB01 - Push Button PB0
        * PD03 - LED1

* Board:  Silicon Labs EFR32MG24 2.4 GHz 10 dBm Radio Board (BRD4186C) + Wireless Starter Kit Mainboard
    * Device: EFR32MG24B210F1536IM48
        * PB01 - Push Button PB0
        * PB04 - LED1

* Board:  Silicon Labs EFR32FG25 902-928 MHz 14 dBm Radio Board (BRD4270B) + Wireless Starter Kit Mainboard
    * Device: EFR32FG25B222F1920IM56
        * PB00 - Push Button PB0
        * PC07 - LED1

* Board:  Silicon Labs EFR32MG26 2.4 GHz 20 dBm Radio Board (BRD4117A) + Wireless Starter Kit Mainboard
    * Device: EFR32MG26B420F3200IM48
        * PB01 - Push Button PB0
        * PB04 - LED1

* Board:  Silicon Labs EFR32xG27 8 dBm Buck DCDC Radio Board (BRD4194A) + Wireless Starter Kit Mainboard
    * Device: EFR32MG27C140F768IM40
        * PB00 - Push Button PB0
        * PB01 - LED1

* Board:  Silicon Labs EFR32xG28 868/915 MHz +14 dBm + 2.4 GHz +10 dBm Radio Board (BRD4400C) + Wireless Starter Kit Mainboard
    * Device: EFR32ZG28B312F1024IM68
        * PB01 - Push Button PB0
        * PD03 - LED1

* Board: Silicon Labs EFR32xG29 Buck Radio Board (BRD4412A) + Wireless Starter Kit Mainboard
    * Device: EFR32MG29B140F1024IM40
        * PB00 - Push Button PB0
        * PB01 - LED1

* Board:  Silicon Labs EFR32FG2D 868-915 MHz 14 dBm Radio Board (BRD4277A) + Wireless Starter Kit Mainboard
    * Device: EFR32FG2DB010F512IM48
        * PB01 - Push Button PB0
        * PD03 - LED1

# Peripheral Examples - GPIO Switch LED Interrupt #

## Summary ##

This project demonstrates how to use GPIO pins to trigger external interrupts.
If push-button 0 is pressed, then LED1 will toggle. The GPIO's even or odd interrupt
handler will be triggered when based on an even or odd pin.

Note:
- The EFR32MG21 can wake from EM2/3 sleep modes only through GPIO interrupts on
  ports A or B, or through EM4WU pins. Since the push buttons on the WSTK are
  connected to port D, they must be re-routed to an interrupt-capable port.
  For this example, jump pins 7 to pins 14 on the Expansion Header of the WSTK.
- On the EFR32xG27 and EFR32xG29, PB0 and LED0 share the same pin.

## Peripherals used ##

* CMU    - HFRCODPLL @ 19 MHz
* EMU

## How to Test ##

1. Build the project and download to the Starter Kit
2. Click the Play/Resume (F8) button in the debugger to run the program
3. Press PB0 to toggle LED1

### How To Test on EFR32xG21 ###

1. Build the project and download to the Starter Kit
2. Connect Push Button 0 to PA6 (pin 7 to 14)
3. Press PB0 to toggle LED1

## Hardware & Connections ##

* Board:  Silicon Labs EFR32xG21 2.4 GHz 10 dBm Board (BRD4181A) + Wireless Starter Kit Mainboard
    * Device: EFR32MG21A010F1024IM32
        * PD2 - Push Button PB0, WSTK EXP Header Pin 7
        * PA6 - WSTK EXP Header Pin 14
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

* Board:  Silicon Labs EFR32xG27 8 dBm Buck DCDC Radio Board (BRD4194A) + Wireless Starter Kit Mainboard
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

* Board:  Silicon Labs EFR32FG2D 868-915 MHz 14 dBm Radio Board (BRD4277A) + Wireless Starter Kit Mainboard
    * Device: EFR32FG2DB010F512IM48
        * PB01 -  Push Button PB0
        * PD03 -  LED1

# Peripheral Examples - TIMER Input Capture Interrupt #

## Summary ##

This project demonstrates use of the TIMER module for interrupt-driven input capture. Button 0 presses trigger a TIMER interrupt, and TIMER interrupt handler captures the CC0 count and stores value to a circular buffer. 

> Note: This example captures falling edge because one side of each STK push button switch is grounded while the other is intended to be pulled high by the GPIO pin to which it is connected.

Comments are provided in the example that explain how to convert the code from interrupt-driven to polled operation.

## Peripherals used ##

* EM01GRPACLK - Sourced from HFRCODPLL @ 19 MHz
* TIMER0 - Input Capture Mode

## How to Test ##

1. Build the project and download it to the Wireless Starter Kit.
2. Go into debug mode and click Run.
3. Press button 0 to trigger the input capture and have the value recorded.
4. Pause the debugger, add the buffer[] variable to the Watch/Variables pane, and expand the array to see each the value of the counter for each edge (button press) captured.

## Hardware & Connections ##

* Board: Silicon Labs EFR32xG21 2.4 GHz 10 dBm Board (BRD4181A) + Wireless Starter Kit Mainboard (BRD4001A)
	* Device: EFR32MG21A010F1024IM32
		* PD02 - Push Button PB0 (WSTK P4, Expansion Header Pin 7)

* Board: Silicon Labs EFR32xG22 Radio Board (BRD4182A) + Wireless Starter Kit Mainboard
	* Device: EFR32MG22C224F512IM40
		* PB00 -  Push Button PB0 (WSTK P4, Expansion Header Pin 7)

* Board: Silicon Labs EFR32xG23 Radio Board (BRD4204D) + Wireless Starter Kit Mainboard
	* Device: EFR32ZG23B010F512IM48
		* PB01 -  Push Button PB0 (WSTK P17)

* Board: Silicon Labs EFR32xG24 Radio Board (BRD4186C) + Wireless Starter Kit Mainboard
	* Device: EFR32MG24B210F1536IM48
		* PB01 -  Push Button PB0 (WSTK P17)

* Board: Silicon Labs EFR32xG25 Radio Board (BRD4270B) + Wireless Starter Kit Mainboard
	* Device: EFR32FG25B222F1920IM56
		* PB00 -  Push Button PB0 (WSTK P17)

* Board: Silicon Labs EFR32xG26 Radio Board (BRD4117A) + Wireless Starter Kit Mainboard
	* Device: EFR32MG26B420F3200IM48
		* PB01 -  Push Button PB0 (WSTK P17)

* Board: Silicon Labs EFR32xG27 Radio Board (BRD4194A) + Wireless Starter Kit Mainboard
	* Device: EFR32MG27C140F768IM40
		* PB00 -  Push Button PB0 (WSTK P4, Expansion Header Pin 7)

* Board: Silicon Labs EFR32xG28 Radio Board (BRD4400C) + Wireless Starter Kit Mainboard
	* Device: EFR32ZG28B312F1024IM68
		* PB01 -  Push Button PB0 (WSTK P17)
		
* Board: Silicon Labs EFR32xG29 Buck Radio Board (BRD4412A) + Wireless Starter Kit Mainboard
    * Device: EFR32MG29B140F1024IM40
        * PB00  - Push Button PB0 (WSTK P4, Expansion Header Pin 7)

* Board:  Silicon Labs EFR32FG2D 868-915 MHz 14 dBm Radio Board (BRD4277A) + Wireless Starter Kit Mainboard
    * Device: EFR32FG2DB010F512IM48
		* PA06  - TIM0_CC0 (WSTK P8, Expansion Header Pin 11)
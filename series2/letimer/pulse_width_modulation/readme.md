# Peripheral Examples - LETIMER Pulse Width Modulation #

## Summary ##

This project demonstrates pulse width modulation using the LETIMER.
The project initializes the letimer for PWM with a set 30 percent 
duty cycle at 1 Hz frequency. The waveform is output on an 
Expansion Header Pin on the board.

## Peripherals used ##
LETIMER0 - PWM mode

## How to Test ##

1. Build the project and download to the Starter Kit.
2. Measure output on the appriopriate pin listed below.

## Hardware & Connections ##

* Board: Silicon Labs EFR32xG21 2.4 GHz 10 dBm Board (BRD4181A) + Wireless Starter Kit Mainboard (BRD4001A)
	* Device: EFR32MG21A010F1024IM32
		* PA06 - LET0_O0 (Expansion Header Pin 14)

* Board: Silicon Labs EFR32xG22 Radio Board (BRD4182A) + Wireless Starter Kit Mainboard
	* Device: EFR32MG22C224F512IM40
		* PA06 - LET0_O0 (Expansion Header Pin 14)

* Board: Silicon Labs EFR32xG23 Radio Board (BRD4204D) + Wireless Starter Kit Mainboard
	* Device: EFR32ZG23B010F512IM48
		* PA00 - LET0_O0 (Expansion Header Pin 5)

* Board: Silicon Labs EFR32xG24 Radio Board (BRD4186C) + Wireless Starter Kit Mainboard
	* Device: EFR32MG24B210F1536IM48
		* PA00 - LET0_O0 (Expansion Header Pin 5)

* Board: Silicon Labs EFR32xG25 Radio Board (BRD4270B) + Wireless Starter Kit Mainboard
	* Device: EFR32FG25B222F1920IM56
		* PA00 - LET0_O0 (Expansion Header Pin 5)

* Board: Silicon Labs EFR32xG26 Radio Board (BRD4117A) + Wireless Starter Kit Mainboard
	* Device: EFR32MG26B420F3200IM48
		* PA00 - LET0_O0 (Expansion Header Pin 5)

* Board: Silicon Labs EFR32xG27 Radio Board (BRD4194A) + Wireless Starter Kit Mainboard
	* Device: EFR32MG27C140F768IM40
		* PA06 - LET0_O0 (Expansion Header Pin 14)

* Board: Silicon Labs EFR32xG28 Radio Board (BRD4400C) + Wireless Starter Kit Mainboard
	* Device: EFR32ZG28B312F1024IM68
		* PA12 - LET0_O0 (Expansion Header Pin 5)
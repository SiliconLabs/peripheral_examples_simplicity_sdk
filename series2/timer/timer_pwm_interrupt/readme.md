# Peripheral Examples - TIMER Pulse Width Modulation (PWM) with Interrupts #

## Summary ##

This project demonstrates pulse width modulation using the TIMER module. TIMER is initialized for PWM on Compare/Capture channel 0 which is routed to the GPIO Pin specified below. In PWM mode, overflow events set the output pin, while compare events clear the pin. Thus the overflow value is set to output the desired signal frequency, while the CCV is set to control the duty cycle. The duty cycle is altered by writing to the CCVB register, which safely updates the compare value on the next overflow event.

## Peripherals used ##

* EM01GRPACLK - Sourced from HFRCODPLL @ 19 MHz
* TIMER0 - Pulse Width Modulation Mode

## How to Test ##

1. Build the project and download it to the Starter Kit.
2. Use an oscilloscope to view the 1 KHz signal with 30% duty cycle on the GPIO pin specified below.

## Hardware & Connections ##

* Board: Silicon Labs EFR32xG21 2.4 GHz 10 dBm Board (BRD4181A) + Wireless Starter Kit Mainboard (BRD4001A)
	* Device: EFR32MG21A010F1024IM32
		* PA06  - TIM0_CC0 (WSTK P11, Expansion Header Pin 14)

* Board: Silicon Labs EFR32xG22 Radio Board (BRD4182A) + Wireless Starter Kit Mainboard
	* Device: EFR32MG22C224F512IM40
		* PA06  - TIM0_CC0 (WSTK P11, Expansion Header Pin 14)

* Board: Silicon Labs EFR32xG23 Radio Board (BRD4204D) + Wireless Starter Kit Mainboard
	* Device: EFR32ZG23B010F512IM48
		* PA06  - TIM0_CC0 (WSTK P8, Expansion Header Pin 11)

* Board: Silicon Labs EFR32xG24 Radio Board (BRD4186C) + Wireless Starter Kit Mainboard
	* Device: EFR32MG24B210F1536IM48
		* PA06  - TIM0_CC0 (WSTK P8, Expansion Header Pin 11)

* Board: Silicon Labs EFR32xG25 Radio Board (BRD4270B) + Wireless Starter Kit Mainboard
	* Device: EFR32FG25B222F1920IM56
		* PA06  - TIM0_CC0 (WSTK P8, Expansion Header Pin 11)

* Board: Silicon Labs EFR32xG26 Radio Board (BRD4117A) + Wireless Starter Kit Mainboard
	* Device: EFR32MG26B420F3200IM48
		* PA06  - TIM0_CC0 (WSTK P8,Expansion Header Pin 11)

* Board: Silicon Labs EFR32xG27 Radio Board (BRD4194A) + Wireless Starter Kit Mainboard
	* Device: EFR32MG27C140F768IM40
		* PA06  - TIM0_CC0 (WSTK P11, Expansion Header Pin 14)

* Board: Silicon Labs EFR32xG28 Radio Board (BRD4400C) + Wireless Starter Kit Mainboard
	* Device: EFR32ZG28B312F1024IM68
		* PB04  - TIM0_CC0 (WSTK P8, Expansion Header Pin 11)
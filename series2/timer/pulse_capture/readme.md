# Peripheral Examples - TIMER Pulse Capture #

## Summary ##

This project demonstrates single pulse capture using the TIMER peripheral. The HFXO is configured and selected as the EM01GRPACLK source, which is the clock branch for high-frequency TIMERs on all Series 2 EFM32/EFR32 devices. The TIMER is then configured for input capture on channel CC0 for all edges but with interrupts on every other edge in order to reduce the CPU overhead.  The TIMER input capture logic is double-buffered and can capture two subsequent edges before requesting an interrupt.

The GPIO pin specified below must be connected to a periodic signal or pulse generator. The two edges captured (one falling and one rising) are read from the CCV register.

> Note: The range of frequencies this program can measure accurately is limited by the selected frequency of the EM01GRPACLK, prescaling of the local TIMER clock, and the counter width of the selected TIMER, e.g. TIMER0 is 32 bits wide while other TIMERs are generally 16 bits wide. The minimum measurable period is around 700 ns (about 1.43 MHz).

## Peripherals used ##

* EM01GRPACLK - Sourced from HFXO
* TIMER0 - Input Capture Mode

## How to Test ##

1. Build the project and download it to the Starter Kit.
2. Connect a periodic signal to GPIO pin specified below.
3. Go into debug mode and click run.
4. View the firstEdge and secondEdge global variables in the watch window.

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
		
* Board: Silicon Labs EFR32xG29 Buck Radio Board (BRD4412A) + Wireless Starter Kit Mainboard
    * Device: EFR32MG29B140F1024IM40
        * PA06  - TIM0_CC0 (WSTK P11, Expansion Header Pin 14)

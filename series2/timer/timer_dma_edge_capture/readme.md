# Peripheral Examples - TIMER DMA Edge Capture #

## Summary ##

This project demonstrates DMA driven edge capture from a TIMER Compare/Capture channel. TIMER0 CC0 is configured to capture rising and falling edges. A GPIO Pin is to be connected to a periodic signal, and edges captured from GPIO pin are stored in CC0. The LDMA is configured to transfer the first 512 edges to a fixed length buffer. The buffer is stored globally for possible future processing.

## Peripherals used ##

* EM01GRPACLK - Sourced from HFRCODPLL @ 19 MHz
* TIMER0 - Input Capture Mode
* LDMA - CC0 Peripheral to Memory Transfer

## How to Test ##

1. Build the project and download to the Starter Kit.
2. Connect a periodic signal to GPIO Pin (see board specific pin below).
3. View the buffer[] global array in the debugger.
4. Observe LED0 toggle upon BUFFERSIZE captures of input signal rising and falling edges.

## Hardware & Connections ##

* Board: Silicon Labs EFR32xG21 2.4 GHz 10 dBm Board (BRD4181A) + Wireless Starter Kit Mainboard (BRD4001A)
	* Device: EFR32MG21A010F1024IM32
		* PA06  - TIM0_CC0 (WSTK P11, Expansion Header Pin 14)
		* PB00  - LED0 (WSTK P8, Expansion Header Pin 11)

* Board: Silicon Labs EFR32xG22 Radio Board (BRD4182A) + Wireless Starter Kit Mainboard
	* Device: EFR32MG22C224F512IM40
		* PA06  - TIM0_CC0 (WSTK P11, Expansion Header Pin 14)
		* PD02  - LED0 (WSTK P8, Expansion Header Pin 11)

* Board: Silicon Labs EFR32xG23 Radio Board (BRD4204D) + Wireless Starter Kit Mainboard
	* Device: EFR32ZG23B010F512IM48
		* PA06  - TIM0_CC0 (WSTK P8, Expansion Header Pin 11)
		* PB02  - LED0 (WSTK P21)

* Board: Silicon Labs EFR32xG24 Radio Board (BRD4186C) + Wireless Starter Kit Mainboard
	* Device: EFR32MG24B210F1536IM48
		* PA06  - TIM0_CC0 (WSTK P8, Expansion Header Pin 11)
		* PB02  - LED0 (WSTK P19)

* Board: Silicon Labs EFR32xG25 Radio Board (BRD4270B) + Wireless Starter Kit Mainboard
	* Device: EFR32FG25B222F1920IM56
		* PA06  - TIM0_CC0 (WSTK P8, Expansion Header Pin 11)
		* PC06  - LED0 (WSTK P27)

* Board: Silicon Labs EFR32xG26 Radio Board (BRD4117A) + Wireless Starter Kit Mainboard
	* Device: EFR32MG26B420F3200IM48
		* PA06  - TIM0_CC0 (WSTK P8,Expansion Header Pin 11)
		* PB02  - LED0 (WSTK P19)

* Board: Silicon Labs EFR32xG27 Radio Board (BRD4194A) + Wireless Starter Kit Mainboard
	* Device: EFR32MG27C140F768IM40
		* PA06  - TIM0_CC0 (WSTK P11, Expansion Header Pin 14)
		* PB00  - LED0 (WSTK P4, Expansion Header Pin 7)

* Board: Silicon Labs EFR32xG28 Radio Board (BRD4400C) + Wireless Starter Kit Mainboard
	* Device: EFR32ZG28B312F1024IM68
		* PB04  - TIM0_CC0 (WSTK P8, Expansion Header Pin 11)
		* PB02  - LED0 (WSTK P19)
		
* Board: Silicon Labs EFR32xG29 Buck Radio Board (BRD4412A) + Wireless Starter Kit Mainboard
    * Device: EFR32MG29B140F1024IM40
        * PA06  - TIM0_CC0 (WSTK P11, Expansion Header Pin 14)
        * PB00  - LED0 (WSTK P4, Expansion Header Pin 7)

# Peripheral Examples - USART Asynch with Interrupts #

## Summary ##

This project demonstrates interrupt-driven operation of the USART in
asynchronous mode.  USART0 is configured for asynchronous operation at
115200 baud with 8 data bits, no parity, and one stop bit (115200N81).
The main loop waits until 80 characters or a carriage return are received
and then echos these back to the user.

Note: This example uses inclusive lexicon wherever possible. For more
information, visit https://www.silabs.com/about-us/inclusive-lexicon-project

## Peripherals used ##

* GPIO
* USART0

The CMU is used indirectly via the USART_InitAsync() function to calculate the
divisor necessary to derive the desired baud rate.

## How to Test ##

1. Build the project and download to the Starter Kit.
2. Open a terminal program and configure it for 115200N81 operation on the
   "JLink CDC UART Port" that is provided by the board controller on the
   Starter Kit mainboard.
3. Type some characters in the terminal program (they will not show) and
   press Enter to have the MCU echo them.

Alternatively, the example may be tested with a USB-to-serial converter,
such as the Silicon Labs CP2102N-EK.  Refer to the list below for the
mapping of USART signals to Expansion (EXP) Header pins.

## Hardware & Connections ##

Listed below are the port and pin mappings for working with this example.

* Board: Silicon Labs EFR32xG21 2.4 GHz 10 dBm Radio Board (BRD4181A) + Wireless Starter/Pro Kit Mainboard (BRD4001A/BRD4002A)
	* Device: EFR32MG21A010F1024IM32
		* PA05  - USART0_TX (Expansion Header Pin 12)
		* PA06  - USART0_RX (Expansion Header Pin 14)

* Board: Silicon Labs EFR32xG22 2.4 GHz 6 dBm Radio Board (BRD4182A) + Wireless Starter/Pro Kit Mainboard (BRD4001A/BRD4002A)
	* Device: EFR32MG22C224F512IM40
		* PA05  - USART0_TX (Expansion Header Pin 12)
		* PA06  - USART0_RX (Expansion Header Pin 14)

* Board:  Silicon Labs EFR32ZG23 868-915 MHz 14 dBm Radio Board (BRD4204D)+ Wireless Starter/Pro Kit Mainboard (BRD4001A/BRD4002A)
	* Device: EFR32ZG23B010F512IM48
		* PA08  - USART0_TX (Expansion Header Pin 12)
		* PA09  - USART0_RX (Expansion Header Pin 14)

* Board:  Silicon Labs EFR32MG24 2.4 GHz 10 dBm Radio Board (BRD4186C)+ Wireless Starter/Pro Kit Mainboard (BRD4001A/BRD4002A)
	* Device: EFR32MG24B210F1536IM48
		* PA08  - USART0_TX (Expansion Header Pin 12)
		* PA09  - USART0_RX (Expansion Header Pin 14)

* Board:  Silicon Labs EFR32MG26 2.4 GHz 20 dBm Radio Board (BRD4117A)+ Wireless Starter/Pro Kit Mainboard (BRD4001A/BRD4002A)
	* Device: EFR32MG26B420F3200IM48
		* PA08  - USART0_TX (Expansion Header Pin 12)
		* PA09  - USART0_RX (Expansion Header Pin 14)

* Board:  Silicon Labs EFR32xG27 Buck Radio Board (BRD4194A) + Wireless Starter/Pro Kit Mainboard (BRD4001A/BRD4002A)
	* Device: EFR32MG27C140F768IM40
		* PA05  - USART0_TX (Expansion Header Pin 12)
		* PA06  - USART0_RX (Expansion Header Pin 14)

* Board:  Silicon Labs EFR32xG28 868/915 MHz +14 dBm + 2.4 GHz +10 dBm Radio Board (BRD4400C) + Wireless Starter/Pro Kit Mainboard (BRD4001A/BRD4002A)
	* Device: EFR32ZG28B312F1024IM68
		* PA08  - USART0_TX (WSTK Breakout Pad J102 P28)
		* PA09  - USART0_RX (WSTK Breakout Pad J102 P30)
		
* Board:  Silicon Labs EFR32xG29 Wireless 2.4 GHz 8 dBm Buck Radio Board (BRD4412A) + Wireless Starter/Pro Kit Mainboard (BRD4001A/BRD4002A)
	* Device: EFR32MG29B140F1024IM40
		* PA05  - USART0_TX (Expansion Header Pin 12)
		* PA06  - USART0_RX (Expansion Header Pin 14)

* Board:  Silicon Labs EFR32FG2D 868-915 MHz 14 dBm Radio Board (BRD4277A) + Wireless Starter/Pro Kit Mainboard (BRD4001A/BRD4002A)
    * Device: EFR32FG2DB010F512IM48
		* PA08  - USART0_TX (Expansion Header Pin 12)
		* PA09  - USART0_RX (Expansion Header Pin 14)

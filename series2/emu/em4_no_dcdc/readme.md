# Peripheral Examples - EM4 no DCDC #

This project demonstrates the datasheet current consumption 
configuration and current consumption numbers in EM4 Energy Mode.

Note: Default project gives EM4 current consumption numbers with no BURTC. 
For EM4 current consumption with BURTC running on LFXO, set BURTC_LFXO_EN to 1

When EFR32xG22 and later devices enter EM2/3, the clock to the CPU debug
block is stopped, effectively dropping the host OC debugger connection.
This examples includes an "escape hatch" mechanism to pause the device so
that a debugger can connect in order to erase flash.  To use this, press
the PB0 push button and, while holding the button, press the RESET button
in the lower right corner of the WSTK.  Upon releasing the RESET button,
the device runs code that sees that PB0 is depressed, turns on LED1, and
issues a breakpoint instruction to halt the CPU.  At this point, a
the debugger connection can be resumed to erase flash, etc.

## Peripherals used ##
BURTC  - 32.768 kHz LFXO clock source

## How to Test ##
1. Open Simplicity Studio and update the kit's firmware using **Device Manager Tool**(if necessary)
2. Build the project and download to the Wireless Starter Kit, then exit
   from the debugger.
3. Open the Simplicity Studio's Energy Profiler.  Select Start Energy Capture
   from the Profiler menu and restart the wireless starter kit using Reset Pin.
4. Zoom in on the Y-axis (current) and observe the change in current draw.

### Note for Testing ###
On BRD4182A, BRD4204D, BRD4270B, BRD4117A, BRD4194A, BRD4400C and BRD4412A
VMCU is a 3.3V supply that powers AVDD and IOVDD. In the 
datasheet, current consumption test conditions have AVDD and IOVDD powered by 
either the DC-DC at 1.8V, an external 1.8V supply, or an external 3.0V supply. 
Due to the design of the radio board, these boards do not replicate the 
datasheet test conditions for current consumption, and the measured value may 
differ from the datasheet value.

### Additional note for Testing on xG25 ###
Simplicity Studio's Energy Profiler tool only measures the VMCU current. VMCU 
only powers IOVDD0-1 and the serial flash on BRD4270B. The 3.6 V LDO powers the
rest of the rails on the radio board. In the datasheet, the current 
consumption with DCDC test conditions requires IOVDD0-2, RFVDD, and DVDD to be 
powered by the output of the DCDC. Keep in mind, due to the design of the radio
board (BRD4270B), this example does not replicate the datasheet test conditions
followed for current consumption with DCDC. Although RFVDD and DVDD are powered 
by the output of the DCDC, IOVDD0-1 is powered by VMCU on the radio board and 
IOVDD2 is powered by the USB_VREG. To measure the 3.6V LDO current, the 
following extra steps must be taken.
1. Complete Steps 1 and 2 from the "How to Test" section above.
2. Resistor R247 is a 0 Ohm resistor that can be removed to  measure the 
   current consumption of the device. Refer to the BRD4270B schematic and assembly 
   documentation for details. Search for these resistor designators in the assembly 
   file to identify their exact locations on the board.
3. Remove R247 and solder two leads to each pad where R247 was previously. 
   Connect these leads to a multimeter to measure the current consumption of the 
   device. 

### Additional note for Testing on xG28 ###
On BRD4400C, there is a diversity SPDT switch on the 2.4 GHz radio output that
will consume ~63 uA of additional current when supplied power via logic high on
GPIO PD02. The pin is configured in disabled mode (high-Z) by default, thus SPDT
switch is powered down by default. Be advised that when PD02 is driven to logic
high, observed current consumption will be higher than what is specified in the 
EFR32ZG28 device datasheet due to this additional integrated circuit.

## Hardware & Connections ##
* Board: Silicon Labs EFR32xG22 2.4 GHz 6 dBm Board (BRD4182A) + Wireless Starter Kit Mainboard (BRD4001A)
	* Device: EFR32MG22C224F512
		* PB00  - push button PB0
		* PD03  - LED1

* Board: Silicon Labs EFR32ZG23 868-915 MHz 14 dBm Board (BRD4204D) + Wireless Starter Kit Mainboard (BRD4001A)
	* Device: EFR32ZG23B010F512IM48
		* PB01  - push button PB0
		* PD03  - LED1

* Board:  Silicon Labs EFR32xG24 Radio Board (BRD4186C) +   Wireless Starter Kit Mainboard (BRD4001A)
	* Device: EFR32MG24B210F1536IM48
		* PB01  - push button PB0
		* PB04  - LED1

* Board:  Silicon Labs EFR32xG25 Radio Board (BRD4270B) +   Wireless Starter Kit Mainboard (BRD4001A)
	* Device: EFR32FG25B222F1920IM56
		* PB00  - push button PB0
		* PC07  - LED1

* Board:  Silicon Labs EFR32xG26 Radio Board (BRD4117A) +   Wireless Starter Kit Mainboard (BRD4001A)
	* Device: EFR32MG26B420F3200IM48
		* PB01 -  Push Button PB0
		* PB04 -  LED1

* Board:  Silicon Labs EFR32xG27 Buck Radio Board (BRD4194A) +   Wireless Starter Kit Mainboard (BRD4001A)
	* Device: EFR32MG27C140F768IM40
		* PB00  - push button PB0
		* PB01  - LED1

* Board:  Silicon Labs EFR32xG28 Radio Board (BRD4400C) +   Wireless Starter Kit Mainboard (BRD4001A)
	* Device: EFR32ZG28B312F1024IM68
		* PB01  - push button PB0
		* PD03  - LED1

* Board:  Silicon Labs EFR32xG29 Wireless 2.4 GHz 8 dBm Buck Radio Board (BRD4412A) + Wireless Starter Kit Mainboard (BRD4001A)
	* Device: EFR32MG29B140F1024IM40
		* PB00 - push button PB0
		* PB01 - LED1

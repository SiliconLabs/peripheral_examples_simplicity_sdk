# Peripheral Examples - DCDC VMON #

## Summary ##
This project demonstrates the DCDC VREGVDD Threshold Comparator.
The device will be switching between DCDC regulator on and DCDC bypass mode 
depending on the input voltage to VREGVDD as below:
- Switch DCDC to bypass mode if VREGVDD is below 2.2 V
- Enable DCDC if VREGVDD is above 2.2 V

## Peripherals used ##
EMU

## How to Test ##

### Basic testing ###
1. Update the kit's firmware from the Simplicity Launcher (if necessary)
2. Build the project and download to the Starter Kit
3. If connected, disconnect Simplicity Studio debugger from the starter kit (just click disconnect button in Simplicity Studio)
4. Place the Starter Kit power switch into BAT to disconnect the debugger's power 
5. Attach a power supply to GND and VMCU pins of the WSTK Expansion Header; Positive 3.30V should be applied to EXP pin 2 and 0.0V applied to EXP pin 1
6. Press the reset button the mainboard
7. Vary the power supply voltage above/below the thresholds as per the instructions on the terminal program to start testing 
8. Observe the state of LED1. LED1 turns on when bypass mode is enabled and turns off again when bypass mode is disabled.

Note: When applying voltages independently to supply rails, please observe the operating conditions in the device's data sheet.

### Simplified Testing With A Pro Kit Mainboard (BRD40002A): ###

1. Follow steps 1, 2, and 3 above.
2. While in the Simplicity Studio IDE, right-click on the target system in the Debug Adapters panel and select Launch Console...
3. Click the Admin tab, then press Enter.
4. Change the VMCU supply voltage to 2 V by typing "target voltage 2 --nocalibrate" (no quotation marks) and pressing Enter. LED1 will turn on. 
5. Raise VMCU back to the typical 3.3 V by typing "target voltage 3.30 --nocalibrate" (again, no quotation marks) and pressing Enter. LED1 will turn off.
6. When testing is complete, close the console tab in the IDE.

### Note for testing on xG25 ###
VMCU only powers IOVDD0-1 and the serial flash on BRD4270B. The 3.6 V LDO powers the
rest of the rails on the radio board. Therefore, in order to use VMCU to control the
threshold for different voltage monitoring rails, VMCU will need to power all the rails.
To test the DC-DC voltage monitoring example on BRD4270B, make the following modification:
1. Complete Steps 1 and 2 from the "How to Test" section above.
2. Select the device in the Debug Adapters pane within the Simplicity Studio
   Launcher view. 
3. Select the Documentation tab, then check the Schematic and Layout file 
   resource checkbox. Open the schematic and assembly files for BRD4270B.
4. Unmount resistor R211 and mount resistor R210. Search for the resistors in the assembly 
   file to determine its position on the board.

## Hardware & Connections ##

* Board:  Silicon Labs EFR32xG22 Radio Board (BRD4182A) + Wireless Starter Kit Mainboard (BRD4001A)
	* Device: EFR32MG22A224F512IM40
		* VMCU - Expansion Header Pin 2
		* GND  - Expansion Header Pin 1
		* PD03  - LED1

* Board:  Silicon Labs EFR32xG23 Radio Board (BRD4204D) + Wireless Starter Kit Mainboard (BRD4001A)
	* Device: EFR32ZG23B010F512IM48
		* VMCU - Expansion Header Pin 2
		* GND  - Expansion Header Pin 1 
		* PD03  - LED1

* Board:  Silicon Labs EFR32xG24 Radio Board (BRD4186C) + Wireless Starter Kit Mainboard (BRD4001A)
	* Device: EFR32MG24B210F1536IM48
		* VMCU - Expansion Header Pin 2
		* GND  - Expansion Header Pin 1 
		* PB04  - LED1

* Board:  Silicon Labs EFR32xG25 Radio Board (BRD4270B) + Wireless Starter Kit Mainboard (BRD4001A)
	* Device: EFR32FG25B222F1920IM56
		* VMCU - Expansion Header Pin 2
		* GND  - Expansion Header Pin 1
		* PC07  - LED1

* Board:  Silicon Labs EFR32xG26 Radio Board (BRD4117A) + Wireless Starter Kit Mainboard (BRD4001A)
	* Device: EFR32MG26B420F3200IM48
		* VMCU - Expansion Header Pin 2
		* GND  - Expansion Header Pin 1
		* PB04 -  LED1

* Board:  Silicon Labs EFR32xG27 Radio Board (BRD4194A) + Wireless Starter Kit Mainboard (BRD4001A)
	* Device: EFR32MG27C140F768IM40
		* VMCU - Expansion Header Pin 2
		* GND  - Expansion Header Pin 1 
		* PB01  - LED1

* Board:  Silicon Labs EFR32xG28 Radio Board (BRD4400C) +Wireless Starter Kit Mainboard (BRD4001A)
	* Device: EFR32ZG28B312F1024IM68
		* VMCU - Expansion Header Pin 2
		* GND  - Expansion Header Pin 1 
		* PD03  - LED1

* Board:  Silicon Labs EFR32xG29 Wireless 2.4 GHz 8 dBm Buck Radio Board (BRD4412A) + Wireless Starter Kit Mainboard (BRD4001A)
	* Device: EFR32MG29B140F1024IM40
		* VMCU - Expansion Header Pin 2
		* GND  - Expansion Header Pin 1 
		* PB01 - LED1

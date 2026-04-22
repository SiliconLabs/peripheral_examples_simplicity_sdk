# Peripheral Examples - DCDC Voltage Monitor #

This project demonstrates using the DCDC VREGVDD comparator interrupt to change 
DCDC regulation mode when above/below the configured VREGVDD input voltage threshold.

This project initiates with the DCDC component in bypass mode. The VREGVDD comparator
is used to monitor the DCDC input voltage (VREGVDD) and associated interrupt is used
to switch the DCDC between bypass and regulation modes (for dual-output devices, 
comparator interrupt switches between DECOUPLE only and dual-output modes).

When the VREGVDD input is above the defined VREGVDD_THRESHOLD_HI threshold, single
output DCDC devices will switch to regulating DVDD and dual-output devices will switch
to dual-output mode regulating both DVDD and DECOUPLE. The VREGVDD threshold is also
modified to VREGVDD_THRESHOLD_LOW adding some hysteresis to the thresholds.

When VREGVDD input drops below VREGVDD_THRESHOLD_LOW threshold, the comparator interrupt
reverts the threshold to VREGVDD_THRESHOLD_HI and switches DCDC mode to bypass for 
single output DCDC devices and DECOUPLE only regulation mode for dual-output devices. 

The VREGVDD comparator is active and generates interrupts in EM0 and EM1 only.

The Power Manager software component allowing deep-sleep is used in this example 
to drop to EM2. A periodic timer is used to wake the device and allow the interrupt
to implement DCDC regulation changes as necessary. The periodic timer toggles LED1
on the Wireless Pro Kit to simulate other firmware actions in a real-world application
(RTOS scheduled tasks, e.g.). While in low energy mode EM2, current consumption is
minimal and battery voltage is expected to remain steady. Battery voltage changes 
most likely occur in higher energy modes, hence monitoring availability in only EM0/1.
LED0 is used to indicate when the DCDC is in regulation for single output devices and 
in dual-ouput mode for dual-output supported devices.

## Peripherals used ##
EMU/DCDC

## How to Test ##

### Basic testing ###
1. Open Simplicity Studio and update the kit's firmware using **Device Manager Tool**(if necessary)
2. Build the project and download to the Wireless Starter or Wireless Pro Kit and associated
3. If debugging is active, terminate the debug session in your IDE to disconnect from the target device
4. Place the kit's power switch into BAT to disconnect the debugger's power 
5. Attach a power supply to VMCU and GND pins of the WSTK/WPK Expansion Header; Positive 3.30V
   should be applied to EXP pin 2 and 0.0V applied to EXP pin 1
6. Press the reset button the mainboard
7. Observe the periodic toggle of LED1 indicating that the firmware is running (LED brightness
   will vary with supplied voltage
8. Vary the power supply voltage above/below the defined VREGVDD_THRESHOLD_HI and 
   VREGVDD_THRESHOLD_LOW thresholds
9. Observe the state of LED0. LED0 turns on when below threshold and turns off when above; 
   Corresponding DCDC regulation mode changes are summarized in the description above

Note: When applying voltages independently to supply rails, please observe the operating conditions in the device's data sheet.

### Simplified Testing With A Pro Kit Mainboard (BRD40002A): ###

1. Follow steps 1, 2, and 3 above.
2. While in the Simplicity Studio IDE, navigate to **Tools → Device Manager Tool**.
3. Select your board, click **Configure**, then open **Terminal → Admin**.
4. Change the VMCU supply voltage to 2.0 V by typing "target voltage 2.0 --nocalibrate" (no quotation marks) and pressing Enter. LED0 will turn on. 
5. Raise VMCU back to the typical 3.3 V by typing "target voltage 3.30 --nocalibrate" (again, no quotation marks) and pressing Enter. LED0 will turn off.
6. When testing is complete, close the console tab in the IDE.

### Note for testing on xG25 ###
VMCU only powers IOVDD0-1 and the serial flash on BRD4270B. The 3.6 V LDO powers the
rest of the rails on the radio board. Therefore, in order to use VMCU to control the
threshold for different voltage monitoring rails, VMCU will need to power all the rails.
To test the DC-DC voltage monitoring example on BRD4270B, make the following modification:
1. Complete Steps 1 and 2 from the "Basic testing" section above.
2. Unmount resistor R211 and mount resistor R210. Refer to the BRD4270B schematic and assembly documentation for details. Search for these resistor designators in the assembly file to identify their exact locations on the board.
3. Complete Steps 4 to 8 from the "Basic testing" section above.

## Hardware & Connections ##

* Board:  Silicon Labs EFR32xG22 Radio Board (BRD4182A) + Wireless Starter Kit Mainboard (BRD4001A)
	* Device: EFR32MG22A224F512IM40
		* VMCU - Expansion Header Pin 2
		* GND  - Expansion Header Pin 1
		* PB00 - Push Button PB0 (Escape Hatch)
        * PD02 - LED0 - DCDC Comparator Output
		* PD03 - LED1

* Board:  Silicon Labs EFR32xG23 Radio Board (BRD4204D) + Wireless Starter Kit Mainboard (BRD4001A)
	* Device: EFR32ZG23B010F512IM48
		* VMCU - Expansion Header Pin 2
		* GND  - Expansion Header Pin 1 
		* PB01 - Push Button PB0 (Escape Hatch)
        * PB02 - LED0 - DCDC Comparator Output
		* PD03 - LED1

* Board:  Silicon Labs EFR32xG24 Radio Board (BRD4186C) + Wireless Starter Kit Mainboard (BRD4001A)
	* Device: EFR32MG24B210F1536IM48
		* VMCU - Expansion Header Pin 2
		* GND  - Expansion Header Pin 1 
		* PB01 - Push Button PB0 (Escape Hatch)
        * PB02 - LED0 - DCDC Comparator Output
		* PB04 - LED1

* Board:  Silicon Labs EFR32xG25 Radio Board (BRD4270B) + Wireless Starter Kit Mainboard (BRD4001A)
	* Device: EFR32FG25B222F1920IM56
		* VMCU - Expansion Header Pin 2
		* GND  - Expansion Header Pin 1
		* PB00 - Push Button PB0 (Escape Hatch)
        * PC06 - LED0 - DCDC Comparator Output
		* PC07 - LED1

* Board:  Silicon Labs EFR32xG26 Radio Board (BRD4117A) + Wireless Starter Kit Mainboard (BRD4001A)
	* Device: EFR32MG26B420F3200IM48
		* VMCU - Expansion Header Pin 2
		* GND  - Expansion Header Pin 1
		* PB01 - Push Button PB0 (Escape Hatch)
        * PB02 - LED0 - DCDC Comparator Output
		* PB04 - LED1

* Board:  Silicon Labs EFR32xG27 Radio Board (BRD4194A) + Wireless Starter Kit Mainboard (BRD4001A)
	* Device: EFR32MG27C140F768IM40
		* VMCU - Expansion Header Pin 2
		* GND  - Expansion Header Pin 1 
		* PB00 - Push Button PB0 (Escape Hatch)
        * PB00 - LED0 - DCDC Comparator Output
		* PB01 - LED1

* Board:  Silicon Labs EFR32xG28 Radio Board (BRD4400C) +Wireless Starter Kit Mainboard (BRD4001A)
	* Device: EFR32ZG28B312F1024IM68
		* VMCU - Expansion Header Pin 2
		* GND  - Expansion Header Pin 1 
		* PB01 - Push Button PB0 (Escape Hatch)
        * PB02 - LED0 - DCDC Comparator Output
		* PD03 - LED1

* Board:  Silicon Labs EFR32xG29 Wireless 2.4 GHz 8 dBm Buck Radio Board (BRD4412A) + Wireless Starter Kit Mainboard (BRD4001A)
	* Device: EFR32MG29B140F1024IM40
		* VMCU - Expansion Header Pin 2
		* GND  - Expansion Header Pin 1 
		* PB00 - Push Button PB0 (Escape Hatch)
        * PB00 - LED0 - DCDC Comparator Output
		* PB01 - LED1
		
* Board:  Silicon Labs EFR32FG2D 868-915 MHz 14 dBm Radio Board (BRD4277A) + Wireless Starter Kit Mainboard
    * Device: EFR32FG2DB010F512IM48
		* VMCU - Expansion Header Pin 2
		* GND  - Expansion Header Pin 1
        * PB01 - Push Button PB0 (Escape Hatch)
        * PB02 - LED0 - DCDC Comparator Output
		* PD03 - LED1

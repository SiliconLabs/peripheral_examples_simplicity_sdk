# Peripheral Examples - IADC Voltage Monitor #

## Summary ##

This project demonstrates how to use the window comparator feature in the IADC 
as a voltage monitor of the MCU supply voltages via internal mux. 
 
The IADC is enabled in continous single conversion mode with the internal 1.2 V 
bandgap voltage as its reference voltage. This project demonstrates supply 
voltage monitoring for AVDD and IOVDD.
 
> Note: AVDD is shorted to VREGVDD on the starter kit. So AVDD can be treated
       as VREGVDD in this case.

The threshold for bypass mode is chosen as a early warning voltage level. 
The Brown Out Detector (BOD) for the AVDD and IOVDD has an upper threshold of
1.71 V, so the window comparator is configured to trigger on output results 
less than 2 V. WSTK LED1 is toggled on each conversion with results within 
the specified window. The most recent samples within the window comparison in
both channels are also stored globally.

## Peripherals Used ##

* CMU
  * HFRCODPLL @ 19 MHz
* EMU  
* GPIO  
* IADC  
  * 12-bit resolution  
  * Automatic 2's Complement (single-ended = unipolar)   
  * Unbuffered 3.3V (AVDD) IADC voltage reference  
  * IADC and reference kept in warmup mode  
  * Conversions initiated by firmware and triggered continuously

## How to Test ##

1. Update the kit's firmware from the Simplicity Launcher (if necessary)
2. Build the project and download to the Starter Kit
3. Disconnect Simplicity Studio debugger from the starter kit (just click 
   disconnect button in Simplicity Studio)
4. Place the Starter Kit power switch into BAT to disconnect the debugger's power
5. Attach a power supply to GND and VMCU pins of the WSTK Expansion Header; 
   Positive voltage should be applied to EXP pin 2 and 0.0V applied to EXP pin 1
6. Vary the supply voltage below the 2 V threshold and observe the window compare
   interrupt providing early warning that the supply voltages are dropping.
7. Observe GPIO output (WSTK LED1) using an oscilloscope while varying the input
   voltage; GPIO will toggle while the input is within the specified window, and
   hold steady to the last state when the input voltage moves outside the window 

> Note: When applying voltages independently to supply rails, please observe the operating conditions in the device's data sheet.

### Testing on xG25: ###

VMCU only powers IOVDD0-1 and the serial flash on BRD4270B. The 3.6 V LDO powers the
rest of the rails on the radio board. Therefore, in order to use VMCU to control the
threshold for different voltage monitoring rails, VMCU will need to power all the rails.
To test the IADC voltage monitoring example on BRD4270B, make the following modification:
1. Complete Steps 1 and 2 from the "How to Test" section above.
2. Select the device in the Debug Adapters pane within the Simplicity Studio
   Launcher view. 
2. Select the Documentation tab, then check the Schematic and Layout file 
   resource checkbox. Open the schematic and assembly files for BRD4270B.
3. Unmount resistor R211 and mount resistor R210. Search for the resistors in the assembly 
   file to determine its position on the board.
  
## Hardware & Connections ##

* Board:  Silicon Labs EFR32xG21 2.4 GHz 10 dBm Board (BRD4181A) + Wireless Starter Kit Mainboard
    * Device: EFR32MG21A010F1024IM32
        * VMCU - Expansion Header Pin 2
        * GND  - Expansion Header Pin 1
        * PB01 - LED1, WSTK EXP Header Pin 13

* Board:  Silicon Labs EFR32xG22 Radio Board (BRD4182A) + Wireless Starter Kit Mainboard
    * Device: EFR32MG22A224F512IM40
        * VMCU - Expansion Header Pin 2
        * GND  - Expansion Header Pin 1
        * PD03 - LED1, WSTK EXP Header Pin 13

* Board:  Silicon Labs EFR32xG23 Radio Board (BRD4204D) + Wireless Starter Kit Mainboard
    * Device: EFR32ZG23B010F512IM48
        * VMCU - Expansion Header Pin 2
        * GND  - Expansion Header Pin 1
        * PD03 - LED1, WSTK P26

* Board:  Silicon Labs EFR32xG24 Radio Board (BRD4186C) + Wireless Starter Kit Mainboard
    * Device: EFR32MG24B210F1536IM48
        * VMCU - Expansion Header Pin 2
        * GND  - Expansion Header Pin 1
        * PB04 - LED1, WSTK P26

* Board:  Silicon Labs EFR32xG25 Radio Board (BRD4270B) + Wireless Starter Kit Mainboard
	* Device: EFR32FG25B222F1920IM56
        * VMCU - Expansion Header Pin 2
        * GND  - Expansion Header Pin 1
        * PC08 - LED1, WSTK P26

* Board:  Silicon Labs EFR32xG26 Radio Board (BRD4117A) + Wireless Starter Kit Mainboard
    * Device: EFR32MG26B420F3200IM48
        * VMCU - Expansion Header Pin 2
        * GND  - Expansion Header Pin 1
        * PB04 - LED1, WSTK P26

* Board:  Silicon Labs EFR32xG27 Radio Board (BRD4194A) + Wireless Starter Kit Mainboard
    * Device: EFR32MG27C140F768IM40
        * VMCU - Expansion Header Pin 2
        * GND  - Expansion Header Pin 1
        * PB01 - LED1, WSTK EXP Header Pin 9

* Board:  Silicon Labs EFR32xG28 Radio Board (BRD4400C) + Wireless Starter Kit Mainboard
    * Device: EFR32ZG28B312F1024IM68
        * VMCU - Expansion Header Pin 2
        * GND  - Expansion Header Pin 1
        * PD03 - LED1, WSTK P23
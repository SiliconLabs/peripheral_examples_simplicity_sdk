# Peripheral Examples - VDAC Sample/Off Mode #

## Summary ##

This project demonstrates the VDAC operating in sample/off mode with a single‑ended output to generate 0.5 V on a pin while the device is in EM3.  
In this mode, the VDAC only drives the output during the output hold time. After that, the voltage is maintained by external circuitry. Depending on the application, additional components such as a bulk capacitor or low‑pass RC filter may be required. Filter design, refresh rate, and hold times should be selected based on the acceptable output ripple.

Note:  
On BRD4270B, the VDAC CH0 Main output is connected to an external pullup resistor, capacitor, and pushbutton on the WSTK. To avoid interference, the EFR32xG25 example uses the VDAC CH0 Auxiliary output on the ABUS instead.

Approximate current consumption values were measured using Simplicity Studio Energy Profiler. Projects were built using the default import configuration: Debug (-g3) and no optimization (gcc -O0).  
This example uses a refresh rate of 32 clock cycles, and additional measurements for slower refresh rates are included below. Slower refresh rates reduce energy consumption but allow more voltage droop between refresh cycles depending on load and filtering.

Testing on xG25: 
For BRD4270B, VMCU powers only IOVDD and the serial flash. The 3.6 V LDO powers the rest of the radio board. To measure the 3.6 V LDO current, replace R247 with ammeter connections. The listed current values represent the combined VMCU + 3.6 V LDO current.

Testing on xG28:  
On BRD4400C, a diversity SPDT switch on the 2.4 GHz RF path consumes ~63 µA when powered via logic high on GPIO PD02.  
By default, PD02 is in disabled (high‑Z) mode, so the switch is off. Driving PD02 high will increase current consumption beyond the values shown in the EFR32ZG28 datasheet.

## Current Consumption Measurements (EM3) ##

Refresh rate: VDAC_REFRESH_CLK cycles

| Board     | 32 clks | 64 clks | 128 clks | 256 clks |
|-----------|---------|---------|----------|----------|
| BRD4204D  | 15 µA   | 10 µA   | 8 µA     | 6 µA     |
| BRD4186C  | 15 µA   | 10 µA   | 8 µA     | 6 µA     |
| BRD4270B  | 17 µA   | 11 µA   | 8 µA     | 7 µA     |
| BRD4117A  | 19 µA   | 13 µA   | 10 µA    | 9 µA     |
| BRD4400C  | 16 µA   | 10 µA   | 8 µA     | 6 µA     |

## Peripherals Used ##

* CMU – HFRCODPLL @ 19 MHz, LFRCO @ 32.768 kHz via EM23GRPACLK  
* EMU  
* USART – used only to power down onboard SPI flash  
* VDAC – internal 1.25 V reference, sample/off mode  

## How to Test ##

1. Build the project and download it to the Starter Kit.  
2. Use an oscilloscope to measure the VDAC output pin relative to ground.

## Hardware & Connections ##

* Board:  Silicon Labs EFR32ZG23 868-915 MHz 14 dBm Radio Board (BRD4204D) + Wireless Starter Kit Mainboard
    * Device: EFR32ZG23B010F512IM48
        * PB00 – VDAC0 CH0 Main Output (Breakout Pad Pin 15)

* Board:  Silicon Labs EFR32MG24 2.4 GHz 10 dBm Radio Board (BRD4186C) + Wireless Starter Kit Mainboard
    * Device: EFR32MG24B210F1536IM48
        * PB00 – VDAC0 CH0 Main Output (Breakout Pad Pin 15)
		
* Board:  Silicon Labs EFR32FG25 902-928 MHz 14 dBm Radio Board (BRD4270B) + Wireless Starter Kit Mainboard
    * Device: EFR32FG25B222F1920IM56
        * PA06 – VDAC0 CH0 Auxiliary Output (Breakout Pad Pin 8)
		
* Board:  Silicon Labs EFR32MG26 2.4 GHz 20 dBm Radio Board (BRD4117A) + Wireless Starter Kit Mainboard
    * Device: EFR32MG26B420F3200IM48
        * PB00 – VDAC0 CH0 Main Output (Breakout Pad Pin 15)
		
* Board:  Silicon Labs EFR32xG28 868/915 MHz +14 dBm + 2.4 GHz +10 dBm Radio Board (BRD4400C) + Wireless Starter Kit Mainboard
    * Device: EFR32ZG28B312F1024IM68
        * PB00 – VDAC0 CH0 Main Output (Breakout Pad Pin 15)

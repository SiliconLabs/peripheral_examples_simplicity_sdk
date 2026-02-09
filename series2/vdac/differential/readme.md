# Peripheral Examples - VDAC Differential Mode #

## Summary ##

This project demonstrates the VDAC operating in continuous mode with a differential output, generating a 0.5 V difference between two pins while the device is in EM3.  
Because the VDAC runs independently of the core, the output remains stable even under load. Continuous operation is reflected in the device’s power consumption.

Note:  
The VDAC uses the channel 0 data register as the input source.  
- CH0 outputs the negative signal  
- CH1 outputs the positive signal  

Approximate current consumption values were measured using Simplicity Studio Energy Profiler. Projects were built using the default import configuration: Debug (-g3) and no optimization (gcc -O0). Refer to device‑specific datasheets for detailed low‑energy mode current specifications.

Testing on xG25: 
For BRD4270B, VMCU powers only IOVDD and the serial flash. The 3.6 V LDO powers the rest of the radio board. To measure the 3.6 V LDO current, replace R247 with ammeter connections. The listed current values represent the combined VMCU + 3.6 V LDO current.

Testing on xG28:  
On BRD4400C, a diversity SPDT switch on the 2.4 GHz RF path consumes ~63 µA when powered via logic high on GPIO PD02.  
By default, PD02 is in disabled (high‑Z) mode, so the switch is off. Driving PD02 high will increase current consumption beyond the values shown in the EFR32ZG28 datasheet.

## Current Consumption Measurements ## 

| Board     | Avg Current EM3 | Avg Current EM1 (Install Power Manager: no deepsleep)    |
|-----------|-----------------|----------------------------------------------------------|
| BRD4204D  | 51 µA           | 470 µA                                                   |
| BRD4186C  | 52 µA           | 570 µA                                                   |
| BRD4270B  | 55 µA           | 905 µA                                                   |
| BRD4117A  | 56 µA           | 1800 µA                                                  |
| BRD4400C  | 52 µA           | 710 µA                                                   |

## Peripherals Used ## 

* CMU – HFRCODPLL @ 19 MHz, HFRCOEM23 @ 19 MHz  
* EMU  
* USART – used only to power down onboard SPI flash  
* VDAC – internal 1.25 V reference, differential mode  

## How to Test ##

1. Build the project and download it to the Starter Kit.  
2. Use an oscilloscope to measure the differential voltage between VDAC CH0 and VDAC CH1.

## Hardware & Connections  ##

* Board:  Silicon Labs EFR32ZG23 868-915 MHz 14 dBm Radio Board (BRD4204D) + Wireless Starter Kit Mainboard
    * Device: EFR32ZG23B010F512IM48
        * PB00 – VDAC0 CH0 Main Output (Breakout Pad Pin 15)
		* PB01 – VDAC0 CH1 Main Output (Breakout Pad Pin 17)

* Board:  Silicon Labs EFR32MG24 2.4 GHz 10 dBm Radio Board (BRD4186C) + Wireless Starter Kit Mainboard
    * Device: EFR32MG24B210F1536IM48
        * PB00 – VDAC0 CH0 Main Output (Breakout Pad Pin 15)
		* PB01 – VDAC0 CH1 Main Output (Breakout Pad Pin 17)
		
* Board:  Silicon Labs EFR32FG25 902-928 MHz 14 dBm Radio Board (BRD4270B) + Wireless Starter Kit Mainboard
    * Device: EFR32FG25B222F1920IM56
        * PB00 – VDAC0 CH0 Main Output (Breakout Pad Pin 17)
		* PB01 – VDAC0 CH1 Main Output (Breakout Pad Pin 21)
		
* Board:  Silicon Labs EFR32MG26 2.4 GHz 20 dBm Radio Board (BRD4117A) + Wireless Starter Kit Mainboard
    * Device: EFR32MG26B420F3200IM48
        * PB00 – VDAC0 CH0 Main Output (Breakout Pad Pin 15)
		* PB01 – VDAC0 CH1 Main Output (Breakout Pad Pin 17)
		
* Board:  Silicon Labs EFR32xG28 868/915 MHz +14 dBm + 2.4 GHz +10 dBm Radio Board (BRD4400C) + Wireless Starter Kit Mainboard
    * Device: EFR32ZG28B312F1024IM68
        * PB00 – VDAC0 CH0 Main Output (Breakout Pad Pin 15)
		* PB01 – VDAC0 CH1 Main Output (Breakout Pad Pin 17)
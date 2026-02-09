# VDAC Timer DMA Waveform Example #

## Summary ##

This project uses the DAC/VDAC and TIMER0 to output a 32‑point sine wave at a particular frequency (10 kHz by default). The project uses the LDMA to write to the CH0F buffer. This project operates in EM1 because the timer cannot operate in EM2/EM3.

Note:  
The BRD4270B's VDAC CH0 Main output is connected to an external pullup resistor, capacitor, and pushbutton on the WSTK. To avoid interference from these components, the EFR32xG25 example uses the VDAC CH0 Auxiliary output to output to a pin on the ABUS.

Approximate current consumption measurements are provided below using Simplicity Studio's built‑in energy profiler. Projects were built with the default import configuration – Debug build configuration (-g3) and no optimization (gcc -O0). Refer to device‑specific datasheets for more details about low‑energy mode currents.

Testing on xG25: 
For BRD4270B, VMCU powers only IOVDD and the serial flash. The 3.6 V LDO powers the rest of the radio board. To measure the 3.6 V LDO current, replace R247 with ammeter connections. The listed current values represent the combined VMCU + 3.6 V LDO current.

Testing on xG28:  
On BRD4400C, there is a diversity SPDT switch on the 2.4 GHz radio output that consumes ~63 µA of additional current when supplied power via logic high on GPIO PD02. The pin is configured in disabled mode (high‑Z) by default, thus the SPDT switch is powered down. When PD02 is driven high, observed current consumption will be higher than what is specified in the EFR32ZG28 device datasheet.

## Current Consumption Measurements (EM1) ##

| Board     | Avg Current EM1 |
|-----------|-----------------|
| BRD4204D  | 615 µA          |
| BRD4186C  | 725 µA          |
| BRD4270B  | 860 µA          |
| BRD4117A  | 1900 µA         |
| BRD4400C  | 880 µA          |

## Peripherals Used ##

* CMU – HFRCODPLL @ 19 MHz via EM01GRPCCLK  
* EMU  
* LDMA – memory‑to‑peripheral data transfer  
* TIMER – TIMER0 @ 320 kHz (WAVEFORM_FREQ x SINE_TABLE_SIZE)  
* VDAC – internal 1.25 V reference, continuous mode  

## How To Test ##

1. Build the project and download it to the Starter Kit.  
2. Measure the VDAC output pin with respect to ground.

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
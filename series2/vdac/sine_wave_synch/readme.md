# Peripheral Examples - VDAC Sine Wave Synchronous Example #

## Summary ##

This project uses the VDAC and the internal sine wave generator to produce a 16‑point sine wave at a frequency of f_sinewave, centered at VREF/2. The example operates in EM1, and by default outputs a 31.25 kHz sine wave.

Channel behavior:  
- The sine wave is always output on Channel 0.  
- Channel 1 may still be used independently as a single‑ended DAC output.  
- If differential mode is enabled, the sine wave appears on both channels, with Channel 1 inverted.

Note:  
On BRD4270B, the VDAC CH0 Main output is connected to an external pullup resistor, capacitor, and pushbutton on the WSTK. To avoid interference, the EFR32xG25 example uses the VDAC CH0 Auxiliary output on the ABUS instead.

DAC clock limitations:  
The DAC clock (DAC_CLK) must be ≤ 1 MHz.  
The input clock is EM01GRPACLK, defaulting to 19 MHz.

    f_DAC_CLK = f_IN_CLK / (PRESCALE + 1)
    f_sinewave = f_EM01GRPACLK / (32 x (PRESCALE + 1))

The prescaler is 7 bits → valid range: 1 to 128.

Although the VDAC can operate in EM3, synchronous reflexes require EM01GRPACLK, which is disabled in EM2/3. Therefore, the sine generator does not operate in EM2/3.

Testing on xG25: 
For BRD4270B, VMCU powers only IOVDD and the serial flash. The 3.6 V LDO powers the rest of the radio board. To measure the 3.6 V LDO current, replace R247 with ammeter connections. The listed current values represent the combined VMCU + 3.6 V LDO current.

Testing on xG28:  
On BRD4400C, a diversity SPDT RF switch consumes ~63 µA when PD02 is driven high. PD02 defaults to disabled (high‑Z), keeping the switch off. Driving PD02 high increases current consumption beyond datasheet values.

## Current Consumption Measurements (EM1) ##

| Board     | Avg Current EM1 (min)  | Avg Current EM1 (max)  |
|-----------|------------------------|------------------------|
| BRD4204D  | 560 µA                 | 1100 µA                |
| BRD4186C  | 660 µA                 | 840 µA                 |
| BRD4270B  | 740 µA                 | 940 µA                 |
| BRD4117A  | 1820 µA                | 2000 µA                |
| BRD4400C  | 830 µA                 | 1050 µA                |

## Peripherals Used ##

* CMU – HFRCODPLL @ 19 MHz via EM01GRPCCLK  
* EMU  
* VDAC – internal 1.25 V reference, continuous mode  

## How to Test ##

1. Build the project and download it to the Starter Kit.  
2. Use an oscilloscope to observe the sine wave output on the VDAC pin.

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
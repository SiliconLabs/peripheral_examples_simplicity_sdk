# Peripheral Examples - ADC Scan Window Compare - Voltage Monitor #

## Summary ##

This project demonstrates using the ADC peripheral's window comparison feature with the four internally routed supply voltage ADC inputs.

The window comparator is configured to interrupt on ADC conversion results which are inside the specified window. Though it is not possible to set different window comparison thresholds for different channels, there are two configuration settings that can be used to adjust the gain. In this case, DECOUPLE supply gain configuration uses a 1X multiplier, while the remaining AVDD, IOVDD, and DVDD use a 0.5X multiplier, such that the same thresholds correspond to 0.85V threshold for DECOUPLE and 1.70V threshold for AVDD, IOVDD, and DVDD.
A GPIO is toggled ON/OFF in the ADC interrupt handler, signaling the last conversion result fell within the specified window. The most recent conversion result within the comparison window is also stored in a global buffer variable.

NOTE: To modify this example to take a differential external measurement, the negative port and pin for scan table entries must be changed. To take a differential measurement, the analog multiplexer selection must consist of one EVEN ABUS channel and one ODD ABUS channel.

## Peripherals used ##

* ADC
    * Clock manager utilizes 38.4 MHz HFXO for ADCCLK input
        * CLK_CMU_ADC and CLK_SRC_ADC running at 38.4 MHz
        * Prescaled CLK_CORE_ADC/CLK_SAR running at 19.2 MHz
    * Acquisition time set to 4 CLK_CORE_ADC cycles
        * Conversion phase lasts 13 CLK_SAR cycles for a total of 17 cycles per conversion
    * Warmup mode set to KEEPWARM after initial power-up
    * Immediate trigger from software with continuous single channel scans
    * Internal reference voltage (1.2 V)
    * Gain setting to 0.5 -> full-scale at 2.4 V
    * Single conversion resultper channel; no averaging
    * GPIO port/pin configured for low noise mode
    * Comparison window set to trigger when input voltage is below 1.70V for AVDD, IOVDD, and DVDD, below 0.85V for DECOUPLE

## How to Test ##

1. Open Simplicity Studio and update the kit's firmware using **Device Manager Tool**(if necessary).
2. Place the Wireless Pro Kit power switch into BAT to disconnect the debugger's power
3. Attach an external adjustable power supply to GND and VMCU pins of the WPK Expansion Header: 
   * Connect positive 3.3 VDC to EXP pin 2
   * Connect negative GND (0.0 VDC) to EXP pin 1
4. Build the example project and download it to the target system.
5. Start a debug session in the IDE and add "sample" and "scanResults" to the Watch/Variables window.
6. Set a breakpoint at the end of the ADC0_Handler().
7. Run the example project.
8. Slowly adjust the external supply voltage down to the threshold for AVDD/IOVDD/DVDD (near or below 1.70V)
9. At the breakpoint, observe the raw data sample and calculated voltages for the four internal supplies within the Watch/Variables window:
10. Observe the LED0 toggle while input voltage is within the specified comparison window.

## Hardware & Connections ##

* Board: Silicon Labs SixG301 Radio Board (BRD4407A) + Wireless Pro Kit Mainboard
    * Device: SIMG301M104LIL
        * PD03 - LED0 output, Lower Breakout Header P31
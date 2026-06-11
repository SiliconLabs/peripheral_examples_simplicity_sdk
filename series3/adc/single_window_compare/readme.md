# Peripheral Examples - ADC Single Window Compare #

## Summary ##

This project demonstrates using the ADC peripheral's window comparison feature with a single-ended, external input.

The window comparator is configured to interrupt on ADC conversion results which are inside the specified window. A GPIO is toggled ON/OFF while within the ADC interrupt handler, signaling that last conversion result fell within the specified window. The most recent conversion result within the comparison window is also stored in a global variable.

NOTE: To modify this example to take a differential external measurement, the negative port and pin for scan table entries must change. To take a differential measurement, the analog multiplexer selection must consist of one EVEN ABUS channel and one ODD ABUS channel.

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
    * Single conversion result; no averaging
    * GPIO port/pin configured for low noise mode
    * Comparison window set to trigger when input voltage is between 0.60V and 1.80V

## How to Test ##

1. Open Simplicity Studio and update the kit's firmware using **Device Manager Tool**(if necessary).
2. Build the example project and download it to the target system.
3. Start a debug session in the IDE and add "sample" and "singleResult" to the Watch/Variables window.
4. Set a breakpoint at the end of the ADC0_Handler().
5. Run the example project.
6. Adjust the analog input voltage between the defined window (between 0.60V and 1.80V)
7. At the breakpoint, observe the raw data and calculated voltage in the Watch/Variables window:
   * Observe how these values respond to different voltage inputs on the corresponding pin.
8. Observe the LED0 toggle while input voltage is within the specified comparison window.

NOTE: ADC peripheral has been configured to allow halting the ADC while debugging. With the ADC continuously converting, the ADC conversion result FIFO can fill and overflow while processing data or while debugging.

## Hardware & Connections ##

* Board: Silicon Labs SixG301 Radio Board (BRD4407A) + Wireless Pro Kit Mainboard
    * Device: SIMG301M104LIL
        * PA06 - ADC positive input, Expansion Header 11
        * PD03 - LED0 output, Lower Breakout Header P31
# Peripheral Examples - ADC Scan with ADC Timer #

## Summary ##

This project demonstrates use of the ADC to take single-ended analog measurements of two input channels, with conversions triggered by the local ADC peripheral timer.

After the ADC peripheral is initialized, an ADC scan is enabled and the internal ADC timer is started. Each ADC timer event triggers a scan conversion of the two external input channels. The ADC interrupt handler, located in RAM to minimize conversion latency, triggers once the conversions complete, storing the calculated voltage to a global buffer, then finally clears the interrupt flag before returning execution to the main application loop.

NOTE: To modify this example to take a differential external measurement, the negative port and pin for scan table entries must be changed. To take a differential measurement, the analog multiplexer selection must consist of one EVEN ABUS channel and one ODD ABUS channel.

## Peripherals used ##

* ADC
    * Clock Manager utilizes 38.4 MHz HFXO for ADCCLK input
        * CLK_CMU_ADC and CLK_SRC_ADC running at 38.4 MHz
        * Prescaled CLK_CORE_ADC/CLK_SAR running at 19.2 MHz
    * Acquisition time set to 4 CLK_CORE_ADC cycles
        * Conversion phase lasts 13 CLK_SAR cycles for a total of 17 cycles per conversion
    * Warmup mode set to KEEPWARM after initial power-up
    * Immediate trigger from ADC timer with scan conversion of two channels per trigger
    * ADC Timer configured to trigger scans at 1000 Hz
    * Internal reference voltage (1.2 V)
    * Gain setting to 0.3125 -> full-scale at 3.84 V
    * Single conversion result per channel; no averaging
    * GPIO port/pin configured for low noise mode

## How to Test ##

1. Open Simplicity Studio and update the kit's firmware from the Simplicity Launcher (if necessary).
2. Build the example project and download it to the target system.
3. Open the Simplicity Debugger perspective and add "sample" and "scanResults" to the Expressions Window.
4. Set a breakpoint at the end of the ADC0_Handler().
5. Run the example project.
6. At the breakpoint, observe the raw data sample and calculated voltages within the Expressions window:
   * Observe how these values respond to different voltage inputs on the corresponding pins.
7. Using an oscilloscope, observe the ADC GPIO output toggle with each two channel scan completion.

## Hardware & Connections ##

* Board: Silicon Labs SixG301 Radio Board (BRD4407A) + Wireless Pro Kit Mainboard
    * Device: SIMG301M104LIL
        * PA00 - ADC output, Expansion Header 5
        * PA06 - ADC positive input 0, Expansion Header 11
        * PA07 - ADC positive input 1, Expansion Header 13

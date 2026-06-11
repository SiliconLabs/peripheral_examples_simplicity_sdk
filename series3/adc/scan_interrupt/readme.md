# Peripheral Examples - ADC Scan Interrupt #

## Summary ##

This project demonstrates using the ADC peripheral to take single-ended analog measurements of two external input channels and four internal supply channels, with interrupt upon conversion completion of the six channel scan.

Within the application initialization, after GPIO and ADC peripherals are configured, the ADC is triggered in software for continuous conversion of the scan table. The ADC interrupt handler, located in RAM to minimize conversion latency, triggers when conversion of the scan table completes and stores the calculated voltage to a results buffer before clearing the interrupt flag and returning to the main application loop. A GPIO output is set at the beginning and cleared at the end of the interrupt handler to demonstrate execution time.

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
    * Gain setting to 0.3125 -> full-scale at 3.84 V
    * Single conversion result per channel; no averaging
    * GPIO ports/pins configured for low noise mode

## How to Test ##

1. Open Simplicity Studio and update the kit's firmware using **Device Manager Tool**(if necessary).
2. Build the example project and download it to the target system.
3. Start a debug session in the IDE and add "sample" and "scanResults" to the Watch/Variables window.
4. Set a breakpoint at the end of the ADC0_Handler().
5. Run the example project.
6. At the breakpoint, observe the raw data sample and calculated voltages in the Watch/Variables window:
   * Observe how these values respond to different voltage inputs on the corresponding pins.

NOTE: ADC peripheral has been configured to allow halting the ADC while debugging. With the ADC continuously converting, the ADC conversion result FIFO can fill and overflow while processing data or while debugging.

## Hardware & Connections ##

* Board: Silicon Labs SixG301 Radio Board (BRD4407A) + Wireless Pro Kit Mainboard
    * Device: SIMG301M104LIL
        * PA00 - ADC output, Expansion Header 5
        * PA06 - ADC positive input 0, Expansion Header 11
		* PA07 - ADC positive input 1, Expansion Header 13
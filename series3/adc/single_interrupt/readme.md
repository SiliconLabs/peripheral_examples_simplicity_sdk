# Peripheral Examples - ADC Single Interrupt #

## Summary ##

This project demonstrates using the ADC peripheral to take single-ended analog measurements using a single, external input. 

Within the application initialization, after GPIO and ADC peripherals are configured, the ADC is triggered in software for continuous conversion of the single scan table entry. The ADC interrupt handler triggers with each conversion, and stores the raw conversion value and calculated voltage to global variables before clearing the interrupt flag and returning to the main application loop.

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
    * Single conversion result; no averaging
    * GPIO port/pin configured for low noise mode

## How to Test ##

1. Open Simplicity Studio and update the kit's firmware from the Simplicity Launcher (if necessary).
2. Build the example project and download it to the target system.
3. Open the Simplicity Debugger perspective and add "sample" and "singleResult" to the Expressions Window.
4. Set a breakpoint at the end of the ADC0_IRQHandler.
5. Run the example project.
6. At the breakpoint, observe the raw data and calculated voltage in the Expressions window:
   * Observe how these values respond to different voltage inputs on the corresponding pin.

NOTE: By setting a breakpoint with the ADC continuously converting, the ADC conversion result FIFO will fill and overflow, requiring multiple cycles through the breakpoint to observe voltage changes on the analog input.

## Hardware & Connections ##

* Board: Silicon Labs SixG301 Radio Board (BRD4407A) + Wireless Pro Kit Mainboard
    * Device: SIMG301M104LIL
        * PA06 - ADC positive input, Expansion Header 11

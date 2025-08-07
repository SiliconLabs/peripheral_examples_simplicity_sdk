# Peripheral Examples - ADC Scan LETIMER Interrupt #

## Summary ##

This project demonstrates using the ADC peripheral to take single-ended analog measurements of two external inputs, with conversions triggered periodically by LETIMER underflow interrupt. 

After the ADC and LETIMER peripherals are initialized, the LETIMER is started. When an LETIMER underflow event occurs, the LETIMER interrupt service routine is triggered, kicking off an ADC converion of the scan table. The ADC interrupt handler triggers once the conversion completes, stores the calculated voltage to global buffer, then finally clears the interrupt flag before returning execution to the main application loop.

NOTE: To modify this example to take a differential external measurement, the negative port and pin for scan table entries must be changed. To take a differential measurement, the analog multiplexer selection must consist of one EVEN ABUS channel and one ODD ABUS channel.

## Peripherals used ##

* ADC
    * Clock Manager utilizes 38.4 MHz HFXO for ADCCLK input
        * CLK_CMU_ADC and CLK_SRC_ADC running at 38.4 MHz
        * Prescaled CLK_CORE_ADC/CLK_SAR running at 19.2 MHz
    * Acquisition time set to 4 CLK_CORE_ADC cycles
        * Conversion phase lasts 13 CLK_SAR cycles for a total of 17 cycles per conversion
    * Warmup mode set to KEEPWARM after initial power-up
    * Immediate trigger from software with one conversion of single channel scan per trigger
    * Internal reference voltage (1.2 V)
    * Gain setting to 0.3125 -> full-scale at 3.84 V
    * Single conversion result; no averaging
    * GPIO port/pin configured for low noise mode
* LETIMER
    * Clock Manager utilizes the 32.768 kHz LFRCO for EM23GRPACLK and LETIMER0CLK
    * Timer configured to utilize top value calculated for LETIMER_FREQ defined frequency
    * Timer runs in free mode, toggling an output GPIO with each underflow

## How to Test ##

1. Open Simplicity Studio and update the kit's firmware from the Simplicity Launcher (if necessary).
2. Build the example project and download it to the target system.
3. Open the Simplicity Debugger perspective and add "sample" and "scanResults" to the Expressions Window.
4. Set a breakpoint at the end of the ADC0_Handler().
5. Run the example project.
6. At the breakpoint, observe the raw data sample and calculated voltages in the Expressions window:
   * Observe how these values respond to different voltage inputs on the corresponding pin.
7. Using oscilloscope, observe the LETIMER GPIO output toggle with each underflow / ADC conversion trigger.

## Hardware & Connections ##

* Board: Silicon Labs SixG301 Radio Board (BRD4407A) + Wireless Pro Kit Mainboard
    * Device: SIMG301M104LIL
        * PA05 - LETIMER output, Expansion Header 9
        * PA06 - ADC positive input 0, Expansion Header 11
        * PA07 - ADC positive input 1, Expansion Header 13

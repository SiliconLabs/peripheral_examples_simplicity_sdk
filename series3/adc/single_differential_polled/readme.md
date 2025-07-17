# Peripheral Examples - ADC Single Differential Polled #

## Summary ##

This project demonstrates using the ADC peripheral to take a single, differential measurement of analog voltage across two inputs. 

After initialization, the application process starts a conversion, then polls the ADC status register until conversion is complete. The raw conversion data and calculated voltage are stored in two global variables.

## Peripherals used ##

* ADC
    * Clock manager utilizes 38.4 MHz HFXO for ADCCLK input
        * CLK_CMU_ADC and CLK_SRC_ADC running at 38.4 MHz
        * Prescaled CLK_CORE_ADC/CLK_SAR running at 19.2 MHz
    * Acquisition time set to 4 CLK_CORE_ADC cycles
        * Conversion phase lasts 13 CLK_SAR cycles for a total of 17 cycles per conversion
    * Warmup mode set to KEEPWARM after initial power-up
    * Immediate trigger from software with one conversion of single channel scan per trigger
    * Internal reference voltage (1.2 V)
    * Gain setting to 0.3125 -> differential full-scale from -3.84 V to 3.84 V (NOTE: all analog inputs must be above -0.3 V as specified in datasheet; negative voltage calculation reflects a higher voltage on the negative input relative to positive input)
    * Single conversion result; no averaging
    * GPIO port/pin configured for low noise mode

## How to Test ##

1. Open Simplicity Studio and update the kit's firmware from the Simplicity Launcher (if necessary).
2. Build the example project and download it to the target system.
3. Open the Simplicity Debugger perspective and add "sample" and "singleResult" to the Expressions Window.
4. Set a breakpoint at the end of the app_process_action().
5. Run the example project.
6. At the breakpoint, observe the raw data and calculated voltage in the Expressions window:
   * Observe how these values respond to different voltage inputs on the corresponding differential input pins.

## Hardware & Connections ##

* Board: Silicon Labs SixG301 Radio Board (BRD4407A) + Wireless Pro Kit Mainboard
    * Device: SIMG301M104LIL
        * PA06 - ADC positive input, Expansion Header 11
        * PA07 - ADC negative input, Expansion Header 13

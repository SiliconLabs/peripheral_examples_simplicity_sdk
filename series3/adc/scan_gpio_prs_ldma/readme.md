# Peripheral Examples - ADC Scan GPIO PRS LDMA #

## Summary ##

This project demonstrates using the ADC peripheral to take single-ended analog measurements of two input channels. Conversions are triggered by rising edge of a GPIO input via PRS. 

After initialization, with each rising edge of the configured GPIO input, an ADC scan is triggered, one conversion per channel enabled in the scan. Upon completion of NUM_SAMPLES ADC conversions, an LDMA data transfer occurs, storing the raw data to a memory buffer using the DMADRV software component. The LDMA interrupts when transfer completes, and DMADRV callback is used to toggle a GPIO output before returning to the main application loop and starting a new data transfer.

NOTE: To modify this example to take a differential external measurement, the negative port and pin for scan table entries must be changed. To take a differential measurement, the analog multiplexer selection must consist of one EVEN ABUS channel and one ODD ABUS channel.

## Peripherals used ##

* ADC
    * Clock manager utilizes 38.4 MHz HFXO for ADCCLK input
        * CLK_CMU_ADC and CLK_SRC_ADC running at 38.4 MHz
        * Prescaled CLK_CORE_ADC/CLK_SAR running at 19.2 MHz
    * Acquisition time set to 4 CLK_CORE_ADC cycles
        * Conversion phase lasts 13 CLK_SAR cycles for a total of 17 cycles per conversion
    * Warmup mode set to NORMAL, powering down the ADC after each conversions and requiring 5 us to power-up
    * Immediate trigger from positive edge of GPIO via PRS
    * Internal reference voltage (1.2 V)
    * Gain setting to 0.3125 -> full-scale at 3.84 V
    * Single conversion result per channel; no averaging
    * GPIO port/pin configured for low noise mode
    * Data valid level (DVL) set to NUM_SAMPLES to kick off LDMA transfer once all samples are collected
* PRS
* LDMA (via DMADRV; used with wireless protocols and channel allocation required to cofunction with radio applications)
    * Peripheral to memory transfer of NUM_SAMPLES from ADC Scan FIFO
    * Note: DMADRV APIs return a status code which can be utilized when debugging and for improved system security

## How to Test ##

1. Open Simplicity Studio and update the kit's firmware using **Device Manager Tool**(if necessary).
2. Build the example project and download it to the target system.
3. Start a debug session in the IDE and add "scanBuffer" to the Watch/Variables window.
4. Set a breakpoint at the end of the ldma_callback() function.
5. Run the example project.
6. Press and release push button BTN0 (rising edge on release) NUM_SAMPLES / 2 times to collect ADC samples (2 conversions per scan)
   * Vary voltage between presses to observe change in conversion results once LDMA transfer completes.
   * Observe LED0 toggle on/off. 
   * Both channel conversions are present in the buffer, meaning every other conversion result corresponds to each of the two channels. Channel ID is stored in the raw data's 8 MSBs. 
7. At the breakpoint, observe the raw data stored in the buffer within the Watch/Variables window:
   * Observe how these values respond to different voltage inputs on the corresponding pin.
8. Repeat steps 5-7

## Hardware & Connections ##

* Board: Silicon Labs SixG301 Radio Board (BRD4407A) + Wireless Pro Kit Mainboard
    * Device: SIMG301M104LIL
        * PA06 - ADC positive input 0, Expansion Header 11
        * PA07 - ADC positive input 1, Expansion Header 13
        * PB01 - Push button BTN0 input, Lower Breakout Header P16
        * PD03 - LED0 output, Lower Breakout Header P31

# Peripheral Examples - ADC Scan Continuous LDMA #

## Summary ##

This project demonstrates use of the ADC to continuously sample two single-ended input channels, using LDMA to transfer conversion results to a global buffer variable.

After the ADC and DMADRV are initialized, continuous ADC conversion of the scan table and DMA transfer are started. After NUM_SAMPLES of ADC conversions, an LDMA data transfer occurs, storing the raw data from the ADC FIFO to a memory buffer using the DMADRV software component. The LDMA interrupts upon transfer completion, and DMADRV callback is used to toggle a GPIO before returning to the main application loop and starting a new data transfer.

NOTE: To modify this example to take a differential external measurement, the negative port and pin for scan table entries must change. To take a differential measurement, the analog multiplexer selection must consist of one EVEN ABUS channel and one ODD ABUS channel.

## Peripherals used ##

* ADC
    * Clock manager utilizes 38.4 MHz HFXO for ADCCLK input
        * CLK_CMU_ADC and CLK_SRC_ADC running at 38.4 MHz
        * Prescaled CLK_CORE_ADC/CLK_SAR running at 19.2 MHz
    * Acquisition time set to 4 CLK_CORE_ADC cycles
        * Conversion phase lasts 13 CLK_SAR cycles for a total of 17 cycles per conversion
    * Warmup mode set to KEEPWARM, keeping the ADC fully powered as conversions are continuous
    * Immediate trigger from software start of ADC
    * Internal reference voltage (1.2 V)
    * Gain setting to 0.3125 -> full-scale at 3.84 V
    * Single conversion result per channel; no averaging
    * GPIO port/pin configured for low noise mode
    * Data valid level (DVL) set to NUM_SAMPLES to kick off LDMA transfer once all samples are collected
* LDMA (via DMADRV; used with wireless protocols and channel allocation required to cofunction with radio applications)
    * Peripheral to memory transfer of NUM_SAMPLES from ADC Scan FIFO
    * Note: DMADRV APIs return a status code which can be utilized when debugging and for improved system security

## How to Test ##

1. Open Simplicity Studio and update the kit's firmware from the Simplicity Launcher (if necessary).
2. Build the example project and download it to the target system.
3. Open the Simplicity Debugger perspective and add "scanBuffer" to the Expressions Window.
4. Set a breakpoint at the end of the ldma_callback() function.
5. Run the example project.
6. At the breakpoint, observe the raw data stored in the buffer within the Expressions window:
   * LDMA callback is called when transfer completes after NUM_SAMPLES of LETIMER underflow events.
   * Observe how these values respond to different voltage inputs on the corresponding pin.
   * Both channel conversions are present in the buffer, meaning every other conversion result corresponds to each of the two channels. Channel ID is stored in the raw data's 8 MSBs. 
7. Repeat steps 5-6
   * Vary voltage between debug breaks to observe change in conversion results once LDMA transfer completes
   * Observe LED0 toggle on/off with each LDMA transfer cycle

## Hardware & Connections ##

* Board: Silicon Labs SixG301 Radio Board (BRD4407A) + Wireless Pro Kit Mainboard
    * Device: SIMG301M104LIL
        * PA06 - ADC positive input 0, Expansion Header 11
        * PA07 - ADC positive input 1, Expansion Header 13
        * PD03 - LED0 output, Lower Breakout Header P31

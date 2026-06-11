# Peripheral Examples - ADC Single LETIMER PRS LDMA #

## Summary ##

This project demonstrates use of the ADC to take single-ended analog measurements of a single channel, triggered periodically through PRS underflow of LETIMER.

After initialization, underflows of the LETIMER triggers an ADC conversion of a single external input via PRS. After NUM_SAMPLES of ADC conversions, an LDMA data transfer occurs, storing the raw data to a memory buffer using the DMADRV software component. The LDMA interrupts upon transfer completion, and DMADRV callback is used to toggle a GPIO before returning to the main application loop and starting a new data transfer. The LETIMER overflow events are observable via GPIO toggle at defined LETIMER_FREQ toggle rate.

NOTE: To modify this example to take a differential external measurement, the negative port and pin for scan table entries must change. To take a differential measurement, the analog multiplexer selection must consist of one EVEN ABUS channel and one ODD ABUS channel.

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
    * Single conversion result; no averaging
    * GPIO port/pin configured for low noise mode
    * Data valid level (DVL) set to NUM_SAMPLES to kick off LDMA transfer once all samples are collected
* PRS
* LDMA (via DMADRV; used with wireless protocols and channel allocation required to cofunction with radio applications)
    * Peripheral to memory transfer of NUM_SAMPLES from ADC Scan FIFO
    * Note: DMADRV APIs return a status code which can be utilized when debugging and for improved system security
* LETIMER
    * Clock Manager utilizes the 32.768 kHz LFRCO for EM23GRPACLK and LETIMER0CLK
    * Timer configured to utilize top value calculated for LETIMER_FREQ defined frequency
    * Timer runs in free mode, toggling an output GPIO with each underflow

## How to Test ##

1. Open Simplicity Studio and update the kit's firmware using **Device Manager Tool**(if necessary).
2. Build the example project and download it to the target system.
3. Start a debug session in the IDE and add "singleBuffer" to the Watch/Variables window.
4. Set a breakpoint at the end of the ldma_callback().
5. Run the example project.
6. At the breakpoint, observe the raw data stored in the buffer within the Watch/Variables window:
   * LDMA callback is called when transfer completes after NUM_SAMPLES of LETIMER underflow events.
   * Observe how these values respond to different voltage inputs on the corresponding pin.
7. Repeat steps 5-6
   * Vary voltage between debug breaks to observe change in conversion results once LDMA transfer completes
   * Observe LED0 toggle on/off with each LDMA transfer cycle

## Hardware & Connections ##

* Board: Silicon Labs SixG301 Radio Board (BRD4407A) + Wireless Pro Kit Mainboard
    * Device: SIMG301M104LIL
        * PA05 - LETIMER output, Expansion Header 9
        * PA06 - ADC positive input, Expansion Header 11
        * PD03 - LED0 output, Lower Breakout Header P31

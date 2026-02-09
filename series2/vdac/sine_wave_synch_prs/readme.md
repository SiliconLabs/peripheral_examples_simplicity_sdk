# Peripheral Examples - VDAC Sine Wave Synchronous PRS Example #

## Summary ##

This project demonstrates how to generate a 16‑point sine wave using the VDAC’s internal sine generator while controlling the waveform timing through the Peripheral Reflex System (PRS). The sine wave is centered at the reference voltage divided by 2 and operates in EM1. With the VDAC clock configured to 1 MHz, the resulting sine wave frequency is approximately 31 kHz.

A GPIO pushbutton is routed through PRS to asynchronously trigger the VDAC sine generator.  
PRS channel 7 captures the raw GPIO signal, PRS channel 8 produces an inverted version of that signal, and the inverted signal is used as the VDAC asynchronous trigger source. This allows the VDAC waveform to be controlled entirely through hardware reflexes without CPU involvement.

The sine wave is always output on Channel 0. Channel 1 may still be used independently as a single‑ended DAC output unless differential mode is enabled.

The VDAC clock must be ≤ 1 MHz. With EM01GRPACLK = 19 MHz and PRESCALE = 18, the VDAC clock is 1 MHz, producing a sine wave of approximately 31 kHz.

Although the VDAC can operate in EM3, synchronous reflexes require EM01GRPACLK, which is disabled in EM2/3. Therefore, the sine generator does not operate in EM2/3.

## Peripherals Used ##

* CMU – HFRCODPLL @ 19 MHz via EM01GRPCCLK  
* EMU  
* PRS  
* GPIO  
* VDAC – internal 1.25 V reference, sine generation mode  

## How to Test ##

1. Build the project and download it to the Starter Kit.  
2. Connect an oscilloscope to the VDAC output pin.  
3. Press BUTTON1 to trigger the sine wave generator via PRS.  
4. Observe the ~31 kHz sine wave on the oscilloscope.

## Hardware & Connections ##

* Board:  Silicon Labs EFR32xG28 868/915 MHz +14 dBm + 2.4 GHz +10 dBm Radio Board (BRD4400C) + Wireless Starter Kit Mainboard
    * Device: EFR32ZG28B312F1024IM68
        * PB00 – VDAC0 CH0 Main Output (Breakout Pad Pin 15)
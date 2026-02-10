# Peripheral Examples – LFRCO Polled Calibration #

## Summary ##

This project demonstrates polled calibration of the LFRCO against the HFXO.

The device is initialized for the board it is running on, and the qualified LFRCO is output to a pin where it can be observed. The BURTC is set up to generate an interrupt every 16 seconds, and the device enters EM1 so that the clock output remains active.

Upon wake, the standard polling mechanism is used to calibrate the LFRCO against the HFXO, with the CMU providing the necessary hardware assistance via its up and down counters. The process repeats every 16 seconds.

## Peripherals Used ##

* CMU  
* BURTC  
* GPIO  
* EMU  

## How to Test ##

1. Build the project and download it to the Starter Kit (STK).  
2. Click the Run/Resume button in the debugger.  
3. If desired, set a breakpoint at the `cal_lfrco(32768)` function call in the main loop.  
4. Connect an oscilloscope probe to the STK pin, listed below, on which the LFRCO output is driven.  
5. Step through the `cal_lfrco()` function to see how the counter results are compared and how adjustments are made to the LFRCO tuning. After each tuning adjustment, the change in LFRCO frequency can be observed.

If desired, change the `LFRCO_CAL.FREQTRIM` bit field to a value that is much larger or much smaller than the current value before running the calibration routine. This will make the change in frequency more visible on the scope as calibration runs.

Note: While the factory-calibrated LFRCO frequency is an average of 32.768 kHz, the cycle-to-cycle deviation can be substantial.  

## Hardware & Connections ##

* Board: Silicon Labs EFR32xG21 2.4 GHz 10 dBm Radio Board (BRD4181A) + Wireless Starter Kit Mainboard
    * Device: EFR32MG21A010F1024IM32
        * PC03 -  LFRCO output (Expansion Header Pin 10)

* Board:  Silicon Labs EFR32ZG23 868-915 MHz 14 dBm Radio Board (BRD4204D) + Wireless Starter Kit Mainboard
    * Device: EFR32ZG23B010F512IM48
        * PC03 -  LFRCO output (Expansion Header Pin 8)

* Board:  Silicon Labs EFR32FG25 902-928 MHz 14 dBm Radio Board (BRD4270B) + Wireless Starter Kit Mainboard
    * Device: EFR32FG25B222F1920IM56
        * PC02 -  LFRCO output (Expansion Header Pin 8)

* Board:  Silicon Labs EFR32xG28 868/915 MHz +14 dBm + 2.4 GHz +10 dBm Radio Board (BRD4400C) + Wireless Starter Kit Mainboard
    * Device: EFR32ZG28B312F1024IM68
        * PD09 -  LFRCO output (Expansion Header Pin 8)
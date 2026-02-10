# Peripheral Examples - LFRCO Interrupt-Driven Calibration #

## Summary ##

This project demonstrates interrupt-driven calibration of the LFRCO using the HFXO as a reference.

The device initializes according to the board configuration, and the qualified LFRCO is routed to an output pin for observation. The calibration mechanism is configured and a calibration run begins.

During this demonstration, the firmware waits in a loop until the global variable `tuned` becomes `true`. In a real application, this time could be used for other initialization tasks that do not depend on a stable LFRCO-derived clock.

LFRCO tuning adjustments occur inside `CMU_IRQHandler()`, triggered by the CALRDY interrupt. The down counter decrements at the HFXO frequency, while the up counter counts LFRCO ticks, allowing the CPU to perform other work.

When the down counter reaches zero:
1. The CPU enters `CMU_IRQHandler()` due to the CALRDY flag.  
2. The up counter value is compared to the ideal count.  
3. The tuning value is adjusted.  
4. The process repeats until the ideal count is reached or the previous tuning value is closer than the current one.

After tuning completes, the BURTC is configured to generate an interrupt after ~16 seconds. The device enters EM1 to wait (keeping the clock output active), and the process repeats.

## Peripherals Used ##

* CMU  
* BURTC  
* GPIO  
* EMU  

## How to Test ##

1. Build the project and download it to the Starter Kit.  
2. Click Run/Resume in the debugger.  
3. Optionally set a breakpoint in `CMU_IRQHandler()`.  
4. Connect an oscilloscope to the LFRCO output pin listed for your board.  
5. Step through `CMU_IRQHandler()` to observe counter comparisons and tuning adjustments.  
   After each adjustment, the LFRCO frequency change is visible on the oscilloscope.

If desired, modify the `LFRCO_CAL.FREQTRIM` field to a significantly higher or lower value before running calibration. This increases the visible frequency shift during tuning.

Note: The factory-calibrated LFRCO frequency averages 32.768 kHz, but cycle-to-cycle deviation can be substantial.  

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

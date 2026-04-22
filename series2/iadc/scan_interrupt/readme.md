# Peripheral Examples - IADC Scan Interrupt

## Summary

This project demonstrates use of the IADC to take single-ended analog
measurements across two different external inputs and six of the
internal voltage supply channels.  The IADC operates in EM2 and wakes
the device to save the completed conversion results, convert them to
volts, and store them in a global array.  Because the example runs in
EM2, only ports A and B remain fully functional and can be used for
the external scan inputs.

> Note: To modify this example to take differential external
measurements, the negative inputs for scan table entries 0 and 1 must
change.  To take a differential measurement, the analog multiplexer
selection must consist of one EVEN ABUS channel and one ODD ABUS
channel.

In this example, scan table entry 0 may reference a pin with an odd-numbered
identifier, so an even-numbered port/pin must be selected for the IADC negative
input. Scan table entry 1 may reference a pin with an even-numbered identifier,
so the negative IADC input must be odd-numbered. In general, either input can
be odd or even, but its pair must be the opposite. As in single-ended mode,
the IADC logic will swap the multiplexer connections to the IADC input if
needed. See the reference manual for more details.

## IADC FIFO Depth

IADC FIFO depth varies across Series 2 EFM32/EFR32 devices.

For devices with 4-entry FIFO (EFR32xG21, EFR32xG22, EFR32xG23, and EFR32xG27),
the IADC scan table is configued to process 4 channels at a time to avoid FIFO
overflow. The IADC interrupts on scan table completion, and the IRQ handler
reads the conversion results from the FIFO. The scan table entry IDs
are used to save the results to the proper channel entry within the results
array. Each time the program halts at the test breakpoint, the values in
"scanResult" in the Expression window will contain the first set of scan table
results.

For devices with 8-entry FIFO (EFR32xG24, EFR32xG25, EFR32xG26, EFR32xG28, and EFR32xG2D),
the IADC scan table is configued to process 8 channels at a time to avoid FIFO
overflow. The IADC interrupts on scan table completion and the IRQ handler reads
the conversion results from the FIFO and using the scan table entry IDs, saves
the results to the proper channel entry within the results array. Each time the
program halts at the test breakpoint, "scanResult" in the Expression window will
update with all eight channels' results.

## Peripherals Used

* FSRCO @ 20 MHz
* GPIO
* IADC
  * 12-bit resolution (2x oversampling)
  * Internal VBGR reference with 0.5x analog gain

## How To Test

1. Open Simplicity Studio and update the kit's firmware using **Device Manager Tool**(if necessary).
2. Build the project and download to the Starter Kit.
3. Open the Debugger and add "scanResult" to the Watch/Variables window.
4. Set a breakpoint at the end of the IADC_IRQHandler.
5. Run the project.
6. At the breakpoint, observe the measured voltages in the Watch/Variables
   window and how they respond to different voltage values on the
   corresponding pins and external supply inputs (see below).

## Hardware & Connections

* Board: Silicon Labs EFR32xG21 Radio Board (BRD4181A) + 
         Wireless Starter Kit Mainboard
  * Device: EFR32MG21A010F1024IM32
    * PA05 - IADC input, single-ended, Expansion Header Pin 12, WSTK P9
    * PB00 - IADC input, single-ended, Expansion Header Pin 11, WSTK P8

* Board: Silicon Labs EFR32xG22 Radio Board (BRD4182A) +
         Wireless Starter Kit Mainboard
  * Device: EFR32MG22A224F512IM40
    * PB02 - IADC input, single-ended, Expansion Header Pin 15, WSTK P12
    * PB03 - IADC input, single-ended, Expansion Header Pin 16, WSTK P13

* Board: Silicon Labs EFR32xG23 Radio Board (BRD4204D) + 
         Wireless Starter Kit Mainboard
  * Device: EFR32ZG23B010F512IM48
    * PA05 - IADC input, single-ended, Expansion Header Pin 7, WSTK P4
    * PA06 - IADC input, single-ended, Expansion Header Pin 11, WSTK P8

* Board: Silicon Labs EFR32xG24 Radio Board (BRD4186C) +
         Wireless Starter Kit Mainboard
  * Device: EFR32MG24B210F1536IM48
    * PA05 - IADC input, single-ended, Expansion Header Pin 7, WSTK P4
    * PA06 - IADC input, single-ended, Expansion Header Pin 11, WSTK P8

* Board: Silicon Labs EFR32xG25 Radio Board (BRD4270B) +
         Wireless Starter Kit Mainboard
  * Device: EFR32FG25B222F1920IM56
    * PB02 - IADC input, single-ended, Expansion Header Pin 15, WSTK P12
    * PB03 - IADC input, single-ended, Expansion Header Pin 16, WSTK P13

* Board: Silicon Labs EFR32xG26 Radio Board (BRD4117A) +
         Wireless Starter Kit Mainboard
  * Device: EFR32MG26B420F3200IM48
    * PA08 - IADC input, single-ended, Expansion Header Pin 12, WSTK P9
    * PA09 - IADC input, single-ended, Expansion Header Pin 14, WSTK P11

* Board: Silicon Labs EFR32xG27 Radio Board (BRD4194A) +
         Wireless Starter Kit Mainboard
  * Device: EFR32MG27C140F768IM40
    * PB02 - IADC input, single-ended, Expansion Header Pin 15, WSTK P12
    * PB03 - IADC input, single-ended, Expansion Header Pin 16, WSTK P13

* Board: Silicon Labs EFR32xG28 Radio Board (BRD4400C) +
         Wireless Starter Kit Mainboard
  * Device: EFR32ZG28B312F1024IM68
    * PB04 - IADC input, single-ended, Expansion Header Pin 11, WSTK P8
    * PB05 - IADC input, single-ended, Expansion Header Pin 13, WSTK P10

* Board:  Silicon Labs EFR32xG29 Wireless 2.4 GHz 8 dBm Buck Radio Board 
          (BRD4412A) + Wireless Starter Kit Mainboard (BRD4001A)
  * Device: EFR32MG29B140F1024IM40
    * PB02 - IADC input, single-ended, Expansion Header Pin 15, WSTK P12
    * PB03 - IADC input, single-ended, Expansion Header Pin 16, WSTK P13

* Board:  Silicon Labs EFR32FG2D 868-915 MHz 14 dBm Radio Board 
		  (BRD4277A) + Wireless Starter Kit Mainboard
  * Device: EFR32FG2DB010F512IM48
    * PA05 - IADC input, single-ended, Expansion Header Pin 7, WSTK P4
    * PA06 - IADC input, single-ended, Expansion Header Pin 11, WSTK P8

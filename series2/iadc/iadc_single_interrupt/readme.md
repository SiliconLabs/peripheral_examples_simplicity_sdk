## Peripheral Examples - IADC Single Interrupt

## Summary

This project demonstrates using the IADC peripheral to take single-ended
analog measurements using a single external input.  After starting a
conversion, the device enters EM2 and wakes in response to the end of
conversion interrupt.

Careful pin selection for peripherals operating in EM2 is required
because only port A and B pins remain functional; port C and D pins are
static in EM2 and cannot be used as peripheral inputs and outputs.  For
this reason, the IADC scan channel inputs and LETIMER output in this
example must all be port A/B pins.  However, when LED0 is toggled after
the conversion results are transferred to RAM, the system is in EM0
because the CPU is executing the LDMA IRQ handler, so any GPIO pin can
be used.

> Note: To modify this example to take differential external
measurements, the negative input for single input must change.
To take a differential measurement, the analog multiplexer
selection must consist of one EVEN ABUS channel and one ODD ABUS
channel.

In this example, the single input channel may be a pin with an odd-numbered identifier, so an even-numbered port/pin must be selected for the IADC negative input. Similarly, if the input channel is an even-numbered pin, the negative IADC input must be odd-numbered. As in single-ended mode, the IADC logic will swap the multiplexer connections to the IADC input if needed. See the reference manual for more details.

## Peripherals Used

* FSRCO @ 20 MHz
* GPIO
* IADC
  * 12-bit resolution (2x oversampling)
  * Internal VBGR reference with 0.5x analog gain (1.21V / 0.5 = 2.42V)
  * Conversions triggered by firmware

## How To Test

1. Update the kit's firmware from the Simplicity Studio Launcher, if
   necessary.
2. Build the project and download to the Starter Kit.
3. Open the Debugger and add "singleResult" to the Expressions window.
4. Set a breakpoint at the end of the IADC_IRQHandler.
5. Run the example project
6. At the breakpoint, observe the measured voltages in the Expressions
   window and how it responds to different voltage values on the
   corresponding pin.

## Hardware & Connections

* Board: Silicon Labs EFR32xG21 Radio Board (BRD4181A) + 
         Wireless Starter Kit Mainboard
  * Device: EFR32MG21A010F1024IM32
    * PA05 - IADC input, single-ended, Expansion Header Pin 12, WSTK P9

* Board: Silicon Labs EFR32xG22 Radio Board (BRD4182A) +
         Wireless Starter Kit Mainboard
  * Device: EFR32MG22A224F512IM40
    * PB02 - IADC input, single-ended, Expansion Header Pin 15, WSTK P12

* Board: Silicon Labs EFR32xG23 Radio Board (BRD4204D) + 
         Wireless Starter Kit Mainboard
  * Device: EFR32ZG23B010F512IM48
    * PA05 - IADC input, single-ended, Expansion Header Pin 7, WSTK P4

* Board: Silicon Labs EFR32xG24 Radio Board (BRD4186C) +
         Wireless Starter Kit Mainboard
  * Device: EFR32MG24B210F1536IM48
    * PA05 - IADC input, single-ended, Expansion Header Pin 7, WSTK P4

* Board: Silicon Labs EFR32xG25 Radio Board (BRD4270B) +
         Wireless Starter Kit Mainboard
  * Device: EFR32FG25B222F1920IM56
    * PB02 - IADC input, single-ended, Expansion Header Pin 15, WSTK P12

* Board: Silicon Labs EFR32xG26 Radio Board (BRD4117A) +
         Wireless Starter Kit Mainboard
  * Device: EFR32MG26B420F3200IM48
    * PA08 - IADC input, single-ended, Expansion Header Pin 12, WSTK P9

* Board: Silicon Labs EFR32xG27 Radio Board (BRD4194A) +
         Wireless Starter Kit Mainboard
  * Device: EFR32MG27C140F768IM40
    * PB02 - IADC input, single-ended, Expansion Header Pin 15, WSTK P12

* Board: Silicon Labs EFR32xG28 Radio Board (BRD4400C) +
         Wireless Starter Kit Mainboard
  * Device: EFR32ZG28B312F1024IM68
    * PB04 - IADC input, single-ended, Expansion Header Pin 11, WSTK P8

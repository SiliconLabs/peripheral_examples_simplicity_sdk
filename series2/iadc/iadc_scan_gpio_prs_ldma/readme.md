# Peripheral Examples - IADC Scan GPIO PRS LDMA

## Summary

This project demonstrates use of the IADC to take single-ended analog
measurements from two external inputs, triggered via the PRS in response
to a positive (rising) edge on a user-specified GPIO pin.  Operation is in EM2
with the GPIO positive edge triggering a scan of the selected channels and the
IADC saving those result and requesting an interrupt when all transfers
are complete.  After the specified number of samples are taken in
response to the GPIO triggers, a GPIO output is toggled on the Wireless Starter
Kit mainboard.

Careful pin selection for peripherals operating in EM2 is required
because only port A and B pins remain functional; port C and D pins are
static in EM2 and cannot be used as peripheral inputs or outputs.  For
this reason, the IADC scan channel inputs and GPIO trigger input in this
example must all be port A/B pins.  However, LDMA transfer complete toggle is
performed in the LDMA ISR, operating in EM0, and may utilize pins on any port.

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

## Peripherals Used

* FSRCO @ 20 MHz
* GPIO
* IADC
  * 12-bit resolution (2x oversampling)
  * Internal VBGR reference with 0.5x analog gain
  * PRS scan trigger input 
* LDMA
  * Channel 0
* PRS

## How To Test

1. Update the kit's firmware from the Simplicity Studio Launcher, if
   necessary.
2. Build the project and download to the Starter Kit.
3. Open the Debugger and add "scanBuffer" to the Expressions window.
4. On BRD4181A for EFR32xG21 only, jumper PA06 to PD02 in order to use
   push button 0 to trigger each scan sequence.  Alternatively, toggle
   PA06 low then high repeatedly to generate the falling edge triggers.
5. Set a breakpoint at the end of the ldma_callback.
6. Run the project.
7. Press push button 0 (or toggle PA06 on EFR32xG21 BRD4181A) to trigger
   a scan conversion sequence.  Do this 5 times to collect 10 (the
   default for NUM_SAMPLES) total IADC samples.
8. At the breakpoint, observe the conversion results in the Expressions
   window and how they respond to different voltage values on the
   corresponding pins.

## Hardware & Connections

* Board: Silicon Labs EFR32xG21 Radio Board (BRD4181A) + 
         Wireless Starter Kit Mainboard
  * Device: EFR32MG21A010F1024IM32
    * PA05 - IADC input, single-ended, Expansion Header Pin 12, WSTK P9
    * PB00 - IADC input, single-ended, Expansion Header Pin 11, WSTK P8
    * PA06 - GPIO input, pull-up, filtered, Expansion Header Pin 12 (WSTK P9),
           jumper to Expansion Header Pin 7 (PD02, WSTK P4)
    * PB01 - GPIO push/pull output, Expansion Header Pin 13, WSTK P10

* Board: Silicon Labs EFR32xG22 Radio Board (BRD4182A) +
         Wireless Starter Kit Mainboard
  * Device: EFR32MG22A224F512IM40
    * PB02 - IADC input, single-ended, Expansion Header Pin 15, WSTK P12
    * PB03 - IADC input, single-ended, Expansion Header Pin 16, WSTK P13
    * PB00 - GPIO input, pull-up, filtered, Expansion Header Pin 7, WSTK P4,
           Push Button 0
    * PD03 - GPIO Push/Pull output, Expansion Header Pin 13, WSTK P10, LED1

* Board: Silicon Labs EFR32xG23 Radio Board (BRD4204D) + 
         Wireless Starter Kit Mainboard
  * Device: EFR32ZG23B010F512IM48
    * PA05 - IADC input, single-ended, Expansion Header Pin 7, WSTK P4
    * PA06 - IADC input, single-ended, Expansion Header Pin 11, WSTK P8
    * PB01 - GPIO input, pull-up, filtered, WSTK P17, Push Button 0
    * PD03 - GPIO Push/Pull output, WSTK P26, LED1

* Board: Silicon Labs EFR32xG24 Radio Board (BRD4186C) +
         Wireless Starter Kit Mainboard
  * Device: EFR32MG24B210F1536IM48
    * PA05 - IADC input, single-ended, Expansion Header Pin 7, WSTK P4
    * PA06 - IADC input, single-ended, Expansion Header Pin 11, WSTK P8
    * PB01 - GPIO input, pull-up, filtered, WSTK P17, Push Button 0
    * PB04 - GPIO Push/Pull output, WSTK P26, LED1

* Board: Silicon Labs EFR32xG25 Radio Board (BRD4270B) +
         Wireless Starter Kit Mainboard
  * Device: EFR32FG25B222F1920IM56
    * PB02 - IADC input, single-ended, Expansion Header Pin 15, WSTK P12
    * PB03 - IADC input, single-ended, Expansion Header Pin 16, WSTK P13
    * PB00 - GPIO input, pull-up, filtered, WSTK P17, Push Button 0
    * PC07 - GPIO Push/Pull output, WSTK P26, LED1

* Board: Silicon Labs EFR32xG26 Radio Board (BRD4117A) +
         Wireless Starter Kit Mainboard
  * Device: EFR32MG26B420F3200IM48
    * PA08 - IADC input, single-ended, Expansion Header Pin 12, WSTK P9
    * PA09 - IADC input, single-ended, Expansion Header Pin 14, WSTK P11
    * PB01 - GPIO input, pull-up, filtered, WSTK P17, Push Button 0
    * PB04 - GPIO Push/Pull output, WSTK P26, LED1

* Board: Silicon Labs EFR32xG27 Radio Board (BRD4194A) +
         Wireless Starter Kit Mainboard
  * Device: EFR32MG27C140F768IM40
    * PB02 - IADC input, single-ended, Expansion Header Pin 15, WSTK P12
    * PB03 - IADC input, single-ended, Expansion Header Pin 16, WSTK P13
    * PB00 - GPIO input, pull-up, filtered, WSTK P4, Expansion Header Pin 7, 
           Push Button 0
    * PB01 - GPIO Push/Pull output, WSTK P6, Expansion Header Pin 9, LED1

* Board: Silicon Labs EFR32xG28 Radio Board (BRD4400C) +
         Wireless Starter Kit Mainboard
  * Device: EFR32ZG28B312F1024IM68
    * PB04 - IADC input, single-ended, Expansion Header Pin 11, WSTK P8
    * PB05 - IADC input, single-ended, Expansion Header Pin 13, WSTK P10
    * PB01 - GPIO input, pull-up, filtered, WSTK P17, Push Button 0
    * PD03 - GPIO Push/Pull output, WSTK P23, LED1

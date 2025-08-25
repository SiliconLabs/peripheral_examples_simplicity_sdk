# Peripheral Examples - IADC Scan LETIMER PRS LDMA

## Summary

This project demonstrates use of the IADC to take single-ended analog
measurements using two external inputs, triggered periodically by
LETIMER underflow via PRS.  Operation is in EM2 with once per second
underflow of the LETIMER triggering an IADC scan of the selected channels,
and the IADC saving those results and requesting an interrupt for the
specified number of transfers.  The once per second LETIMER scan
trigger pulse can be observed on a GPIO, as can the completion of each
LDMA transfer with a second GPIO output.

Careful pin selection for peripherals operating in EM2 is required
because only port A and B pins remain functional; port C and D pins are
static in EM2 and cannot be used as peripheral inputs and outputs.  For
this reason, the IADC scan channel inputs and LETIMER output in this
example must all be port A/B pins.  However, when LDMA scan completion toggles
a GPIO after the conversion results are transferred to RAM, the system is in EM0
because the CPU is executing the LDMA IRQ handler, so any GPIO pin can
be used.

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
* LFXO @ 32.768 kHz
* GPIO
* IADC
  * 12-bit resolution (2x oversampling)
  * Internal VBGR reference with 0.5x analog gain (1.21V / 0.5 = 2.42V)
  * PRS scan trigger input 
* LDMA
  * Channel 0
* LETIMER
  * Underflow output on PRS to IADC

## How to Test

1. Update the kit's firmware from the Simplicity Studio Launcher, if
   necessary.
2. Build the project and download to the Starter Kit.
3. Open the Debugger and add "scanBuffer" to the Expressions window.
4. Set a breakpoint at the end of the ldma_callback.
5. Run the project.
6. At the breakpoint, observe the measured voltages in the Expressions
   window and how they respond to different voltage values on the
   corresponding pins.

## Hardware & Connections

* Board: Silicon Labs EFR32xG21 Radio Board (BRD4181A) + 
         Wireless Starter Kit Mainboard
  * Device: EFR32MG21A010F1024IM32
    * PA05 - IADC input, single-ended, Expansion Header Pin 12, WSTK P9
    * PB00 - IADC input, single-ended, Expansion Header Pin 11, WSTK P8
    * PB01 - GPIO Push/Pull output, Expansion Header Pin 13, WSTK P10, LED1
    * PA06 - GPIO Push/Pull output, Expansion Header Pin 14, WSTK P11, LETIMER0

* Board: Silicon Labs EFR32xG22 Radio Board (BRD4182A) +
         Wireless Starter Kit Mainboard
  * Device: EFR32MG22A224F512IM40
    * PB02 - IADC input, single-ended, Expansion Header Pin 15, WSTK P12
    * PB03 - IADC input, single-ended, Expansion Header Pin 16, WSTK P13
    * PD03 - GPIO Push/Pull output, Expansion Header Pin 13, WSTK P10, LED1
    * PB01 - GPIO Push/Pull output, Expansion Header Pin 9, WSTK P6, LETIMER0

* Board: Silicon Labs EFR32xG23 Radio Board (BRD4204D) + 
         Wireless Starter Kit Mainboard
  * Device: EFR32ZG23B010F512IM48
    * PA05 - IADC input, single-ended, Expansion Header Pin 7, WSTK P4
    * PA06 - IADC input, single-ended, Expansion Header Pin 11, WSTK P8 
    * PD03 - GPIO Push/Pull output, WSTK P26, LED1
    * PA07 - GPIO Push/Pull output, Expansion Header Pin 13, WSTK P10, LETIMER0

* Board: Silicon Labs EFR32xG24 Radio Board (BRD4186C) +
         Wireless Starter Kit Mainboard
  * Device: EFR32MG24B210F1536IM48
    * PA05 - IADC input, single-ended, Expansion Header Pin 7, WSTK P4
    * PA06 - IADC input, single-ended, Expansion Header Pin 11, WSTK P8
    * PB04 - GPIO Push/Pull output, WSTK P26, LED1
    * PA07 - GPIO Push/Pull output, Expansion Header Pin 13, WSTK P10, LETIMER0

* Board: Silicon Labs EFR32xG25 Radio Board (BRD4270B) +
         Wireless Starter Kit Mainboard
  * Device: EFR32FG25B222F1920IM56
    * PB02 - IADC input, single-ended, Expansion Header Pin 15, WSTK P12
    * PB03 - IADC input, single-ended, Expansion Header Pin 16, WSTK P13
    * PC07 - GPIO Push/Pull output, WSTK P26, LED1
    * PA07 - GPIO Push/Pull output, Expansion Header Pin 13, WSTK P10, LETIMER0

* Board: Silicon Labs EFR32xG26 Radio Board (BRD4117A) +
         Wireless Starter Kit Mainboard
  * Device: EFR32MG26B420F3200IM48
    * PA08 - IADC input, single-ended, Expansion Header Pin 12, WSTK P9
    * PA09 - IADC input, single-ended, Expansion Header Pin 14, WSTK P11
    * PB04 - GPIO Push/Pull output, WSTK P26, LED1
    * PA07 - GPIO Push/Pull output, Expansion Header Pin 13, WSTK P10, LETIMER0

* Board: Silicon Labs EFR32xG27 Radio Board (BRD4194A) +
         Wireless Starter Kit Mainboard
  * Device: EFR32MG27C140F768IM40
    * PB02 - IADC input, single-ended, Expansion Header Pin 15, WSTK P12
    * PB03 - IADC input, single-ended, Expansion Header Pin 16, WSTK P13
    * PB01 - GPIO Push/Pull output, WSTK P6, Expansion Header Pin 9, LED1
    * PA08 - GPIO Push/Pull output, Expansion Header Pin 13, WSTK P10, LETIMER0

* Board: Silicon Labs EFR32xG28 Radio Board (BRD4400C) +
         Wireless Starter Kit Mainboard
  * Device: EFR32ZG28B312F1024IM68
    * PB04 - IADC input, single-ended, Expansion Header Pin 11, WSTK P8
    * PB05 - IADC input, single-ended, Expansion Header Pin 13, WSTK P10
    * PD03 - GPIO Push/Pull output, WSTK P23, LED1
    * PA11 - GPIO Push/Pull output, Expansion Header Pin 3, WSTK P0, LETIMER0

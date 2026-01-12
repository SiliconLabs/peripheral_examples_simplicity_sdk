# Peripheral Examples - IADC Single LETIMER Interrupt

## Summary

This project demonstrates using the IADC peripheral to take single-ended
analog measurements using a single external input, triggered periodically by 
LETIMER underflow interrupt. The IADC/LETIMER operate in EM2; LETIMER interrupts
wake to EM0 to trigger IADC to start a single conversion, and IADC interrupts
wake to EM0 to handle completed conversions and store the converted voltage
result in a global variable. Since the IADC conversion is done in EM2, only
Port A/B pins can be used. GPIO are toggled with LETIMER underflow and IADC
conversion complete. LETIMER GPIO is toggled through LETIMER output, 
and thus must be routed to port A/B pins when operating in EM2. IADC
conversion complete toggle is performed in the IADC ISR, thus operating in EM0,
and may utilize pins on any port. 

> Note: To modify this example to take differential external
measurements, the negative input for single input must change.
To take a differential measurement, the analog multiplexer
selection must consist of one EVEN ABUS channel and one ODD ABUS
channel.

In this example, the single input channel may be a pin with an odd-numbered
identifier, so an even-numbered port/pin must be selected for the IADC 
negative input. Similarly, if the input channel is an even-numbered pin, the
negative IADC input must be odd-numbered. As in single-ended mode, the IADC
logic will swap the multiplexer connections to the IADC input if needed. See
the reference manual for more details.

## Peripherals Used

* FSRCO @ 20 MHz
* LFXO @ 32.768 kHz
* GPIO
* IADC
  * 12-bit resolution (2x oversampling)
  * Internal VBGR reference with 0.5x analog gain
  * Conversions triggered by firmware
* LETIMER
  * 1 Hz LETIMER interrupt

## How To Test

1. Update the kit's firmware from the Simplicity Launcher (if necessary)
2. Build the project and download to the Starter Kit
3. Open the Simplicity Debugger and add "singleResult" to the Expressions Window
4. Set a breakpoint at the end of the IADC_IRQHandler (IADC_command)
5. Run the example project
6. At the breakpoint, observe the measured voltage in the Expressions Window
   and how they respond to different voltage values on the corresponding pin 
   (see below)

The IADC interrupts on conversion completion, wakes the MCU into EM0 where the 
IADC interrupt handler converts the result to a voltage before returning to EM2.
IADC raw conversion data is stored in global variable "sample" and voltage
conversion is stored in global variable "singleResult".

## Hardware & Connections

* Board: Silicon Labs EFR32xG21 Radio Board (BRD4181A) + 
         Wireless Starter Kit Mainboard
  * Device: EFR32MG21A010F1024IM32
    * PA05 - IADC input, single-ended, Expansion Header Pin 12, WSTK P9
    * PB00 - GPIO Push/Pull output, Expansion Header Pin 11, WSTK P8, LED0
    * PA06 - GPIO Push/Pull output, Expansion Header Pin 14, WSTK P11, LETIMER0

* Board: Silicon Labs EFR32xG22 Radio Board (BRD4182A) +
         Wireless Starter Kit Mainboard
  * Device: EFR32MG22A224F512IM40
    * PB02 - IADC input, single-ended, Expansion Header Pin 15, WSTK P12
    * PD02 - GPIO Push/Pull output, Expansion Header Pin 11, WSTK P8, LED0
    * PB01 - GPIO Push/Pull output, Expansion Header Pin 9, WSTK P6, LETIMER0

* Board: Silicon Labs EFR32xG23 Radio Board (BRD4204D) + 
         Wireless Starter Kit Mainboard
  * Device: EFR32ZG23B010F512IM48
    * PA05 - IADC input, single-ended, Expansion Header Pin 7, WSTK P4
    * PB02 - GPIO Push/Pull output, WSTK P19, LED0
    * PA07 - GPIO Push/Pull output, Expansion Header Pin 13, WSTK P10, LETIMER0

* Board: Silicon Labs EFR32xG24 Radio Board (BRD4186C) +
         Wireless Starter Kit Mainboard
  * Device: EFR32MG24B210F1536IM48
    * PA05 - IADC input, single-ended, Expansion Header Pin 7, WSTK P4
    * PB02 - GPIO Push/Pull output, WSTK P19, LED0
    * PA07 - GPIO Push/Pull output, Expansion Header Pin 13, WSTK P10, LETIMER0

* Board: Silicon Labs EFR32xG25 Radio Board (BRD4270B) +
         Wireless Starter Kit Mainboard
  * Device: EFR32FG25B222F1920IM56
    * PB02 - IADC input, single-ended, Expansion Header Pin 15, WSTK P12
    * PC06 - GPIO Push/Pull output, WSTK P27, LED0
    * PA07 - GPIO Push/Pull output, Expansion Header Pin 13, WSTK P10, LETIMER0

* Board: Silicon Labs EFR32xG26 Radio Board (BRD4117A) +
         Wireless Starter Kit Mainboard
  * Device: EFR32MG26B420F3200IM48
    * PA08 - IADC input, single-ended, Expansion Header Pin 12, WSTK P9
    * PB02 - GPIO Push/Pull output, WSTK P19, LED0
    * PA07 - GPIO Push/Pull output, Expansion Header Pin 13, WSTK P10, LETIMER0

* Board: Silicon Labs EFR32xG27 Radio Board (BRD4194A) +
         Wireless Starter Kit Mainboard
  * Device: EFR32MG27C140F768IM40
    * PB02 - IADC input, single-ended, Expansion Header Pin 15, WSTK P12
    * PB00 - GPIO Push/Pull output, Expansion Header Pin 7, WSTK P4, LED0
    * PA08 - GPIO Push/Pull output, Expansion Header Pin 13, WSTK P10, LETIMER0

* Board: Silicon Labs EFR32xG28 Radio Board (BRD4400C) +
         Wireless Starter Kit Mainboard
  * Device: EFR32ZG28B312F1024IM68
    * PB04 - IADC input, single-ended, Expansion Header Pin 11, WSTK P8
    * PB02 - GPIO Push/Pull output, WSTK P19, LED0
    * PA11 - GPIO Push/Pull output, Expansion Header Pin 3, WSTK P0, LETIMER0

* Board:  Silicon Labs EFR32xG29 Wireless 2.4 GHz 8 dBm Buck Radio Board 
          (BRD4412A) + Wireless Starter Kit Mainboard (BRD4001A)
  * Device: EFR32MG29B140F1024IM40
    * PB02 - IADC input, single-ended, Expansion Header Pin 15, WSTK P12
    * PB00 - GPIO Push/Pull output, Expansion Header Pin 7, WSTK P4, LED0
    * PA06 - GPIO Push/Pull output, Expansion Header Pin 14, WSTK P11, LETIMER0

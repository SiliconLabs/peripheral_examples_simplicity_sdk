# Peripheral Examples - IADC Single Oversampling 16-bit / 20-bit #

## Summary ##

This project demonstrates using the IADC peripheral's oversampling feature to 
acquire 16-bit and 20-bit resolution conversion results while operating in EM2. 
IADC interrupts on conversion completion wake the MCU into EM0, where the IADC 
interrupt handler converts the result to a voltage before returning to EM2.

The IADC sampling rate is:  
- 77 ksps with an oversampling rate of 32 (16-bit resolution)  
- 4.8 ksps with an oversampling rate of 512 (20-bit resolution)  

The IADC reads GPIO pin as input. The PRS peripheral outputs a pulse on a 
GPIO pin whenever the IADC finishes one single conversion.

## Peripherals Used ##

* FSRCO @ 20 MHz
* GPIO  
* IADC  
  * 16-bit resolution with OVS 32x  
  * 20-bit resolution with OVS 512  
  * Two's complement (bipolar) result (unipolar in single-ended mode)
  * Internal VBGR reference with 0.5x analog gain
  * Conversions initiated by firmware and triggered continuously  
* PRS  
  * Toggles LED when IADC conversion is complete  

## How to Test ##

1. Update the kit's firmware from the Simplicity Launcher (if necessary).
2. Build the project and download it to the Starter Kit.
3. Open the Simplicity Debugger and add `sample` and `singleResult` to the 
   Expressions Window.
4. Apply a voltage to the IADC input pin (PA05).
5. Observe the `sample` field:  
  - 16-bit result: `singleResult = sample * VREF / (2^16)`  
  - 20-bit result: `singleResult = sample * VREF / (2^20)`  
6. Observe `OUTPUT0` toggling on an oscilloscope:  
  - 16-bit resolution: period ≈ 13 µs (≈ 77 kHz sampling)  
  - 20-bit resolution: period ≈ 208 µs (≈ 4.8 kHz sampling)  
7. Suspend the debugger and observe how the measured voltage in the 
   Expressions Window responds to different input voltages.

## Hardware & Connections ##

* Board:  Silicon Labs EFR32xG22 2.4 GHz 6 dBm Radio Board (BRD4182A) + Wireless Starter Kit Mainboard
  * Device: EFR32MG22A224F512IM40
    * PB02 - IADC positive differential input, WSTK EXP Header Pin 15
    * PB01 - GPIO Push/Pull output, WSTK EXP Header Pin 9, WSTK P6

* Board:  Silicon Labs EFR32xG23 868–915 MHz 14 dBm Radio Board (BRD4204D) + Wireless Starter Kit Mainboard
  * Device: EFR32ZG23B010F512IM48
    * PA05 - IADC positive differential input, WSTK EXP Header Pin 7
    * PB01 - GPIO Push/Pull output, WSTK P17

* Board:  Silicon Labs EFR32xG24 2.4 GHz 10 dBm Radio Board (BRD4186C) + Wireless Starter Kit Mainboard
  * Device: EFR32MG24B210F1536IM48
    * PA05 - IADC positive differential input, WSTK EXP Header Pin 7
    * PB01 - GPIO Push/Pull output, WSTK P17

* Board:  Silicon Labs EFR32xG25 902–928 MHz 14 dBm Radio Board (BRD4270B) + Wireless Starter Kit Mainboard
  * Device: EFR32FG25B222F1920IM56
    * PB02 - IADC positive differential input, WSTK EXP Header Pin 15
    * PB01 - Push Button 1, WSTK P21  

* Board:  Silicon Labs EFR32xG26 2.4 GHz 20 dBm Radio Board (BRD4117A) + Wireless Starter Kit Mainboard
  * Device: EFR32MG26B420F3200IM48
    * PA08 - IADC positive differential input, WSTK EXP Header Pin 12
    * PB01 - GPIO Push/Pull output, WSTK P17  

* Board:  Silicon Labs EFR32xG27 8 dBm Buck DCDC Radio Board (BRD4194A) + Wireless Starter Kit Mainboard
  * Device: EFR32MG27C140F768IM40
    * PB02 - IADC positive differential input, WSTK EXP Header Pin 15
    * PB01 - GPIO Push/Pull output, WSTK EXP Header Pin 9, WSTK P6

* Board:  Silicon Labs EFR32xG28 868/915 MHz +14 dBm + 2.4 GHz +10 dBm Radio Board (BRD4400C) + Wireless Starter Kit Mainboard
  * Device: EFR32ZG28B312F1024IM68
    * PB04 - IADC positive differential input, WSTK EXP Header Pin 11
    * PB01 - GPIO Push/Pull output, WSTK P17

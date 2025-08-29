# Peripheral Examples - IADC Single Calibration #

## Summary ##

This project demonstrates calibrating the IADC peripheral, followed by single 
polled differential analog measurements. The main program steps through the 
procedure outlined in the Gain and Offset Correction section of the device 
Reference Manual for calibrating the onboard IADC. Once calibration is complete, 
the program resembles the single differential polled example, taking 
measurements and storing the IADC result and a voltage conversion of the result 
into two global variables.

> Note: On EFR32xG21 devices, oscillators and clock branches are automatically 
enabled/disabled based on peripheral demand. Therefore, manually enabling 
clocks has no effect on xG21 devices.

## Peripherals Used ##

* FSRCO @ 20 MHz  
* GPIO  
* IADC  
  * 12-bit resolution  
  * Two's complement (bipolar) result (unipolar in single-ended mode)
  * Unbuffered 3.3V (AVDD) IADC voltage reference  
  * IADC and reference kept in warmup mode
  * Conversions triggered by firmware  

## How to Test ##

1. Update the kit's firmware from the Simplicity Launcher (if necessary).
2. Build the project and download it to the Starter Kit.
3. Open the Simplicity Debugger and add `sample` and `singleResult` to the 
   Expressions Window.
4. Set a breakpoint at the first function within the infinite while loop 
   (`IADC_command`).
5. Run the example.
6. Using a precision voltage source, supply full scale positive voltage across 
   differential positive and negative inputs:  
   * This example uses AVDD (3.30V) as the reference voltage.  
   * Apply 3.30V to the positive input and 0.0V to the negative input.
7. Once the power source is connected, press and release PB0.
8. Remove the power source from the differential inputs and jumper the two pins 
   to apply a zero volt potential across the IADC inputs.
9. Once the IADC inputs are shorted, press and release PB0 again.
10. At the breakpoint, observe the raw data and measured voltage in the 
    Expressions Window. Measurement results should be near zero voltage as the 
    IADC inputs are still shorted.
11. Remove the jumper from across the differential inputs, and connect a 
    variable voltage source (stay at or below full scale to avoid damaging the 
    IADC and MCU).
12. Vary the input voltage and resume the debugger to make new conversions. 
    Observe how the IADC result responds to different voltage values on the 
    corresponding pins.

> The first time the program halts at the breakpoint, the values in the 
Expression Window will be zero (default values). Each subsequent break will 
show the previous conversion loop results.
  
## Hardware & Connections ##

* Board:  Silicon Labs EFR32xG21 2.4 GHz 10 dBm Board (BRD4181A) + Wireless Starter Kit Mainboard
  * Device: EFR32MG21A010F1024IM32
    * PD02 - Push Button PB0
    * PA05 - IADC positive differential input, WSTK EXP Header Pin 12, WSTK P9
    * PB00 - IADC negative differential input, WSTK EXP Header Pin 11, WSTK P8
    * PB01 - LED1, WSTK EXP Header Pin 13, WSTK P10

* Board: Silicon Labs EFR32xG22 2.4 GHz 6 dBm Radio Board (BRD4182A) + Wireless Starter Kit Mainboard
  * Device: EFR32MG22C224F512IM40
    * PB00 - Push Button PB0
    * PB02 - IADC positive differential input, WSTK EXP Header Pin 15
    * PB03 - IADC negative differential input, WSTK EXP Header Pin 16
    * PD03 - LED1, WSTK EXP Header Pin 11

* Board:  Silicon Labs EFR32ZG23 868-915 MHz 14 dBm Radio Board (BRD4204D) + Wireless Starter Kit Mainboard
  * Device: EFR32ZG23B010F512IM48
    * PB01 - Push Button PB0
    * PA05 - IADC positive differential input, WSTK EXP Header Pin 7, WSTK P4
    * PA06 - IADC negative differential input, WSTK EXP Header Pin 11, WSTK P8
    * PD03 - LED1, WSTK P19

* Board:  Silicon Labs EFR32MG24 2.4 GHz 10 dBm Radio Board (BRD4186C) + Wireless Starter Kit Mainboard
  * Device: EFR32MG24B210F1536IM48
    * PB01 - Push Button PB0
    * PA05 - IADC positive differential input, WSTK EXP Header Pin 7, WSTK P4
    * PA06 - IADC negative differential input, WSTK EXP Header Pin 11, WSTK P8
    * PB04 - LED1, WSTK P19

* Board:  Silicon Labs EFR32FG25 902-928 MHz 14 dBm Radio Board (BRD4270B) + Wireless Starter Kit Mainboard
  * Device: EFR32FG25B222F1920IM56
    * PB00 - Push Button PB0
    * PB02 - IADC positive differential input, WSTK EXP Header Pin 15
    * PB03 - IADC negative differential input, WSTK EXP Header Pin 16
    * PC07 - LED1, WSTK P19

* Board:  Silicon Labs EFR32MG26 2.4 GHz 20 dBm Radio Board (BRD4117A) + Wireless Starter Kit Mainboard
  * Device: EFR32MG26B420F3200IM48
    * PB01 - Push Button PB0
    * PA08 - IADC positive differential input, WSTK EXP Header Pin 12
    * PD09 - IADC negative differential input, WSTK EXP Header Pin 14
    * PB04 - LED1, WSTK P19

* Board:  Silicon Labs EFR32xG27 8 dBm Buck DCDC Radio Board (BRD4194A) + Wireless Starter Kit Mainboard
  * Device: EFR32MG27C140F768IM40
    * PB00 - Push Button PB0
    * PB02 - IADC positive differential input, WSTK EXP Header Pin 15
    * PB03 - IADC negative differential input, WSTK EXP Header Pin 16
    * PB01 - LED1, WSTK EXP Header Pin 9

* Board:  Silicon Labs EFR32xG28 868/915 MHz +14 dBm + 2.4 GHz +10 dBm Radio Board (BRD4400C) + Wireless Starter Kit Mainboard
  * Device: EFR32ZG28B312F1024IM68
    * PB01 - Push Button PB0
    * PB04 - IADC positive differential input, WSTK EXP Header Pin 11
    * PB05 - IADC negative differential input, WSTK EXP Header Pin 13
    * PD03 - LED1, WSTK P19
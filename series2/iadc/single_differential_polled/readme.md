# Peripheral Examples - IADC Single Differential Polled #

## Summary ##

This project demonstrates using the IADC peripheral to take a differential 
analog measurement. The main program polls continuously for conversions, then 
stores the IADC raw result and a voltage conversion of the result into two 
global variables.

> Note: On EFR32xG21 devices, oscillators and clock branches are automatically 
enabled/disabled based on peripheral demand. Therefore, manually enabling 
clocks has no effect on xG21 devices.

## Peripherals Used ##

* FSRCO @ 20 MHz  
* GPIO  
* IADC  
  * 12-bit resolution  
  * Two's complement (bipolar) result (unipolar in single-ended mode)
  * Internal VBGR reference with 0.5x analog gain
  * IADC and reference kept in warmup mode  
  * Conversions triggered by firmware 
  
## How to Test ##

1. Update the kit's firmware from the Simplicity Launcher (if necessary).
2. Build the project and download it to the Starter Kit.
3. Open the Simplicity Debugger and add `singleResult` to the Expressions 
   Window.
4. Set a breakpoint at the first function within the infinite while loop 
   (`IADC_command`).
5. Run the example project.
6. At the breakpoint, observe the measured voltage in the Expressions Window 
   and how it responds to different voltage values on the corresponding pins.

> The first time the program halts at the breakpoint, the values in the 
Expression Window will be zero (default values). Each subsequent break will 
show the previous conversion loop results.

> Note: If you want to view the raw result from the IADC instead of the 
converted voltage, you can watch `sample` in addition to `singleResult` in 
the Expressions Window. 

## Hardware & Connections ##

* Board:  Silicon Labs EFR32xG21 2.4 GHz 10 dBm Radio Board (BRD4181A) + Wireless Starter Kit Mainboard
  * Device: EFR32MG21A010F1024IM32
    * PA05 - IADC positive differential input, WSTK EXP Header Pin 12
    * PB00 - IADC negative differential input, WSTK EXP Header Pin 11

* Board:  Silicon Labs EFR32xG22 2.4 GHz 6 dBm Radio Board (BRD4182A) + Wireless Starter Kit Mainboard
  * Device: EFR32MG22A224F512IM40
    * PB02 - IADC positive differential input, WSTK EXP Header Pin 15
    * PB03 - IADC negative differential input, WSTK EXP Header Pin 16 

* Board:  Silicon Labs EFR32xG23 868–915 MHz 14 dBm Radio Board (BRD4204D) + Wireless Starter Kit Mainboard
  * Device: EFR32ZG23B010F512IM48
    * PA05 - IADC positive differential input, WSTK EXP Header Pin 7
    * PA06 - IADC negative differential input, WSTK EXP Header Pin 11

* Board:  Silicon Labs EFR32xG24 2.4 GHz 10 dBm Radio Board (BRD4186C) + Wireless Starter Kit Mainboard
  * Device: EFR32MG24B210F1536IM48
    * PA05 - IADC positive differential input, WSTK EXP Header Pin 7
    * PA06 - IADC negative differential input, WSTK EXP Header Pin 11

* Board:  Silicon Labs EFR32xG25 902–928 MHz 14 dBm Radio Board (BRD4270B) + Wireless Starter Kit Mainboard
  * Device: EFR32FG25B222F1920IM56
    * PB02 - IADC positive differential input, WSTK EXP Header Pin 15
    * PB03 - IADC negative differential input, WSTK EXP Header Pin 16 

* Board:  Silicon Labs EFR32xG26 2.4 GHz 20 dBm Radio Board (BRD4117A) + Wireless Starter Kit Mainboard
  * Device: EFR32MG26B420F3200IM48
    * PA08 - IADC positive differential input, WSTK EXP Header Pin 12
    * PD09 - IADC negative differential input, WSTK EXP Header Pin 14  

* Board:  Silicon Labs EFR32xG27 8 dBm Buck DCDC Radio Board (BRD4194A) + Wireless Starter Kit Mainboard
  * Device: EFR32MG27C140F768IM40
    * PB02 - IADC positive differential input, WSTK EXP Header Pin 15
    * PB03 - IADC negative differential input, WSTK EXP Header Pin 16 

* Board:  Silicon Labs EFR32xG28 868/915 MHz +14 dBm + 2.4 GHz +10 dBm Radio Board (BRD4400C) + Wireless Starter Kit Mainboard
  * Device: EFR32ZG28B312F1024IM68
    * PB04 - IADC positive differential input, WSTK EXP Header Pin 11
    * PB05 - IADC negative differential input, WSTK EXP Header Pin 13 

* Board:  Silicon Labs EFR32xG29 Wireless 2.4 GHz 8 dBm Buck Radio Board 
          (BRD4412A) + Wireless Starter Kit Mainboard
  * Device: EFR32MG29B140F1024IM40
    * PB02 - IADC positive differential input, WSTK EXP Header Pin 15
    * PB03 - IADC negative differential input, WSTK EXP Header Pin 16

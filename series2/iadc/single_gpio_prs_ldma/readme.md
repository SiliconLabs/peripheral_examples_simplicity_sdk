# Peripheral Examples - IADC Single GPIO PRS LDMA #

## Summary ##

This project demonstrates use of the IADC peripheral to take single-ended
analog measurements from a single external input triggered by the rising edge
of a GPIO via PRS. Upon completion, an LDMA transfer stores the raw data to
memory. The IADC/LDMA operate in EM2; LDMA interrupts wake to EM0 to toggle
a GPIO before returning to EM2.

Since conversions are done in EM2, only Port A/B pins can be used. 
However, the LDMA transfer complete toggle is performed in the LDMA ISR, 
operating in EM0, and may utilize pins on any port.

> Note: To utilize differential-ended analog measurements for the external 
input, the negative input must be modified for an external port/pin. Analog 
multiplexer selection must consist of one EVEN xBUS selection and one ODD 
xBUS selection for differential mode to operate correctly.  
For example, for a single input referencing Port A Pin 0 (xG21 devices) or 
Port A Pin 5 (non-xG21 devices), an ODD/EVEN Port/Pin selection must be used 
for the IADC negative input. The IADC logic will automatically swap the 
multiplexer connections if needed. See the reference manual for more details.

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
  * Conversions triggered by rising GPIO via PRS
* LDMA
  * Channel 0
  * 32-bit transfer from IADC to buffer
  * Loop continuously until `NUM_SAMPLES` are captured
  * LDMA interrupt toggles LED
* PRS

## How to Test ##

1. Open Simplicity Studio and update the kit's firmware using **Device Manager Tool**(if necessary).
2. Build the project and download it to the Starter Kit.
3. Start a debug session in the IDE and add `singleBuffer` to the Watch/Variables 
   Window.
4. Set a breakpoint at the end of the `ldma_callback`.  
   * For EFR32MG21 Radio Board BRD4181A, jumper PA06 to PD02 to use the 
     pushbutton or toggle `IADC input` to trigger IADC conversion. (Port A/B required 
     for PRS in EM2.)
5. Run the example project; press push button PB0 `NUM_SAMPLES` times to 
   collect IADC samples.
6. At the breakpoint, observe `singleBuffer` in the Watch/Variables window and how 
   it responds to different voltage values on the corresponding pin.

## Hardware & Connections ##

* Board:  Silicon Labs EFR32xG21 2.4 GHz 10 dBm Radio Board (BRD4181A) + Wireless Starter Kit Mainboard
  * Device: EFR32MG21A010F1024IM32
    * PB01 - LED1, WSTK EXP Header Pin 13
    * PA05 - IADC positive differential input, WSTK EXP Header Pin 12, WSTK P9
    * PA06 - WSTK EXP Header Pin 14, WSTK P9 (jumper to PD02)
    * PD02 - Push Button PB0, WSTK EXP Header Pin 7, WSTK P4

* Board:  Silicon Labs EFR32xG22 2.4 GHz 6 dBm Radio Board (BRD4182A) + Wireless Starter Kit Mainboard
  * Device: EFR32MG22A224F512IM40
    * PD03 - LED1, WSTK EXP Header Pin 11
    * PB02 - IADC positive differential input, WSTK EXP Header Pin 15
    * PB00 - Push Button PB0

* Board:  Silicon Labs EFR32xG23 868–915 MHz 14 dBm Radio Board (BRD4204D) + Wireless Starter Kit Mainboard
  * Device: EFR32ZG23B010F512IM48
    * PD03 - LED1, WSTK P19
    * PA05 - IADC positive differential input, WSTK EXP Header Pin 7, WSTK P4
    * PB01 - Push Button PB0

* Board:  Silicon Labs EFR32xG24 2.4 GHz 10 dBm Radio Board (BRD4186C) + Wireless Starter Kit Mainboard
  * Device: EFR32MG24B210F1536IM48
    * PB04 - LED1, WSTK P19
    * PA05 - IADC positive differential input, WSTK EXP Header Pin 7, WSTK P4
    * PB01 - Push Button PB0

* Board:  Silicon Labs EFR32xG25 902–928 MHz 14 dBm Radio Board (BRD4270B) + Wireless Starter Kit Mainboard
  * Device: EFR32FG25B222F1920IM56
    * PC07 - LED1, WSTK P19
    * PB02 - IADC positive differential input, WSTK EXP Header Pin 15
    * PB00 - Push Button PB0

* Board:  Silicon Labs EFR32xG26 2.4 GHz 20 dBm Radio Board (BRD4117A) + Wireless Starter Kit Mainboard
  * Device: EFR32MG26B420F3200IM48
    * PB04 - LED1, WSTK P19
    * PA08 - IADC positive differential input, WSTK EXP Header Pin 12
    * PB01 - Push Button PB0

* Board:  Silicon Labs EFR32xG27 8 dBm Buck DCDC Radio Board (BRD4194A) + Wireless Starter Kit Mainboard
  * Device: EFR32MG27C140F768IM40
    * PB01 - LED1, WSTK EXP Header Pin 9
    * PB02 - IADC positive differential input, WSTK EXP Header Pin 15
    * PB00 - Push Button PB0

* Board:  Silicon Labs EFR32xG28 868/915 MHz +14 dBm + 2.4 GHz +10 dBm Radio Board (BRD4400C) + Wireless Starter Kit Mainboard
  * Device: EFR32ZG28B312F1024IM68
    * PD03 - LED1, WSTK P19
    * PB04 - IADC positive differential input, WSTK EXP Header Pin 11
    * PB01 - Push Button PB0

* Board:  Silicon Labs EFR32xG29 Wireless 2.4 GHz 8 dBm Buck Radio Board (BRD4412A) + Wireless Starter Kit Mainboard
  * Device: EFR32MG29B140F1024IM40
    * PB01 - LED1, WSTK EXP Header Pin 9
    * PB02 - IADC positive differential input, WSTK EXP Header Pin 15
    * PB00 - Push Button PB0

* Board:  Silicon Labs EFR32FG2D 868-915 MHz 14 dBm Radio Board (BRD4277A) + Wireless Starter Kit Mainboard
  * Device: EFR32FG2DB010F512IM48
    * PD03 - LED1, WSTK P26
    * PA05 - IADC positive differential input, WSTK EXP Header Pin 7, WSTK P4
    * PB01 - Push Button PB0

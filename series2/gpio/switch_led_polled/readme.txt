switch_led_polled

This project demonstrates how to set up simple digital inputs and
outputs. Button PB0 on the WSTK is the input; LED0 is the output.
While PB0 is pressed, LED0 is on.

Note for EFR32xG27, LED1 is the output. When button 0 is pressed, LED1
turns on. The button PB0 and LED0 are tied to the same pin, so LED0 will
be on while PB0 is pressed.

How To Test:
1. Build the example project and download it to the target system.
2. Click the Play/Resume (F8) button in the debugger to run the program.
3. Press PB0 to turn the LED on.
4. Release PB0 to turn the LED off.

Peripherals Used:
CMU    - HFRCODPLL @ 19 MHz

Board: Silicon Labs EFR32xG21 2.4 GHz 10 dBm Radio Board (BRD4181A) 
       + Wireless Starter Kit Mainboard (BRD4001A)
Device: EFR32MG21A010F1024IM32
PB00 -  LED0
PD02 -  Push Button PB0

Board: Silicon Labs EFR32xG22 2.4 GHz 6 dBm Radio Board (BRD4182A) 
       + Wireless Starter Kit Mainboard (BRD4001A)
Device: EFR32MG22C224F512IM40
PD02 -  LED0
PB00 -  Push Button PB0

Board:  Silicon Labs EFR32ZG23 868-915 MHz 14 dBm Radio Board (BRD4204D)
        + Wireless Starter Kit Mainboard (BRD4001A)
Device: EFR32ZG23B010F512IM48
PB02 -  LED0
PB01 -  Push Button PB0

Board:  Silicon Labs EFR32MG24 2.4 GHz 10 dBm Radio Board (BRD4186C)
        + Wireless Starter Kit Mainboard (BRD4001A)
Device: EFR32MG24B210F1536IM48
PB02 -  LED0
PB01 -  Push Button PB0

Board:  Silicon Labs EFR32FG25 902-928 MHz 14 dBm Radio Board (BRD4270B)
        + Wireless Starter Kit Mainboard (BRD4001A)
Device: EFR32FG25B222F1920IM56
PC06 -  LED0
PB00 -  Push Button PB0

Board:  Silicon Labs EFR32MG26 2.4 GHz 20 dBm Radio Board (BRD4117A)
        + Wireless Starter Kit Mainboard (BRD4001A)
Device: EFR32MG26B420F3200IM48
PB02 -  LED0
PB01 -  Push Button PB0

Board:  Silicon Labs EFR32xG27 2.4 GHz 8 dBm Buck DCDC Radio Board (BRD4194A)
        + Wireless Starter Kit Mainboard (BRD4001A)
Device: EFR32MG27C140F768IM40
PB01 -  LED1
PB00 -  Push Button PB0

Board:  Silicon Labs EFR32xG28 868/915 MHz +14 dBm + 2.4 GHz +10 dBm Radio
        Board (BRD4400C) + Wireless Starter Kit Mainboard
Device: EFR32ZG28B312F1024IM68
PB02 -  LED0
PB01 -  Push Button PB0
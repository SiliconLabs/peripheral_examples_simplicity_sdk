switch_led_interrupt

This project demonstrates how to use GPIO pins to trigger external
interrupts. If button 0 is pressed, LED0 will toggle; if button 1 is
pressed, LED1 will toggle. There are two sources for GPIO interrupts.
By default, one is assigned to even-numbered pins and the other to
odd-numbered pins. (The GPIO_EXTIPINSELH and GPIO_EXTIPINSELL registers
permit some offsetting from the even and odd defaults that is beyond
the scope of this example. See the Standard Interrupt Generation section
of the reference manual GPIO chapter for details).

Note: PB0 provides an "escape hatch" mechanism which can be used to pause the
device before entering EM4 so that a debugger can connect in order to
erase flash, among other things. Before proceeding with this example,
make sure PB0 is not pressed.

How To Test:
1. Build the example project and download it to the target system.
2. Click the Play/Resume (F8) button in the debugger to run the program.
3. Press PB0 to toggle LED0
4. Press PB1 to toggle LED1

Peripherals Used:
CMU    - HFRCODPLL @ 19 MHz
EMU

Board: Silicon Labs EFR32xG22 2.4 GHz 6 dBm Radio Board (BRD4182A) 
       + Wireless Starter Kit Mainboard (BRD4001A)
Device: EFR32MG22C224F512IM40
PD02 -  LED0
PD03 -  LED1
PB00 -  Push Button PB0
PB01 -  Push Button PB1

Board:  Silicon Labs EFR32ZG23 868-915 MHz 14 dBm Radio Board (BRD4204D)
        + Wireless Starter Kit Mainboard (BRD4001A)
Device: EFR32ZG23B010F512IM48
PB02 -  LED0
PD03 -  LED1
PB01 -  Push Button PB0
PB03 -  Push Button PB1

Board:  Silicon Labs EFR32MG24 2.4 GHz 10 dBm Radio Board (BRD4186C)
        + Wireless Starter Kit Mainboard (BRD4001A)
Device: EFR32MG24B210F1536IM48
PB01 -  Push Button PB0
PB02 -  LED0
PB03 -  Push Button PB1
PB04 -  LED1

Board:  Silicon Labs EFR32FG25 902-928 MHz 14 dBm Radio Board (BRD4270B)
        + Wireless Starter Kit Mainboard (BRD4001A)
Device: EFR32FG25B222F1920IM56
PB00 -  Push Button PB0
PB01 -  Push Button PB1
PC06 -  LED0
PC07 -  LED1

Board:  Silicon Labs EFR32MG26 2.4 GHz 20 dBm Radio Board (BRD4117A)
        + Wireless Starter Kit Mainboard (BRD4001A)
Device: EFR32MG26B420F3200IM48
PB01 -  Push Button PB0
PB02 -  LED0
PB03 -  Push Button PB1
PB04 -  LED1

Board:  Silicon Labs EFR32xG28 868/915 MHz +14 dBm + 2.4 GHz +10 dBm Radio
        Board (BRD4400C) + Wireless Starter Kit Mainboard
Device: EFR32ZG28B312F1024IM68
PB01 -  Push Button PB0
PB03 -  Push Button PB1
PB02 -  LED0
PD03 -  LED1
gpio_slew_rate

This project demonstrates how to set the slew rate for GPIO pins. A 1 MHz
square is driven on a pin and by connecting a capacitor, the effect of
slew rate changes on pin rise and fall times can be observed.  In this
example, pressing button PB0 increments the slew rate setting such
pressing the button 8 times cycles the through all slew rate settings.

Unlike Series 1 devices, which provided separate drive strength and slew
rate control settings, Series 2 devices provide only a single 3-bit wide
field to specify slew rate. The lowest slew rate setting of 3'b000 (0) is
a low drive mode limited to about 1 mA. A setting of 3'b001 (1) or higher
provides full drive strength with 3'b110 (6) being the highest setting
in normal use cases. A setting of 3'b111 (7) should only be used for
high-speed clock signals (above 10 MHz) and should not be used on more
than one pin per port.

Slew rate is easily set using the emlib GPIO_SlewrateSet() function for
both the primary and alternate pin configurations, e.g. the
gpioModePushPull and gpioModePushPullAlt enumerated configuration types
as used in the GPIO_PinModeSet() function.

How To Test:
1. Place a 50uF capacitor between the output pin and GND.
2. Build the example project and download it to the target system.
3. Click the Play/Resume (F8) button in the debugger to run the program.
4. While observing the rise and fall times of the waveform on the output pin, 
   press PB0 to change the slew rate.
    
Peripherals Used:
CMU    - HFRCODPLL @ 19 MHz
EMU
TIMER  - FSRCO @ 20 MHz, Toggles GPIO at 1 MHz

Board: Silicon Labs EFR32xG21 2.4 GHz 10 dBm Radio Board (BRD4181A) 
       + Wireless Starter Kit Mainboard (BRD4001A)
Device: EFR32MG21A010F1024IM32
PD02 -  Push Button 0
PC00 -  1 MHz output (Expansion Header Pin 4)

Board: Silicon Labs EFR32xG22 2.4 GHz 6 dBm Radio Board (BRD4182A) 
       + Wireless Starter Kit Mainboard (BRD4001A)
Device: EFR32MG22C224F512IM40
PB00 -  Push Button 0
PC00 -  1 MHz output (Expansion Header Pin 4)

Board:  Silicon Labs EFR32ZG23 868-915 MHz 14 dBm Radio Board (BRD4204D)
        + Wireless Starter Kit Mainboard (BRD4001A)
Device: EFR32ZG23B010F512IM48
PB01 -  Push Button 0
PC00 -  1 MHz output (Expansion Header Pin 10)

Board:  Silicon Labs EFR32MG24 2.4 GHz 10 dBm Radio Board (BRD4186C)
        + Wireless Starter Kit Mainboard (BRD4001A)
Device: EFR32MG24B210F1536IM48
PB01 -  Push Button PB0
PC00 -  1 MHz output (Expansion Header Pin 10)

Board:  Silicon Labs EFR32FG25 902-928 MHz 14 dBm Radio Board (BRD4270B)
        + Wireless Starter Kit Mainboard (BRD4001A)
Device: EFR32FG25B222F1920IM56
PB00 -  Push Button PB0
PC00 -  1 MHz output (Expansion Header Pin 10)

Board:  Silicon Labs EFR32MG26 2.4 GHz 20 dBm Radio Board (BRD4117A)
        + Wireless Starter Kit Mainboard (BRD4001A)
Device: EFR32MG26B420F3200IM48
PB01 -  Push Button PB0
PC00 -  1 MHz output (Expansion Header Pin 10)

Board:  Silicon Labs EFR32xG27 2.4 GHz 8 dBm Buck DCDC Radio Board (BRD4194A)
        + Wireless Starter Kit Mainboard (BRD4001A)
Device: EFR32MG27C140F768IM40
PB00 -  Push Button PB0
PC03 -  1 MHz output (Expansion Header Pin 10)

Board:  Silicon Labs EFR32xG28 868/915 MHz +14 dBm + 2.4 GHz +10 dBm Radio
        Board (BRD4400C) + Wireless Starter Kit Mainboard
Device: EFR32ZG28B312F1024IM68
PB01 -  Push Button PB0
PD10 -  1 MHz output (Expansion Header Pin 10)
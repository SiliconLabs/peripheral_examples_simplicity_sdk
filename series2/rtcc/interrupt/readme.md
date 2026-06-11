# Peripheral Examples - RTCC Interrupt #

This project uses the RTCC (Real Time Clock with Capture) to wake the device
from EM2 mode (LFRCO, LFXO, or ULFRCO).

The wake up interval is defined by WAKEUP_INTERVAL_MS, default value is 500ms.
The RTCC clock source is defined by RTCC_CLOCK, default source is LFXO.

This project also shows how to use the RTCC compare channel PRS output to toggle
an LED.

## Peripherals Used: ##
* CMU
  * HFRCODPLL @ 19 MHz
  * LFXO @ 32768 Hz, RTCC clock source
* RTCC - Interrupt every 500ms
* PRS - Channel 0, RTCC compare channel PRS output
* GPIO

## How To Test: ##
1. Open Simplicity Studio and update the kit's firmware using **Device Manager Tool**(if necessary)
2. Build the project and download it to the Starter Kit
3. Close debug session in IDE
4. Press the reset button on the mainboard
5. Open Energy Profiler by right clicking on the project, clicking "Profile As", and 
   selecting "Simplicity Energy Profiler Target." Monitor the GPIO toggling every 500ms.

* Board:  Silicon Labs EFR32xG21 2.4 GHz 10 dBm Board (BRD4181A) + Wireless Starter Kit Mainboard
    * Device: EFR32MG21A010F1024IM32
        * PB00 - WSTK LED0 and EXP Header Pin 11

* Board:  Silicon Labs EFR32xG22 Radio Board (BRD4182A) + Wireless Starter Kit Mainboard
    * Device: EFR32MG22A224F512IM40
        * PB00 - WSTK EXP Header Pin 7

* Board:  Silicon Labs EFR32xG27 Radio Board (BRD4194A) + Wireless Starter Kit Mainboard
    * Device: EFR32MG27C140F768IM40
        * PB00 - WSTK LED0 and EXP Header Pin 7
        
* Board:  Silicon Labs EFR32xG29 Wireless 2.4 GHz 8 dBm Buck Radio Board (BRD4412A) + Wireless Starter Kit Mainboard
    * Device: EFR32MG29B140F1024IM40
        * PB00 - WSTK LED0 and EXP Header Pin 7
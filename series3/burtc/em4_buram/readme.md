# Peripheral Examples - BURTC EM4 BURAM #

## Summary ##

This project uses the BURTC (Backup Real Time Counter) to wake the device from EM4 mode and thus trigger a reset. This project also shows how to use the BURAM retention registers to have data persist between resets. The number of resets triggered by the BURTC is printed via the device's EUSART and the mainboard's JLink CDC UART Port for view in a PC terminal program.

## Peripherals used ##

* BURAM
* BURTC
* EMU
* EUSART

## How to Test ##

1. Open Simplicity Studio and update the kit's firmware using **Device Manager Tool**(if necessary)
2. Build the example project and download it to the target system.
3. Open a terminal program and connect to the COM port associated with Starter Kit's Jlink CDC UART Port (see Windows Device Manager) using 115200 baud, 8-N-1, no flow control
4. Press the reset button the mainboard
5. Follow instructions in the terminal program to enter EM4
6. Observe the number of EM4 wakeups increases each time an EM4 wakeup occurs

## Hardware & Connections ##

* Board: Silicon Labs SixG301 Radio Board (BRD4407A) + Wireless Pro Kit Mainboard
    * Device: SIMG301M104LIL
        * PB00 - VCOM RX, Lower Breakout Header F7
        * PB02 - VCOM TX, Lower Breakout Header F6
        * PB01 - Push button BTN0 input, Lower Breakout Header P16
        * PD03 - LED0, Lower Breakout Header P14

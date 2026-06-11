# Peripheral Examples - I2C Follower #

## Summary ##

This project demonstrates the follower configuration of the SiMG301 I2C peripheral. This example is intended to be used with an accompanying leader example, connected together, with one development board running a leader project and the other running the follower project. The leader reads the follower's current buffer values, increments each value, and writes new values back to the follower device. The leader then reads back the follower values again and verifies that the new values match what was previously written. This program runs in a continuous loop, entering and exiting low energy modes via Power Manager, waking to handle I2C transmissions when necessary. Follower toggles LED0 on during I2C transaction and off once complete. Follower will set LED1 if an I2C transmission error is encountered.

## Peripherals used ##

* I2C
    * Clock Manager utilizes 38.4 MHz HFXO for I2C0CLK input
	* I2C bus frequency is configured for 100 kHz
	* I2C peripheral configured to be addressable as follower with default address set to 0xE2
* GPIO
	* LED0 is used to indicate when application is handling an I2C transmission
	* LED1 is turned on to indicate a fault or error with I2C transfer

## How to Test ##

1.  Connect the SDA, SCL and GND lines between two kits via the EXP header.
2.  This example is intended to be used with one of the accompanying I2C Leader examples which enable external I2C bus pull-ups on the Wireless Pro Kit. If not using with this specific configuration, external pull-up resistors are likely needed from VMCU to the SDA and SCL lines (only one pull-up needed for each line).
3.  Open Simplicity Studio and update the kit's firmware using **Device Manager Tool**(if necessary)
4.  Build both the leader and follower projects and download to two Wireless Starter or Wireless Pro Kits.
5.  For follower kit, in the drop-down menu, select "Profile As Simplicity
    Energy Profiler Target".
6.  The project will compile, load and start-up, proceeding to the main loop
    which sits in EM1.
7.  Observe the current consumption in Energy Profiler of the kit in EM1
8.  Toggle push button PB0 on the leader kit.
9.  Observe the current spike as the follower kit wakes to EM0/1 to handle the
    I2C transaction and then returns to EM1 consumption with each button press
    of PB0 on the leader kit.


## Hardware & Connections ##

* Board: Silicon Labs SixG301 Radio Board (BRD4407A) + Wireless Pro Kit Mainboard
    * Device: SIMG301M104LIL
        * PD00 - I2C SCL input, Expansion Header 15
        * PD01 - I2C SDA input/output, Expansion Header 16
        * PD03 - LED0
        * PD04 - LED1
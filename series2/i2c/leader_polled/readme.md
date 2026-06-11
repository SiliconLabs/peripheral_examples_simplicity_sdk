# Peripheral Examples - I2C Leader Polled

This project demonstrates the leader configuration of the EFx32xG2x I2C
peripheral. Two EFx32xG2x devices are connected together, one running the leader
project, the other running the follower project. The leader starts up and enters
a while loop waiting for a button press on push button 0. When push button 0 is
pressed, the program performs an I2C test. This routine reads the follower
device's current buffer values, increments each value by 1, and writes the new
values back to the follower device. The leader then reads back the follower
values again and verifies the new values match what was previously written. Upon
a successful write, LED1 is toggled and the device re-enters the while loop,
waiting again for user input through push button 0. This project runs in a
continuous loop, re-running the I2C test with every PB0 button press and
toggling LED1 with each successful iteration. If there is an I2C transmission
error, or if the verification step of the I2C test fails, LED0 is turned on and
the leader halts code execution at a breakpoint. 

> Note: This example uses inclusive lexicon wherever possible. For more
information, visit https://www.silabs.com/about-us/inclusive-lexicon-project

### Note regarding pull-up resistors

This example differs from the I2C follower version in that it
generally does not require jumpering of pull-up resistors to the SCL
and SDA pins.

Both the Wireless Starter Kit and Wiresless Pro Kit mainboards include
a Si7021 relative humidity and temperature sensor with the necessary
pull-ups on a pair of pins connected to pins 15 (SCL) and 16 (SDA) on
the EXP header.

Power for the sensor domain and thus its pull-ups is controlled by
whatever GPIO is connected to the WSTK/WPK radio board connector P37
signal (named SENSOR_ENABLE in the schematics).  For most devices, the
example code drives the specified GPIO high to power the sensor domain.

The sensor itself has a different I2C address (0x40) than that
configured for the MCU running the follower example project (0x71).
Both devices reside on the bus, but this leader code communicates only
with the matching follower.

Unfortunately, projects for two devices do not follow this convention:

1. BRD4181A for EFR32xG21 does not connect any GPIOs to P12, P13, and
P37 on the WSTK/WPK radio board connectors.  There are "no mount" pads
for 0 ohmn resistors that can be installed to make these connections
with the proviso that the GPIOs used are assigned other functions on
the mainboard.  See the schematic for details.

For this reason, pull-ups to VMCU must be connected to PC0 (EXP header
pin 4) for SCL and PC2 (EXP header pin 6) for SDA.

2. BRD4270B for EFR32FG25 does connect GPIOs to EXP header pins 15/16
for SCL/SDA but does not connect anything to P37, so there is no way to
power the sensor domain pull-ups.  For this reason, external pull-ups
must be provided when running this project.

## Peripherals Used

* HFRCODPLL - 19 MHz
* I2C0      - 100 kHz

## How To Test

1.  Connect the SCL and SDA lines (generally EXP header pins 15 and 16,
    respectively) between the two kits.  Do the same with GND (EXP
    header pin 1).

2.  If necessary, connect 4.7 kOhms pull-up resistor from SCL and SDA to
    VMCU (EXP header pin 2).  Only one pull-up is needed for each line
    (e.g. SCL can be connected to VMCU on one kit and SDA to VMCU on the
    other kit).

3.  Open Simplicity Studio and update the kit's firmware using **Device Manager Tool**(if necessary).

4.  Build the follower project and download it to one of the kits.  It
    is not necessary for this kit to be connected to a PC running
    Simplicity Studio.  Power can, for example, be provided by a USB
    wall charger.

5.  Build the leader polled project and download it to the kit that will be
    connected to the debugger during testing.

6.  Run the leader polled example.  It will enter the main loop and wait
    for PB0.  Before continuing, press the reset button of the kit running
    the follower example.

7.  Press PB0 on the leader kit to trigger an I2C test (read follower buffer,
    increment, write, read, verify).

8.  Observe LED1 toggle with a successful I2C test.

9.  Click the Pause button (parallel yellow bars) in the debugger to
    suspend code execution.

10. Observe that the values received from the follower match the values
    transmitted by leader by adding "i2c_rxBuffer" and "i2c_txBuffer",
    which are global variables, to the list in the Watch/Variables tab.

## Hardware & Connections

* Board:  Silicon Labs EFR32xG21 Radio Board (BRD4181A) + 
        Wireless Starter/Pro Kit Mainboard (BRD4001A/BRD4002A)
  * Device: EFR32MG21A010F1024IM32
    * PD02 -  Button PB0, Expansion Header Pin 7
    * PB00 -  LED0, Expansion Header Pin 11
    * PB01 -  LED1, Expansion Header Pin 13
    * PC00 -  I2C_SCL, Expansion Header Pin 4
    * PC01 -  I2C_SDA, Expansion Header Pin 6

* Board:  Silicon Labs EFR32xG22 Radio Board (BRD4182A) + 
        Wireless Starter/Pro Kit Mainboard (BRD4001A/BRD4002A)
  * Device: EFR32MG22C224F512IM40
    * PB00 -  Button PB0, Expansion Header Pin 7
    * PD02 -  LED0, Expansion Header Pin 11
    * PD03 -  LED1, Expansion Header Pin 13
    * PB02 -  I2C_SCL, Expansion Header Pin 15
    * PB03 -  I2C_SDA, Expansion Header Pin 16
    * PC07 -  SENSOR_ENABLE, Mainboard Breakout Pad Pin P37

* Board:  Silicon Labs EFR32ZG23 Radio Board (BRD4204D) + 
        Wireless Starter/Pro Kit Mainboard (BRD4001A/BRD4002A)
  * Device: EFR32ZG23B010F512IM48
    * PB01 -  Button PB0, Mainboard Breakout Pad Pin P17
    * PB02 -  LED0, Mainboard Breakout Pad Pin P19
    * PD03 -  LED1, Mainboard Breakout Pad Pin P26
    * PC05 -  I2C_SCL, Expansion Header Pin 15
    * PC07 -  I2C_SDA, Expansion Header Pin 16
    * PC09 -  SENSOR_ENABLE, Mainboard Breakout Pad Pin P37

* Board:  Silicon Labs EFR32MG24 Radio Board (BRD4186C) + 
        Wireless Starter/Pro Kit Mainboard (BRD4001A/BRD4002A)
  * Device: EFR32MG24B210F1536IM48
    * PB01 -  Button PB0, Mainboard Breakout Pad Pin P17
    * PB02 -  LED0, Mainboard Breakout Pad Pin P19
    * PB04 -  LED1, Mainboard Breakout Pad Pin P26
    * PC05 -  I2C_SCL, Expansion Header Pin 15
    * PC07 -  I2C_SDA, Expansion Header Pin 16
    * PD03 -  SENSOR_ENABLE, Mainboard Breakout Pad Pin P37

* Board:  Silicon Labs EFR32xG25 Radio Board (BRD4270B) + 
        Wireless Starter/Pro Kit Mainboard (BRD4001A/BRD4002A)
  * Device: EFR32FG25B222F1920IM56
    * PB00 -  Button PB0, Mainboard Breakout Pad Pin P17
    * PC06 -  LED0, Mainboard Breakout Pad Pin P27
    * PC07 -  LED1,Mainboard Breakout Pad Pin P28
    * PB02 -  I2C_SCL, Expansion Header Pin 15
    * PB03 -  I2C_SDA, Expansion Header Pin 16

* Board:  Silicon Labs EFR32MG26 2.4 GHz 20 dBm Radio Board (BRD4117A)
        + Wireless Starter Kit Mainboard (BRD4001A)
  * Device: EFR32MG24B210F1536IM48
    * PB01 -  Button PB0, Mainboard Breakout Pad Pin P17
    * PB02 -  LED0, Mainboard Breakout Pad Pin P19
    * PB04 -  LED1, Mainboard Breakout Pad Pin P26
    * PC05 -  I2C_SCL, Expansion Header Pin 15
    * PC07 -  I2C_SDA, Expansion Header Pin 16
    * PD03 -  SENSOR_ENABLE, Mainboard Breakout Pad Pin P37

* Board:  Silicon Labs EFR32xG27 Radio Board (BRD4194A) + 
        Wireless Starter/Pro Kit Mainboard (BRD4001A/BRD4002A)
  * Device: EFR32MG27C140F768IM40
    * PB00 -  Button PB0, Expansion Header Pin 7
    * PB01 -  LED1, Expansion Header Pin 9
    * PB02 -  I2C_SCL, Expansion Header Pin 15
    * PB03 -  I2C_SDA, Expansion Header Pin 16
    * PC07 -  SENSOR_ENABLE, Mainboard Breakout Pad Pin P37

* Board:  Silicon Labs EFR32xG28 Radio Board (BRD4400C) + 
        Wireless Starter/Pro Kit Mainboard (BRD4001A/BRD4002A)
  * Device: EFR32ZG28B312F1024IM68
    * PB01 -  Button PB0, Mainboard Breakout Pad Pin P17
    * PB02 -  LED0, Mainboard Breakout Pad Pin P19
    * PD03 -  LED1, Mainboard Breakout Pad Pin P23
    * PC05 -  I2C_SCL, Expansion Header Pin 15
    * PC07 -  I2C_SDA, Expansion Header Pin 16
    * PC11 -  SENSOR_ENABLE, Mainboard Breakout Pad Pin P37

* Board:  Silicon Labs EFR32xG29 Wireless 2.4 GHz 8 dBm Buck Radio Board 
          (BRD4412A) + Wireless Starter Kit Mainboard (BRD4001A)
  * Device: EFR32MG29B140F1024IM40
    * PB00 -  Button PB0, Expansion Header Pin 7
    * PB01 -  LED1, Expansion Header Pin 9
    * PB02 -  I2C_SCL, Expansion Header Pin 15
    * PB03 -  I2C_SDA, Expansion Header Pin 16
    * PC07 -  SENSOR_ENABLE, Mainboard Breakout Pad Pin P37

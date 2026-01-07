# Peripheral Examples - I2C Leader with LDMA #

## Summary ##

This project demonstrates I2C leader operation in conjunction with LDMA so that the host CPU can be freed to perform other tasks while data is being moved to and from the follower device.

As with the polled I2C leader example, two devices are connected, with one running this leader example project and the other running the follower project. The leader starts up and sits in a while loop waiting for a press on push button 0.

When button 0 is pressed, the program runs an I2C test that consists of reading the follower device's current buffer values, incrementing each value by 1, writing the new values back to the follower, and re-reading the follower's buffer to verify that the new values were correctly received and stored.

Upon a successful write, LED0 is toggled and the device re-enters the while loop, waiting again for button 0. The project runs in this continuous loop, re-running the I2C test with every button 0 press.

If an I2C error is detected or the write data is not verified when re-read from the follower, LED1 is turned on and the code halts at a breakpoint so that the received data and I2C status and interrupt flag registers can be examined.

> NOTE REGARDING PULL-UP RESISTORS:
> * Both the Wireless Starter Kit and Wireless Pro Kit mainboards include a Si7021 relative humidity and temperature sensor with the necessary pull-ups on a pair of pins connected to pins 15 (SCL) and 16 (SDA) on the EXP header.
> * Power for the sensor domain and thus its pull-ups is controlled by whatever GPIO is connected to the WSTK/WPK radio board connector P37 signal (named SENSOR_ENABLE in the schematics). For most devices, the example code drives the specified GPIO high to power the sensor domain.
> * The sensor itself has a different I2C address (0x40) than that configured for the MCU running the follower example project (0x71). Both devices reside on the bus, but this leader code communicates only with the matching follower.


## Peripherals used ##

* I2C
    * Clock Manager utilizes 38.4 MHz HFXO for I2C0CLK input
	* I2C bus frequency is configured for 100 kHz
	* I2C peripheral configured for leader operation
* LDMA (via DMADRV; used with wireless protocols and channel allocation required to cofunction with radio applications)
    * Two DMA channels are utilized, one for transmit and one for receive:
        * Peripheral to memory transfer of received I2C data to "i2c_rxBuffer"
	    * Memory to peripheral transfer of I2C commands and transmit data from "i2c_txBuffer"
    * Note: DMADRV APIs return a status code which can be utilized when debugging and for improved system security
* GPIO
	* LED0 is used to indicate when application has successfully written new values to follower device
	* LED1 is turned on to indicate a fault or error with I2C transfer

## How to Test ##

1.  Connect the SDA, SCL and GND lines between two kits via the EXP header.
2.  This example enables external I2C bus pull-ups on the Wireless Pro Kit as stated in the note in the description. If developing with custom or different hardware, external pull-up resistors are likely needed from VMCU to the SDA and SCL lines (only one pull-up needed for each line).
3.  Open Simplicity Studio and update each kit's firmware from the Simplicity
    Launcher (if necessary).
4.  Build the follower project and download it to one of the kits. It is not necessary for this kit to be connected to a PC running Simplicity Studio. Power can, for example, be provided by a USB wall charger.
5.  Build the leader DMA project and download it to the kit that will be connected to the debugger during testing.
6.  Run the leader DMA example. It will enter the main loop and wait for PB0. Before continuing, press the reset button of the kit running the follower example.
7.  Press PB0 on the leader kit to trigger an I2C test (read follower buffer, increment, write, read, verify).
8.  Observe LED0 toggle with a successful I2C test.
9.  Click the Pause button (parallel yellow bars) in the debugger to suspend code execution.
10. Observe that the values received from the follower match the values transmitted by leader by adding "i2c_rxBuffer" and "i2c_txBuffer", which are global variables, to the list in the Expressions tab.

## Hardware & Connections ##

* Board: Silicon Labs SixG301 Radio Board (BRD4407A) + Wireless Pro Kit Mainboard
    * Device: SIMG301M104LIL
        * PB01 - Push Button PB0
        * PD00 - I2C SCL input, Expansion Header 15
        * PD01 - I2C SDA input/output, Expansion Header 16
        * PD03 - LED0
        * PD04 - LED1

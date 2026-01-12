# Peripheral Examples - PIXELRZ Sequential #

## Summary ##

This project demonstrates how to configure the PIXELRZ in sequential mode to drive an LED strip of 60 WS2812Bs. Pressing Push Button 0 changes the LED strip's color between Red, Yellow, Green, Cyan, Blue, Purple, and Rainbow mode.

When changing the LED strip to a solid color, the RGB pixel data for each LED is stored in an array. A software command triggers the PIXELRZ to start transmitting. This sends a request to the LDMA to write the pixel data from the array to the PIXELRZ's FIFO. The PIXELRZ peripheral reads the pixel data from the FIFO, formats the data according to the PIXELRZ's configuration settings, and outputs the formatted data onto a pin.

In rainbow mode, the LETIMER is configured to underflow at a frequency of 500 Hz. The LETIMER is connected to the PIXELRZ via PRS. The LETIMER underflow signal triggers the PIXELRZ to start transmitting. LDMA transfers the pixel data from an array to the PIXELRZ's FIFO. Once the LDMA transfer completes, the LDMA callback function is called and updates the array with new pixel data. This process repeats to change the color of the LED strip by fading through all the colors of the rainbow.

## Peripherals used ##

* LDMA
* LETIMER
* GPIO
* PIXELRZ
* PRS

## How to Test ##

1. Open Simplicity Studio and update the kit's firmware from the Simplicity Launcher (if necessary).
2. Power the LED strip to a 5V power supply.
3. Connect the LED strip Data In signal to the Wireless Pro Kit as defined in Hardware & Connections section.
4. Build the example project and download it to the target system.
5. Press Push Button 0 to cycle through each color.

## Hardware & Connections ##

* Board: Silicon Labs SixG301 Radio Board (BRD4407A) + Wireless Pro Kit Mainboard
    * Device: SIMG301M104LIL
        * PB01 - Push button BTN0 input, Lower Breakout Header P16
        * PC01 - PIXELRZ output, Expansion Header 6

# Peripheral Examples - ACMP Polled #

## Summary ##

This project demonstrates polled operation of the ACMP by comparing the analog voltage on an input pin to the internal 1.25 V bandgap reference (VREF).

The input pin connected to Push Button 0 (PB0) is utilized in this code, such that when PB0 is pressed, the pin's input voltage is pulled to GND, and the comparator output goes low. When PB0 is released, the pin input voltage is pulled through a pull-up resistor to the common AVDD/IOVDD supply, and the comparator output goes high.

During operation, the CPU constantly polls the state of the comparator output (ACMP_STATUS_ACMPOUT bit) and when an input change is detected, the GPIO pin connected to LED0 is driven low or high in response.

## Peripherals used ##

* ACMP
    * Full input range from GND to VMCU (common AVDD/IOVDD supply)
    * Low accuracy mode, less current usage
    * Hysteresis disabled
    * VREFDIV set to maximum to disable reference division

## How to Test ##

1. Open Simplicity Studio and update the kit's firmware from the Simplicity Launcher (if necessary).
2. Build the example project and download it to the target system.
3. Press and hold button PB0; LED0 will turn on.
4. When button PB0 is released; LED0 will turn off.

## Hardware & Connections ##

* Board: Silicon Labs SixG301 Radio Board (BRD4407A) + Wireless Pro Kit Mainboard
    * Device: SIMG301M104LIL
        * PB01 - ACMP positive input, Push Button PB0
        * PD03 - LED0

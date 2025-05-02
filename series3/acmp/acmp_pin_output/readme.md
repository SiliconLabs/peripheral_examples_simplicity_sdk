# Peripheral Examples - ACMP Interrupt #

## Summary ##

This project demonstrates GPIO output from the ACMP without using interrupts or having firmware otherwise drive a pin in response to input voltage changes by routing the comparator output directly to a pin.

When the button is pushed, the input voltage is pulled to GND, and the comparator output goes low. Similarly, when the button is released, the input is pulled to the common AVDD/IOVDD supply, and the comparator output goes high.

During operation, the CPU core remains in EM1 while the comparator continues to operate. Input changes that cause the comparator output to change are reflected at the configured output pin with no need for software intervention once the ACMP is initialized. This functionality is available in all energy modes except EM4.

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
        * PD03 - ACMP output, LED0

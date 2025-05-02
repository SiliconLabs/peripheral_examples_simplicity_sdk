# Peripheral Examples - ACMP Interrupt #

## Summary ##

This project demonstrates interrupt-driven use of the ACMP by comparing the analog voltage on an input pin to the internal 1.25 V bandgap reference (VREF).

The input pin connected to Push Button 0 (PB0) is utilized in this code, such that when PB0 is pressed, the pin's input voltage is pulled to GND, and the comparator output goes low. When PB0 is released, the pin input voltage is pulled through a pull-up resistor to the common AVDD/IOVDD supply, and the comparator output goes high.

During operation, the CPU core remains in EM1 while the comparator continues to operate. When the input rises above or falls below the comparison voltage, the comparator requests an interrupt that wakes the CPU, which turns LED0 on or off in response before returning to EM1.

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

NOTE: PB0 provides an easy way to see the example in action, but mechanical bouncing may cause extra rising or falling edges on the ACMP input that cross the VREF threshold. Any of these can result in an interrupt request that changes the state of the GPIO pin driving LED0. Consider using an active device, such as a programmable power supply or the output from another processor, to drive the ACMP input so that its behavior can be observed in the absence of undamped mechanical noise.

## Hardware & Connections ##

* Board: Silicon Labs SixG301 Radio Board (BRD4407A) + Wireless Pro Kit Mainboard
    * Device: SIMG301M104LIL
        * PB01 - ACMP positive input, Push Button PB0
        * PD03 - LED0

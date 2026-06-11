# Peripheral Examples - ACMP Interrupt #

## Summary ##

This project shows how the ACMP module and its VSENSE inputs can be used to monitor a supply voltage without CPU intervention.

VSENSE provides access to the AVDD, IOVDD (ACMP0), and DVDD (ACMP1) supplies that are internally divided-by-4 and connected to the ACMP positive input when selected. This makes it simple to compare the specified supply against an arbitrary voltage derived by scaling either the 1.25 V or 2.5 V internal reference with the VREFDIV divider as the negative input.

In this example, AVDD is monitored with VSENSE01DIV4 such that the positive comparator input would range from 1.71 V / 4 = 0.4275 V to 3.8 V / 4 = 0.95 V with the device operating within datasheet-specified supply limits.

As mentioned above, crossing a desired supply threshold is detected by using VREFDIV to scale one of the internal references as the negative comparator input. A 2.54 V threshold is derived here by setting VREFDIV to 0x10 (16), such that the voltage compared against VSENSE01DIV4 is 2.5 V * (16 / 63) = 0.6349 V. Because the VSENSE interface presents the supply divided-by-4, VSENSE01DIV4 = AVDD / 4 = 2.54 V / 4 = 0.635 V.

Like the acmp_interrupt and acmp_pin_output examples, operation is in EM1, such that crossing the threshold in either direction wakes the device. LED0 is turned on in response to ACMP_IF_RISE interrupt and off in response to ACMP_IF_FALL.

## Peripherals used ##

* ACMP
    * Full input range from GND to VMCU (common AVDD/IOVDD supply)
    * Low accuracy mode, less current usage
    * Hysteresis disabled
    * VREFDIV set to maximum to disable reference division

## How to Test ##

1. Open Simplicity Studio and update the kit's firmware using **Device Manager Tool**(if necessary).
2. Build the example project and download it to the target system.
3. If debugging is active, terminate the debug session in your IDE to disconnect from the target device.
4. Slide the target system power switch to the BAT positrion to disconnect the debugger's power.
5. Attach a power supply to GND and VMCU pins of the mainboard expansion header; 3.30 V should be connected to EXP pin 2 and 0.0 V applied EXP pin 1.
6. Vary the power supply voltage above and below the 2.54 V threshold while otherwise observing the datasheet-specified supply range.
7. Observe the states of LED0 on the mainboard when the supply is above and below the threshold.

Simplified Testing With A Pro Kit Mainboard (BRD40002A):
1. Follow steps 1, 2, and 3 above.
2. While in the Simplicity Studio IDE, navigate to **Tools → Device Manager Tool**.
3. Select your board, click **Configure**, then open **Terminal → Admin**.
4. Change the VMCU supply voltage to 2.2 V by typing "target voltage 2.20" (no quotation marks) and pressing Enter. LED0 will be on.
5. Raise VMCU back to the typical 3.3 V by typing "target voltage 3.30" (again, no quotation marks) and pressing Enter. LED0 will be off.
6. When testing is complete, close the console tab in the IDE.

## Hardware & Connections ##

* Board: Silicon Labs SixG301 Radio Board (BRD4407A) + Wireless Pro Kit Mainboard
    * Device: SIMG301M104LIL
        * PD03 - LED0
        * VMCU - Expansion Header Pin 2
        * GND  - Expansion Header Pin 1 

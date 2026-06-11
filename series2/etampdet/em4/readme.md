# Peripheral Examples - ETAMPDET EM4 #

This project demonstrates the tamper detection module available on EFR32xG25, 
EFR32xG27 and EFR32xG29. The example uses #defines to enable the ETAMPDET peripheral's
channel 0 and/or channel 1, and requires an external jumper-wire connection
between the `ETAMPIN0 <-> ETAMPOUT0` and/or `ETAMPIN1 <-> ETAMPOUT1` pins, as
specified in device datasheet and README. The application is configured
similarly to the em4_no_dcdc example, allowing EM4 entry with BURTC with LFXO
or EM4 entry without BURTC utilizing ULFRCO. ETAMPDET is enabled for EM4 wakeup. If MCU reset is caused by EM4 wakeup
and ETAMPDET interrupt flag is set, `LED0` or `LED1` will toggle to indicate the
ETAMPDET tamper event on channel 0 or channel 1, respectively.

> Note: Default project gives EM4 current consumption numbers with BURTC.

> Note: When EFR32xG22 and later devices enter EM2/3, the clock to the CPU debug block 
is stopped, effectively dropping the host OC debugger connection. This examples 
includes an "escape hatch" mechanism to pause the device so that a debugger can
connect in order to erase flash.  To use this, press the PB0 push button and, 
while holding the button, press the RESET button in the lower right corner of 
the WSTK.  Upon releasing the RESET button, the device runs code that sees that 
PB0 is depressed, turns on `LED0`, and issues a breakpoint instruction to halt the
CPU. At this point, a the debugger connection can be resumed to erase flash, etc.

## Peripherals used ##
BURTC  - 32.768 kHz LFXO clock source
RMU

## How to Test ##
1. Open Simplicity Studio and update the kit's firmware using **Device Manager Tool**(if necessary)
2. Using a jumper-wire, electrically connect `ETAMPOUT0` to `ETAMPIN0` and 
   `ETAMPOUT1` to `ETAMPIN1`
3. Build the project and download to the Wireless Starter Kit, then exit
   from the debugger
4. Open the Simplicity Studio's Energy Profiler. Select Start Energy Capture
   from the Profiler menu and restart the wireless starter kit using Reset Pin.
5. Zoom in on the Y-axis (current) and observe the change in current draw.
6. Remove either jumper-wire
7. Observe `LED0` turn on when channel 0 jumper-wire is removed or `LED1` turn on 
   when channel 1 jumper-wire is removed, and additional current consumption in 
   Energy Capture as MCU wakes from EM4

### How to change to ULFRCO ###
1. After Step 1, in Clock Manager configuration change the `Clock Source Selection for EM4GRPACLK branch` to `ULFRCO`
2. Comment out the `burtc_init` function from line 105 to line 116 and in line 292
3. Continue with Step 2 

### Note for Testing on xG25 ###
Simplicity Studio's Energy Profiler tool only measures the VMCU current. On
BRD4270B Rev. A06 and beyond, RFVDD and DVDD are powered by the output of the
DCDC, IOVDD0-1 are powered by VMCU, VREGVDD and AVDD are powered by net VDD, and
IOVDD2 is powered by the VLDO_3V3. To best replicate datasheet test conditions
for current consumption measurements, the VDD and VLDO_3V3 net configurations
need to also be powered by VMCU. Please use the following extra steps:
1. Complete Steps 1 and 2 from the "How to Test" section above.
2. Resistors R210 and R211 are 0 Ohm resistor jumpers used to select the supply
   configuration for net VDD. By default, R210 is not populated and R211 is
   populated, powering VDD through VLDO_3V6. Refer to the BRD4270B schematic and assembly 
   documentation for details. Search for these resistor designators in the assembly 
   file to identify their exact locations on the board.
3. Remove R211 and solder a 0 Ohm resistor to R210.
4. Resistors R202 and R248 are 0 Ohm resistor jumpers used to select the supply
   configuration for net VLDO_3V3. By default, R202 is not populated and R248 is
   populated, powering VLDO_3V3 through onboard LDO. 
5. Remove R248 and solder a 0 Ohm resistor to R202.

### Note for Testing on xG27 ###
On BRD4194A, VMCU is a 3.3V supply that powers AVDD and IOVDD on xG27.
In the datasheet, current consumption test conditions have AVDD and IOVDD
powered by either the DC-DC at 1.8V, an external 1.8V supply, 
or an external 3.0V supply. Due to the design of the radio board, this 
board does not replicate the datasheet test conditions for current consumption,
and the measured value may differ from the datasheet value. 
Additionally, due to the connection of `LED1` to ETAMPDET channel 0 input,
additional current consumption will be observed in EM4 with channel 0 enabled
(default setting) and `LED1` will "flicker" with the random generated bit stream
from the ETAMPDET peripheral.

### Note for Testing on xG29 ###
On BRD4194A, VMCU is a 3.3V supply that powers AVDD and IOVDD on xG27.
In the datasheet, current consumption test conditions have AVDD and IOVDD
powered by either the DC-DC at 1.8V, an external 1.8V supply, 
or an external 3.0V supply. Due to the design of the radio board, this 
board does not replicate the datasheet test conditions for current consumption,
and the measured value may differ from the datasheet value. 
Additionally, due to the connection of `LED1` to ETAMPDET channel 0 input,
additional current consumption will be observed in EM4 with channel 0 enabled
(default setting) and `LED1` will "flicker" with the random generated bit stream
from the ETAMPDET peripheral.

## Hardware & Connections ##
* Board:  Silicon Labs EFR32xG25 Radio Board (BRD4270B) + 
        Wireless Starter Kit Mainboard
	* Device: EFR32FG25B222F1920IM56
		* PA05 - ETAMPDET ETAMPIN0, Expansion Header Pin 7, WSTK Pin 4
		* PA06 - ETAMPDET ETAMPOUT0, Expansion Header Pin 11, WSTK Pin 8
		* PB00 - Push Button PB0
		* PC06 - LED0
		* PC07 - LED1
		* PD04 - ETAMPDET ETAMPOUT1, WSTK Pin 24
      * PD05 - ETAMPDET ETAMPIN1, WSTK Pin 31

* Board:  Silicon Labs EFR32xG27 Buck Radio Board (BRD4194A) + 
        Wireless Starter Kit Mainboard
	* Device: EFR32MG27C140F768IM40
		* PB00 - Push Button PB0
		* PB00 - LED0
		* PB01 - LED1
      * PB01 - ETAMPDET ETAMPIN0,  Expansion Header Pin 9, WSTK Pin 6
		* PC00 - ETAMPDET ETAMPIN1,  Expansion Header Pin 4, WSTK Pin 1
		* PC01 - ETAMPDET ETAMPOUT0, Expansion Header Pin 6, WSTK Pin 3
		* PC02 - ETAMPDET ETAMPOUT1, Expansion Header Pin 8, WSTK Pin 5

* Board:  Silicon Labs EFR32xG29 Buck Radio Board (BRD4412A) + 
        Wireless Starter Kit Mainboard
	* Device: EFR32MG29B140F1024IM40
		* PB00 - Push Button PB0
		* PB00 - LED0
		* PB01 - LED1
      * PB01 - ETAMPDET ETAMPIN0,  Expansion Header Pin 9, WSTK Pin 6
		* PC00 - ETAMPDET ETAMPIN1,  Expansion Header Pin 4, WSTK Pin 1
		* PC01 - ETAMPDET ETAMPOUT0, Expansion Header Pin 6, WSTK Pin 3
		* PC02 - ETAMPDET ETAMPOUT1, Expansion Header Pin 8, WSTK Pin 5
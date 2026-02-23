# Peripheral Examples - EM1 and EM1DIV16 #

## Summary ##

This project demonstrates the datasheet current consumption configuration and expected current levels for EM1 and EM1DIV16 Energy Modes. 

The project contains both EM1 and EM1DIV16 examples in a single `app.c` file. The active mode is selected by setting the `USE_EM1DIV16` variable inside the file. By default, the project runs the EM1DIV16 example. 

If EM1 is selected, the application provides two SYSCLK configuration options, controlled by the `USE_EM1_HFRCODPLL_100MHZ` macro:
- Default SLCP clocking (SOCPLL @ 150 MHz, HFXO‑referenced)
- Optional HFRCODPLL configuration (100 MHz)

## Peripherals used ##

### EM1 Example
Option 1 – Default SLCP configuration
* SYSCLK: SOCPLL @ 150 MHz
* Reference: HFXO (38.4 MHz)
* Energy Mode: EM1

Option 2 – HFRCODPLL configuration
* SYSCLK: HFRCODPLL @ 100 MHz
* Energy Mode: EM1

### EM1DIV16 Example
* SYSCLK: HFXO @ 38.4 MHz
* HCLK: Divided by 16
* PCLK: Divided by 2
* Energy Mode: EM1DIV16

## How to Test ##

1. Update the kit’s firmware from the Simplicity Studio Launcher (if necessary).  
2. Open `app.c` and select the desired mode by setting the `USE_EM1DIV16` macro (`1` for EM1DIV16 or `0` for EM1); when EM1 is selected (`USE_EM1DIV16 = 0`), the SYSCLK source can be chosen using the `USE_EM1_HFRCODPLL_100MHZ` macro.
3. Build the project and download it to the Wireless Starter Kit, then exit the debugger.
4. Open Simplicity Studio’s Energy Profiler.
5. Select Start Energy Capture, then press the RESET button on the board.
6. Zoom in on the Y‑axis (current) and observe the steady‑state current draw.

## Hardware & Connections ##

* Board: Silicon Labs SixG301 Radio Board (BRD4407A) + Wireless Pro Kit Mainboard
    * Device: SIMG301M104LIL
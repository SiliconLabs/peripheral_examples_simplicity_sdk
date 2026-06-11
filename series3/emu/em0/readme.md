# Peripheral Examples - EM0 #

## Summary ##

This project demonstrates two EM0 operating configurations that can be selected at build time. Both configurations are implemented in a single `app.c` file, and the active mode is chosen by setting the `USE_HFRCODPLL_100MHZ` variable. 

By default, the project sets `USE_HFRCODPLL_100MHZ` to `1`, which switches the SYSCLK to HFRCODPLL. Setting the variable to `0` keeps the device in its default startup configuration as defined in the SLCP file.

## Peripherals used ##

### EM0 Example using SOCPLL
* SYSCLK: SOCPLL @ 150 MHz (startup configuration from SLCP)
* Reference Clock: HFXO (38.4 MHz crystal)

### EM0 Example using HFRCODPLL
* SYSCLK: HFRCODPLL @ 100 MHz 

## How to Test ##

1. Open Simplicity Studio and update the kit's firmware using **Device Manager Tool**(if necessary). 
2. Open `app.c` and select the desired mode by setting the `USE_HFRCODPLL_100MHZ` variable to `1` for the HFRCODPLL based EM0 example or `0` for the SOCPLL based EM0 example.
3. Build the project and download it to the Wireless Starter Kit, then exit the debugger.  
4. Open Simplicity Studio’s Energy Profiler.  
5. Select Start Energy Capture, then press the RESET button on the board.  
6. Zoom in on the Y‑axis (current) and observe the steady‑state EM0 behavior.

## Hardware & Connections ##

* Board: Silicon Labs SixG301 Radio Board (BRD4407A) + Wireless Pro Kit Mainboard
    * Device: SIMG301M104LIL
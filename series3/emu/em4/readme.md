# Peripheral Examples – EM4 #

## Summary ##

This project demonstrates the datasheet current consumption configuration and expected behavior when entering EM4 Energy Mode. 

Two EM4 configurations are provided in a single `app.c` file, selected using the `USE_BURTC_IN_EM4` macro:
- EM4 with BURTC disabled, No low‑frequency activity is requested. LF oscillators are automatically powered down by hardware during EM4 entry, resulting in the lowest achievable EM4 current.
- EM4 with BURTC enabled, BURTC continues running during EM4 using the EM4GRPACLK domain. The EM4GRPACLK source is configured in the project’s SLCP settings (default: LFRCO @ 32.768 kHz). Because BURTC requires a low‑frequency clock, the LF oscillator remains active.

In both configurations, all GPIOs are disabled to eliminate external leakage paths, and the device enters EM4 where wake‑up requires a reset event.

## Peripherals Used ##

### EM4 with BURTC Disabled
* Energy Mode: EM4 with no BURTC and no LF oscillator

### EM4 with BURTC Enabled
* BURTC running from LFRCO @ 32.768 kHz
* Energy Mode: EM4 

## How to Test ##

1. Open Simplicity Studio and update the kit's firmware using **Device Manager Tool**(if necessary). 
2. Open `app.c` and select the desired EM4 configuration by setting the `USE_BURTC_IN_EM4` macro (`1` to enable BURTC during EM4, `0` to disable it).  
3. Build and flash the project to the Wireless Starter Kit, then exit the debugger.  
4. Open Simplicity Studio’s Energy Profiler.  
5. Start an energy capture session and press the RESET button on the board.  
6. Observe the steady‑state current draw after the device enters EM4.

## Hardware & Connections ##

* Board: Silicon Labs SixG301 Radio Board (BRD4407A) + Wireless Pro Kit Mainboard  
  * Device: SIMG301M104LIL
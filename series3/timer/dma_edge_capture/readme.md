# Peripheral Examples - TIMER DMA Edge Capture #

## Summary ##

This project demonstrates DMA driven edge capture from a TIMER Compare/Capture channel. TIMER0 CC0 is configured to capture rising and falling edges. A GPIO Pin is to be connected to a periodic signal, and edges captured from GPIO pin are stored in CC0. The LDMA is configured to transfer the first 512 edges to a fixed length buffer. The buffer is stored globally for possible future processing.

## Peripherals used ##

* EM01GRPACLK - Sourced from HFRCODPLL @ 38 MHz
* TIMER0 - Input Capture Mode
* LDMA - CC0 Peripheral to Memory Transfer

## How to Test ##

1. Build the project and download to the Starter Kit.
2. Connect a periodic signal to GPIO Pin (see board specific pin below).
3. View the buffer[] global array in the debugger.
4. Observe LED0 toggle upon BUFFERSIZE captures of input signal rising and falling edges.

## Hardware & Connections ##

* Board: Silicon Labs SixG301 Radio Board (BRD4407A) + Wireless Pro Kit Mainboard
    * Device: SIMG301M104LIL
        * PA06 - TIM0_CC0 (Expansion Header Pin 11, P08)

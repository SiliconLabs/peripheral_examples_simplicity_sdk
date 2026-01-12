# Peripheral Examples - GPCRC with DMA #

## Summary ##

This project demonstrates the GPCRC used to check 32-bit data using the 
IEEE 802.3 polynomial standard in EM1. Data is fed into the GPCRC via the LDMA.
The size of the data array being fed into LDMA and GPCRC can be increased 
depending on the application. The example processes 16 words but can be expanded
to process more data as needed. 

Using the GPCRC with LDMA has two benefits:

It does not tie up the CPU. Once the LDMA descriptors are programmed and the 
selected channel is started, the full data array is fed to the GPCRC without any 
CPU intervention.
Execution is deterministic. The LDMA always feeds an entire array into the GPCRC, 
and this is going to have a set execution time (unless other bandwidth intensive 
LDMA traffic is present).

Using the GPCRC peripheral also greatly reduces the software overhead required 
to calculate CRC32. 

The GPCRC can also be used to blank check flash pages. This example (along with
benchmarking data) is available here - https://github.com/SiliconLabs/platform_applications/tree/master/platform_gpcrc_blank_check 

## Peripherals used ##

* GPCRC - IEEE 802.3 poly standard
* LDMA  - Channel 0

## How to Test ##

1. Update the kit's firmware from the Simplicity Launcher (if necessary)
2. Build the project and download to the Starter Kit
3. Open the Simplicity Debugger and add "crcResult" to the Expressions window
4. Add a breakpoint in the `crc_check_result()` function where crcResult is read out
5. Run the debugger. You should see it the same values in crcResult and 
   FIBONACCI_CRC_16WORDS.  

## Hardware & Connections ##

* Board: Silicon Labs SixG301 Radio Board (BRD4407A) + Wireless Pro Kit Mainboard
    * Device: SIMG301M104LIL


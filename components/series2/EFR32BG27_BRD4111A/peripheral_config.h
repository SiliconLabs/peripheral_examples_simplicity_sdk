// LED active low
#define LED_ON                     0
#define LED_OFF                    1

/*
 * RAM power-down end for EM2 and EM3
 *
 * NOTE:
 * The top 8 KB of RAM (BLK1) MUST remain powered in EM2/EM3
 * (SYSCFG_DMEM0RETNCTRL_RAMRETNCTRL != 2), otherwise the device
 * may hard fault on wake-up depending on stack contents,
 * e.g., the return address from EM3.
 */
#define RAM_POWER_DOWN_END         (SRAM_BASE + SRAM_SIZE - 0x2000)
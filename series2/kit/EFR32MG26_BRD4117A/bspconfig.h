/***************************************************************************//**
 * @file
 * @brief Provide BSP (board support package) configuration parameters.
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/
 

#ifndef BSPCONFIG_H
#define BSPCONFIG_H

#define BSP_STK
#define BSP_WSTK
#define BSP_WSTK_BRD4117A

#define BSP_BCC_USART         USART0
#define BSP_BCC_USART_INDEX   0
#define BSP_BCC_CLK           cmuClock_USART0
#define BSP_BCC_TXPORT        gpioPortA
#define BSP_BCC_TXPIN         8
#define BSP_BCC_RXPORT        gpioPortA
#define BSP_BCC_RXPIN         9

#define BSP_BCC_ENABLE_PORT   gpioPortB
#define BSP_BCC_ENABLE_PIN    0                 /* VCOM_ENABLE */

#define BSP_DISP_ENABLE_PORT  gpioPortC
#define BSP_DISP_ENABLE_PIN   9

#define BSP_EXTFLASH_PRESENT  1

#define BSP_GPIO_LEDS
#define BSP_NO_OF_LEDS          2
#define BSP_GPIO_LED0_PORT      gpioPortB
#define BSP_GPIO_LED0_PIN       2
#define BSP_GPIO_LED1_PORT      gpioPortB
#define BSP_GPIO_LED1_PIN       4
#define BSP_GPIO_LEDARRAY_INIT  { { BSP_GPIO_LED0_PORT, BSP_GPIO_LED0_PIN }, { BSP_GPIO_LED1_PORT, BSP_GPIO_LED1_PIN } }

#define BSP_GPIO_BUTTONS
#define BSP_NO_OF_BUTTONS       2
#define BSP_GPIO_PB0_PORT       gpioPortB
#define BSP_GPIO_PB0_PIN        1
#define BSP_GPIO_PB1_PORT       gpioPortB
#define BSP_GPIO_PB1_PIN        3

#define BSP_GPIO_BUTTONARRAY_INIT { { BSP_GPIO_PB0_PORT, BSP_GPIO_PB0_PIN }, { BSP_GPIO_PB1_PORT, BSP_GPIO_PB1_PIN } }

#define BSP_INIT_DEFAULT        0

#if !defined(EMU_DCDCINIT_WSTK_DEFAULT)
/* Use emlib defaults */
#define EMU_DCDCINIT_WSTK_DEFAULT EMU_DCDCINIT_DEFAULT
#endif

/* The LFXO external crystal needs a load capacitance of 7.0 pF (CL=7.0 pF).
 * This means that we need 14 pF on each pin. Since this board has approximately
 * 4.5 pF parasitic capacitance on each pin we configure a 9.5 pF internal
 * capacitance
 *
 * (37+1) * 0.25 pF = 9.5 pF
 */
#define BSP_LFXO_CTUNE          37U
#define BSP_HFXO_CTUNE          124U

#if !defined(CMU_HFXOINIT_WSTK_DEFAULT)
/* HFXO initialization values for XTAL mode. */
#define CMU_HFXOINIT_WSTK_DEFAULT                                   \
  {                                                                 \
    cmuHfxoCbLsbTimeout_416us,                                      \
    cmuHfxoSteadyStateTimeout_833us,  /* First lock              */ \
    cmuHfxoSteadyStateTimeout_83us,   /* Subsequent locks        */ \
    0U,                         /* ctuneXoStartup                */ \
    0U,                         /* ctuneXiStartup                */ \
    32U,                        /* coreBiasStartup               */ \
    32U,                        /* imCoreBiasStartup             */ \
    cmuHfxoCoreDegen_None,                                          \
    cmuHfxoCtuneFixCap_Both,                                        \
    BSP_HFXO_CTUNE,             /* ctuneXoAna                    */ \
    BSP_HFXO_CTUNE,             /* ctuneXiAna                    */ \
    60U,                        /* coreBiasAna                   */ \
    false,                      /* enXiDcBiasAna                 */ \
    cmuHfxoOscMode_Crystal,                                         \
    false,                      /* forceXo2GndAna                */ \
    false,                      /* forceXi2GndAna                */ \
    false,                      /* DisOndemand                   */ \
    false,                      /* ForceEn                       */ \
    false                       /* Lock registers                */ \
  }
#endif

#if !defined(BSP_LFXOINIT_DEFAULT)
/* LFXO initialization values optimized for the board. */
#define BSP_LFXOINIT_DEFAULT                                                           \
  {                                                                                    \
    1,                            /* CL=7.0 pF which makes gain=1 a good value. */     \
    BSP_LFXO_CTUNE,               /* Optimal CTUNE value to get 32768 Hz. */           \
    cmuLfxoStartupDelay_2KCycles, /* Timeout before oscillation is stable. */          \
    cmuLfxoOscMode_Crystal,       /* Crystal mode */                                   \
    false,                        /* Disable high amplitude mode */                    \
    true,                         /* Enable AGC for automatic amplitude adjustment. */ \
    false,                        /* Disable failure detection in EM4. */              \
    false,                        /* Disable failure detection. */                     \
    false,                        /* LFXO starts on demand. */                         \
    false,                        /* LFXO starts on demand. */                         \
    false                         /* Don't lock registers.  */                         \
  }
#endif

#define BSP_BCP_VERSION 2
#include "bsp_bcp.h"

#endif /* BSPCONFIG_H */

/***************************************************************************//**
 * @file
 * @brief Top level application functions
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

#include "sl_clock_manager.h"
#include "sl_interrupt_manager.h"
#include "sl_gpio.h"
#include "sl_hal_acmp.h"
#include "pin_config.h"

const sl_gpio_t GPIO_LED0 = { .port = LED0_PORT, .pin = LED0_PIN };

/***************************************************************************//**
 * Initialize GPIO.
 ******************************************************************************/
void gpio_init(void)
{
  // Initialize GPIO driver
  sl_gpio_init();

  // Configure LEDs as push-pull with output driving LED off
  sl_gpio_set_pin_mode(&GPIO_LED0, SL_GPIO_MODE_PUSH_PULL, LED_OFF);
}

/***************************************************************************//**
 * Initialize ACMP.
 ******************************************************************************/
void acmp_init(void)
{
  // Enable ACMP bus clock
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_ACMP0);

  // Initialize with settings for supply monitoring
  sl_hal_acmp_init_t init = SL_HAL_ACMP_INIT_DEFAULT;
  init.bias_prog = 0x2;                      // Use reduced bias for lower current
  init.accuracy = SL_HAL_ACMP_ACCURACY_HIGH; // High accuracy mode to minimize wake-ups
  init.vref_div = 0x10;                      // Scaled VREFDIV for comparison against VSENSE

  sl_hal_acmp_init(ACMP0, &init);
  
  // Enable ACMP
  sl_hal_acmp_enable(ACMP0);
  
  // For supply monitoring, the selected reference is the negative input
  // and the desired VSENSE channel is the positive input.
  //
  // When ACMP0 is used, VSENSE0 is the AVDD supply and VSENSE1 is the
  // IOVDD supply (IOVDD0 on a device that also has an IOVDD1).  In
  // either case, the 1DIV4 designation indicates that the input voltage
  // to the comparator is divided by 4.
  //
  // In this example, the 2.5 V internal reference is multiplied by
  // 16 (0x10) / 63 (0x3F) to set a comparison threshold of 0.6349 V.
  // When AVDD = 2.54 V, VENSE0 = 2.54 V / 4 = 0.635 V.
  sl_hal_acmp_set_input(ACMP0,
                        SL_HAL_ACMP_INPUT_VREFDIV2V5,
                        SL_HAL_ACMP_INPUT_VSENSE01DIV4);

  // Wait for warmup
  while(!(ACMP0->IF & ACMP_IF_ACMPRDY));

  // Clear pending ACMP interrupts
  sl_interrupt_manager_clear_irq_pending(ACMP0_IRQn);
  sl_hal_acmp_clear_interrupts(ACMP0, ACMP_IF_RISE | ACMP_IF_FALL);

  // Enable ACMP interrupts
  sl_interrupt_manager_enable_irq(ACMP0_IRQn);
  sl_hal_acmp_enable_interrupts(ACMP0, ACMP_IEN_RISE | ACMP_IEN_FALL);
}

/***************************************************************************//**
 * ACMP ISR.
 ************************************************************************(*****/
void ACMP0_IRQHandler(void)
{
  uint32_t flags = sl_hal_acmp_get_pending_interrupts(ACMP0);
  sl_hal_acmp_clear_interrupts(ACMP0, flags);

  // ACMP output high (supply above threshold)
  if(flags & ACMP_IF_RISE) {
    // Turn on LED
    sl_gpio_set_pin(&GPIO_LED0);
  }

  // ACMP output low (supply below threshold)
  if(flags & ACMP_IF_FALL) {
    // Turn off LED
    sl_gpio_clear_pin(&GPIO_LED0);
  }
}

/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  // Initialize GPIO and ACMP
  gpio_init();
  acmp_init();
}

/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{
}

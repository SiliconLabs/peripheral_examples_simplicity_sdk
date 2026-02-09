/***************************************************************************//**
 * @file
 * @brief Top level application functions
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
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
#include "sl_power_manager.h"

#include "sl_gpio.h"

#include "em_vdac.h"
#include "em_timer.h"
#include "em_ldma.h"

#include "pin_config.h"

// 32‑point lookup table representing one full sine wave cycle.
// The LDMA engine repeatedly streams these values to the VDAC output buffer.
#define SINE_TABLE_SIZE 32
static const uint16_t sineTable[SINE_TABLE_SIZE] = {
  2048 , 2447 , 2831 , 3185 , 3495 , 3750 , 3939 , 4056 ,
  4095 , 4056 , 3939 , 3750 , 3495 , 3185 , 2831 , 2447 ,
  2048 , 1648 , 1264 , 910  , 600  , 345  , 156  , 39   ,
  0    , 39   , 156  , 345  , 600  , 910  , 1264 , 1648 ,
};

// Select which VDAC channel to use (0 or 1)
#define CHANNEL_NUM 0

// Set the VDAC to max frequency of 1 MHz
#define CLK_VDAC_FREQ              1000000

// Note: change this to determine the frequency of the sine wave
#define WAVEFORM_FREQ              10000

// The timer needs to run at SINE_TABLE_SIZE times faster than the desired
// waveform frequency because it needs to output SINE_TABLE_SIZE points in that
// same amount of time
#define TIMER0_FREQ                (WAVEFORM_FREQ * SINE_TABLE_SIZE)

const sl_gpio_t LED0 = { .port = LED0_PORT,
                         .pin  = LED0_PIN };

const sl_gpio_t ESCAPE_HATCH = { .port = BUTTON0_PORT,
                                 .pin  = BUTTON0_PIN };

/*
 * The VDAC output port and pin are configured through the VDAC_OUTCTRL register.
 * The ABUSPORTSELCHx fields in this register select the GPIO port used for the
 * CHx ABUS output. The field values are defined as follows:
 *
 *   0  No GPIO selected for CHx ABUS output
 *   1  Port A selected
 *   2  Port B selected
 *   3  Port C selected
 *   4  Port D selected
 *
 * When using the main VDAC output, no additional port/pin configuration is
 * required. Refer to the device Reference Manual and Datasheet for further
 * information. In this example, CH0 is routed to the main output pin PB00
 * on all devices except BRD4270B (xG25), where CH0 must instead use the
 * auxiliary output on PA06. On BRD4270B, the CH0 main output is tied to an
 * external pullup resistor, capacitor, and pushbutton on the WSTK, which
 * interferes with the analog output signal; therefore, the auxiliary output
 * provides a clean, usable path.
 */
#if defined(EFR32FG25B222F1920IM56)
const sl_gpio_t VDAC_CH0_AUXOUT = { .port = VDAC_CH0_AUXOUT_PORT,
                                 .pin  = VDAC_CH0_AUXOUT_PIN };
#else
const sl_gpio_t VDAC_CH0_MAINOUT = { .port = VDAC0_CH0_MAINOUT_PORT,
                                     .pin  = VDAC0_CH0_MAINOUT_PIN };
#endif

/**************************************************************************//**
 * @brief
 *    VDAC initialization
 *****************************************************************************/
void vdac_init(void)
{
  // Load default VDAC configuration structures
  VDAC_Init_TypeDef        init        = VDAC_INIT_DEFAULT;
  VDAC_InitChannel_TypeDef initChannel = VDAC_INITCHANNEL_DEFAULT;

  // Enable VDAC clock
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_VDAC0);

  // Compute the prescaler value required to achieve a 1 MHz VDAC clock
  init.prescaler = VDAC_PrescaleCalc(VDAC0, (uint32_t)CLK_VDAC_FREQ);

  // Set reference to internal 1.25V low noise reference
  init.reference = vdacRef1V25;

  // Since the minimum load requirement for high capacitance mode is 25 nF, turn
  // this mode off
  initChannel.highCapLoadEnable = false;

#if defined(EFR32FG25B222F1920IM56)
  // Enable the GPIO clock
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_GPIO);

  // Disable PA06 input
  sl_gpio_set_pin_mode(&VDAC_CH0_AUXOUT, SL_GPIO_MODE_DISABLED, false);

  // Connect VDAC0 CH0 to an even pin on Port A via the ABUS
  GPIO->ABUSALLOC = GPIO_ABUSALLOC_AEVEN0_VDAC0CH0;

  // Disable Main output
  initChannel.mainOutEnable = false;

  // Enable Auxiliary output
  initChannel.auxOutEnable = true;

  // Output to PA06
  initChannel.port = vdacChPortA;
  initChannel.pin = 6;
#else
  // Disable VDAC0 main output pin
  sl_gpio_set_pin_mode(&VDAC_CH0_MAINOUT, SL_GPIO_MODE_DISABLED, 0);
#endif
  // Initialize the VDAC and selected channel
  VDAC_Init(VDAC0, &init);
  VDAC_InitChannel(VDAC0, &initChannel, CHANNEL_NUM);

  // Enable the VDAC
  VDAC_Enable(VDAC0, CHANNEL_NUM, true);
}

/**************************************************************************//**
 * @brief
 *    Timer initialization
 *****************************************************************************/
void timer_init(void)
{
  uint32_t timerFreq, topValue;

  // Enable clock for TIMER0 module
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_TIMER0);

  // Initialize TIMER0
  TIMER_Init_TypeDef init = TIMER_INIT_DEFAULT;
  init.dmaClrAct = true;
  init.enable = false;
  TIMER_Init(TIMER0, &init);

  // Get EM01GRPACLK frequency in MHz for compare value calculations
  sl_clock_manager_get_clock_branch_frequency(SL_CLOCK_BRANCH_EM01GRPACLK,
                                              &timerFreq);
  topValue = (timerFreq / TIMER0_FREQ);

  // Set top value to overflow at the desired TIMER0_FREQ frequency
  TIMER_TopSet(TIMER0, topValue);

  // Enable TIMER0
  TIMER_Enable(TIMER0, true);
}

/**************************************************************************//**
 * @brief
 *    Initialize the LDMA module
 *
 * @details
 *    Configure the channel descriptor to use the default memory to
 *    peripheral transfer descriptor. Modify it to not generate an interrupt
 *    upon transfer completion (we don't need it for this example).
 *    Also make the descriptor link to itself so that the descriptor runs
 *    continuously. Additionally, the transfer configuration selects the
 *    TIMER0 overflow signal as the trigger for the DMA transaction to occur.
 *
 * @note
 *    The descriptor object needs to at least have static scope persistence so
 *    that the reference to the object is valid beyond its first use in
 *    initialization. This is because this code loops back to the same
 *    descriptor after every dma transfer. If the reference isn't valid anymore,
 *    then all dma transfers after the first one will fail.
 ******************************************************************************/
void ldma_init(void)
{
  // Descriptor loops through the sine table and outputs its values to the VDAC
  static LDMA_Descriptor_t loopDescriptor =
    LDMA_DESCRIPTOR_LINKREL_M2P_BYTE(&sineTable[0],   // Memory source address
                                     &VDAC0->CH0F,    // Peripheral destination address
                                     SINE_TABLE_SIZE, // Number of halfwords per transfer
                                     0);              // Link to same descriptor

  // Don't trigger interrupt when transfer is done
  loopDescriptor.xfer.doneIfs = 0;

  // Transfer halfwords (VDAC data register is 12 bits)
  loopDescriptor.xfer.size = ldmaCtrlSizeHalf;

  // Transfer configuration and trigger selection
  // Trigger on TIMER0 overflow and set loop count to size of the sine table
  // minus one
  LDMA_TransferCfg_t transferConfig =
    LDMA_TRANSFER_CFG_PERIPHERAL(ldmaPeripheralSignal_TIMER0_UFOF);

  // LDMA initialization
  LDMA_Init_t init = LDMA_INIT_DEFAULT;
  LDMA_Init(&init);

  // Start the transfer
  uint32_t channelNum = 0;
  LDMA_StartTransfer(channelNum, &transferConfig, &loopDescriptor);
}
/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{

  // Initialize the VDAC peripheral and channel
  vdac_init();

  // Initialize LDMA
  ldma_init();

  // Initialize TIMER
  timer_init();
}

/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{
}
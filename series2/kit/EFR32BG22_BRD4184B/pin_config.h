// <<< sl:start pin_tool >>>

// =========================================================
// MICROPHONE CONTROL
// =========================================================
// $[GPIO_MIC_ENABLE]
#define MIC_ENABLE_PORT      SL_GPIO_PORT_C
#define MIC_ENABLE_PIN       7
// [GPIO_MIC_ENABLE]$

// =========================================================
// PDM INTERFACE
// =========================================================
// <gpio> PDM_CLK
// $[GPIO_PDM_CLK]
#define PDM_CLK_PORT         SL_GPIO_PORT_B
#define PDM_CLK_PIN          0
// [GPIO_PDM_CLK]$

// <gpio> PDM_DAT0
// $[GPIO_PDM_DAT0]
#define PDM_DAT0_PORT        SL_GPIO_PORT_B
#define PDM_DAT0_PIN         1
// [GPIO_PDM_DAT0]$

// <<< sl:end pin_tool >>>

// NOTE:
// Each physical GPIO is defined exactly once in pin_tool.
// All other signals must reuse these definitions.
// Multiple aliases may map to the same physical GPIO pin.
// For a given GPIO only one alias/function is used at runtime

#define SLEW_RATE_PORT       PDM_CLK_PORT
#define SLEW_RATE_PIN        PDM_CLK_PIN

#define PDM_DAT1_PORT        PDM_DAT0_PORT
#define PDM_DAT1_PIN         PDM_DAT0_PIN

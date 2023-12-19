#ifndef DEVICE_NAME
#define DEVICE_NAME "LEA RX"
#endif
// There is some special handling for this target
//#define TARGET_RX_GHOST_ATTO_V1
#define TARGET_RX_LEA

// GPIO pin definitions
#define GPIO_PIN_NSS                PB_12
#define GPIO_PIN_BUSY               PB_1
#define GPIO_PIN_DIO1               PB_2
#define GPIO_PIN_MOSI               PB_15
#define GPIO_PIN_MISO               PB_14
#define GPIO_PIN_SCK                PB_13
#define GPIO_PIN_RST                PB_0

//#define GPIO_PIN_RCSIGNAL_RX    PB7
//#define GPIO_PIN_RCSIGNAL_TX    PB6
#define GPIO_PIN_RCSIGNAL_RX        PA_10
#define GPIO_PIN_RCSIGNAL_TX        PA_9

//#define GPIO_PIN_LED            PA7
//#define GPIO_PIN_LED_WS2812      PA7
//#define GPIO_PIN_LED_WS2812_FAST PA_7
//#define GPIO_PIN_BUTTON         PA12

#define GPIO_PIN_DEBUG_RX           PA_3
#define GPIO_PIN_DEBUG_TX           PA_2

// Output Power - use default SX1280
#define POWER_OUTPUT_FIXED 13 //MAX power for 2400 RXes that doesn't have PA is 12.5dbm

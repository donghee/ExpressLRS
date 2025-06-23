#if !defined(DEVICE_NAME)
    #define DEVICE_NAME             "LEA AIO TX"
#endif
// There is some special handling for this target
#define TARGET_AIO_TX_LEA

#define HAL_SPI_MODULE_ENABLED

// Any device features
// #if !defined(USE_OLED_SPI_SMALL)
//     #define USE_OLED_SPI
// #endif

// #define HAS_FIVE_WAY_BUTTON

// GPIO pin definitions
#define GPIO_PIN_NSS                PI0
#define GPIO_PIN_BUSY               PG13
#define GPIO_PIN_DIO1               PG14
#define GPIO_PIN_MOSI               PB15
#define GPIO_PIN_MISO               PB14
#define GPIO_PIN_SCK                PI1
#define GPIO_PIN_RST                PB5
// #define GPIO_PIN_TX_ENABLE          PA2  // Works on Lite
// #define GPIO_PIN_RX_ENABLE          PA3 // Works on Lite
// #define GPIO_PIN_ANT_CTRL           PA9
// #define GPIO_PIN_ANT_CTRL_COMPL     PB13
#define GPIO_PIN_RCSIGNAL_RX        PA10 // S.PORT (Only needs one wire )
#define GPIO_PIN_RCSIGNAL_TX        PA9  // Needed for CRSF libs but does nothing/not hooked up to JR module.

#define GPIO_PIN_LED            PG9
//#define GPIO_PIN_LED_WS2812         PB_8
//#define GPIO_PIN_LED_WS2812_FAST    PB_6
// #define GPIO_PIN_PA_ENABLE          PB11  // https://www.skyworksinc.com/-/media/SkyWorks/Documents/Products/2101-2200/SE2622L_202733C.pdf
// #define GPIO_PIN_RF_AMP_DET         PA3  // Voltage detector pin
//#define GPIO_PIN_BUZZER             PB_7
// #define GPIO_PIN_OLED_CS            PC14
// #define GPIO_PIN_OLED_RST           PB12
// #define GPIO_PIN_OLED_DC            PC15
// #define GPIO_PIN_OLED_MOSI          PB5
// #define GPIO_PIN_OLED_SCK           PB3
// #define GPIO_PIN_JOYSTICK           A0

// #define GPIO_PIN_DEBUG_RX           PA_3 // WTF! It's used for AMP_DET
// #define GPIO_PIN_DEBUG_TX           PA_2

#define GPIO_PIN_DEBUG_RX           PC11 // UART 4
#define GPIO_PIN_DEBUG_TX           PC10 // UART 4

// Output Power
#define MinPower                    PWR_10mW
#define MaxPower                    PWR_250mW
#define POWER_OUTPUT_VALUES         {-16,-14,-11,-8,-4}

// #ifndef JOY_ADC_VALUES
// [> Joystick values              {UP, DOWN, LEFT, RIGHT, ENTER, IDLE}<]
// #define JOY_ADC_VALUES          {459, 509, 326, 182, 91, 1021}
// #endif

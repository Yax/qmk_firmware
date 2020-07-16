#pragma once

#define RGBLED_NUM 20
#define RGBLIGHT_ANIMATIONS
#define RGBLIGHT_LIMIT_VAL 128

#define WS2812_SPI SPID1 // default: SPID1
#define WS2812_SPI_MOSI_PAL_MODE 5 // Pin "alternate function", see the respective datasheet for the appropriate values for your MCU. default: 5
#define RGB_DI_PIN A7

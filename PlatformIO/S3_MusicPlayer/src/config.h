#pragma once

// #define USE_ES8388_BOARD_V1
// #define USE_ES8388_BOARD_V2_WITH_WAVESHARE
#define USE_ES8388_BOARD_V2_WITH_ESP32S3_PICO

// #define USE_RAW_I2S

#if defined(USE_ES8388_BOARD_V2_WITH_WAVESHARE)
// I2S GPIOs
#define I2S_DOUT     16
#define I2S_BCK      4
#define I2S_WS       10
#define I2S_MCLK     38

// I2C GPIOs
#define IIC_CLK       39
#define IIC_DATA      40

// D0, D1 板子上接反了，小问题
#define SD_MMC_D0  11
#define SD_MMC_D1  13
#define SD_MMC_D2  14
#define SD_MMC_D3  12
#define SD_MMC_CLK 2
#define SD_MMC_CMD 3

#elif defined(USE_ES8388_BOARD_V2_WITH_ESP32S3_PICO)

// I2S GPIOs
#define I2S_DOUT     9
#define I2S_BCK      7
#define I2S_WS       8
#define I2S_MCLK     4

// I2C GPIOs
#define IIC_CLK       5
#define IIC_DATA      6

// D0, D1 板子上接反了，小问题
#define SD_MMC_D0  17
#define SD_MMC_D1  33
#define SD_MMC_D2  34
#define SD_MMC_D3  18
#define SD_MMC_CLK 15
#define SD_MMC_CMD 16

#else
// Use Raw I2S

/* M5Stack Node I2S pins */
#define I2S_BCK 13
#define I2S_WS 12
#define I2S_DOUT 11
// #define I2S_DIN 4
// NO USE MCLK while using UDA1334A
#define I2S_MCLK -1

#define SD_MMC_D0 18
#define SD_MMC_CLK 17
#define SD_MMC_CMD 16

#endif
#pragma once

// #define USE_ES8388_BOARD_V1
// #define USE_ES8388_BOARD_V2_WITH_WAVESHARE
// #define USE_ES8388_BOARD_V2_WITH_ESP32S3_PICO

#define USE_RAW_I2S

#if defined(USE_ES8388_BOARD_V2_WITH_WAVESHARE)

#else
// Use Raw I2S

#define I2S_BCK 15
#define I2S_WS 6
#define I2S_DOUT 7
// #define I2S_DIN 4
// NO USE MCLK while using UDA1334A
#define I2S_MCLK -1


/*
SDMMC, SPI
DAT2, X
CD/DAT3, CS
CMD, MOSI
CLK, SCLK
DAT0, MISO
DAT1, X
*/

// CS : 13
// MOSI: 12
// CLK: 11
// MISO: 10

#define SD_MMC_D0 10
#define SD_MMC_CLK 11
#define SD_MMC_CMD 12


#endif
#pragma once

#define USE_ES8388_BOARD
// #define USE_RAW_I2S

#ifdef USE_ES8388_BOARD
// I2S GPIOs
#define I2S_DOUT     42
#define I2S_BCK      2
#define I2S_WS       4
#define I2S_MCLK     1

// I2C GPIOs
#define IIC_CLK       5
#define IIC_DATA      6

// D0, D1 板子上接反了，小问题
#define SD_MMC_D0  18
#define SD_MMC_D1  17
#define SD_MMC_D2  33
#define SD_MMC_D3  34
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
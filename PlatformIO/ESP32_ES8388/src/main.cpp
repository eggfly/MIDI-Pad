// found this sketch on https://www.mikrocontroller.net/topic/474383
// thanks to Michael U. (amiga)

#include "Arduino.h"
#include "WiFi.h"
#include "SPI.h"
#include "SD.h"
#include "FS.h"
#include "Wire.h"
// #include "AC101.h" //https://github.com/schreibfaul1/AC101
#include "ES8388.h" // https://github.com/maditnerd/es8388
#include "Audio.h"  //https://github.com/schreibfaul1/ESP32-audioI2S

#define ENABLE_WIFI  1
#define USE_SDCARD   0

// SPI GPIOs
#define SD_CS 13
#define SPI_MOSI 15
#define SPI_MISO 2
#define SPI_SCK 14

// I2S GPIOs, the names refer on AC101, AS1 Audio Kit V2.2 2379
// #define I2S_DSIN      25
// #define I2S_BCLK      27
// #define I2S_LRC       26
// #define I2S_MCLK       0
// #define I2S_DOUT      35

// I2S GPIOs, the names refer on ES8388, AS1 Audio Kit V2.2 3378
// #define I2S_DSIN 35
#define I2S_BCLK 5
#define I2S_LRC 25
#define I2S_MCLK 0
#define I2S_DOUT 26

// I2C GPIOs
#define IIC_CLK  23
#define IIC_DATA 18

// buttons
// #define BUTTON_2_PIN 13             // shared mit SPI_CS
#define BUTTON_3_PIN 19
// #define BUTTON_4_PIN 23
#define BUTTON_5_PIN 18 // Stop
#define BUTTON_6_PIN 5  // Play

// amplifier enable
#define GPIO_PA_EN 21

// Switch S1: 1-OFF, 2-ON, 3-ON, 4-OFF, 5-OFF

String ssid = "MIWIFI8";
String password = "12345678";

// static AC101 dac;                                 // AC101
ES8388 dac;      // ES8388 (new board)
int volume = 40; // 0...100

Audio audio;

// #####################################################################

bool listDir(fs::FS &fs, const char *dirname, uint8_t levels)
{
       Serial.printf("Listing directory: %s\n", dirname);

       File root = fs.open(dirname);
       if (!root)
       {
              Serial.println("Failed to open directory");
              return false;
       }
       if (!root.isDirectory())
       {
              Serial.println("Not a directory");
              return false;
       }

       File file = root.openNextFile();
       while (file)
       {
              if (file.isDirectory())
              {
                     Serial.print("  DIR : ");
                     Serial.println(file.name());
                     if (levels)
                     {
                            listDir(fs, file.path(), levels - 1);
                     }
              }
              else
              {
                     Serial.print("  FILE: ");
                     Serial.print(file.name());
                     Serial.print("  SIZE: ");
                     Serial.println(file.size());
              }
              file = root.openNextFile();
       }
       Serial.println("listDir end");
       return true;
}

void setup()
{
       Serial.begin(115200);
       Serial.println("\r\nReset");
       Serial.printf_P(PSTR("Free mem=%d\n"), ESP.getFreeHeap());

       // SD Card initialization
       pinMode(SD_CS, OUTPUT);
       digitalWrite(SD_CS, HIGH);

       // 增加初始化延时
       delay(100);

       SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
       // 降低SPI频率到500kHz
       SPI.setFrequency(500000);

       if (USE_SDCARD)
       {
              // 添加SD卡初始化检查
              if (!SD.begin(SD_CS))
              {
                     Serial.println("SD Card Mount Failed");
                     return;
              }
              Serial.println("SD Card Mounted Successfully");

              // 检查SD卡类型
              uint8_t cardType = SD.cardType();
              if (cardType == CARD_NONE)
              {
                     Serial.println("No SD card attached");
                     return;
              }

              Serial.print("SD Card Type: ");
              if (cardType == CARD_MMC)
              {
                     Serial.println("MMC");
              }
              else if (cardType == CARD_SD)
              {
                     Serial.println("SDSC");
              }
              else if (cardType == CARD_SDHC)
              {
                     Serial.println("SDHC");
              }
              else
              {
                     Serial.println("UNKNOWN");
              }

              uint64_t cardSize = SD.cardSize() / (1024 * 1024);
              Serial.printf("SD Card Size: %lluMB\n", cardSize);

              listDir(SD, "/", 0);
       }

       if (ENABLE_WIFI)
       {
              WiFi.mode(WIFI_STA);
              WiFi.begin(ssid.c_str(), password.c_str());

              while (WiFi.status() != WL_CONNECTED)
              {
                     Serial.print(".");
                     delay(100);
              }

              Serial.printf_P(PSTR("Connected\r\nRSSI: "));
              Serial.print(WiFi.RSSI());
              Serial.print(" IP: ");
              Serial.println(WiFi.localIP());
       }

       Serial.printf("Connect to DAC codec... ");
       while (not dac.begin(IIC_DATA, IIC_CLK))
       {
              Serial.printf("Failed!\n");
              delay(1000);
       }
       Serial.printf("OK\n");

       // dac.SetVolumeSpeaker(volume);
       // dac.SetVolumeHeadphone(volume);
       // ac.DumpRegisters();

       // Enable amplifier
       pinMode(GPIO_PA_EN, OUTPUT);
       digitalWrite(GPIO_PA_EN, HIGH);

       // set I2S_MasterClock
       // audio.i2s_mclk_pin_select(I2S_MCLK);

       audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
       audio.setVolume(6); // 0...21

       // audio.connecttoFS(SD, "/320k_test.mp3");
       // audio.connecttoSD("/ygzjhyl.mp3");
       audio.connecttohost("http://42.193.120.65:8002/%E9%80%83%E8%B7%91%E8%AE%A1%E5%88%92-%E9%98%B3%E5%85%89%E7%85%A7%E8%BF%9B%E5%9B%9E%E5%BF%86%E9%87%8C.mp3");
       //  audio.connecttohost("http://dg-rbb-http-dus-dtag-cdn.cast.addradio.de/rbb/antennebrandenburg/live/mp3/128/stream.mp3");
       //  audio.connecttospeech("Wenn die Hunde schlafen, kann der Wolf gut Schafe stehlen.", "de");
}

//-----------------------------------------------------------------------

void loop()
{
       audio.loop();
}

// optional
void audio_info(const char *info)
{
       Serial.print("info        ");
       Serial.println(info);
}
void audio_id3data(const char *info)
{ // id3 metadata
       Serial.print("id3data     ");
       Serial.println(info);
}
void audio_eof_mp3(const char *info)
{ // end of file
       Serial.print("eof_mp3     ");
       Serial.println(info);
}
void audio_showstation(const char *info)
{
       Serial.print("station     ");
       Serial.println(info);
}
void audio_showstreamtitle(const char *info)
{
       Serial.print("streamtitle ");
       Serial.println(info);
}
void audio_bitrate(const char *info)
{
       Serial.print("bitrate     ");
       Serial.println(info);
}
void audio_commercial(const char *info)
{ // duration in sec
       Serial.print("commercial  ");
       Serial.println(info);
}
void audio_icyurl(const char *info)
{ // homepage
       Serial.print("icyurl      ");
       Serial.println(info);
}
void audio_lasthost(const char *info)
{ // stream URL played
       Serial.print("lasthost    ");
       Serial.println(info);
}
void audio_eof_speech(const char *info)
{
       Serial.print("eof_speech  ");
       Serial.println(info);
}

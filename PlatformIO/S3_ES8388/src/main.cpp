// the pin assignment matches the Olimex ADF board

#include "Arduino.h"
#include "WiFi.h"
#include "SPI.h"
#include "FS.h"
#include "Wire.h"
#include "ES8388.h"  // https://github.com/schreibfaul1/es8388
#include "Audio.h"   // https://github.com/schreibfaul1/ESP32-audioI2S

// I2S GPIOs
#define I2S_SDOUT     42
#define I2S_BCLK      2
#define I2S_LRCK      4
#define I2S_MCLK      1

// I2C GPIOs
#define IIC_CLK       5
#define IIC_DATA      6

// Amplifier enable

char ssid[] =     "MIWIFI8";
char password[] = "12345678";


int volume = 60;                            // 0...100

ES8388 es;
Audio audio;

//----------------------------------------------------------------------------------------------------------------------

void setup()
{
  Serial.begin(115200);
  Serial.println("\r\nReset");

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);
  }

  Serial.printf_P(PSTR("Connected\r\nRSSI: "));
  Serial.print(WiFi.RSSI());
  Serial.print(" IP: ");
  Serial.println(WiFi.localIP());

  Serial.printf("Connect to ES8388 codec... ");
  while (not es.begin(IIC_DATA, IIC_CLK))
  {
    Serial.printf("Failed!\n");
    delay(1000);
  }
  Serial.printf("OK\n");

  es.volume(ES8388::ES_MAIN, volume);
  es.volume(ES8388::ES_OUT1, volume);
  es.volume(ES8388::ES_OUT2, volume);
  es.mute(ES8388::ES_OUT1, false);
  es.mute(ES8388::ES_OUT2, false);
  es.mute(ES8388::ES_MAIN, false);

  // Enable amplifier

  audio.setPinout(I2S_BCLK, I2S_LRCK, I2S_SDOUT, I2S_MCLK);
  audio.setVolume(21); // 0...21

  audio.connecttohost("http://42.193.120.65:8002/%E9%80%83%E8%B7%91%E8%AE%A1%E5%88%92-%E9%98%B3%E5%85%89%E7%85%A7%E8%BF%9B%E5%9B%9E%E5%BF%86%E9%87%8C.mp3");
  //  audio.connecttoFS(SD, "320k_test.mp3");
  //  audio.connecttospeech("Wenn die Hunde schlafen, kann der Wolf gut Schafe stehlen.", "de");
}
//----------------------------------------------------------------------------------------------------------------------
void loop()
{
  audio.loop();
}
//----------------------------------------------------------------------------------------------------------------------

// optional
void audio_info(const char *info) {
  Serial.print("info        "); Serial.println(info);
}
void audio_id3data(const char *info) { //id3 metadata
  Serial.print("id3data     "); Serial.println(info);
}
void audio_eof_mp3(const char *info) { //end of file
  Serial.print("eof_mp3     "); Serial.println(info);
}
void audio_showstation(const char *info) {
  Serial.print("station     "); Serial.println(info);
}
void audio_showstreaminfo(const char *info) {
  Serial.print("streaminfo  "); Serial.println(info);
}
void audio_showstreamtitle(const char *info) {
  Serial.print("streamtitle "); Serial.println(info);
}
void audio_bitrate(const char *info) {
  Serial.print("bitrate     "); Serial.println(info);
}
void audio_commercial(const char *info) { //duration in sec
  Serial.print("commercial  "); Serial.println(info);
}
void audio_icyurl(const char *info) { //homepage
  Serial.print("icyurl      "); Serial.println(info);
}
void audio_lasthost(const char *info) { //stream URL played
  Serial.print("lasthost    "); Serial.println(info);
}
void audio_eof_speech(const char *info) {
  Serial.print("eof_speech  "); Serial.println(info);
}

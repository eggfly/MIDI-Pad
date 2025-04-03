#include <Arduino.h>

#include <Audio.h> /* https://github.com/schreibfaul1/ESP32-audioI2S */

/* M5Stack Node I2S pins */
#define I2S_BCK 4
#define I2S_WS 2
#define I2S_DOUT 1
// #define I2S_DIN 4
#define I2S_MCLK 5

Audio audio;

void setup()
{
       delay(2000);
       Serial.begin(115200);
       Serial.println("PCM5102A I2S DAC test");
       /* set the i2s pins */
       audio.setPinout(I2S_BCK, I2S_WS, I2S_DOUT, I2S_MCLK);

       WiFi.begin("MIWIFI8", "12345678");
       while (!WiFi.isConnected())
       {
              delay(10);
       }
       log_i("Connected. Starting MP3...");
       audio.connecttohost("http://42.193.120.65:8002/ygzjhyl.mp3");
       audio.setVolume(8);
       auto vol = audio.getVolume();
       Serial.println(vol);
}

void loop()
{
       audio.loop();
}

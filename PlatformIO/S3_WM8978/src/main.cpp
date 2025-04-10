#include <WM8978.h> /* https://github.com/CelliesProjects/wm8978-esp32 */
#include <Audio.h>  /* https://github.com/schreibfaul1/ESP32-audioI2S */

/* M5Stack Node WM8978 I2C pins */
#define I2C_SDA 17
#define I2C_SCL 18

/* M5Stack Node I2S pins */
#define I2S_BCK 5
#define I2S_WS 13
#define I2S_DOUT 2
#define I2S_DIN 4
#define I2S_MCLK 0

Audio audio;
WM8978 dac;

void setup()
{
       delay(1000);
       Serial.begin(115200);
       /* Setup wm8978 I2C interface */
       if (!dac.begin(I2C_SDA, I2C_SCL))
       {
              log_e("Error setting up dac. System halted");
              while (1)
                     delay(100);
       }
       
       /* set the i2s pins */
       audio.setPinout(I2S_BCK, I2S_WS, I2S_DOUT, I2S_MCLK);

       WiFi.begin("MIWIFI8", "12345678");
       while (!WiFi.isConnected())
       {
              delay(10);
       }
       log_i("Connected. Starting MP3...");
       audio.connecttohost("http://42.193.120.65:8002/ygzjhyl.mp3");
       auto vol = audio.getVolume();
       Serial.println(vol);
       dac.setHPvol(30, 30);
       // audio.setVolume(20, 1); // 0-100

       dac.setSPKvol(40); /* max 63 */
       // dac.setHPvol(32, 32);
}

void loop()
{
       audio.loop();
}

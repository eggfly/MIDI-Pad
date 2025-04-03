#include <Arduino.h>

#include <Audio.h> /* https://github.com/schreibfaul1/ESP32-audioI2S */

#include <SparkFun_WM8960_Arduino_Library.h>

WM8960 codec;

/* M5Stack Node WM8978 I2C pins */
#define I2C_SDA 17
#define I2C_SCL 18

/* M5Stack Node I2S pins */
#define I2S_BCK 4
#define I2S_WS 2
#define I2S_DOUT 1
// #define I2S_DIN 4
#define I2S_MCLK 5

Audio audio;

void setup()
{
       delay(3000);
       Serial.begin(115200);

       Wire.begin(I2C_SDA, I2C_SCL);

       if (codec.begin(Wire) == false) // Begin communication over I2C
       {
              Serial.println("The device did not respond. Please check wiring.");
              while (1)
                     ; // Freeze
       }
       Serial.println("Device is connected properly.");

       // General setup needed
       codec.enableVREF();
       codec.enableVMID();

       // codec.enablePLL();
       // codec.setPLLPRESCALE(0);
       // codec.setPLLN(2);
       // codev

       // Setup signal flow through the analog audio bypass connections

       // Enable left output mixer
       codec.enableLOMIX();

       // Enable bypass connection from Left INPUT3 to Left output mixer, note, the
       // default gain on this input (LI2LOVOL) is -15dB
       codec.enableLI2LO();

       // Sets volume control between "left input" to "left output mixer"
       codec.setLI2LOVOL(WM8960_OUTPUT_MIXER_GAIN_0DB);

       codec.enableROMIX(); // Now for the right channel of INPUT3
       codec.enableRI2RO();
       codec.setRI2ROVOL(WM8960_OUTPUT_MIXER_GAIN_0DB);

       codec.enableHeadphones();
       codec.enableOUT3MIX(); // Provides VMID as buffer for headphone ground

       // Serial.println("Volume set to +6.00dB (max)");
       // codec.setHeadphoneVolumeDB(6.00);
       // delay(5000);

       // Serial.println("Volume set to +0.00dB");
       // codec.setHeadphoneVolumeDB(0.00);
       // delay(1000);

       Serial.println("Volume set to -6.00dB");
       codec.setHeadphoneVolumeDB(-6.00);
       delay(1000);

       // Serial.println("Volume set to -12.00dB");
       // codec.setHeadphoneVolumeDB(-12.00);
       // delay(1000);

       // Serial.println("Volume set to -74.00dB, aka MUTE");
       // codec.setHeadphoneVolumeDB(-74.00);
       // delay(5000);

       Serial.println("Example complete. Hit Reset to try again.");

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
}

void loop()
{
       audio.loop();
}

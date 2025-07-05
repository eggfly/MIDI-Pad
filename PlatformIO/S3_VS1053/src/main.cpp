#include <Arduino.h>
#include "config.h"

#include <VS1053Driver.h>

// Default volume
#define VOLUME 78

VS1053 player(VS1053_CS, VS1053_DCS, VS1053_DREQ, VS1053_RST, SPI);
WiFiClient client;

// WiFi settings example, substitute your own
const char *ssid = "MIWIFI8";
const char *password = "12345678";

//  http://comet.shoutca.st:8563/1
// http://42.193.120.65:8002/520AM.mp3
// http://42.193.120.65:8002/%E9%80%83%E8%B7%91%E8%AE%A1%E5%88%92-%E9%98%B3%E5%85%89%E7%85%A7%E8%BF%9B%E5%9B%9E%E5%BF%86%E9%87%8C.mp3
// http://42.193.120.65:8002/%E9%A9%AC%E8%B5%9B%E5%85%8B-%E9%9C%93%E8%99%B9%E7%94%9C%E5%BF%83.flac
// http://42.193.120.65:8002/1%20-%20Hotel%20California.mp3
// http://42.193.120.65:8002/%E8%BE%BE%E8%BE%BE-Song%20F.mp3
// http://42.193.120.65:8002/%E4%B8%80%E7%9B%B4%E5%BE%88%E5%AE%89%E9%9D%99-%E9%98%BF%E6%A1%91.mp3
// http://42.193.120.65:8002/G.E.M.%E9%82%93%E7%B4%AB%E6%A3%8B-%E5%85%89%E5%B9%B4%E4%B9%8B%E5%A4%96.wav
// http://42.193.120.65:8002/%E8%8F%B2%E8%8F%B2%E5%85%AC%E4%B8%BB-%E7%AC%AC57%E6%AC%A1%E5%8F%96%E6%B6%88%E5%8F%91%E9%80%81.mp3
// http://42.193.120.65:8002/%E8%94%A1%E7%90%B4%20-%20%E6%B8%A1%E5%8F%A3.mp3
// http://42.193.120.65:8002/%E9%A9%AC%E8%B5%9B%E5%85%8B-%E9%9C%93%E8%99%B9%E7%94%9C%E5%BF%83.mp3
const char *host = "42.193.120.65";
const char * defaultPath = "/1%20-%20Hotel%20California.mp3";
char path[512];
int httpPort = 8002;

// The buffer size 64 seems to be optimal. At 32 and 128 the sound might be brassy.
uint8_t mp3buff[64];

void setup()
{
    Serial.begin(115200);
    strncpy(path, defaultPath, strlen(defaultPath) + 1);

    // Wait for VS1053 and PAM8403 to power up
    // otherwise the system might not start up correctly
    delay(3000);

    // This can be set in the IDE no need for ext library
    // system_update_cpu_freq(160);

    Serial.println("\n\nSimple Radio Node WiFi Radio");

    SPI.begin(VS1053_SPI_SCK, VS1053_SPI_MISO, VS1053_SPI_MOSI);
    // SPI.setFrequency(1000000); // Set SPI frequency to 20MHz
    player.beginOutput();
    auto version = player.getChipVersion();
    Serial.printf("VS1053 Chip Version: %d\n", version);
    player.setVolume(VOLUME);
    auto bassVal = player.bass();
    Serial.printf("Default Bass: %d\n", bassVal);
    player.setBass(100);
    auto newBassVal = player.bass();
    Serial.printf("New Bass: %d\n", newBassVal);
    Serial.print("Connecting to SSID ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    Serial.print("connecting to ");
    Serial.println(host);

    if (!client.connect(host, httpPort))
    {
        Serial.println("Connection failed");
        return;
    }

    Serial.print("Requesting stream: ");
    Serial.println(path);

    client.print(String("GET ") + path + " HTTP/1.1\r\n" +
                 "Host: " + host + "\r\n" +
                 "Connection: close\r\n\r\n");
}

bool mode = false;
bool newUrl = false;

void parseSerialCommand()
{
       if (Serial.available())
       {
              Serial.setTimeout(50);
              String r = Serial.readStringUntil('\n');
              r.trim();
              if (r.length() > 5)
              {
                auto newPath = r.c_str();
                strncpy(path, newPath, strlen(newPath) + 1);
                // path = r.c_str();
                newUrl = true;
              }
              // log_i("free heap=%i", ESP.getFreeHeap());
       }
}

void loop()
{
    parseSerialCommand();
    if (!client.connected() || newUrl)
    {
        client.clear();
        newUrl = false;
        client.stop();
        Serial.printf("Reconnecting to %s\n", path);
        if (client.connect(host, httpPort))
        {
            client.print(String("GET ") + path + " HTTP/1.1\r\n" +
                         "Host: " + host + "\r\n" +
                         "Connection: close\r\n\r\n");
            player.beginOutput();
        }
    }

    if (client.available() > 0)
    {
        // The buffer size 64 seems to be optimal. At 32 and 128 the sound might be brassy.
        uint8_t bytesread = client.read(mp3buff, 64);
        player.playChunk(mp3buff, bytesread);
    }

    // if (millis() % 10000 > 5000)
    // {
    //     if (!mode)
    //     {
    //         // player.setEarSpeaker(VS1053_EARSPEAKER_MAX);
    //         player.setTreble(100);
    //         // player.setTrebleFrequencyLimit(5000);
    //         Serial.println("ON");
    //         mode = true;
    //     }
    // }
    // else
    // {
    //     if (mode)
    //     {
    //         // player.setEarSpeaker(VS1053_EARSPEAKER_OFF);
    //         player.setTreble(0);
    //         Serial.println("OFF");
    //         mode = false;
    //     }
    // }
}

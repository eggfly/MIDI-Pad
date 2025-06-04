#include <Arduino.h>
#include "config.h"

#include <SD_MMC.h>
#include <FS.h>
#include <vector>
#include <unordered_set>

#include <Audio.h>  /* https://github.com/schreibfaul1/ESP32-audioI2S */
#include "ES8388.h" // https://github.com/schreibfaul1/es8388

#define MY_SD SD_MMC
#define ENABLE_WIFI 0

#ifdef USE_ES8388_BOARD
ES8388 es;
#endif

const bool APP_DEBUG = false;

Audio audio;

bool shuffle_mode = true;

std::vector<String> m_songFiles{};
int m_activeSongIdx{-1};

int strncmpci(const char *str1, const char *str2, size_t num)
{
       int ret_code = 0;
       size_t chars_compared = 0;

       if (!str1 || !str2)
       {
              ret_code = INT_MIN;
              return ret_code;
       }

       while ((chars_compared < num) && (*str1 || *str2))
       {
              ret_code = tolower((int)(*str1)) - tolower((int)(*str2));
              if (ret_code != 0)
              {
                     break;
              }
              chars_compared++;
              str1++;
              str2++;
       }

       return ret_code;
}

bool startsWithIgnoreCase(const char *pre, const char *str)
{
       return strncmpci(pre, str, strlen(pre)) == 0;
}

bool endsWithIgnoreCase(const char *base, const char *str)
{
       int blen = strlen(base);
       int slen = strlen(str);
       return (blen >= slen) && (0 == strncmpci(base + blen - slen, str, strlen(str)));
}

void startNextSong(bool isNextOrPrev);

void autoPlayNextSong()
{
       if (m_songFiles.size() == 0)
       {
              delay(100);
              return;
       }
       if (!audio.isRunning())
       {
              Serial.println("autoPlay: playNextSong()");
              startNextSong(true);
       }
}

std::unordered_set<int> m_played_songs{};

void startNextSong(bool isNextOrPrev)
{
       if (m_songFiles.size() == 0)
       {
              return;
       }
       m_played_songs.insert(m_activeSongIdx);
       if (m_played_songs.size() * 2 > m_songFiles.size())
       {
              Serial.println("re-shuffle.");
              m_played_songs.clear();
       }
       if (isNextOrPrev)
       {
              m_activeSongIdx++;
       }
       else
       {
              m_activeSongIdx--;
       }
       if (shuffle_mode)
       {
              do
              {
                     m_activeSongIdx = random(m_songFiles.size());
              } while (m_played_songs.find(m_activeSongIdx) != std::end(m_played_songs));
       }
       //  if (m_activeSongIdx >= m_songFiles.size() || m_activeSongIdx < 0) {
       //    m_activeSongIdx = 0;
       //  }
       m_activeSongIdx %= m_songFiles.size();
       Serial.print("songIndex=");
       Serial.print(m_activeSongIdx);
       Serial.print(", total=");
       Serial.println(m_songFiles.size());

       if (audio.isRunning())
       {
              audio.stopSong();
       }
       // walkaround
       // setupButtonsNew();
       audio.connecttoFS(MY_SD, m_songFiles[m_activeSongIdx].c_str());

       Serial.println(m_songFiles[m_activeSongIdx].c_str());
}

void populateMusicFileList(String path, size_t depth)
{
       Serial.printf("search: %s, depth=%d\n", path.c_str(), depth);
       File musicDir = MY_SD.open(path);
       bool nextFileFound;
       do
       {
              nextFileFound = false;
              File entry = musicDir.openNextFile();
              if (entry)
              {
                     nextFileFound = true;
                     if (entry.isDirectory())
                     {
                            if (depth)
                            {
                                   populateMusicFileList(entry.path(), depth - 1);
                            }
                     }
                     else
                     {
                            const bool entryIsFile = entry.size() > 4096;
                            if (entryIsFile)
                            {
                                   if (APP_DEBUG)
                                   {
                                          Serial.print(entry.path());
                                          Serial.print(" size=");
                                          Serial.println(entry.size());
                                   }
                                   if (endsWithIgnoreCase(entry.name(), ".mp3") || endsWithIgnoreCase(entry.name(), ".flac") || endsWithIgnoreCase(entry.name(), ".aac") || endsWithIgnoreCase(entry.name(), ".wav"))
                                   {
                                          m_songFiles.push_back(entry.path());
                                   }
                            }
                     }
                     entry.close();
              }
       } while (nextFileFound);
}

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
       // pinMode(I2S_WS, OUTPUT);
       // pinMode(4, OUTPUT);
       pinMode(0, INPUT_PULLUP);
       // digitalWrite(4, LOW);
       delay(100);
       Serial.begin(115200);

#ifdef USE_ES8388_BOARD

       Serial.printf("Connect to ES8388 codec... ");
       while (not es.begin(IIC_DATA, IIC_CLK))
       {
              Serial.printf("Failed!\n");
              delay(1000);
       }
       Serial.printf("OK\n");
       int volume = 78; // 0...100

       es.volume(ES8388::ES_MAIN, volume);
       es.volume(ES8388::ES_OUT1, volume);
       es.volume(ES8388::ES_OUT2, volume);
       es.mute(ES8388::ES_OUT1, false);
       es.mute(ES8388::ES_OUT2, false);
       es.mute(ES8388::ES_MAIN, false);

#endif

       Serial.println("I2S DAC test");
       Serial.printf("PSRAM大小: %d字节\n", ESP.getPsramSize());

       // pinMode(SD_MMC_D0, INPUT_PULLUP);
       SD_MMC.setPins(SD_MMC_CLK, SD_MMC_CMD, SD_MMC_D0, SD_MMC_D1, SD_MMC_D2, SD_MMC_D3);
       // SD_MMC.setPins(SD_MMC_CLK, SD_MMC_CMD, SD_MMC_D0);
       if (!SD_MMC.begin("/sdmmc", false, false, 80000))
       {
              Serial.println("Card Mount Failed");
              return;
       }
       auto populateStart = millis();
       populateMusicFileList("/", 3);
       auto cost = millis() - populateStart;
       Serial.printf("populateMusicFileList cost %d ms, found %d songs\n", cost, m_songFiles.size());

       /* set the i2s pins */
       audio.setPinout(I2S_BCK, I2S_WS, I2S_DOUT, I2S_MCLK);
       if (ENABLE_WIFI)
       {
              WiFi.begin("MIWIFI8", "********");
              while (!WiFi.isConnected())
              {
                     delay(10);
              }
       }
       log_i("Connected. Starting MP3...");
       // audio.connecttohost("http://42.193.120.65:8002/%E9%80%83%E8%B7%91%E8%AE%A1%E5%88%92-%E9%98%B3%E5%85%89%E7%85%A7%E8%BF%9B%E5%9B%9E%E5%BF%86%E9%87%8C.mp3");
       // audio.setVolume(6);
       auto vol = audio.getVolume();
       Serial.println(vol);
}

void loop()
{
       vTaskDelay(1);
       if (digitalRead(0) == LOW)
       {
              while (digitalRead(0) == LOW)
              {
                     delay(1);
              }
              Serial.println("play next song");
              startNextSong(true);
       }
       audio.loop();
       if (Serial.available())
       {
              // put streamURL in serial monitor
              audio.stopSong();
              String r = Serial.readString();
              r.trim();
              if (r.length() > 5)
                     audio.connecttohost(r.c_str());
              log_i("free heap=%i", ESP.getFreeHeap());
       }
       autoPlayNextSong();
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

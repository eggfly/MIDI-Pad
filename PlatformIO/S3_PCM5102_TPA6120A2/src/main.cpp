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

#if !defined(USE_RAW_I2S)
ES8388 es;
#endif

#define LIST_DIR_RECURSION_DEPTH 5

const bool APP_DEBUG = false;

Audio audio;

bool shuffle_mode = true;

std::vector<String> m_songFiles{};
int m_activeSongIdx{-1};

// 0.5 代表随机去掉一半, 1.0 代表全都去掉, 越大去掉越多
float randomSkipRatio = 0.5;
size_t MaxSongsCount = 200; // max songs to keep in memory, if more than this, will not add to m_songFiles

size_t totalSongs = 0;

void stopSongWithMute()
{
       // es.mute(ES8388::ES_MAIN, true);
       // es.mute(ES8388::ES_OUT1, true);
       // es.mute(ES8388::ES_OUT2, true);
       audio.stopSong();
}

void unmute()
{
       // es.mute(ES8388::ES_MAIN, false);
       // es.mute(ES8388::ES_OUT1, false);
       // es.mute(ES8388::ES_OUT2, false);
}

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
              stopSongWithMute();
              Serial.println("stop song");
              delay(2000);
              Serial.println("start next song");
       }
       // walkaround
       // setupButtonsNew();
       audio.connecttoFS(MY_SD, m_songFiles[m_activeSongIdx].c_str());

       Serial.println(m_songFiles[m_activeSongIdx].c_str());
}

void populateMusicFileList(String path, size_t depth)
{
       Serial.printf("search: %s, depth=%d\n", path.c_str(), LIST_DIR_RECURSION_DEPTH - depth);
       File musicDir = MY_SD.open(path);
       bool nextFileFound;
       do
       {
              nextFileFound = false;
              File entry = musicDir.openNextFile();
              if (entry)
              {
                     nextFileFound = true;
                     if (!entry.name() || entry.name()[0] == '.')
                     {
                            continue;
                     }
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
                                          totalSongs++;
                                          long r = random(10000);
                                          bool keep = r > 10000 * randomSkipRatio;
                                          if (keep)
                                          {
                                                 m_songFiles.push_back(entry.path());
                                          }
                                   }
                            }
                     }
                     entry.close();
              }
       } while (nextFileFound && m_songFiles.size() < MaxSongsCount);
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

int volume = 9; // 0...100


void setup()
{
       // pinMode(I2S_WS, OUTPUT);
       // pinMode(4, OUTPUT);
       pinMode(0, INPUT_PULLUP);
       // digitalWrite(4, LOW);
       delay(100);
       Serial.begin(115200);

#if !defined(USE_RAW_I2S)

       Serial.printf("Connect to ES8388 codec...\n");
       while (not es.begin(IIC_DATA, IIC_CLK, 100 * 1000))
       {
              Serial.printf("Failed!\n");
              delay(1000);
       }
       Serial.printf("ES8388 Ready OK!!\n");

       delay(100);

       es.volume(ES8388::ES_MAIN, volume);
       es.volume(ES8388::ES_OUT1, 100);
       es.volume(ES8388::ES_OUT2, 100);
       es.mute(ES8388::ES_OUT1, true);
       es.mute(ES8388::ES_OUT2, true);
       es.mute(ES8388::ES_MAIN, true);

#endif

       Serial.println("I2S DAC test");
       Serial.printf("PSRAM大小: %d字节\n", ESP.getPsramSize());

       // pinMode(SD_MMC_D0, INPUT_PULLUP);
       // SD_MMC.setPins(SD_MMC_CLK, SD_MMC_CMD, SD_MMC_D0, SD_MMC_D1, SD_MMC_D2, SD_MMC_D3);
       SD_MMC.setPins(SD_MMC_CLK, SD_MMC_CMD, SD_MMC_D0);
       if (!SD_MMC.begin("/sdmmc", true, false, 80000))
       {
              Serial.println("Card Mount Failed");
              return;
       }
       auto populateStart = millis();

       Serial.printf("PSRAM剩余大小: %d字节\n", ESP.getFreePsram());
       populateMusicFileList("/", LIST_DIR_RECURSION_DEPTH);
       Serial.printf("PSRAM剩余大小: %d字节\n", ESP.getFreePsram());
       auto cost = millis() - populateStart;
       Serial.printf("populateMusicFileList cost %d ms, keep %d songs, total %d songs\n", cost, m_songFiles.size(), totalSongs);

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
       audio.setVolume(volume);
       auto vol = audio.getVolume();
       Serial.println(vol);
}

void parseSerialCommand()
{
       if (Serial.available())
       {
              Serial.setTimeout(50);
              String r = Serial.readStringUntil('\n');
              r.trim();
              if (r.equalsIgnoreCase("n"))
              {
                     Serial.println("play next song");
                     startNextSong(true);
              }
              else if (r.equalsIgnoreCase("p"))
              {
                     Serial.println("play previous song");
                     startNextSong(false);
              }
              else if (r.equalsIgnoreCase("r"))
              {
                     // toggle random shuffle mode
                     shuffle_mode = !shuffle_mode;
                     Serial.printf("shuffle mode: %s\n", shuffle_mode ? "on" : "off");
              }
              else if (r.equalsIgnoreCase("s"))
              {
                     stopSongWithMute();
                     Serial.println("stop song");
              }
              else if (r.equalsIgnoreCase("p"))
              {
                     audio.pauseResume();
                     Serial.println("pause/resume song");
              }
              else if (r.equalsIgnoreCase("+") || r.equalsIgnoreCase("="))
              {
                     volume += 1;
                     if (volume > 21)
                     {
                            volume = 21;
                     }
                     if (volume > 0)
                     {
                            unmute();
                     }
                     audio.setVolume(volume);
                     // es.volume(ES8388::ES_MAIN, volume);
                     Serial.printf("volume up: %d\n", volume);
              }
              else if (r.equalsIgnoreCase("-"))
              {
                     volume -= 1;
                     if (volume < 0)
                     {
                            volume = 0;
                     }
                     if (volume == 0)
                     {
                            // es.mute(ES8388::ES_MAIN, true);
                            // es.mute(ES8388::ES_OUT1, true);
                            // es.mute(ES8388::ES_OUT2, true);
                     }
                     audio.setVolume(volume);
                     // es.volume(ES8388::ES_MAIN, volume);
                     Serial.printf("volume down: %d\n", volume);
              }
              else if (r.equalsIgnoreCase("info"))
              {
                     Serial.println("Audio info:");
                     Serial.printf("  codec: %s\n", audio.getCodecname());
                     Serial.printf("  sample rate: %d\n", audio.getSampleRate());
                     Serial.printf("  bits per sample: %d\n", audio.getBitsPerSample());
                     Serial.printf("  channels: %d\n", audio.getChannels());
                     Serial.printf("  bitrate: %d\n", audio.getBitRate());
                     Serial.printf("  file size: %d\n", audio.getFileSize());
                     Serial.printf("  file pos: %d\n", audio.getFilePos());
                     Serial.printf("  file duration: %d sec\n", audio.getAudioFileDuration());
              }
              else if (r.equalsIgnoreCase("free"))
              {
                     Serial.printf("free heap=%i, free psram=%i\n", ESP.getFreeHeap(), ESP.getFreePsram());
              }
              else if (r.equalsIgnoreCase("list"))
              {
                     Serial.println("list songs:");
                     for (int i = 0; i < m_songFiles.size(); i++)
                     {
                            Serial.printf("%d: %s\n", i, m_songFiles[i].c_str());
                     }
              }
              else if (r.length() > 5)
              {
                     // put streamURL in serial monitor
                     stopSongWithMute();
                     audio.connecttoFS(MY_SD, r.c_str());
              }
              // log_i("free heap=%i", ESP.getFreeHeap());
       }
}

void handleButton()
{
       if (digitalRead(0) == LOW)
       {
              while (digitalRead(0) == LOW)
              {
                     delay(1);
              }
              Serial.println("play next song");
              startNextSong(true);
       }
}

void loop()
{
       delayMicroseconds(1);
       handleButton();
       audio.loop();
       auto start = millis();
       parseSerialCommand();
       auto cost = millis() - start;
       if (cost > 500)
       {
              Serial.printf("%d ms\n", cost);
       }
       autoPlayNextSong();
}

// optional
void audio_info(const char *info)
{
       // if info starts with BitRate
       if (strncmp(info, "BitRate", 7) == 0)
       {
              unmute();
       }
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

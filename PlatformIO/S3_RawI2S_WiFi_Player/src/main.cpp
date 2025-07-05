#include <Arduino.h>
#include <WiFi.h>

#include "Audio.h" //see my repository at github "https://github.com/schreibfaul1/ESP32-audioI2S"

#include "config.h"

Audio audio;

const char *ssid = "MIWIFI8";
const char *password = "12345678";

//     const size_t    m_frameSizeFLAC   = 4096 * 4 + 92; or 37779(不行)
String stations[] = {
    // "http://42.193.120.65:8002/%E6%96%B0%E8%A3%A4%E5%AD%90/%E4%BD%A0%E8%A6%81%E8%B7%B3%E8%88%9E%E5%90%97.flac"
    // "http://42.193.120.65:8002/%E4%B8%87%E8%83%BD%E9%9D%92%E5%B9%B4%E6%97%85%E5%BA%97/04.%20%E5%A4%A7%E7%9F%B3%E7%A2%8E%E8%83%B8%E5%8F%A3.flac", // 万能青年旅店 - FLAC格式, 24bit WRONG!
    "http://42.193.120.65:8002/%E9%80%83%E8%B7%91%E8%AE%A1%E5%88%92-%E9%98%B3%E5%85%89%E7%85%A7%E8%BF%9B%E5%9B%9E%E5%BF%86%E9%87%8C.mp3", // 逃跑计划 - MP3格式
    "http://42.193.120.65:8002/%E9%A9%AC%E8%B5%9B%E5%85%8B-%E9%9C%93%E8%99%B9%E7%94%9C%E5%BF%83.flac",                                    // 马赛克 - FLAC格式
    "http://42.193.120.65:8002/%E8%94%A1%E7%90%B4%20-%20%E6%B8%A1%E5%8F%A3.mp3",                                                          // 蔡琴 - MP3格式
    "https://www.soundhelix.com/examples/mp3/SoundHelix-Song-1.mp3",                                                                      // 测试MP3流
    "https://www.soundhelix.com/examples/mp3/SoundHelix-Song-2.mp3",                                                                      // 测试MP3流
    "www.surfmusic.de/m3u/100-5-das-hitradio,4529.m3u",
    "stream.1a-webradio.de/deutsch/mp3-128/vtuner-1a",
    "mp3.ffh.de/radioffh/hqlivestream.aac", //  128k aac
    "www.antenne.de/webradio/antenne.m3u",
    "listen.rusongs.ru/ru-mp3-128",
    "edge.audio.3qsdn.com/senderkw-mp3",
    "https://stream.srg-ssr.ch/rsp/aacp_48.asx", // SWISS POP
};

uint8_t cur_station = 0; // current station(nr), will be set later
uint8_t volume = 6;  // will be set from stored preferences

void setup()
{
       pinMode(I2S_BCLK, OUTPUT);
       pinMode(I2S_WS, OUTPUT);
       pinMode(I2S_DOUT, OUTPUT);
       digitalWrite(I2S_BCLK, HIGH);
       digitalWrite(I2S_WS, LOW);
       digitalWrite(I2S_DOUT, LOW);
       Serial.begin(115200);
       Serial.println();
       Serial.print("[WiFi] Connecting to ");
       Serial.println(ssid);

       WiFi.begin(ssid, password);
       WiFi.setSleep(false); // this code solves problem?

       // 等待WiFi连接，添加超时和状态检查
       int tryDelay = 500;
       int numberOfTries = 40; // 20秒超时

       while (numberOfTries > 0)
       {
              switch (WiFi.status())
              {
              case WL_NO_SSID_AVAIL:
                     Serial.println("[WiFi] SSID not found");
                     break;
              case WL_CONNECT_FAILED:
                     Serial.println("[WiFi] Failed - WiFi not connected!");
                     break;
              case WL_CONNECTION_LOST:
                     Serial.println("[WiFi] Connection was lost");
                     break;
              case WL_DISCONNECTED:
                     Serial.println("[WiFi] WiFi is disconnected");
                     break;
              case WL_CONNECTED:
                     Serial.println("[WiFi] WiFi is connected!");
                     Serial.print("[WiFi] IP address: ");
                     Serial.println(WiFi.localIP());
                     goto wifi_connected;
                     break;
              default:
                     Serial.print("[WiFi] WiFi Status: ");
                     Serial.println(WiFi.status());
                     break;
              }
              delay(tryDelay);
              numberOfTries--;
       }

       Serial.println("[WiFi] Failed to connect to WiFi!");
       return;

wifi_connected:
       log_i("Connected to %s", WiFi.SSID().c_str());
       audio.setPinout(I2S_BCLK, I2S_WS, I2S_DOUT);

       // 增加FLAC支持 - 设置更大的缓冲区大小
       // audio.setBufsize(16384, 16384); // 为FLAC文件设置更大的缓冲区

       audio.setVolume(volume); // 0...21
       Serial.printf("next station %d: %s\n", cur_station, stations[cur_station].c_str());
       audio.connecttohost(stations[cur_station].c_str());
       // audio.setVolume(0);
}


void stopSongWithMute()
{
       audio.stopSong();
       // audio.setVolume(0);
}

void unmute()
{
       audio.setVolume(volume);
}

void startNextSong(bool isNextOrPrev)
{
       if (audio.isRunning())
       {
              stopSongWithMute();
              Serial.println("stop song");
              delay(2000);
              Serial.println("start next song");
       }
       // walkaround
       // setupButtonsNew();
       cur_station = (cur_station + (isNextOrPrev ? 1 : -1)) % (sizeof(stations) / sizeof(stations[0]));
       audio.connecttohost(stations[cur_station].c_str());
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
              // else if (r.equalsIgnoreCase("p"))
              // {
              //        Serial.println("play previous song");
              //        startNextSong(false);
              // }
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
              else if (r.length() > 5)
              {
                     // put streamURL in serial monitor
                     stopSongWithMute();
                     Serial.printf("play stream: %s\n", r.c_str());
                     audio.connecttohost(r.c_str());
              }
              // log_i("free heap=%i", ESP.getFreeHeap());
       }
}

// // 智能音频格式检测和缓冲区调整
// void adjustBufferForAudioFormat(const String& url) {
//     if (url.indexOf(".flac") != -1) {
//         // FLAC文件需要更大的缓冲区
//         audio.setBufsize(16384, 16384);
//         Serial.println("检测到FLAC文件，调整缓冲区大小为16384字节");
//     } else if (url.indexOf(".mp3") != -1 || url.indexOf(".aac") != -1) {
//         // MP3和AAC文件使用默认缓冲区
//         audio.setBufsize(1600, 1600);
//         Serial.println("检测到MP3/AAC文件，使用默认缓冲区大小");
//     } else {
//         // 其他格式使用中等缓冲区
//         audio.setBufsize(8192, 8192);
//         Serial.println("未知格式，使用中等缓冲区大小");
//     }
// }

// // 显示当前音频格式信息
// void showCurrentAudioInfo() {
//     Serial.print("当前播放: ");
//     Serial.print(stations[cur_station]);
//     Serial.print(" (");

//     String url = stations[cur_station];
//     if (url.indexOf(".flac") != -1) {
//         Serial.print("FLAC格式");
//     } else if (url.indexOf(".mp3") != -1) {
//         Serial.print("MP3格式");
//     } else if (url.indexOf(".aac") != -1) {
//         Serial.print("AAC格式");
//     } else {
//         Serial.print("未知格式");
//     }
//     Serial.println(")");
// }

void autoPlayNextSong()
{
       auto station_size = sizeof(stations) / sizeof(stations[0]);
       if (station_size == 0)
       {
              delay(100);
              return;
       }
       if (!audio.isRunning())
       {
              Serial.println("autoPlay: playNextSong()");
              cur_station = (cur_station + 1) % station_size;
              // showCurrentAudioInfo(); // 显示当前音频信息
              // adjustBufferForAudioFormat(stations[cur_station]);
              Serial.printf("autoPlay: next station %d: %s\n", cur_station, stations[cur_station].c_str());
              audio.connecttohost(stations[cur_station].c_str());
       }
}

void loop()
{
       audio.loop();
       parseSerialCommand();
       // // 检测FLAC帧大小错误并自动恢复
       // if (strstr(info, "FLAC maxFrameSize too large") != nullptr) {
       //        Serial.println("检测到FLAC帧大小错误，尝试增加缓冲区大小...");
       //        // 尝试增加缓冲区大小
       //        audio.setBufsize(32768, 32768); // 进一步增加缓冲区
       //        Serial.println("已增加缓冲区大小到32768字节，请重新尝试播放");
       //        // 延迟一下再重试
       //        delay(1000);
       //        autoPlayNextSong();
       // }

       // // 检测其他FLAC相关错误
       // if (strstr(info, "FLAC") != nullptr && strstr(info, "error") != nullptr) {
       //        Serial.println("检测到FLAC错误，尝试切换到下一个音频流");
       //        delay(500);
       //        autoPlayNextSong();
       // }
}
void audio_showstation(const char *info)
{
       Serial.print("audio_showstation: ");
       Serial.println(info);
}
void audio_showstationurl(const char *info)
{
       Serial.print("audio_showstationurl: ");
       Serial.println(info);
}
void audio_showstationartist(const char *info)
{
       Serial.print("audio_showstationartist: ");
       Serial.println(info);
}

// void audio_showstreamtitle(const char *info)
// {
// }

// void audio_showstreamurl(const char *info)
// {
//        Serial.print("audio_showstreamurl: ");
//        Serial.println(info);
// }
// void audio_showstreamartist(const char *info)
// {
//        Serial.print("audio_showstreamartist: ");
//        Serial.println(info);
// }
// void audio_showstreamtitleartist(const char *info)
// {
//        Serial.print("audio_showstreamtitleartist: ");
//        Serial.println(info);
// }
// void audio_showstreamtitlealbum(const char *info)
// {
//        Serial.print("audio_showstreamtitlealbum: ");
//        Serial.println(info);
// }
// void audio_showstreamtitlealbumartist(const char *info)
// {
//        Serial.print("audio_showstreamtitlealbumartist: ");
//        Serial.println(info);
// }
// void audio_bitrate(const char *info)
// {
//        Serial.print("audio_bitrate: ");
//        Serial.println(info);
// }
// void audio_commercial(const char *info)
// {
//        Serial.print("audio_commercial: ");
//        Serial.println(info);
// }
// void audio_icyurl(const char *info)
// {
//        Serial.print("audio_icyurl: ");
//        Serial.println(info);
// }
// void audio_icyname(const char *info)
// {
//        Serial.print("audio_icyname: ");
//        Serial.println(info);
// }
// void audio_icylogo(const char *info)
// {
//        Serial.print("audio_icylogo: ");
//        Serial.println(info);
// }
// void audio_icydescription(const char *info)
// {
//        Serial.print("audio_icydescription: ");
//        Serial.println(info);
// }
// void audio_lasthost(const char *info)
// {
//        Serial.print("audio_lasthost: ");
//        Serial.println(info);
// }
void audio_eof_mp3(const char *info)
{
       Serial.print("audio_eof_mp3: ");
       Serial.println(info);
       autoPlayNextSong(); // 自动播放下一首歌曲
}

void audio_showstreamtitle(const char *info){
    Serial.print("streamtitle ");Serial.println(info);
}


void audio_id3data(const char *info){  //id3 metadata
    // audio.setVolume(0);
    Serial.print("id3data     ");Serial.println(info);
}

void audio_bitrate(const char *info){
    audio.setVolume(volume);
    Serial.print("bitrate     ");Serial.println(info);
}
// void audio_eof_speech(const char *info)
// {
//        Serial.print("audio_eof_speech: ");
//        Serial.println(info);
// }
// void audio_eof_stream(const char *info)
// {
//        Serial.print("audio_eof_stream: ");
//        Serial.println(info);
// }

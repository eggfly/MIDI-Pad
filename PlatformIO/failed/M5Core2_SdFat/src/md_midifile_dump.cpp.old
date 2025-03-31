// Look at the internals of a MIDI file, as interpreted by the library.
// Good for close level debugging of a file and how and in what order it
// is it is being parsed by the library.
// Also good just for curiosity of what is in the file!

#include <SPI.h>
#include <SD.h>
#include <SdFat.h>
#include <MD_MIDIFile.h>

// MIDIFile.h library settings must be set to values below before compiling
#define DUMP_DATA 1
#define SHOW_UNUSED_META 1
//
// -= REMEMBER =-
//  * Turn line ending in Serial Monitor to NEWLINE
//  * Turn DUMP_DATA and SHOW_METADATA after back to default (0) when done
//

// SD chip select pin.
// Default SD chip select is the SPI SS pin (10 on Uno, 53 on Mega).

int sck = 18;
int miso = 38;
int mosi = 23;
int sd_cs = 4;
const uint8_t SD_SELECT = sd_cs;

// states for the state machine
enum fsm_state
{
  STATE_BEGIN,
  STATE_PROMPT,
  STATE_READ_FNAME,
  STATE_LOAD,
  STATE_PROCESS,
  STATE_CLOSE
};

SDFAT MY_SD;
MD_MIDIFile SMF;

void midifile_setup(void)
{
  Serial.println(F("[MIDI_File_Dumper]"));
  
  // 初始化SPI
  SPI.begin(sck, miso, mosi, sd_cs);
  Serial.println(F("--- eggfly SPI initialized"));
  
  // 配置SD卡
  SdSpiConfig config(SD_SELECT, USER_SPI_BEGIN, SPI_FULL_SPEED, &SPI);
  Serial.println(F("--- eggfly SdSpiConfig created"));
  
  // 尝试初始化SD卡
  bool sd_begin_result = MY_SD.begin(config);
  if (!sd_begin_result)
  {
    Serial.println(F("SD init failed!"));
    Serial.print(F("Error code: "));
    Serial.println(MY_SD.sdErrorCode());
    Serial.print(F("Error data: "));
    Serial.println(MY_SD.sdErrorData());
    while (true)
      ;
  }
  
  Serial.println(F("SD card initialized successfully"));
  
  // 获取并打印SD卡信息
  uint32_t cardSize = MY_SD.card()->cardSize();
  Serial.print(F("Card size: "));
  Serial.println(cardSize);
  
  uint8_t cardType = MY_SD.card()->type();
  Serial.print(F("Card type: "));
  Serial.println(cardType);
  
  SMF.begin(&SD);
}

void midifile_loop(void)
{
  int err;
  static fsm_state state = STATE_BEGIN;
  static char fname[50];

  switch (state)
  {
  case STATE_BEGIN:
  case STATE_PROMPT:
    Serial.print(F("\n\n"));
    MY_SD.ls(Serial);
    Serial.print(F("\nEnter file name: "));
    state = STATE_READ_FNAME;
    break;

  case STATE_READ_FNAME:
  {
    uint8_t len = 0;
    char c;

    // read until end of line
    do
    {
      while (!Serial.available())
        ; // wait for the next character
      c = Serial.read();
      fname[len++] = c;
    } while (c != '\n');

    // properly terminate
    --len;
    fname[len++] = '\0';

    Serial.print(fname);
    state = STATE_LOAD;
  }
  break;

  case STATE_LOAD:
    err = SMF.load(fname);
    if (err != MD_MIDIFile::E_OK)
    {
      Serial.print(F("\nSMF load Error "));
      Serial.print(err);
      state = STATE_PROMPT;
    }
    else
    {
      SMF.dump();
      state = STATE_PROCESS;
    }
    break;

  case STATE_PROCESS:
    if (!SMF.isEOF())
      SMF.getNextEvent();
    else
      state = STATE_CLOSE;
    break;

  case STATE_CLOSE:
    SMF.close();
    state = STATE_BEGIN;
    break;

  default:
    state = STATE_BEGIN; // reset in case things go awry
  }
}

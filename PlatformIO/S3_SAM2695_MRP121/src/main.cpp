#include <Arduino.h>
#include <Wire.h>

#include "Adafruit_MPR121.h"
#include "M5_SAM2695.h"

M5_SAM2695 midi;

#define I2C_SDA 4
#define I2C_SCL 3

#define MIDI_DATA_PIN 13
#define MIDI_NC_PIN 14

Adafruit_MPR121 cap = Adafruit_MPR121();

uint16_t lasttouched = 0;
uint16_t currtouched = 0;

uint8_t currInstrument = GrandPiano_1;
uint8_t currDrum = SynthDrum;
bool isDrum = false;

void setup()
{
       // ENABLE 3V3
       pinMode(1, OUTPUT);
       digitalWrite(1, HIGH);
       // ADDR to HIGH
       pinMode(5, OUTPUT);
       digitalWrite(5, HIGH);

       midi.begin(&Serial2, MIDI_BAUD, MIDI_NC_PIN, MIDI_DATA_PIN);
       // midi.setInstrument(0, 9, SynthDrum); // synth drum
       // midi.setInstrument(0, 0, ElGrdPiano_3);  // synth piano 1
       midi.setInstrument(0, 0, currInstrument); // midi piano 1

       delay(1000);
       Serial.begin(115200);
       Serial.println("MIDI Drum");
       midi.setMasterVolume(50);
       // midi.setVolume
       Wire.begin(I2C_SDA, I2C_SCL);
       Serial.println("Adafruit MPR121 Capacitive Touch sensor test");

       // Default address is 0x5A, if tied to 3.3V its 0x5B
       // If tied to SDA its 0x5C and if SCL then 0x5D
       if (!cap.begin(0x5A))
       {
              Serial.println("MPR121 not found, check wiring?");
              while (1)
                     ;
       }
       Serial.println("MPR121 found!");
       // range 0-255
       cap.setThresholds(
           8,
           1); // set touch and release thresholds
}
// 底鼓
#define DRUM1 36

// 军鼓
#define DRUM2 38

// 踩镲闭
#define DRUM3 42

// 踩镲开
#define DRUM4 46

// 边鼓
#define DRUM5 37

// 手鼓
#define DRUM6 39

// 高音地鼓
#define DRUM7 41

// 低音地鼓
#define DRUM8 43

// 高音中鼓
#define DRUM9 50

// 低音中鼓
#define DRUM10 45

// 镲片
#define DRUM11 49

uint8_t convertKeyToPitch(uint8_t keyIndex)
{
       // Map key index to MIDI pitch values
       switch (keyIndex)
       {
       case 0:
              return NOTE_B3;
       case 1:
              return NOTE_C4;
       case 2:
              return NOTE_D4;
       case 3:
              return NOTE_E4;
       case 4:
              return NOTE_F4;
       case 5:
              return NOTE_G4;
       case 6:
              return NOTE_A4;
       case 7:
              return NOTE_B4;
       case 8:
              return NOTE_C5;
       default:
              return 0; // Default to 0 if keyIndex is out of range
       }
}

void loop()
{
       // cap.setThresholds(0, 0); // set touch and release thresholds
       // Get the currently touched pads
       currtouched = cap.touched();

       for (uint8_t i = 0; i < 12; i++)
       {
              // it if *is* touched and *wasnt* touched before, alert!
              if ((currtouched & _BV(i)) && !(lasttouched & _BV(i)))
              {
                     Serial.print(i);
                     Serial.println(" touched");
                     if (i == 11)
                     {
                            isDrum = !isDrum;
                            if (!isDrum)
                            {
                                   currInstrument++;
                                   midi.setInstrument(0, 0, currInstrument);
                            }
                            else
                            {
                                   currDrum -= 2;
                                   if (currDrum < SteelDrums)
                                   {
                                          currDrum = SynthDrum;
                                   }
                                   midi.setInstrument(0, 9, currInstrument);
                            }
                            if (!isDrum)
                            {
                                   Serial.printf("Instrument: %d\n", currInstrument);
                            }
                            else
                            {
                                   Serial.printf("Drum: %d\n", currDrum);
                            }
                     }
                     else
                     {
                            if (isDrum)
                            {
                                   midi.setNoteOn(9, DRUM1 + i, 127);
                            }
                            else
                            {
                                   auto pitch = convertKeyToPitch(i);
                                   if (pitch)
                                   {
                                          midi.setNoteOn(0, pitch, 127);
                                   }
                            }
                     }
              }
              // if it *was* touched and now *isnt*, alert!
              if (!(currtouched & _BV(i)) && (lasttouched & _BV(i)))
              {
                     auto pitch = convertKeyToPitch(i);
                     if (pitch)
                     {
                            midi.setNoteOff(0, pitch, 127);
                     }
                     Serial.print(i);
                     Serial.println(" released");
              }
       }

       // reset our state
       lasttouched = currtouched;
       // midi.setNoteOn(9, 36, 127);
       // midi.setNoteOn(9, 42, 127);
       // delay(300);
       // midi.setNoteOn(9, 42, 127);
       // delay(300);
       // midi.setNoteOn(9, 38, 127);
       // midi.setNoteOn(9, 42, 127);
       // delay(300);
       // midi.setNoteOn(9, 42, 127);
       // delay(300);

       // midi.setNoteOn(9, 36, 127);
       // midi.setNoteOn(9, 42, 127);
       // delay(300);
       // midi.setNoteOn(9, 36, 127);
       // midi.setNoteOn(9, 42, 127);
       // delay(300);
       // midi.setNoteOn(9, 38, 127);
       // midi.setNoteOn(9, 42, 127);
       // delay(300);
       // midi.setNoteOn(9, 42, 127);
       // delay(300);
}

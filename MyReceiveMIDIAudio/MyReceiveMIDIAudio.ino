/*
   SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD

   SPDX-License-Identifier: MIT
*/

/*
   @file receiveMIDIAudio.ino
   @brief Unit MIDI receives MIDI signals
   @version 0.1
   @date 2024-07-17

   @Hardwares: M5Core + Unit MIDI(Need to connect speakers/headphones)
   @Platform Version: Arduino M5Stack Board Manager v2.1.0
   @Dependent Library:
   M5_SAM2695: https://github.com/m5stack/M5_SAM2695

   @Usage: For specific usage, please see README
*/

#include "M5_SAM2695.h"

M5_SAM2695 midi;

uint16_t cnt = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("Receives MIDI signals");
  midi.begin(&Serial2, MIDI_BAUD, 32, 26);
  midi.setMasterVolume(20);
  Serial.println("Receiving");
}

void loop() {
  if (Serial.available()) {
    byte midiData = Serial.read();
    Serial2.write(midiData);
    Serial.print(midiData);
    if (++cnt > 260) {
      cnt = 0;
      Serial.println("Receiving");
    }
  }
}

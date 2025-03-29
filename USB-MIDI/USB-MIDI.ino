/**
 * Simple example MIDI class
 * author: chegewara
 */
#include <M5Unified.h>
#include "midiusb.h"
#include "sam2695.h"
// #ifdef CFG_TUD_MIDI

MIDIusb midi;
M5_SAM2695 sam2695;

void setup() {
    M5.begin();
    M5.Display.setTextScroll(true);
    midi.begin();
    sam2695.begin(&Serial2, MIDI_BAUD, 1, 2);
    sam2695.setMasterVolume(127);
    delay(1000);
  }

void loop() {
  M5.update();
  uint8_t _mid[3] = {0};
  if(tud_midi_stream_read(_mid, 3)) {
    M5.Display.printf("%x %x %x\n", _mid[0], _mid[1], _mid[2]);
    sam2695._serial->write(_mid, 3);
  }
}

// #endif

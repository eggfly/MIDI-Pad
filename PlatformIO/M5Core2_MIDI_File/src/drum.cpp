
 #include "M5_SAM2695.h"
 
 
 void midi_setup( M5_SAM2695 &midi ) {
     Serial.println("MIDI Drum");
     
     midi.begin(&Serial2, MIDI_BAUD, 33, 32);
     midi.setInstrument(0, 9, SynthDrum);  // synth drum
 }
 
 void midi_loop(M5_SAM2695 &midi) {
     midi.setNoteOn(9, 36, 127);
     midi.setNoteOn(9, 42, 127);
     delay(300);
     midi.setNoteOn(9, 42, 127);
     delay(300);
     midi.setNoteOn(9, 38, 127);
     midi.setNoteOn(9, 42, 127);
     delay(300);
     midi.setNoteOn(9, 42, 127);
     delay(300);
 
     midi.setNoteOn(9, 36, 127);
     midi.setNoteOn(9, 42, 127);
     delay(300);
     midi.setNoteOn(9, 36, 127);
     midi.setNoteOn(9, 42, 127);
     delay(300);
     midi.setNoteOn(9, 38, 127);
     midi.setNoteOn(9, 42, 127);
     delay(300);
     midi.setNoteOn(9, 42, 127);
     delay(300);
 }
 
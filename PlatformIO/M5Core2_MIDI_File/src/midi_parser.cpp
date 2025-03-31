/**
 * We provide a midi file as hex array and parse it incrementally.
 * This sketch has quite some complexity for reading the data.
 * This can be avoided when you use the MemoryStream and StreamCopy
 * from the AudioTools project.
 * @author Phil Schatzmann
 */
#include <Arduino.h>

#include "MidiFileParser.h"
// #include "examples/example-midi.h"
#include "midi_data.h"

#include <algorithm>

#include "M5_SAM2695.h"

#define DEBUG_MIDI_MSG (true)

M5_SAM2695 sam2695;

void midi_setup(M5_SAM2695 &midi);
void midi_loop(M5_SAM2695 &midi);

using namespace midi;

const int write_size = 256;
MidiFileParser mf;
int pos = 0;
bool debug = false;

size_t midi_len = sizeof(midi_data);

void midi_parser_setup()
{
       // midi_setup(sam2695);
       // return;

#ifdef ARDUINO
       Serial.begin(115200);
#endif
       mf.begin(debug, 256 * 5);

       sam2695.begin(&Serial2, MIDI_BAUD, 33, 32);
       sam2695.setMasterVolume(10);
       // sam2695.setNoteOn(0, 0, 0); // set note on
       // sam2695.setInstrument(0, 9, SynthDrum);  // synth drum
       // preload all data
       // while (true)
       // {
       //        int written = 0;
       //        int len = std::min((size_t)write_size, (size_t)midi_len - pos);
       //        if (pos < midi_len)
       //        {
       //               written = mf.write(midi_data + pos, len);
       //        }
       //        else
       //        {
       //               // mf.endWrite(); // parse final unparsed bytes
       //               break;
       //        }
       //        pos += written;
       // }
       // data has been loaded
       // printf("Number of events: %ld\n", (long)mf.size());
}

void sendMidiMessage(const struct midi_midi_event &midi)
{
       // 合并status和channel到状态字节
       uint8_t status_byte = (midi.status << 4) | (midi.channel & 0x0F);

       // 发送状态字节
       Serial2.write(status_byte);

       // 根据状态类型发送参数
       switch (midi.status)
       {
       case 0x8: // Note Off
       case 0x9: // Note On
       case 0xA: // Poly Aftertouch
       case 0xB: // Control Change
       case 0xE: // Pitch Bend
              Serial2.write(midi.param1);
              Serial2.write(midi.param2);
              break;

       case 0xC: // Program Change
       case 0xD: // Channel Aftertouch
              Serial2.write(midi.param1);
              break;
       default:
              Serial.printf("Unknown MIDI status: %02X\n", midi.status);
              // 系统消息可在此扩展
       }
}

void midi_parser_loop()
{
       if (!mf)
              return;

       // Try to keep buffer filled
       if (mf.availableForWrite() > write_size)
       {
              int written = 0;
              int len = std::min(write_size, (int)midi_len - len);
              if (pos < midi_len)
              {
                     written = mf.write(midi_data + pos, len);
              }
              else
              {
                     // when there is no more data we let the parser know
                     mf.end();
              }
              pos += written;
       }

       // Parse midi
       auto state = mf.parseTimed(); // parseTimed() or parse();

       // Process Result
       switch (state.status)
       case MIDI_PARSER_TRACK_MIDI:
       {
              // process midi
              printf("process");
              printf("  time: %ld - %ld", (long)state.timeInTicks(), (long)state.timeInMs());
              printf("  status: %d [%s]", state.midi.status,
                     mf.midi_status_name(state.midi.status));
              printf("  channel: %d", state.midi.channel);
              printf("  param1: %d", state.midi.param1);
              printf("  param2: %d\n", state.midi.param2);
              sendMidiMessage(state.midi); // 直接传递结构体
              break;
       case MIDI_PARSER_ERROR:
              printf("Error\n");
       case MIDI_PARSER_EOB:
#ifdef ARDUINO
              while (true)
                     ;
#else
              exit(-1);
#endif
              break;
       default:
              break;
       }
}

// void midi_parser_loop()
// {
//        // midi_loop(sam2695);
//        // return;
//        // Parse midi
//        auto state = mf.parseTimed(); // parseTimed() or parse();

//        // Process Result
//        switch (state.status)
//        case MIDI_PARSER_TRACK_MIDI:
//        {
//               // process midi
//               if (DEBUG_MIDI_MSG)
//               {
//                      printf("process");
//                      printf("  time-cumulated ms: %ld", (long)state.timeInMs());
//                      printf("  status: %d [%s]", state.midi.status,
//                             mf.midi_status_name(state.midi.status));
//                      printf("  channel: %d", state.midi.channel);
//                      printf("  param1: %d", state.midi.param1);
//                      printf("  param2: %d\n", state.midi.param2);
//               }
//               // sam2695
//               // write raw midi command
//               sendMidiMessage(state.midi); // 直接传递结构体
//               break;
//        case MIDI_PARSER_ERROR:
//               printf("Error\n");
//               break;
//        case MIDI_PARSER_TRACK_SYSEX:
//               Serial.printf("SysEx length: %d\n", state.sysex.length);
//               break;
//        case MIDI_PARSER_DELAY:
//               // printf("Delay: %ld ms\n", (long)state.timeInMs());
//               break;
//        case MIDI_PARSER_EOB:
// #ifdef ARDUINO
//               while (true)
//                      ;
// #else
//               exit(-1);
// #endif
//               break;
//        default:
//               break;
//        }
// }

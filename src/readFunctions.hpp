#include <iostream>

#define u8 uint8_t
#define u32 uint32_t

using namespace std;

vector<u8> MD;
vector<vector<uint64_t>> notes;          // timestamp, (channel | note | velocity | on/off)
vector<vector<uint64_t>> systemMessages; // timestamp, (value index, channel, value, 0)
vector<vector<uint64_t>> meta;           // timestamp, tempo

u32 delta;
uint64_t rI;
uint64_t trackTime;
u8 eventType;
u8 channel;
uint8_t byte1, byte2, byte3, byte4;
bool report = false;

uint16_t eight2sixteen(u8 b1, u8 b2) { return b1 << 8 | b2; }
u32 eight2twentyfour(u8 b1, u8 b2, u8 b3) { return b1 << 16 | b2 << 8 | b3; }
u32 eight2thirtytwo(u8 b1, u8 b2, u8 b3, u8 b4)
{
  return b1 << 24 | b2 << 16 | b3 << 8 | b4;
}
void thirdytwo2eight(u32 value)
{
  byte1 = (value >> 24) & 0xFF;
  byte2 = (value >> 16) & 0xFF;
  byte3 = (value >> 8) & 0xFF;
  byte4 = value & 0xFF;
}

void getDelta()
{
  delta = 0;
  while (MD[rI] > 127)
  {
    delta = delta << 7 | MD[rI] & 127;
    rI++;
  }
  delta = delta << 7 | MD[rI] & 127;
  rI++;
}

void getMeta()
{
  rI++;
  u32 tempo = 0;
  switch (MD[rI])
  {
  case 0x51:
    tempo = eight2twentyfour(MD[rI + 2], MD[rI + 3], MD[rI + 4]);
    meta.push_back({trackTime, tempo});
    break;
  }
  rI++;
  rI += MD[rI];
  rI++;
}

void getEvent()
{
  if ((MD[rI] & 0xF0) >> 4 > 7)
  {
    eventType = (MD[rI] & 0xF0) >> 4;
    channel = MD[rI] & 0x0F;
    rI++;
  }
  switch (eventType)
  {
  case 0b1000: // note off
    notes.push_back({trackTime, eight2thirtytwo(channel, MD[rI], MD[rI + 1], 0)});
    rI += 2;
    break;
  case 0b1001: // note on
    if (MD[rI + 1] == 0)
    {
      notes.push_back({trackTime, eight2thirtytwo(channel, MD[rI], MD[rI + 1], 0)});
    }
    else
    {
      notes.push_back({trackTime, eight2thirtytwo(channel, MD[rI], MD[rI + 1], 1)});
    }
    rI += 2;
    break;
  case 0b1010: // aftertouch
    rI += 2;
    break;
  case 0b1011: // control change
    rI += 2;
    break;
  case 0b1100: // program change
    systemMessages.push_back({trackTime, eight2thirtytwo(0, channel, MD[rI], 0)});
    rI += 1;
    break;
  case 0b1101: // channel pressure
    rI += 1;
    break;
  case 0b1110: // pitch wheel
    rI += 2;
    break;
  case 0b1111: // system exclusive
    while (MD[rI] != 0xF7)
    {
      rI++;
    }
    rI++;
    break;
  default:
    cerr << "Unknown event type 0x" << hex << uppercase << eventType << " at 0x" << hex
         << rI - 1 << endl;
    report = true;
  }
}
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <cmath>
#include <filesystem>

#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>

#include "windows.hpp"

#define u8 uint8_t
#define u32 uint32_t
#define OFFSET 100

using namespace std;
namespace fs = filesystem;

u8 header[8] = {0x4D, 0x54, 0x68, 0x64, 0x00, 0x00, 0x00, 0x06};
u8 trackHeader[4] = {0x4D, 0x54, 0x72, 0x6B};

int playingNotes;

uint16_t trackCount;
uint64_t totalSize;

int main()
{
  initFont();
  loadSprites();

  for (const auto &entry : fs::directory_iterator("./midi_files"))
  {
    if (entry.is_regular_file())
    {
      string ext = entry.path().extension().string();
      if (ext == ".mid" || ext == ".midi")
      {
        files.push_back(entry.path().filename().string());
      }
    }
  }

  while (true)
  {
    if (displaySelectionScreen() == 0)
      return 0;

    ifstream file("./midi_files/" + files[selectedFile], ios::binary);

    if (!file.is_open())
    {
      cerr << "Error: Could not open file.\n";
      return 1;
    }

    char byte;
    midiData.clear();
    MIDITime = 0;
    globalIndex = 0;
    lastMIDITime = 0;
    for(int i = 0; i < 128; i++){
      for(int j = 0; j < 16; j++){
        activeNotes[j][i] = false;
      }
    }
    while (file.get(byte))
    {
      MD.push_back(static_cast<u8>(byte));
    }

    file.close();

    // check header
    for (rI = 0; rI < 8; rI++)
    {
      if (MD[rI] != header[rI])
      {
        cerr << "Invalid header" << endl;
        cerr << "Expected 0x" << hex << uppercase << header[rI] << endl;
        cerr << "Got 0x" << hex << uppercase << MD[rI] << endl;
        return 1;
      }
    }
    rI += 2;
    trackCount = eight2sixteen(MD[rI], MD[rI + 1]);
    cout << "Track count: " << trackCount << endl;
    rI += 2;
    PPQN = eight2sixteen(MD[rI], MD[rI + 1]);
    cout << "PPQN: " << PPQN << endl;
    rI += 2;
    for (int track = 0; track < trackCount; track++)
    {
      for (int i = 0; i < 4; i++)
      {
        if (MD[rI + i] != trackHeader[i])
        {
          cerr << "Invalid track header at 0x" << hex << uppercase << rI + i << endl;
          cerr << "Expected 0x" << hex << uppercase << trackHeader[i] << endl;
          cerr << "Got 0x" << hex << MD[rI + i] << endl;
          return 1;
        }
      }
      rI += 4;
      u32 trackLenght =
          eight2thirtytwo(MD[rI], MD[rI + 1], MD[rI + 2], MD[rI + 3]);
      cout << "Track #" << track << ", Size: " << trackLenght << " bytes" << endl;
      totalSize += trackLenght;
      rI += 4;
      uint64_t end = rI + trackLenght;
      trackTime = 0;
      while (rI < end)
      {
        getDelta();
        if (MD[rI] == 0xFF)
          getMeta();
        else
          getEvent();
        if (report)
          return 1;
      }
    }

    sort(midiData.begin(), midiData.end(), [](const vector<uint64_t> &a, const vector<uint64_t> &b)
         { return a[0] < b[0]; });

    cout << "Done decoding " << totalSize / 1000 << " kB" << endl;
    cout << "Total data length is " << midiData.size() << endl;
    MD.clear();

    unsigned int nPorts = midiOut.getPortCount();

    midiOut.openPort(selectedDevice);

    while(true){
      thirdytwo2eight(midiData[globalIndex][1]);
      if(byte4 == 0xFF){
        Tempo = 60000000.0 / (byte1 << 16 | byte2 << 8 | byte3);
        break;
      }
      globalIndex++;
    }
    globalIndex = 0;

    if (displayPlayerScreen() == 0)
      return 0;

    stopAllNotes(midiOut);

    midiOut.closePort();
  }

  return 0;
}

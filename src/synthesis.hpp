#include <RtMidi.h>

using namespace std;

vector<string> MIDIDevices;

bool activeNotes[16][128];

void playNote(RtMidiOut &midiOut, int channel, int note, int velocity)
{

  vector<unsigned char> message;
  message.push_back(0x90 | channel); // Note On message for the channel
  message.push_back(note);           // MIDI note number
  message.push_back(velocity);       // Velocity
  midiOut.sendMessage(&message);
}

// Function to turn off a MIDI note on a specific channel
void stopNote(RtMidiOut &midiOut, int channel, int note, bool force)
{
  if (activeNotes[channel][note] || force)
  {
    vector<unsigned char> message;
    message.push_back(0x80 | channel); // Note Off message for the channel
    message.push_back(note);           // MIDI note number
    message.push_back(0);              // Velocity (ignored for Note Off)
    midiOut.sendMessage(&message);
  }
}

// Function to send a Program Change message
void sendProgramChange(RtMidiOut &midiOut, unsigned char channel, unsigned char programNumber)
{
  unsigned char status = 0xC0 | channel;
  vector<unsigned char> message = {status, programNumber};
  midiOut.sendMessage(&message);
  // cout << "Program changed to " << (int)programNumber << " on channel " << (int)(channel + 1) << endl;
}

void listMidiDevices()
{
  try
  {
    RtMidiOut midiOut;
    MIDIDevices.clear();

    // List output devices
    cout << "\nMIDI Output Devices:" << endl;
    unsigned int nOutputPorts = midiOut.getPortCount();
    for (unsigned int i = 0; i < nOutputPorts; ++i)
    {
      try
      {
        string portName = midiOut.getPortName(i);
        // cout << i << ": " << portName << endl;
        MIDIDevices.push_back(portName);
      }
      catch (RtMidiError &error)
      {
        cerr << "Error getting output port name: " << error.getMessage() << endl;
      }
    }
  }
  catch (RtMidiError &error)
  {
    cerr << "An RtMidi error occurred: " << error.getMessage() << endl;
  }
}

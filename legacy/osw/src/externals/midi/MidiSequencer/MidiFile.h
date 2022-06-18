
/*
 * Copyright (c) 1999 Regents of the University of California.
 * All rights reserved.

 * The name of the University may not be used to endorse or
 * promote products derived from this software without
 * specific prior written permission.  THIS SOFTWARE IS
 * PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE.
 */

/*
  midifile.h
  C++ classes for standard MIDI files
  Made into a valid OSW type
  Amar Chaudhary
  */

#include <vector>
#include <list>

using namespace std;

typedef unsigned char byte;

/* miscellaneous definitions */
#define MThd 0x4d546864
#define MTrk 0x4d54726b
#define LOBYTE(x) ((byte)(x & 0xff))
#define HIBYTE(x) ((byte)((x & 0xff00)>>8))


struct MidiEvent {
  int ticks;
  byte status;
  enum EventType {Channel, Sysex, Common, Meta} type;
  vector<byte> bytes;

  MidiEvent (int aticks = 0, EventType atype = Channel,
	     byte astatus = 0, int abytes = 0) :
    ticks(aticks),
    type(atype),
    status(astatus),
    bytes(abytes) {
  }

  bool operator == (const MidiEvent &toCompare) const {
    return ticks == toCompare.ticks;
  }

  bool operator < (const MidiEvent &toCompare) const {
    return ticks <= toCompare.ticks;
  }
};

struct MidiFile {

  typedef list<MidiEvent>  Track;
  vector<Track> tracks;
  vector<Track::iterator> trackptr;
  int division;
  long tempo;
  
  MidiFile (int anumtracks = 1, int adivision = 96) :
    tracks(anumtracks),
    trackptr(anumtracks){
      division = adivision;
      tempo = 500000;
  }

  
  void Reset () {
    for (int i = 0; i < trackptr.size(); ++i) {
      trackptr[i] = tracks[i].begin();
    }
  }

  int FindEndPoint () {
    Track::reverse_iterator endptr;
    int result = 0;
    for (int i = 0; i < tracks.size(); ++i) {
      endptr = tracks[i].rbegin();
      if (endptr != tracks[i].rend()) {
	if (endptr->ticks > result) {
	  result = endptr->ticks;
	}
      }
    }
    return result;
  }

  bool EventReady(int index, int ticks) {
    return trackptr[index] != tracks[index].end() && ticks >= trackptr[index]->ticks;
  }

  MidiEvent &GetNextEvent(int index) {
    MidiEvent &result = *(trackptr[index]);
    ++(trackptr[index]);
    return result;
  }

  friend MidiFile * ReadMidiFile (const char *filename);
};  

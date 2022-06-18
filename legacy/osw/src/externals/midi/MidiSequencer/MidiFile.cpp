
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
  midifile.cpp
  Handy Routines for reading Standard MIDI files
  Amar Chaudhary
  
  */

#if defined(__sgi) && !defined(__GNUC__)
#include <stdio.h>
#else
#include <cstdio>
#endif
#include <string>
#include <exception>
#include <iostream>
#include "MidiFile.h"

using namespace std;

  struct my_exception : public std::exception  {
    my_exception(const string &amsg) throw () :
      xmsg(amsg) {
    }

    ~my_exception() throw () {}

    virtual const char *what () const throw () {
      return xmsg.c_str();
    }

  private:
    const string xmsg;
  };

static unsigned long ReadVarLen (FILE *infile, unsigned int *trackLength) {

  register long value;
  register byte c;

  if ((--*trackLength,value = getc(infile)) & 0x80) {
    value &= 0x7f;
    do {
      value = (value << 7) + (--*trackLength,(c = getc(infile)) & 0x7f);
    } while (c & 0x80);
  }

  return value;
}


static unsigned long to32bit(int c1, int c2, int c3, int c4) {
  unsigned long value = 0L;
  
  value = (c1 & 0xff);
  value = (value<<8) + (c2 & 0xff);
  value = (value<<8) + (c3 & 0xff);
  value = (value<<8) + (c4 & 0xff);
  return (value);
}

static unsigned short to16bit(int c1,int c2) {
  return ((c1 & 0xff ) << 8) + (c2 & 0xff);
}

static unsigned long read32bit(FILE *infile) {
  int c1, c2, c3, c4;
  
  c1 = getc(infile);
  c2 = getc(infile);
  c3 = getc(infile);
  c4 = getc(infile);
  return to32bit(c1,c2,c3,c4);
}

static short read16bit(FILE *infile) {
  int c1, c2;
  c1 = getc(infile);
  c2 = getc(infile);
  return to16bit(c1,c2);
}

static bool ReadTrack (FILE *infile, MidiFile *midiFile, int trackIndex);
static bool
ReadMetaEvent (FILE *infile, MidiFile::Track &track, int ticks,
	       unsigned int *trackLength);
static bool
ReadSysexEvent (FILE *infile, MidiFile::Track &track, int ticks,
		int status, unsigned int *trackLength);
static bool
ReadWhatever(FILE *infile, MidiFile::Track &track, int ticks,
	     unsigned int *trackLength);

/************************/


MidiFile *ReadMidiFile (const char *filename) {

  FILE *infile = fopen (filename,"rb");
  if (infile == NULL) {
    throw (my_exception((string("File not found: ") + filename).c_str()));
  }

  unsigned long chunkTag = read32bit(infile);
  if (chunkTag != MThd) {
    throw (my_exception("Not a valid MIDI file header"));
  }

  unsigned int length = read32bit(infile);
  int format = read16bit(infile);
  int numtracks = read16bit(infile);
  int division = read16bit(infile);

  length -= 6;
  while (length > 0) {
    getc(infile);
    --length;
  }

  MidiFile *midiFile = new MidiFile (numtracks,division);
  
  for (int i = 0;  ReadTrack(infile,midiFile,i); ++i);

  return midiFile;
}

/********************/

static bool ReadTrack (FILE *infile, MidiFile *midiFile, int trackIndex) {

  unsigned long chunkTag = read32bit(infile);
  if (chunkTag != MTrk) {
    if (feof(infile)) {
      return false;
    }
    throw (my_exception("Not a valid MIDI track header"));
  }

  unsigned int trackLength = read32bit(infile);

  unsigned int ticks = 0;
  bool sysexcont = false;
  bool running = false;
  byte status = 0;
  
  MidiFile::Track &track = midiFile->tracks[trackIndex];

  while (trackLength > 0) {
    ticks += ReadVarLen(infile,&trackLength);
    
    int c = getc(infile);
    --trackLength;
    if (c == EOF) {
      throw (my_exception("Unexpected end of file"));
    }

    if (!sysexcont && c == 0xF7) {
      throw (my_exception("Expected sysex continuation"));
    }

    if ((c & 0x80) == 0) {
      if (status == 0) {
	throw (my_exception("Unexpected running status"));
      }
      running = true;
      ungetc(c,infile);
      ++trackLength;
    } else {
      status = c;
      running = false;
    }

    switch (status >> 4) {
    case 0x8 :
    case 0x9 :
    case 0xA :
    case 0xB :
    case 0xE :
      track.push_back(MidiEvent(ticks,MidiEvent::Channel,status,2));
      track.back().bytes[0] = getc(infile);
      track.back().bytes[1] = getc(infile);
      trackLength -= 2;
      break;
    case 0xC :
    case 0xD :
      track.push_back(MidiEvent(ticks,MidiEvent::Channel,status,1));
      track.back().bytes[0] = getc(infile);
      --trackLength;
      break;
    case 0xF :
      switch (status) {
      case 0xFF :
	ReadMetaEvent(infile,track,ticks,&trackLength);
	break;
      case 0xF0 :
	sysexcont = ReadSysexEvent(infile,track,ticks,0xF0,&trackLength);
	break;
      case 0xF7 :
	if (sysexcont) {
	  sysexcont = ReadSysexEvent(infile,track,ticks,0xF7,&trackLength);
	} else {
	  ReadWhatever (infile,track,ticks,&trackLength);
	}
      default :
	throw (my_exception("Bad byte in file"));
      }
      break;
    default :
      throw (my_exception("Bad byte in file (2)"));
    }
  }

  return true;
}

static bool
ReadMetaEvent (FILE *infile, MidiFile::Track &track, int ticks,
	       unsigned int *trackLength) {

  char type = getc(infile);
  --*trackLength;

  int length = ReadVarLen(infile,trackLength);
    
  track.push_back(MidiEvent(ticks,MidiEvent::Meta,0xFF,length+1));
  track.back().bytes[0] = type;
  for (int i = 1; i <= length; ++i) {
    track.back().bytes[i] = getc(infile);
    --*trackLength;
  }

  return true;
}

static bool
ReadSysexEvent (FILE *infile, MidiFile::Track &track, int ticks,
	       int status, unsigned int *trackLength) {

  int length = ReadVarLen(infile,trackLength);
    
  track.push_back(MidiEvent(ticks,MidiEvent::Meta,status,length));
  for (int i = 0; i < length; ++i) {
    track.back().bytes[i] = getc(infile);
    --*trackLength;
  }
  
  return track.back().bytes[length-1] != 0xF7;
}

static bool
ReadWhatever (FILE *infile, MidiFile::Track &track, int ticks,
	      unsigned int *trackLength) {

  int length = ReadVarLen(infile,trackLength);
    
  for (int i = 0; i < length; ++i) {
    getc(infile);
    --*trackLength;
  }
  
  return true;
}


/*****************/

#ifdef JUST_MIDIFILE
int main (int argc, char *argv[]) {

  try {
    MidiFile *midiFile = ReadMidiFile(argv[1]);
    
    cout << midiFile->tracks.size() << " Tracks\n";
    cout << midiFile->division << " Division\n";
    for (int i = 0 ; i < midiFile->tracks.size(); ++i) {
      cout << "Track " << i << " with " << midiFile->tracks[i].size()
	   << " events.\n";
    }
  } catch (exception e) {
    cout << e.what() << endl;
  }
  return 0;
};
#endif







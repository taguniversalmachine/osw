/* 
Copyright (c) 2001-2003 Amar Chaudhary. All rights reserved.
Copyright (c) 1998-2001 Regents of the University of California.
All rights reserved.

     ``The contents of this file are subject to the Open Sound World Public
     License Version 1.0 (the "License"); you may not use this file except in
     compliance with the License. A copy of the License should be included
     in a file named "License" or "License.txt" in the distribution from 
     which you obtained this file. 

     Software distributed under the License is distributed on an "AS IS"
     basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
     License for the specific language governing rights and limitations
     under the License.

     The Original Code is Open Sound World (OSW) software.

     The Initial Developer of the Original Code is Amar Chaudhary.
     Portions created by Amar Chaudhary are Copyright (C) 1998-2001 
     Regents of the University of California and Amar Chaudhary. 
     All Rights Reserved.

     Contributor(s):

  
From the UC Regents:

Permission to use, copy, modify, and distribute this software and its
documentation, without fee and without a signed licensing agreement, is hereby
granted, provided that the above copyright notice, this paragraph and the
following two paragraphs appear in all copies, modifications, and
distributions.  Contact The Office of Technology Licensing, UC Berkeley, 2150
Shattuck Avenue, Suite 510, Berkeley, CA 94720-1620, (510) 643-7201, for
commercial licensing opportunities.

Written by Amar Chaudhary, The Center for New Music and Audio Technologies, 
University of California, Berkeley.

     IN NO EVENT SHALL REGENTS BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT,
     SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST
     PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS
     DOCUMENTATION, EVEN IF REGENTS HAS BEEN ADVISED OF THE POSSIBILITY OF
     SUCH DAMAGE.

     REGENTS SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT
     LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
     FOR A PARTICULAR PURPOSE. THE SOFTWARE AND ACCOMPANYING
     DOCUMENTATION, IF ANY, PROVIDED HEREUNDER IS PROVIDED "AS IS".
     REGENTS HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES,
     ENHANCEMENTS, OR MODIFICATIONS.
     
*/

/*
  oswLinuxMidi.cpp
  MIDI drivers for Linux
*/

#include "oswLinuxMidi.h"
#include "oswSelect.h"

using namespace std;

namespace osw {

  static char *devicename[] = {
    "/dev/midi",
    "/dev/midi00","/dev/midi01","/dev/midi02","/dev/midi03","/dev/midi04",
    "/dev/midi0","/dev/midi1","/dev/midi2","/dev/midi3","/dev/midi4",
    NULL
  };

  static fd_set midi_rfds;
  static struct timeval null_tv;

  /*************************/

  LinuxMidiOutputDriver::LinuxMidiOutputDriver (const string &aname, 
						Container *acontainer,
						const string &aFullName) :
    MidiOutputDriver (aname,acontainer,aFullName),
    xport(-1) {
  }
  
  LinuxMidiOutputDriver::~LinuxMidiOutputDriver () {
    CloseDriver();
  }

  void LinuxMidiOutputDriver::OpenDriver() {
    if ((xport = open(FullName->c_str(),O_RDWR | O_NDELAY)) < 0) {
      Error ("Could not open MIDI device "+FullName);
    }
    Alert("Driver started");
  }
  
  void LinuxMidiOutputDriver::CloseDriver() {
    if (xport >= 0) {
      close(xport);
    }
    xport = -1;
  }
  
  
  void LinuxMidiOutputDriver::SendMidiMessage (byte channel, byte status,
					       byte first, byte second) {
    if (xport >= 0) {
      byte firstByte = (status << 4) | channel;
      write(xport,(char *)&firstByte,1);
      write(xport,(char *)&first,1);
      if (status != 0xC && status != 0xD) {
	write(xport,(char *)&second,1);
      }
    }
  }

  void LinuxMidiOutputDriver::SendMidiSysex (void *buffer, int size) {
    if (xport > 0) {
      write (xport,buffer,size);
    }
  }
  
  /*************************/

  LinuxMidiInputDriver::LinuxMidiInputDriver (const string &aname, 
						Container *acontainer,
						const string &aFullName) :
    MidiInputDriver (aname,acontainer,aFullName),
    xport(-1) {
    xinSysex = false;
    xchannel = xstatus = xvalue1 = xvalue2 = -1;
  }
  
  LinuxMidiInputDriver::~LinuxMidiInputDriver () {
    CloseDriver();
  }
  
  void LinuxMidiInputDriver::OpenDriver() {
    if ((xport = open(FullName->c_str(),O_RDWR | O_NDELAY)) < 0) {
      Error ("Could not open MIDI device "+FullName);
    }
  }
  
  void LinuxMidiInputDriver::CloseDriver() {
    if (xport >= 0) {
      close(xport);
    }
    xport = -1;
  } 
  
  void LinuxMidiInputDriver::ReceiveMidiBytes() {
    unsigned char c;
    while (read(xport,&c,1) > 0) {
      if (xinSysex && c != 0xF7) continue;
      if ((c & 0xF0) != 0xF0) {
	if (c < 0x80) {
	  if (xvalue1 < 0) {
	    xvalue1 = c;
	    if (xstatus == 0xC || xstatus == 0xD) {
	      FireMidiEvents(xchannel,xstatus,xvalue1,0);
	      xvalue1 = -1;
	    }
	  } else {
	    xvalue2 = c;
	    FireMidiEvents(xchannel,xstatus,xvalue1,xvalue2);
	  }
	} else {
	  xchannel = c & 0xF;
	  xstatus = (c & 0xF0) >> 4;
	}
      } else {
	if (c == 0xF0) {
	  xinSysex = true;
	} else if (c == 0xF7) {
	  xinSysex = false;
	}
      }
    }
  }

  /*************************/

  static Container *MidiInputs,*MidiOutputs;
  static list<LinuxMidiInputDriver *> input_drivers;
  static list<LinuxMidiOutputDriver *> output_drivers;


  static void MidiInputSetupProc (SelectEventManager &eventManager) {
    if (input_drivers.empty()) {
      return;
    }
    for (list<LinuxMidiInputDriver *>::iterator  index = input_drivers.begin();
	 index != input_drivers.end();
	 ++index) {    
      if ((*index)->GetPort() > 0) {
	eventManager.AddRead((*index)->GetPort());   
      }
    }
  }

  static void MidiInputDispatchProc (SelectEventManager &eventManager) {
    for (list<LinuxMidiInputDriver *>::iterator index = input_drivers.begin();
	 index != input_drivers.end();
	 ++index) {
      int port = (*index)->GetPort();
      if (port > 0 && eventManager.IsSetRead(port)) {
	(*index)->ReceiveMidiBytes();
      }
    }
  }

  void InitMidi () {
    int i = 0;
    char buffer[32];
    MidiInputs = new Container("midiIn",NULL);
    MidiOutputs = new Container("midiOut",NULL);

    null_tv.tv_sec = 0; 
    null_tv.tv_usec = 0;

    for (char **devname = devicename; *devname != NULL; ++devname) {
      int port;
      if ((port = open(*devname,O_RDWR | O_NDELAY)) < 0) {
	continue;
      }
      close(port);
      sprintf (buffer,"%d",i);
      cout << "Found midi driver " << *devname << endl;
      input_drivers.push_back(new LinuxMidiInputDriver(buffer,MidiInputs,*devname));
      output_drivers.push_back(new LinuxMidiOutputDriver(buffer,MidiOutputs,*devname));
      ++i;
    }
    new State<int> ("count",MidiInputs,"Number of MIDI input devices",i);
    new State<int> ("count",MidiOutputs,"Number of MIDI output devices",i);

    AddSelectGroup(MidiInputSetupProc,MidiInputDispatchProc);
  }

  void CleanupMidi () {
    for (list<LinuxMidiInputDriver *>::iterator iindex = input_drivers.begin();
	 iindex != input_drivers.end();
	 ++iindex) {
      if (*iindex) {
	delete *iindex;
      }
    }
    for (list<LinuxMidiOutputDriver *>::iterator oindex = output_drivers.begin();
	 oindex != output_drivers.end();
	 ++oindex) {
      if (*oindex) {
	delete *oindex;
      }
    }
  }
}



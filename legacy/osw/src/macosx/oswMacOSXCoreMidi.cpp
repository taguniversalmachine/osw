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

*/

/*  
  oswMacOSXCoreMIDI
  Core MIDI drivers for Mac OS X
*/

#include "osw.h"
#include "oswSched.h"
#include "oswMidi.h"
#include <CoreMIDI/CoreMIDI.h>
#include <CoreAudio/HostTime.h>
#include <CoreFoundation/CoreFoundation.h>

namespace osw {
  
  class CoreMidiInputDriver;
  class CoreMidiOutputDriver;

  /*************************/
/* MIDI System Messages */
#define MD_SYSTEM_MSG   ((byte)0xF0)
#define MD_PITCH_BEND		((byte)0xE0)
#define MessageType(MSG)  (byte)((MSG) & ((byte)0xF0))

#define MIDI_BUFFER_SIZE 256

  typedef struct { 
    byte data[3]; 
    float delta_time; 
    void *ref;
  } MIDIMESSAGE;
  
  MIDIMESSAGE *s_midiBuffer;
  UInt64 s_lastTime = 0;  
  int  s_readIndex,s_writeIndex;

  /******************/

  static MIDIClientRef s_client;
  
  static Container *MidiInputs;
  static stl::list<CoreMidiInputDriver *> MidiInDrivers;
  static Container *MidiOutputs;
  static stl::list<CoreMidiOutputDriver *> MidiOutDrivers;
  
  static MIDIClientRef client;
  void midiInputCallback( const MIDIPacketList *list, void *procRef, void *srcRef );

  /******************/

  class CoreMidiInputDriver : public MidiInputDriver {

  public:

    CoreMidiInputDriver (const string &aname, 
                         osw::Container *acontainer,
                         const string &aFullName,
                         int aport) :
      MidiInputDriver (aname,acontainer,aFullName),
      xport (aport) {
      xisOpen = false;
    }

    ~CoreMidiInputDriver () {
      if (xisOpen) {
        CloseDriver();
      }
    }

    void OpenDriver() {
      if (!xisOpen) {
	CFStringRef cf = CFStringCreateWithCString(NULL,
						   PathName().c_str(),
						   kCFStringEncodingASCII);
        OSStatus result = MIDIInputPortCreate(s_client, cf,
                                              midiInputCallback,(void *) this,
                                              &xportRef);
        if (result != 0) {
          Error("Could not create MIDI Input Port");
          return;
        }
        
	xsrc = MIDIGetSource(xport);
	result = MIDIPortConnectSource(xportRef, xsrc, NULL );
	if (result != 0) {
	  Error("Could not connect to " + FullName);
	  MIDIPortDispose(xportRef);
	  return;
	}
      }
      
      xisOpen = true;
    }
    
    void CloseDriver() {
      if (xisOpen) {
        MIDIPortDisconnectSource(xportRef,xsrc);
        MIDIPortDispose(xportRef);
      }      
      xisOpen = false;
    }
  private:

    int xport;
    MIDIPortRef xportRef;
    bool xisOpen;
    MIDIEndpointRef xsrc;

  };

  void midiInputCallback( const MIDIPacketList *list, void *procRef, void *srcRef )
  {
    MIDIMESSAGE newMessage;
    UInt64 deltaTime;
    
    const MIDIPacket *packet = &list->packet[0];
    for (unsigned int i=0; i<list->numPackets; ++i){
      
      // Block unwanted messages.
      if ( packet->length > 3 ) continue;
      if (MessageType(packet->data[0]) == MD_SYSTEM_MSG) continue;
      
      memcpy( newMessage.data, packet->data, packet->length );
      deltaTime = packet->timeStamp - s_lastTime;
      deltaTime = AudioConvertHostTimeToNanos( deltaTime );
      newMessage.delta_time = deltaTime * 0.000000001;
      newMessage.ref = procRef;
      // Put newMessage in the circular buffer
      s_midiBuffer[s_writeIndex] = newMessage;
      s_writeIndex++;
      
      if( s_writeIndex >= MIDI_BUFFER_SIZE )
        s_writeIndex = 0;
      
      packet = MIDIPacketNext(packet);
    }
  }

  static void CreateMidiInDrivers () {
    
    char portNumStr[16];
    MidiInputs = new Container("midiIn",NULL);
    
    int numPorts = MIDIGetNumberOfSources();
    
    new State<int> 
      ("count",MidiInputs,"Number of MIDI input devices.",numPorts);
    
    cout << "Detected " << numPorts << " Midi Input Drivers:\n";
    
    for (int i = 0; i < numPorts; ++i) {
        sprintf (portNumStr,"%d",i);
        MidiInDrivers.push_back(new CoreMidiInputDriver(portNumStr,
							MidiInputs,
							portNumStr,
							i));
        cout << '\t' << portNumStr << ": "  << endl;
      
    }
    // Set up the circular buffer for the Midi input messages.
    s_midiBuffer = new MIDIMESSAGE[MIDI_BUFFER_SIZE];
    s_readIndex = 0;
    s_writeIndex = 0;
  }
  
  static void DestroyMidiInDrivers () {
    for (stl::list<CoreMidiInputDriver *>::iterator driver = MidiInDrivers.begin();
         driver != MidiInDrivers.end();
         ++driver) {
      //delete *driver;
    }
    delete MidiInputs;
    delete [] s_midiBuffer;
  }
  
  static void CreateMidiOutDrivers () {
    
    char portNumStr[16];
    MidiOutputs = new Container("midiOut",NULL);
    
    int numPorts = 0;  // fixme
    new State<int> 
      ("count",MidiOutputs,"Number of MIDI output devices.",numPorts);
    
    
    cout << "Detected " << numPorts << " Midi Output Drivers:\n";
    for (int i = 0; i < numPorts; ++i) {
      /*
      midiOutGetDevCaps(i,&caps,sizeof(caps));
      sprintf (portNumStr,"%d",i);
      MidiOutDrivers.push_back(new CoreMidiOutputDriver(portNumStr,
      MidiOutputs,
      caps.szPname,
      i));
      cout << '\t' << portNumStr << ": " << caps.szPname << endl;
      */
    }
  }
  
  static void DestroyMidiOutDrivers () {
    for (stl::list<CoreMidiOutputDriver *>::iterator driver = MidiOutDrivers.begin();
         driver != MidiOutDrivers.end();
         ++driver) {
      //delete *driver;
    }
    delete MidiOutputs;
  }
  

  void InitMidi () {
    OSStatus result = MIDIClientCreate( CFSTR("OSW MIDI Input Client"), NULL,
                                        NULL, &s_client);
    if (result != 0) {
      osw::Error("Error creating MIDI client");
    } else {
      CreateMidiInDrivers();
      CreateMidiOutDrivers();
    }
  }
  
  void CleanupMidi () {
    DestroyMidiInDrivers();
    DestroyMidiOutDrivers();
    OSStatus result = MIDIClientDispose(s_client);
    if (result != 0) {
      osw::Error("Error destroying MIDI client");
    }
    
  }

} // namespace osw
  






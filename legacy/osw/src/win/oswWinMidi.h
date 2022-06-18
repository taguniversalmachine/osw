/* 
Copyright (c) 2001 Amar Chaudhary. All rights reserved.
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
  oswWinMidi.h
  MIDI drivers for Windows
*/

#ifndef _WIN32
#define _WIN32
#endif

#include <windows.h>
#include <mmsystem.h>
#include "oswmidi.h"

void CALLBACK midiInProc(
			   HMIDIIN hMidiIn,	
			   UINT wMsg,	
			   DWORD dwInstance,	
			   DWORD dwParam1,	
			   DWORD dwParam2	
			   );

void CALLBACK midiOutProc(
			   HMIDIOUT hMidiOut,	
			   UINT wMsg,	
			   DWORD dwInstance,	
			   DWORD dwParam1,	
			   DWORD dwParam2	
			   );


namespace osw {

  class Win32MidiOutputDriver : public MidiOutputDriver {

  public:

    Win32MidiOutputDriver (const string &aname, Container *acontainer,
			  const string &aFullName, int aport) :
      MidiOutputDriver (aname,acontainer,aFullName),
      xport (aport) {
      xisOpen = false;
      xMidiHdr.lpData = NULL;
      xMidiHdr.dwBufferLength = 0;
    }

    ~Win32MidiOutputDriver () {
      if (xisOpen) {
	CloseDriver();
      }
    }

    virtual void SendMidiMessage (byte channel, byte status, 
				  byte first, byte second);
    virtual void SendMidiSysex (void *buffer, int size);
    virtual void SendMidiClock ();

  protected:

    int xport;
    HMIDIOUT xhMidiOut;
    MIDIHDR  xMidiHdr;

#ifdef _MSC_VER
    friend void CALLBACK ::midiOutProc(
				     HMIDIOUT hmo,      
				     UINT wMsg,         
				     DWORD dwInstance,  
				     DWORD dwParam1,    
				     DWORD dwParam2     
				     );
#else
    friend void ::midiOutProc(
			    HMIDIOUT hmo,      
			    UINT wMsg,         
			    DWORD dwInstance,  
			    DWORD dwParam1,    
			    DWORD dwParam2    
			    );
#endif
    
    virtual void OpenDriver  ();
    virtual void CloseDriver ();
    bool xisOpen;
  };


  class Win32MidiInputDriver : public MidiInputDriver {

#ifdef _MSC_VER    
    friend void CALLBACK ::midiInProc(
				      HMIDIIN hMidiIn,	
				      UINT wMsg,	
				      DWORD dwInstance,	
				      DWORD dwParam1,	
				      DWORD dwParam2	
				      );
#else
    friend void midiInProc(
			   HMIDIIN hMidiIn,	
			   UINT wMsg,	
			   DWORD dwInstance,	
			   DWORD dwParam1,	
			   DWORD dwParam2	
			   );
#endif
    
  public:
    
    Win32MidiInputDriver (const string &aname, Container *acontainer,
			  const string &aFullName, int aport);

    ~Win32MidiInputDriver () {
      if (xisOpen) {
	CloseDriver();
      }
    }
    
  protected:
    
    int xport;
    HMIDIIN xhMidiIn;

    virtual void OpenDriver  ();
    virtual void CloseDriver ();
    bool xisOpen;
  };


} // namespace osw




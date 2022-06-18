
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
  oswmidi.h
  Declarations for MIDI I/O transforms and drivers
*/  


#include "osw.h"


using namespace std;

namespace osw {

  class _oswexport MidiInputTransform : public Transform {

  public:

    static const int AllChannels;
    static const int AllStatus;
    static const int AllParameters;

    State<int>    statusFilter, channelFilter, paramFilter;
    State<string> statusString,driverName;
    Outlet<int>   statusOut, channelOut, value1Out, value2Out;

    MidiInputTransform (const string &aname, Container *acontainer, 
			int argc, char *argv[]);

    ~MidiInputTransform ();

    string PersistentState () const {
      return " driver channel status parameter ";
    }
    
    const char *Dialog () {
      return " oswLoadScript midiInSelect.tcl \n"
	"midiInSelect $transformPath \n"
	;
    }

    const char *Description () const {
      return "Receives and filters incoming MIDI messages according to "
	"selected channels, status or parameters.";
    }

  protected:

    Activation<MidiInputTransform> activation1,activation2;

    friend class MidiInputDriver;
    
    void Nullify () {
      xdriver = NULL;
    }

    void FireMidiEvent (int achannel, int astatus, int aval1, int aval2);
    void FireMidiClock ();
    
    MidiInputDriver *xdriver;

    void activation1Expr();
    void activation2Expr();
  };


  class _oswexport MidiInputDriver : public Container {
    
  public:

    State<string>  FullName;

    MidiInputDriver (const string &aname, Container *acontainer, 
		     const string &aFullName) :
      Container (aname,acontainer),
      FullName ("full_name",this,"Full Driver Name",aFullName) {
	xdriverOpen = false;
    }

    virtual ~MidiInputDriver ();

    void AddTransform (MidiInputTransform *toAdd) {
      xtransforms.push_back(toAdd);
      if (!xdriverOpen) {
	OpenDriver();
	xdriverOpen = true;
      }
    }

    void RemoveTransform (MidiInputTransform *toRemove) {
      xtransforms.erase(remove(xtransforms.begin(),xtransforms.end(),toRemove),
			xtransforms.end());
      if (xtransforms.size() == 0) {
	CloseDriver();
      }
    }

    void FireMidiEvents (int achannel, int astatus, int aval1, int aval2);

  protected:

    typedef stl::list<MidiInputTransform *> TransformList;

    TransformList xtransforms;
    bool xdriverOpen;

    virtual void OpenDriver  () = 0;
    virtual void CloseDriver () = 0;

  };

  /*************************/

  class _oswexport MidiOutputTransform;
  class _oswexport SysexOutputTransform;
  class _oswexport MidiClockOutputTransform;

  class _oswexport MidiOutputDriver : public Container {
    
  public:

    State<string>  FullName;

    MidiOutputDriver (const string &aname, Container *acontainer, 
		     const string &aFullName) :
      Container (aname,acontainer),
      FullName ("full_name",this,"Full Driver Name",aFullName) {
	xdriverOpen = false;
    }

    virtual ~MidiOutputDriver ();

    void AddTransform (MidiOutputTransform *toAdd);
    void RemoveTransform (MidiOutputTransform *toRemove);
    void AddTransform (SysexOutputTransform *toAdd);  
    void RemoveTransform (SysexOutputTransform *toRemove);
    void AddTransform (MidiClockOutputTransform *toAdd);  
    void RemoveTransform (MidiClockOutputTransform *toRemove);

    virtual void SendMidiMessage(byte channel, byte status, 
				 byte first, byte second) = 0;
    virtual void SendMidiSysex(void *sysex, int size);
    virtual void SendMidiClock();

  protected:

    typedef stl::list<Transform *> TransformList;

    TransformList xtransforms;
    bool xdriverOpen;

    virtual void OpenDriver  () = 0;
    virtual void CloseDriver () = 0;

  };

  // Implement the functions once for each platform

  void InitMidi();
  void CleanupMidi();
  
} // namespace osw








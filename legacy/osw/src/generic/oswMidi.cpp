/* 
Copyright (c) 2001-2004 Amar Chaudhary. All rights reserved.
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
  oswmidi.cpp
  Implementation of MIDI I/O transforms and generic drivers
*/  

#include "oswMidi.h"
#include "MidiMessage.h"


#define ACTIVE_SENSE 0xFE


namespace osw {

using namespace std;

const int MidiInputTransform::AllChannels = -1;
const int MidiInputTransform::AllStatus = -1;
const int MidiInputTransform::AllParameters = -1;

struct StatusTypes {
  char *statusName;
  int  statusNum;
};

static StatusTypes statusTypes[] = {
  {"noteoff",0x8},
  {"note",0x9},
  {"noteon",0x9},
  {"polyaftertouch",0xA},
  {"poly",0xA},
  {"control",0xB},
  {"program",0xC},
  {"aftertouch",0xD},
  {"pitchbend",0xE},
  {"all",-1},
  {NULL,0L}
};



MidiInputTransform::MidiInputTransform (const string &aname,
					     Container *acontainer,
					     int argc, char *argv[]) :
  Transform(aname,acontainer,argc,argv),
  driverName("driver",this,"MIDI input driver.",
	     ScanArguments<string>("-driver","unknown",
				   acontainer,argc,argv)),
  channelFilter ("channel",this,"MIDI channel filter.",AllChannels),
  statusFilter ("statusAsInt",this,"MIDI status filter (number).", AllStatus),
  paramFilter ("parameter",this,"MIDI parameter filter.",AllParameters),
  statusString ("status",this,"MIDI status filter.",
		ScanArguments<string>("-status","all",
				      acontainer,argc,argv)),
  channelOut ("channelOut",this,"MIDI channel.",0),
  statusOut ("statusOut",this,"MIDI status.",0),
  value1Out ("firstOut",this,"First value byte.",0),
  value2Out ("secondOut",this,"Second value byte.",0),
  
  activation1(&statusString,this,&MidiInputTransform::activation1Expr),
  activation2(&driverName,this,&MidiInputTransform::activation2Expr) {

  xdriver = dynamic_cast<MidiInputDriver *>
    (Nameable::Find(driverName));
  
  if (xdriver) {
    xdriver->AddTransform(this);
  }
    
  for (int i = 0; i < argc; ++i) {
    if (!strcmp (argv[i],"-channel")) {
      if (i + 1 < argc) {
	++i;
	if (!strcmp (argv[i],"all")) {
	  channelFilter = AllChannels;
	} else {
	  channelFilter = atoi(argv[i]);
	}
      }
    } else if (!strcmp (argv[i],"-parameter")) {
      if (i + 1 < argc) {
	++i;
	if (!strcmp (argv[i],"all")) {
	  paramFilter = AllParameters;
	} else {
	  paramFilter = atoi(argv[i]);
	}
      }
    }
  }

#ifdef OSW_LAME_FOR_LOOP  
  for (i = 0; statusTypes[i].statusName != NULL; ++i) {
#else
  for (int i = 0; statusTypes[i].statusName != NULL; ++i) {
#endif
    if (statusString == string(statusTypes[i].statusName)) {
      statusFilter = statusTypes[i].statusNum;
      break;
    }
  }
  //cout << PathName() << ": Status Filter " << statusFilter << endl;

  channelOut.SetOrder(0);
  statusOut.SetOrder(1);
  value1Out.SetOrder(2);
  value2Out.SetOrder(3);

}

MidiInputTransform::~MidiInputTransform () {

  if (xdriver) {
    xdriver->RemoveTransform(this);
  }
}

void MidiInputTransform::FireMidiEvent (int achannel, int astatus, 
					     int aval1, int aval2) {
  if (channelFilter == AllChannels || achannel == channelFilter) {
    if (statusFilter == AllStatus || astatus == statusFilter) {
      if (paramFilter == AllParameters || aval1 == paramFilter) {
	channelOut = achannel;
	statusOut = astatus;
	value1Out = aval1;
	value2Out = aval2;
      }
    }
  }
}

/************************/

void MidiInputTransform::activation1Expr () {

  for (int i = 0; statusTypes[i].statusName != NULL; ++i) {
    if (!strnicmp(statusTypes[i].statusName,statusString->c_str(),
		  strlen(statusTypes[i].statusName))) {
      statusFilter = statusTypes[i].statusNum;
      break;
    }
  }
}

void MidiInputTransform::activation2Expr () {
  if (xdriver) {
    if (xdriver->PathName() == driverName) {
      return;
    }
    xdriver->RemoveTransform(this);
  }
  xdriver = dynamic_cast<MidiInputDriver *>
    (Nameable::Find(driverName));
  
  if (xdriver) {
    xdriver->AddTransform(this);
  }
}

/************************/

static TransformFactory<MidiInputTransform> IntToFloatFactory ("MidiInput");


/*************************/

MidiInputDriver::~MidiInputDriver () {

  for (TransformList::iterator p = xtransforms.begin();
       p != xtransforms.end();
       ++p) {
    if (*p != NULL) {
      (*p)->Nullify ();
    }
  }
}

void MidiInputDriver::FireMidiEvents (int achannel, int astatus, 
					   int aval1, int aval2) {

  if (achannel == 0xE && astatus == 0xF) {
    return;
  }
  if (achannel == 0x8 && astatus == 0xF) {
    return;
  }

  if (astatus == 0x9 && aval2 == 0) {
    // this is really a Note Off event
    astatus = 0x8;
  }

  for (TransformList::iterator p = xtransforms.begin();
       p != xtransforms.end();
       ++p) {
    if (*p != NULL) {
      (*p)->FireMidiEvent(achannel,astatus,aval1,aval2);
    }
  }
}

/********************/

class _oswexport MidiOutputTransform : public Transform {

public:


  State<string>       driverName;
  Inlet<MidiMessage>  message;
  

  MidiOutputTransform (const string &aname, Container *acontainer, 
		       int argc, char *argv[]) :
    Transform(aname,acontainer,argc,argv),
    driverName("driver",this,"MIDI input driver.",
	       ScanArguments<string>("-driver","unknown",
				     acontainer,argc,argv)),    
    message("message",this,"MIDI message to send."),
    activation1(&message,this,&MidiOutputTransform::activation1Expr),
    activation2(&driverName,this,&MidiOutputTransform::activation2Expr){
      xdriver = dynamic_cast<MidiOutputDriver *>
	(Nameable::Find(driverName));
      if (xdriver) {
	xdriver->AddTransform(this);
      }            
  }

  ~MidiOutputTransform () {
    if (xdriver) {
      xdriver->RemoveTransform(this);
    }
  }
  
  string PersistentState () const {
    return " driver ";
  }
  
  const char *Dialog () {
    return " oswLoadScript midiOutSelect.tcl \n"
      "midiOutSelect $transformPath \n"
      ;
  }
  
  const char *Description () const {
    return "Sends MIDI messages to selected output device.";
  }
  
protected:

  Activation<MidiOutputTransform> activation1,activation2;
  
  friend class MidiOutputDriver;
  
  void Nullify () {
    xdriver = NULL;
  }
  
  MidiOutputDriver *xdriver;
  
  void activation1Expr() {
    if (xdriver) {
      xdriver->SendMidiMessage(message->channel,message->status,
			       message->first,message->second);
    }
  }

  void activation2Expr() {
    if (xdriver) {
      xdriver->RemoveTransform(this);
    }
    xdriver = dynamic_cast<MidiOutputDriver *>
      (Nameable::Find(driverName));
    
    if (xdriver) {
      xdriver->AddTransform(this);
    }
  }
};



static TransformFactory<MidiOutputTransform> MidiOutputFactory  ("MidiOutput");

REGISTER_TYPE(MidiMessage);



/*****************************/

 class SysexOutputTransform : public Transform {
   
   State<string>       driverName;
   Inlet<Raw>          message;
   
 public:
   
   SysexOutputTransform (const string &aname,
			 Container *acontainer,
			 int argc,
			 char *argv[]) :
     Transform(aname,acontainer,argc,argv),
     driverName("driver",this,"MIDI input driver.",
		ScanArguments<string>("-driver","unknown",
				      acontainer,argc,argv)),    
     message("message",this,"MIDI system-exclusive message to send."),
     activation1(&message,this,&SysexOutputTransform::activation1Expr),
     activation2(&driverName,this,&SysexOutputTransform::activation2Expr){
     xdriver = dynamic_cast<MidiOutputDriver *>
       (Nameable::Find(driverName));
     if (xdriver) {
       xdriver->AddTransform(this);
     }            
   }
   
   ~SysexOutputTransform () {
     if (xdriver) {
       xdriver->RemoveTransform(this);
     }
   }
   
   string PersistentState () const {
     return " driver ";
   }
   
   const char *Dialog () {
     return " oswLoadScript midiOutSelect.tcl \n"
       "midiOutSelect $transformPath \n"
       ;
   }
   
   const char *Description () const {
     return "Sends MIDI system-exclusive messages to selected output device.";
   }
   
 protected:
   
   Activation<SysexOutputTransform> activation1,activation2;
   
   friend class MidiOutputDriver;
   
   void Nullify () {
     xdriver = NULL;
   }
   
   MidiOutputDriver *xdriver;
   
   void activation1Expr() {
     if (xdriver) {
       xdriver->SendMidiSysex(message->begin(),message->size());
     }
   }
   
   void activation2Expr() {
     if (xdriver) {
       xdriver->RemoveTransform(this);
     }
     xdriver = dynamic_cast<MidiOutputDriver *>
       (Nameable::Find(driverName));
     
     if (xdriver) {
       xdriver->AddTransform(this);
     }
   }
 };
 
 static TransformFactory<SysexOutputTransform> SysexOutputFactory  ("SysexOutput");

/*****************************/

 class MidiClockOutputTransform : public Transform {
   
   State<string>       driverName;
   Inlet<Unit>         tick;
   
 public:
   
   MidiClockOutputTransform (const string &aname,
			 Container *acontainer,
			 int argc,
			 char *argv[]) :
     Transform(aname,acontainer,argc,argv),
     driverName("driver",this,"MIDI input driver.",
		ScanArguments<string>("-driver","unknown",
				      acontainer,argc,argv)),    
     tick("tick",this,"Send MIDI clock message."),
     activation1(&tick,this,&MidiClockOutputTransform::activation1Expr),
     activation2(&driverName,this,&MidiClockOutputTransform::activation2Expr){
     xdriver = dynamic_cast<MidiOutputDriver *>
       (Nameable::Find(driverName));
     if (xdriver) {
       xdriver->AddTransform(this);
     }            
   }
   
   ~MidiClockOutputTransform () {
     if (xdriver) {
       xdriver->RemoveTransform(this);
     }
   }
   
   string PersistentState () const {
     return " driver ";
   }
   
   const char *Dialog () {
     return " oswLoadScript midiOutSelect.tcl \n"
       "midiOutSelect $transformPath \n"
       ;
   }
   
   const char *Description () const {
     return "Sends MIDI Clock messages to selected output device.";
   }
   
 protected:
   
   Activation<MidiClockOutputTransform> activation1,activation2;
   
   friend class MidiOutputDriver;
   
   void Nullify () {
     xdriver = NULL;
   }
   
   MidiOutputDriver *xdriver;
   
   void activation1Expr() {
     if (xdriver) {
       xdriver->SendMidiClock();
     }
   }
   
   void activation2Expr() {
     if (xdriver) {
       xdriver->RemoveTransform(this);
     }
     xdriver = dynamic_cast<MidiOutputDriver *>
       (Nameable::Find(driverName));
     
     if (xdriver) {
       xdriver->AddTransform(this);
     }
   }
 };
 
 static TransformFactory<MidiClockOutputTransform> MidiClockOutputFactory  ("MidiClockOutput");

/********************/


MidiOutputDriver::~MidiOutputDriver () {

  for (TransformList::iterator p = xtransforms.begin();
       p != xtransforms.end();
       ++p) {
    if (*p != NULL) {
      if (dynamic_cast<MidiOutputTransform *>(*p)) {
	(dynamic_cast<MidiOutputTransform *>(*p))->Nullify ();
      } else if (dynamic_cast<SysexOutputTransform *>(*p)) {
	(dynamic_cast<SysexOutputTransform *>(*p))->Nullify ();
      } else if (dynamic_cast<MidiClockOutputTransform *>(*p)) {
	(dynamic_cast<MidiClockOutputTransform *>(*p))->Nullify ();
      }
    }
  }
}

 void MidiOutputDriver::AddTransform (MidiOutputTransform *toAdd) {
   xtransforms.push_back(toAdd);
   if (!xdriverOpen) {
     OpenDriver();
     xdriverOpen = true;
   }
 }
 
 void MidiOutputDriver::RemoveTransform (MidiOutputTransform *toRemove) {
   xtransforms.erase(remove(xtransforms.begin(),xtransforms.end(),toRemove),
		     xtransforms.end());
   if (xtransforms.size() == 0) {
     CloseDriver();
   }
 }
 
 void MidiOutputDriver::AddTransform (SysexOutputTransform *toAdd) {
   xtransforms.push_back(toAdd);
   if (!xdriverOpen) {
     OpenDriver();
     xdriverOpen = true;
   }
 }
 
 void MidiOutputDriver::RemoveTransform (SysexOutputTransform *toRemove) {
   xtransforms.erase(remove(xtransforms.begin(),xtransforms.end(),toRemove),
		     xtransforms.end());
   if (xtransforms.size() == 0) {
     CloseDriver();
   }
 }
 

 void MidiOutputDriver::AddTransform (MidiClockOutputTransform *toAdd) {
   xtransforms.push_back(toAdd);
   if (!xdriverOpen) {
     OpenDriver();
     xdriverOpen = true;
   }
 }
 
 void MidiOutputDriver::RemoveTransform (MidiClockOutputTransform *toRemove) {
   xtransforms.erase(remove(xtransforms.begin(),xtransforms.end(),toRemove),
		     xtransforms.end());
   if (xtransforms.size() == 0) {
     CloseDriver();
   }
 }

 void MidiOutputDriver::SendMidiSysex (void *, int) {
   // hopefully implemented in subclasses
   Error("Sysex not supported");
 }

 void MidiOutputDriver::SendMidiClock () {
   // hopefully implemented in subclasses
   Error("MIDI Clock not supported");
 }



/********************/
}

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
  oswthread.h
  Implementation of class for managing multiple threads in OSW

  Amar Chaudhary

  6/27/2002 Added patches for soft realtime Linux by Andrew W. Schmeder
  
*/

#ifdef _USING_TCL
#include <tcl.h>
#endif
#include "oswThread.h"
#include "oswSched.h"
#include "oswSelect.h"
#include "oswSock.h"

using namespace std;

// I really dislike this, but I see no other way to deal with
// callback-based MIDI in Windows
#ifdef _WINDOWS
extern void HandleMidiEvents ();
#endif

namespace osw {

  Container ThreadContainer ("thread");
  static bool xquit = false;

  State<int> net_granularity("net_granularity",NULL,"Controls the amount of time reserved for net operations",8);

#ifdef OSW_USE_ITOA
  static char s_threadnumBuffer[32];
#endif

  Thread::Thread (int athreadnum) :
#ifdef OSW_USE_ITOA
    Container (itoa(athreadnum,s_threadnumBuffer,10), &ThreadContainer) {
#else
    Container (((ostringstream &)(ostringstream() << athreadnum)).str(), 
                &ThreadContainer) {
#endif
  }
 
  Thread::Thread (const string &athreadname) :
    Container (athreadname, &ThreadContainer) {
  }
  
  static Container overhead("overhead");

  /*
   * Loop used by Windows and SGI
   */   
  void Thread::Loop () {
    int net = 0;
    ProfileOn(&overhead);
    while (!xquit) {
      while (TheParallelScheduler->NothingToDo() && !xquit) {
	++net;
	if (net == net_granularity) {
	  CheckSelectEvents();
#ifdef _WINDOWS
	  HandleMidiEvents();
#endif
	  net = 0;
	}
      }
      BaseActivation *nextActivation = 
	TheParallelScheduler->GetNextActivation ();
      if (nextActivation == NULL) {
	ProfileOff(&overhead);
	DoSomethingUseful();
	ProfileOn(&overhead);
      } else {
	ProfileOff(&overhead);
	//ProfileOn(nextActivation->GetContainerFast());
	(*nextActivation)();
	//ProfileOff(nextActivation->GetContainerFast());
	ProfileOn(&overhead);
      }
      ++net;
      if (net == net_granularity) {
	CheckSelectEvents();
#ifdef _WINDOWS
	HandleMidiEvents();
#endif
	net = 0;
      }
    }
    ProfileOff(&overhead);
  }

  /*
   * Watch activity of other threads
   * Eventually watchdog should check to see if other threads are running out of control and force OSW to exit.
   * This will handle the situation where an infinite loop freezes the machine (a potential side effect of using SCHED_FIFO mode).
   */
  void Thread::WatchdogLoop() {
  	//SetDescription("This thread is a watchdog which ensures other threads are running properly.");
	void *heartbeat = OpenUDPSocket("255.255.255.255", 7706);
	int stat_data[2];
	while(!xquit) {
#ifndef _WINDOWS
		sleep(10);
#else
		SleepEx(10000,FALSE);
		// Add something for Windows here
#endif
#ifndef _WINDOWS
		//TheParallelScheduler->Lock();
#endif
  		TheParallelScheduler->PrintStats();
                stat_data[0] = TheParallelScheduler->GetEventsDone();
                stat_data[1] = TheParallelScheduler->GetSelectInts();
                UDPSocketSend(heartbeat, (char*)stat_data, sizeof(stat_data));
#ifndef _WINDOWS
  		//TheParallelScheduler->Release();
#endif
  	}
  }

  /*
   * The TaskLoop() throttles CPU usage by using BlockingCheckSelectEvents()
   * which will block for up to 10 milliseconds.
   *
   * This model will be refined in the future!
   */
  void Thread::TaskLoop() {
    TheParallelScheduler->Lock();
    while(!xquit) {
      try {
        // While there is something to do...
        while((! TheParallelScheduler->NothingToDo()) && (! xquit)) {
          BaseActivation *nextActivation = TheParallelScheduler->RawGetNextActivation ();
          TheParallelScheduler->Release();
          if (nextActivation == NULL) {  // This should never happen.
            Alert("LazyLoop Recieved NULL Activation.");
          } else {
             (*nextActivation)();
          }
          TheParallelScheduler->Lock();
        }
        DoSomethingUseful();
        TheParallelScheduler->IncrementIdle();
#ifdef _WINDOWS
	HandleMidiEvents();
#endif
        BlockingCheckSelectEvents();
        TheParallelScheduler->Release();
        SelectEventsDispatch();
        TheParallelScheduler->Lock();
        TheParallelScheduler->DecrementIdle();
        TheParallelScheduler->IncrementSelect();
      }
      catch(Exception *e) {
        Error(e->GetMessage());
      }
      catch(const std::exception &e) {
	Error(e.what());
      }
      catch(...) {
        // We are going to try to continue running...  but things may not be okay!
        Error("A fatal error has occured!!!");
	//exit(-1);
      }
    }
    TheParallelScheduler->Release();
  }

  /*
   * Experimental: This loop is used when placing TCL in its own thread.
   * The problem with this is that TCL will corrupt its internal state (something is not thread-safe??)
   * Not currently in use.
   */
  void Thread::TclLoop() {
    // Do the UI thing... hopefully this is fast.... uh-oh, its not!
    while(! xquit) {
      Tcl_DoOneEvent(TCL_ALL_EVENTS | TCL_DONT_WAIT);
#ifndef _WINDOWS
      usleep(1000);
#endif
    }
  }

  // Try running Tcl loop in the "main" thread (i.e., the one that
  // initiated tcl) [AC]
  void TclLoop() {
    // Do the UI thing... hopefully this is fast.... uh-oh, its not!
    while(! xquit) {
      Tcl_DoOneEvent(TCL_ALL_EVENTS);
#ifndef _WINDOWS
      usleep(1000);
#endif
    }
  }

  State<int> ui_granularity("ui_granularity",NULL,"Controls the amount of time reserved for UI operations",20);

  
  void DoOneEvent () {
    static int net = 0;
    //Alert("Do One Event");
    BaseActivation *nextActivation = 
      TheParallelScheduler->GetNextActivation ();
    if (nextActivation == NULL) {
      DoSomethingUseful();
    } else {
      //ProfileOn(nextActivation->GetContainerFast());
      (*nextActivation)();
      //ProfileOff(nextActivation->GetContainerFast());
    }
    ++net;
    if (net == net_granularity) {
#ifdef _WINDOWS
      HandleMidiEvents();
#endif
      CheckSelectEvents();
      net = 0;
    }
  }

  void UILoop () {

    int net=0;
    ProfileOn(&overhead);
    while (!xquit) {
      while (TheParallelScheduler->NothingToDo() && !xquit) {
#ifdef _USING_TCL
	Tcl_DoOneEvent(TCL_ALL_EVENTS);	
#endif
	++net;
	if (net == net_granularity) {
#ifdef _WINDOWS
	  HandleMidiEvents();
#endif
	  CheckSelectEvents();
	  net = 0;
	}
      }
      for (int ui = ui_granularity; ui; --ui) {
	BaseActivation *nextActivation = 
	  TheParallelScheduler->GetNextActivation ();
	if (nextActivation == NULL) {
	  ProfileOff(&overhead);
	  DoSomethingUseful();
	  ProfileOn(&overhead);
	} else {
	  ProfileOff(&overhead);
	  //ProfileOn(nextActivation->GetContainerFast());
	  (*nextActivation)();
	  //ProfileOff(nextActivation->GetContainerFast());
	  ProfileOn(&overhead);
	}
	++net;
	if (net == net_granularity) {
#ifdef _WINDOWS
	  HandleMidiEvents();
#endif
	  CheckSelectEvents();
	  net = 0;
	}
      }
#ifdef _USING_TCL
      Tcl_DoOneEvent(TCL_ALL_EVENTS | TCL_DONT_WAIT);
#endif
    }
    ProfileOff(&overhead);
  }

  void Quit () {
    //StopThreads();
    xquit = true;
  }
  
}


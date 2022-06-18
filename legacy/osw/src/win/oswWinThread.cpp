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
  winthread.cpp
  Windows-specific thread class and routines
*/

#include <windows.h>
#include "oswthread.h"
#include "oswPrefs.h"
#include <vector>

using namespace std;

namespace osw {

  DWORD WINAPI MainThreadProc (LPVOID threadAsVoid);

  class Win32Thread : public Thread {

  public:

    Win32Thread (int athreadnum, 
		 DWORD (WINAPI *threadProc)(LPVOID threadAsVoid) 
		   = MainThreadProc) :
      Thread (athreadnum) {

	xhThread = CreateThread(NULL,0,threadProc,this,0,&xthreadID);
    }

    ~Win32Thread () {
      if (WaitForSingleObject(xhThread,5000) != WAIT_OBJECT_0) {
	TerminateThread(xhThread,0L);
      }
      CloseHandle(xhThread);
    }

    void Run () {
    }

    void Wait () {
      WaitForSingleObject(xhThread,INFINITE);
    }

    void Join () {
      WaitForSingleObject(xhThread,INFINITE);
    }

    bool IAmRunning () {
      return xthreadID == GetCurrentThreadId();
    }

  private:

    HANDLE xhThread;
    DWORD xthreadID;
  };

  /*************/

  DWORD WINAPI MainThreadProc (LPVOID threadAsVoid) {    
    cout << "Starting Thread\n";
    ((Win32Thread *) threadAsVoid)->TaskLoop();
    return 0L;
  }

  DWORD WINAPI TclThreadProc (LPVOID threadAsVoid) {
    cout << "Starting Tcl Thread\n";
    ((Win32Thread *) threadAsVoid)->TclLoop();
    return 0L;
  }

  DWORD WINAPI WatchdogThreadProc (LPVOID threadAsVoid) {
    cout << "Starting Watchdog\n";
    ((Win32Thread *) threadAsVoid)->WatchdogLoop();
    return 0L;
  }

  /*************/

  ThreadID _oswexport GetThreadID () {
    return (ThreadID) GetCurrentThreadId();
  }

  bool _oswexport IsThreadIDRunning (ThreadID id) {
    return (HANDLE)id == (HANDLE) GetCurrentThreadId();
  }

  
  /*************/

  static std::vector<Win32Thread *> threads;

  void InitializeThreads (bool ui) {

    // This is bad news!
    HANDLE hProcess = GetCurrentProcess();
    //if (!SetPriorityClass (hProcess,HIGH_PRIORITY_CLASS)) {
    //	cerr << "Could not set process to real-time\n";
    //}

    SYSTEM_INFO sysinfo;

    GetSystemInfo(&sysinfo);
    sysinfo.dwNumberOfProcessors = 1;
    threads.reserve(sysinfo.dwNumberOfProcessors);
    cout << sysinfo.dwNumberOfProcessors << " processors.\n";
    for (int i = 0; i < sysinfo.dwNumberOfProcessors; ++i) {
      threads.push_back(new Win32Thread(i));
    }
    String watchdog;
    GetPreference("Watchdog",watchdog,"0");
    if (watchdog == "0") {
      threads.push_back(new Win32Thread(1000,WatchdogThreadProc));
    }
    if (ui) {
      TclLoop();
      //threads.push_back(new Win32Thread(1001,TclThreadProc));
      //threads[threads.size()-1]->Join();
    } else {
      // join the watchdog thread
      threads[threads.size()-1]->Join();
    }
  }

  void StopThreads () {

    SetPriorityClass (GetCurrentProcess(),NORMAL_PRIORITY_CLASS);
    for (int i = 1; i < threads.size(); ++i) {
      delete threads[i];
    }

  }

  void Sleep () {
    SleepEx(0,TRUE);
  }

}

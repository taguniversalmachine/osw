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

/* oswLinuxThread.cpp

   Linux thread implementation
   
   Amar Chaudhary

   6/27/2002 Added patches by Andrew W. Schmeder
*/



#include "oswThread.h"
#include "oswSched.h"
#include "oswSelect.h"
#include "oswPrefs.h"
#include "oswArgs.h"
#include <pthread.h>
#include <unistd.h>

// Includes for RT priority setting -aws

#include <sched.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>
 
using namespace std;

namespace osw {

  void *WatchdogThreadProc (void *threadAsVoid);
  void *TaskEventThreadProc (void *threadAsVoid);
  void *TclThreadProc (void *threadAsVoid);
  
  int linux_thread_priority_base = 50;
  int linux_task_threads = -1;
  
  class LinuxThread : public Thread {

  public:

    LinuxThread (const string &athreadname, int sched_mode, int priority, void * ThreadProc(void *)) :
      Thread (athreadname) {

      struct sched_param rtp;
      int policy, x;
      pthread_attr_t  attr;

      pthread_attr_init (&attr);
      pthread_attr_setscope (&attr,PTHREAD_SCOPE_PROCESS);

      pthread_create(&xthread,&attr,ThreadProc,(void *) this);
      //pthread_detach(xthread);
      
      // Get the current priority structure
      pthread_getschedparam(xthread, &policy, &rtp);

      // TODO: Get priority from preferences
      //priority = 99;
      rtp.sched_priority = priority;

      // Set thread priority to the given number
      // If the linux kernel is appropriately patched,
      // the kernel will aggressively break locks to give more time to high priority threads.
      fprintf(stderr, "Task Process setting priority level %d... ", priority);
      x = pthread_setschedparam(xthread, sched_mode, &rtp);
      fprintf(stderr, "%d : %s.\n", x, strerror(x));

    }

    ~LinuxThread () {
      pthread_cancel(xthread);
    }

    void Run () {
    }

    void Wait () {
      // don't need it at the moment
    }

    void Join () {
    	int status;
    	// Blocks until the thread terminates via pthread_cancel / pthread_exit.
    	pthread_join(xthread, (void **)&status);
    }

    bool IAmRunning () {
      return xthread == pthread_self();
    }

  private:

    pthread_t  xthread;
  };

  /*************/

  void *WatchdogThreadProc (void *threadAsVoid) {
    cout << "Starting Thread\n";
    ((LinuxThread *) threadAsVoid)->WatchdogLoop();
    return NULL;
  }
  void *TaskThreadProc (void *threadAsVoid) {
    cout << "Starting Thread\n";
    ((LinuxThread *) threadAsVoid)->TaskLoop();
    return NULL;
  }
  void *TclThreadProc (void *threadAsVoid) {
    cout << "Starting Thread\n";
    ((LinuxThread *) threadAsVoid)->TclLoop();
    return NULL;
  }

  /*************/

  ThreadID _oswexport GetThreadID () {
    return (ThreadID) pthread_self();
  }

  bool _oswexport IsThreadIDRunning (ThreadID id) {
    return (pthread_t)id == pthread_self();
  }


  /*************/

  static std::vector<LinuxThread *> threads;

  extern State<int> ui_granularity;

  static bool s_ui_mode = false;

  void InitializeThreads (bool ui) {
    int i = 0;
    int pri = linux_thread_priority_base;
      
    // Number of CPUs online
    if(linux_task_threads == -1) {
        linux_task_threads = sysconf(_SC_NPROCESSORS_ONLN);
    }
    
    threads.reserve(1 + linux_task_threads);
    //cout << smp_num_cpus << " processors.\n";

    // Start watchdog thread
    threads.push_back(new LinuxThread("watchdog", SCHED_FIFO, pri, WatchdogThreadProc));
    
    // Start select thread
    char tname[64];
    for(i = 0; i < linux_task_threads; i++) {
      sprintf(tname, "task%d", i);
      threads.push_back(new LinuxThread(tname, SCHED_FIFO, pri-1, TaskThreadProc));
    }

    if (ui) {
      // Start TCL thread    
      //threads.push_back(new LinuxThread("tcl", SCHED_FIFO, 97, TclThreadProc));
      s_ui_mode = true;
    } else {
      // No UI, then no Tcl, join the watchdog thread
      Watch();
    }

  }

  void StopThreads () {
    for (int i = 0; i < threads.size(); ++i) {
      TheParallelScheduler->Lock();
      // don't delete yourself
      if (!threads[i]->IAmRunning()) {
        delete threads[i];
      }
      TheParallelScheduler->Release();
    }
  }

  void Sleep () {
    sched_yield();
  }
  
  void Watch () {
    // Watch the watchdog thread...
    Alert("Watch and Wait...");
    threads[0]->Join();
  }
  
  static bool SetThreadPriorityProc (int argc, char* argv[], int &index) {
    ++index;
    if (index >= argc) {
      Error("-p: must be followed by an integer.");
      return true;
    }
    linux_thread_priority_base = atoi(argv[index]);
    return false;
  }
  
  static CommandLineArg s_setLinuxThreadPriority("-p", "--priority", "Set max priority for POSIX SCHED_FIFO task threads.", SetThreadPriorityProc);

  static bool SetThreadCountProc (int argc, char* argv[], int &index) {
    ++index;
    if (index >= argc) {
      Error("-tt: must be followed by an integer.");
      return true;
    }
    linux_task_threads = atoi(argv[index]);
    if(linux_task_threads < 0) {
      Error("-tt: must be followed by a positive integer.");
      return true;
    }
    return false;
  }
  
  static CommandLineArg s_setLinuxThreadCount("-tt", "--task-threads", "Set number of task threads to use (default is num-cpus).", SetThreadCountProc);
  
}

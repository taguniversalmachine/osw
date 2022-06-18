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

/* oswMacOSXThread.cpp

   MacOSX thread implementation
   
   Amar Chaudhary
*/



#include "oswThread.h"
#include "oswSched.h"
#include "oswSelect.h"
#include "oswArgs.h"
#include <pthread.h>
#include <unistd.h>

// Includes for RT priority setting -aws

#include <sched.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <errno.h>
 
// RT priority stuff for Mac OS X (AC)

#include <mach/mach.h>
#include <mach/mach_error.h>
#include <mach/mach_time.h>
#include <mach/mach_init.h>
#include <mach/task_policy.h>
#include <mach/thread_act.h>
#include <mach/thread_policy.h>
#include <sys/sysctl.h>

#define FEEDER_THREAD_IMPORTANCE 6

using namespace std;

namespace osw {

  int macosx_thread_priority_base = 80;
  int macosx_task_threads = -1;
 

  // sets real-time priority for a thread
  int get_bus_speed()
  {
    int mib[2];
    unsigned int miblen;
    int busspeed;
    int retval;
    size_t len;
    
    mib[0]=CTL_HW;
    mib[1]=HW_BUS_FREQ;
    miblen=2;
    len=4;
    retval = sysctl(mib, miblen, &busspeed, &len, NULL, 0);
    return busspeed;
  }

  int set_realtime() {
    struct thread_time_constraint_policy ttcpolicy;
    int ret;    
    ttcpolicy.period= get_bus_speed() / 120; 
    ttcpolicy.computation=get_bus_speed() / 1440 ; 
    ttcpolicy.constraint=get_bus_speed() / 720; 
    ttcpolicy.preemptible=1;
    if ((ret=thread_policy_set(mach_thread_self(),
			       THREAD_TIME_CONSTRAINT_POLICY, 
			       (int *)&ttcpolicy,
			       THREAD_TIME_CONSTRAINT_POLICY_COUNT))
	!= KERN_SUCCESS) { 
      fprintf(stderr, "set_realtime() failed.\n");
      return 0;    
    }
    return 1;
  }



  void *WatchdogThreadProc (void *threadAsVoid);
  void *TaskEventThreadProc (void *threadAsVoid);
  void *TclThreadProc (void *threadAsVoid);
  
  class MacOSXThread : public Thread {

  public:

    MacOSXThread (const string &athreadname, int sched_mode, int priority, void * ThreadProc(void *)) :
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
      //priority = 63;
      int old_priority = rtp.sched_priority;
      rtp.sched_priority = priority;

      // Set thread priority to the given number
      // If the linux kernel is appropriately patched,
      // the kernel will aggressively break locks to give more time to high priority threads.
      fprintf(stderr, "Task Process setting priority level %d... ", priority);
      x = pthread_setschedparam(xthread, sched_mode, &rtp);
      if (x) {
	fprintf(stderr, "%d : %s.\n", x, strerror(x));
      }
      
      
      kern_return_t error;
      thread_extended_policy_data_t extendedPolicy;
      thread_precedence_policy_data_t precedencePolicy;
      
      mach_port_t my_mach_thread = pthread_mach_thread_np(xthread); 
      
      // make thread fixed
      extendedPolicy.timeshare = false;
      int result = thread_policy_set(my_mach_thread,THREAD_EXTENDED_POLICY,
				     (thread_policy_t)&extendedPolicy,THREAD_EXTENDED_POLICY_COUNT);
      if (result) {
	printf ("Error:%s=0x%lX,%ld,%s\n\n",		 						"Timeshare",result, result, (char*)&result);
      }
      precedencePolicy.importance = priority;
      result = thread_policy_set(my_mach_thread,THREAD_PRECEDENCE_POLICY,
				 (thread_policy_t) &precedencePolicy,THREAD_PRECEDENCE_POLICY_COUNT);
      if (result) {
	printf ("Error:%s=0x%lX,%ld,%s\n\n",		 						"Precendence",result, result, (char*)&result);
      }
      
      
    }

    ~MacOSXThread () {
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
    ((MacOSXThread *) threadAsVoid)->WatchdogLoop();
    return NULL;
  }
  void *TaskThreadProc (void *threadAsVoid) {
    cout << "Starting Thread\n";
    //set_realtime();
    ((MacOSXThread *) threadAsVoid)->TaskLoop();
    return NULL;
  }
  void *TclThreadProc (void *threadAsVoid) {
    cout << "Starting Thread\n";
    ((MacOSXThread *) threadAsVoid)->TclLoop();
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

  static std::vector<MacOSXThread *> threads;

  // No SMP for now!!
  static int smp_num_cpus = 1;

  extern State<int> ui_granularity;

  void InitializeThreads (bool ui) {
    int i = 0;
    int xsmp_num_cpus = smp_num_cpus;
    int pri = macosx_thread_priority_base;


    threads.reserve(2);
    //cout << smp_num_cpus << " processors.\n";

    // Start watchdog thread
    threads.push_back(new MacOSXThread("watchdog", SCHED_FIFO, pri, WatchdogThreadProc));
    
    // Start select thread
    threads.push_back(new MacOSXThread("task0", SCHED_FIFO, pri - 1, TaskThreadProc));
    
    
    if (ui) {
      // Start TCL thread (not, AC) 
      //threads.push_back(new MacOSXThread("tcl", SCHED_FIFO, 61, TclThreadProc));
      //Watch();
      thread_extended_policy_data_t extendedPolicy;
      // make thread fixed
      
      extendedPolicy.timeshare = false;      
      int result = thread_policy_set(mach_thread_self(),THREAD_EXTENDED_POLICY,
				     (thread_policy_t)&extendedPolicy,THREAD_EXTENDED_POLICY_COUNT);
      if (result) {
	printf ("Error:%s=0x%lX,%ld,%s\n\n",		 						"Timeshare",result, result, (char*)&result);
      }
      thread_precedence_policy_data_t precedencePolicy;
      precedencePolicy.importance = 0;
      result = thread_policy_set(mach_thread_self(),THREAD_PRECEDENCE_POLICY,
				 (thread_policy_t) &precedencePolicy,THREAD_PRECEDENCE_POLICY_COUNT);
      if (result) {
	printf ("Error:%s=0x%lX,%ld,%s\n\n",		 						"Precendence",result, result, (char*)&result);
      }
      
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
    macosx_thread_priority_base = atoi(argv[index]);
    return false;
  }
  
  static CommandLineArg s_setThreadPriority("-p", "--priority", "Set max priority for POSIX SCHED_FIFO task threads.", SetThreadPriorityProc);


#if 0 //not currently supported
  static bool SetThreadCountProc (int argc, char* argv[], int &index) {
    ++index;
    if (index >= argc) {
      Error("-tt: must be followed by an integer.");
      return true;
    }
    macosx_task_threads = atoi(argv[index]);
    if(macosx_task_threads < 0) {
      Error("-tt: must be followed by a positive integer.");
      return true;
    }
    return false;
  }
  
  static CommandLineArg s_setThreadCount("-tt", "--task-threads", "Set number of task threads to use (default is num-cpus).", SetThreadCountProc);
#endif
  
}

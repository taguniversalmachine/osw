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


#include "oswThread.h"
#include <pthread.h>


using namespace std;

namespace osw {

  void *MainThreadProc (void *threadAsVoid);

  class SGIThread : public Thread {

  public:

    SGIThread (int athreadnum) :
      Thread (athreadnum) {

      pthread_attr_t  attr;

      pthread_attr_init (&attr);
      pthread_attr_setscope (&attr,PTHREAD_SCOPE_PROCESS);

      pthread_create(&xthread,&attr,MainThreadProc,(void *) this);
    }

    ~SGIThread () {
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

  private:

    pthread_t  xthread;
  };

  /*************/

  void *MainThreadProc (void *threadAsVoid) {
    
    cout << "Starting Thread\n";
    ((SGIThread *) threadAsVoid)->Loop();
    return NULL;
  }

  /*************/

  static std::vector<SGIThread *> threads;

  extern State<int> ui_granularity;

  void InitializeThreads (bool ui) {

    int smp_num_cpus = 1;
    cout << smp_num_cpus << " processors.\n";
    --smp_num_cpus;
    threads.reserve(smp_num_cpus);
    for (int i = 0; i < smp_num_cpus; ++i) {
      threads.push_back(new SGIThread(i));
    }
    ui_granularity = 10;
    UILoop();
  }

  void StopThreads () {

    for (int i = 0; i < threads.size(); ++i) {
      delete threads[i];
    }

  }

  void Sleep () {
    //sched_yield();
  }
}



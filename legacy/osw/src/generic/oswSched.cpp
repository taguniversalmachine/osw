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
  oswsched.cpp
  Implementation of OSW schedulers

  Amar Chaudhary 

  6/27/2002 - patches for soft-RT Linux by Andrew W. Shmeder
*/


#include "oswSched.h"
#include "oswDelete.h"
#include "oswThread.h"
#include <algorithm>

using namespace std;

namespace osw {

  
  void ParallelScheduler::Schedule (BaseActivation *toSchedule) {
    if (toSchedule == NULL) {
      Error("Can't schedule NULL event!");
      return;
    }
#ifndef OSW_NO_LOCK_ON_SCHEDULE
    Lock();
#endif
    if (toSchedule->IsScheduled()) {
      Release();
      return;
    }
    //cout << "Scheduling " << toSchedule->GetContainer()->PathName() << endl;
    xqueue.push(toSchedule);
    toSchedule->SetScheduled(true);
    ++xnumEvents;
#ifndef OSW_NO_LOCK_ON_SCHEDULE
    Release();
#endif
    //if (idleThreads > 0) {
      // Send the broadcast packet to wake up our friend the task thread.
      // TODO: Alert the n-th idle task thread via a unix-domain socket.
    //}
  }

  BaseActivation *(ParallelScheduler::GetNextActivation)() {
    
    BaseActivation *result;
    
    Lock();
    if (xqueue.empty()) {
      result = NULL;
      xnumEvents = 0;
    } else {
      result = xqueue.top();
      result->SetScheduled(false);
      xqueue.pop();
      --xnumEvents;
      xnumEventsDone++;
    }
    Release();
    return result;
  }
  
  /*
   * Same as GetNextActivation() above but assumes the caller has already locked TheParallelScheduler.
   */
  BaseActivation *(ParallelScheduler::RawGetNextActivation)() {
        
    BaseActivation *result;
    
    if (xqueue.empty()) {
      result = NULL;
      xnumEvents = 0;
    } else {
       result = xqueue.top();
       result->SetScheduled(false);
       xqueue.pop();
       --xnumEvents;
       xnumEventsDone++;
     }
     
     return result;
  }
  

  void ParallelScheduler::Lock () {
    xsemaphore.Acquire();
  }
  
  void ParallelScheduler::Release () {
    xsemaphore.Release();
  }
  
  void ParallelScheduler::Wait() {
    idleThreads++;
    xsemaphore.Wait();
  }
  
  void ParallelScheduler::Awake() {
    idleThreads--;
    xsemaphore.Signal();
  }

  void ParallelScheduler::IncrementIdle() {
    idleThreads++;
  }
  
  void ParallelScheduler::DecrementIdle() {
    idleThreads--;
  }

  void ParallelScheduler::PrintStats() {
  	char buffer[256];
	sprintf(buffer, "%d activations, %d interupts", xnumEventsDone, xnumSelectInts);
	Alert(buffer);
	xnumEventsDone = 0;
	xnumSelectInts = 0;
  }

  int ParallelScheduler::GetEventsDone() {
        return xnumEventsDone;
  }
       
  int ParallelScheduler::GetSelectInts() {
        return xnumSelectInts;
  }
  
  void ParallelScheduler::IncrementSelect() {
  	xnumSelectInts++;
  }

  /*****************/
  
  ParallelScheduler *(TheParallelScheduler);
    
  void InitParallelScheduler () {
    TheParallelScheduler = new ParallelScheduler("scheduler",NULL);
  }
    
  /*****************/
  
  static std::vector<SomethingUseful *> UsefulThings;

  _oswexport SomethingUseful::SomethingUseful (bool (*aproc)()) {
    proc = aproc;
    UsefulThings.push_back(this);
  }

  /*****************/

  void _oswexport DoSomethingUseful () {
    if (DoOneDeferredDelete()) return;
    int n = UsefulThings.size();
    for (int i = 0; i < n; ++i) {
      if ((*UsefulThings[i])()) {
	return;
      }
    }
    // other useful things will be added
    //cout << "Sleep\n";
    Sleep();
  }

  /******************/

  //! This cleanup handler removes the scheduler
  BEGIN_CLEANUP_HANDLER(Scheduler)
  {
    delete TheParallelScheduler;
  }
  END_CLEANUP_HANDLER(Scheduler)

}  

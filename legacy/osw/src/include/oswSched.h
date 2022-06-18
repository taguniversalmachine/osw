
/* 
Copyright (c) 2001-2002 Amar Chaudhary. All rights reserved.
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
  oswsched.h
  interface to the OSW schedulers

  Amar Chaudhary 

   6/27/2002 Added patches by Andrew W. Schmeder
*/

#ifndef _OSW_SCHED_H
#define _OSW_SCHED_H

#include <queue>
#include "oswTime.h"

using namespace std;

namespace osw {

  /***********************/
  
  template<class T>
  class circular_array {

  public:

    circular_array (int anumelements = 256) :
      v(anumelements) {
      xnumelements = anumelements;
      xfront = xback = 0;
    }

    bool empty () const {
      return xfront == xback;
    }

    T &top() {
      return v[xfront];
    }

    void pop() {
      ++xfront;
      xfront %= xnumelements;
    }

    void push (const T &toPush) {
      v[xback] = toPush;
      ++xback;
      xback %= xnumelements;
    }
    
  private:
    int  xfront,xback,xnumelements;
    stl::vector<T>  v;
  };
  
  class _oswexport ParallelScheduler : public Container {

  public:

    State<int> num_threads;
    State<int> num_activations;
    
    ParallelScheduler (const string &aname, Container *acontainer) :
      Container (aname,acontainer) {	
      xnumEvents = 0;
      xnumEventsDone = 0;
      xnumSelectInts = 0;
      idleThreads = 0;
    }

    void Schedule (BaseActivation *toSchedule);
    BaseActivation *GetNextActivation();
    BaseActivation *RawGetNextActivation();
    void Lock();
    void Release();
    void Wait();
    void Awake();
    void IncrementIdle();
    void DecrementIdle();
    void PrintStats();
    int GetEventsDone();
    int GetSelectInts();
    void IncrementSelect();
 
    bool NothingToDo() const {
      return xqueue.empty();
    }

  protected:

    struct comparison {
      bool operator () (const BaseActivation *a,
			const BaseActivation *b) {
	return a->Order() > b->Order();	
      }
    };

    typedef stl::priority_queue<BaseActivation *,stl::deque<BaseActivation *>,comparison>
        OrderedActivationQueue;
    typedef circular_array<BaseActivation *> StandardQueue;
  
    OrderedActivationQueue xqueue;

    int xnumEvents;
    int xnumEventsDone;
    int xnumSelectInts;
    int idleThreads;
    //StandardQueue  xqueue;
    //BaseActivation *xlast;
    Semaphore       xsemaphore;
  };


  /***********************/
  
  extern ParallelScheduler	*TheParallelScheduler;
  

  struct _oswexport SomethingUseful {
    bool (*proc)();
    SomethingUseful(bool (*aproc)());
    bool operator () () const {
      return proc();
    }
  };


  void _oswexport InitParallelScheduler ();
  void _oswexport DoSomethingUseful ();
  

  /**********************/

} // namespace osw



#endif // _OSW_SCHED_H


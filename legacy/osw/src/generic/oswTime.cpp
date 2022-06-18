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
   oswtime.cpp
   The TimeMachine transform and clock management routines

   Amar Chaudhary

*/

#include "oswTime.h"
#include <set>

namespace osw {

  /********************/

  class _oswexport TimeMachine : public TimeDomainTransform {

  public:
    
    Outlet<Time>  timeOut;
    Inlet<float>  rate;
    Inlet<double> gotoTime;
    Inlet<unit>   zero;
    Inlet<Time>   timeIn;
    
    friend State<Time> *GetClockFromString (char *tag, int argc, char *argv[]);
    
    TimeMachine (const string &aname, Container *acontainer, State<Time> *aclock = &MainClock, int aorder = 0)
      : TimeDomainTransform (aname,acontainer),
	timeOut ("timeOut",this,"\"Virtual\" time."),
	rate("rate",this,"Rate of \"virtual\" time vs. real time.",1.0),
	gotoTime("goto",this,"Jump to this \"virtual\" time.",0.0),
	zero("zero",this,"Reset virtual time to zero"),
	timeIn("timeIn",this,"Slave to another time machine."),
	xclock(aclock),
	event2(&gotoTime,this,&TimeMachine::eventProc2),
	event1(aclock,aorder,
	       this,&TimeMachine::eventProc1),
	rate_is_changed(&rate,this,&TimeMachine::rate_is_changedExpr),
	event3(&timeIn,this,&TimeMachine::eventProc3),
	zeroed(&zero,this,&TimeMachine::zeroed_expr) {
	  xslave = false;
	  event1.AddEffect(&timeOut);
	  event3.AddEffect(&timeOut);
	  rate.SetOrder(0);
	  gotoTime.SetOrder(1);
	  zero.SetOrder(2);
	  timeIn.SetOrder(3);
	  if (xclock) {
	    SampleRate.SetWithoutActivating((*xclock)->sample_rate);
	    NumberOfSamples.SetWithoutActivating((*xclock)->num_samples);
	  }
    }
    
    TimeMachine (const string &aname, Container *acontainer, int argc, char *argv[])
      : TimeDomainTransform (aname,acontainer),
	timeOut ("timeOut",this,"\"Virtual\" time."),
	rate("rate",this,"Rate of \"virtual\" time vs. real time.",
	     ScanArguments<float>("-rate",1.0f,acontainer,argc,argv)),
	gotoTime("goto",this,"Jump to this \"virtual\" time.",ScanArguments<double>("-goto",0.0,acontainer,argc,argv)),
	zero("zero",this,"Reset virtual time to zero"),
	timeIn("timeIn",this,"Slave to another time machine."),
	xclock(GetClockFromString("-clock",argc,argv)),
	event2(&gotoTime,this,&TimeMachine::eventProc2),
	event1(GetClockFromString("-clock",argc,argv),
	       ScanArguments<int>("-order",0,acontainer,argc,argv),
	       this,&TimeMachine::eventProc1),
	rate_is_changed(&rate,this,&TimeMachine::rate_is_changedExpr),
	event3(&timeIn,this,&TimeMachine::eventProc3),
	zeroed(&zero,this,&TimeMachine::zeroed_expr) {
	  xslave = false;
	  event1.AddEffect(&timeOut);
	  event3.AddEffect(&timeOut);
	  rate.SetOrder(0);
	  gotoTime.SetOrder(1);
	  zero.SetOrder(2);
	  timeIn.SetOrder(3);
	  if (xclock) {
	    SampleRate.SetWithoutActivating((*xclock)->sample_rate);
	    NumberOfSamples.SetWithoutActivating((*xclock)->num_samples);
	  }
    }
	  

    const char *Description () const {
      return "Manages Virtual Time.";
    }

    const char *Draw(void *DrawState) {
      return 
	"HideName\n"
	"label $drawingSpace.icon -image [oswLoadBitmap photo timemachine.gif] \n"
	"$drawingSpace configure -bg [$drawingSpace.icon cget -bg]\n"
	"label $drawingSpace.name -text [file tail $transform]  "
	"     -bg [$drawingSpace cget -bg]\n"
	"pack $drawingSpace.icon $drawingSpace.name -side top\n"
	;
    }
    
  protected:
    bool         xslave;
    State<Time>  *xclock;
    
    Activation <TimeMachine> event1,event2,event3,rate_is_changed,zeroed;
    
    void eventProc1 () {
      if (!xslave) {
	  timeOut->sample_rate = (*xclock)->sample_rate;
	  timeOut->sampling_interval = (*xclock)->sampling_interval;
	  timeOut->num_samples = (*xclock)->num_samples;
	  ++timeOut;
	  timeOut->scale(rate);
	  //timeOut = (const Time &)(timeOut) * rate;
	  timeOut = timeOut;
      }
    }

    void eventProc3 () {
      if (xslave) {
	timeOut->sample_rate = timeIn->sample_rate;
	timeOut->sampling_interval = timeIn->sampling_interval;
	timeOut->num_samples = timeIn->num_samples;
	++timeOut;
	timeOut->scale(rate * timeIn->scale_factor);
	timeOut = timeOut;
      } 
    }
    
    void eventProc2() {
      timeOut->SetTime(gotoTime);
    }

    void rate_is_changedExpr () {
      if (xslave) {
	timeOut->scale(rate * timeIn->scale_factor);
      } else {
	timeOut->scale(rate);
      }
    }

    void zeroed_expr() {
      timeOut->SetTime(0.0);
    }

    bool Connect (BaseOutlet *source, BaseState *&sink) {
      bool result = TimeDomainTransform::Connect(source,sink);
      if (sink == &timeIn) {
	xslave = (source != NULL);
	if (xslave) {
	  Debug ("Time machine is slaved");
	} else {
	  Debug ("Time machine is independent");
	}
      }
      return result;
    }
  };
  
  /******************/

  _oswexport State<Time> MainClock("main_clock",NULL,"Master clock",Time(0.0));
  
  static TransformConstructor<TimeMachine>   TimeMachineConstructor ("TimeMachine");
  
  /********************/
  
  State<Time> *(GetClockFromString) (char *tag, int argc, char *argv[]) {
    
    string clockName = ScanArguments<string>(tag,"main_clock",NULL,argc,argv);
    State<Time> *clock = dynamic_cast<State<Time> *> 
      (Nameable::Find(clockName));
    if (clock == NULL) {
      return &MainClock;
    } else {
      return clock;
    }
  }

  REGISTER_TYPE(Time);
  REGISTER_TIMEDOMAIN_TYPE(Time);


  void PropagateBufferSizeImplicitly();
  void PropagateSampleRateImplicitly();

  void StartSampleRatePropagation (State<Time> &clock) {
    BaseState::ActivationIterator  activations;
    for (activations = clock.BeginActivations(); 
	 !clock.EndActivations(activations);
	 ++activations) {
      TimeMachine *tm = dynamic_cast<TimeMachine *>
	((*activations)->GetContainer());
      if (tm != NULL) {
	tm->SampleRate = clock->sample_rate;
      }
    }
    if (&clock == &MainClock) {
      PropagateSampleRateImplicitly();
    }
  }

  void StartBufferSizePropagation (State<Time> &clock) {
    BaseState::ActivationIterator  activations;
    for (activations = clock.BeginActivations(); 
	 !clock.EndActivations(activations);
	 ++activations) {
      TimeMachine *tm = dynamic_cast<TimeMachine *>
	((*activations)->GetContainer());
      if (tm != NULL) {
	tm->NumberOfSamples = (unsigned int) clock->num_samples;
      }
    }
    if (&clock == &MainClock) {
      PropagateBufferSizeImplicitly();
    }
  }

  /********************/
  
  typedef set<Inlet<Time> *,less<Inlet<Time> *> >  ImplicitTimeMachineList;
  static ImplicitTimeMachineList  implicitTimeMachines;
  
  void _TimeExplicit(Inlet<Time> *atimeInlet) {
    if (typeid(atimeInlet->GetContainer()) != typeid(TimeMachine *)) {
      //cerr << "Time is Explicit\n";
      implicitTimeMachines.erase(atimeInlet);
    }
  }

  void _TimeImplicit(Inlet<Time> *atimeInlet) {
    if (typeid(atimeInlet->GetContainer()) != typeid(TimeMachine *)) {
      //cerr << "Time is Implicit\n";
      implicitTimeMachines.insert(atimeInlet);
    }
  }

  void PropagateSampleRateImplicitly(){
    for (ImplicitTimeMachineList::iterator index = implicitTimeMachines.begin();
	 index != implicitTimeMachines.end();
	 ++index) {
      TimeDomainTransform *transform 
	= dynamic_cast<TimeDomainTransform *>((*index)->GetContainer());
      if (transform != NULL) {
	transform->SampleRate = MainClock->sample_rate;
      }
    }
  }

  void PropagateBufferSizeImplicitly(){
    for (ImplicitTimeMachineList::iterator index = implicitTimeMachines.begin();
	 index != implicitTimeMachines.end();
	 ++index) {
      TimeDomainTransform *transform 
	= dynamic_cast<TimeDomainTransform *>((*index)->GetContainer());
      if (transform != NULL) {
	transform->NumberOfSamples = (unsigned int) MainClock->num_samples;
      }
    }
  }

  struct ImplicitTimeManager : public Container {

    ImplicitTimeManager (const string &aname) :
      Container (aname,NULL),
      activation1(&MainClock,this,&ImplicitTimeManager::activation1Expr) {
      MainClock.SetParallel();
    }

    Activation<ImplicitTimeManager>  activation1;

    void activation1Expr () {
      for (ImplicitTimeMachineList::iterator index = implicitTimeMachines.begin();
	   index != implicitTimeMachines.end();
	   ++index) {
	**index = MainClock;
      }
    }

  };

  static ImplicitTimeManager implicitTimeManager("implicit_time_manager");
}







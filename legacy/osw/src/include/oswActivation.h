

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
/* oswActivation.h
   Classes for OSW Activation expressions
*/

#ifndef _OSW_ACTIVATION_H
#define _OSW_ACTIVATION_H

#include "oswProfile.h"

namespace osw {
  
  class _oswexport BaseActivation {
    
  public:
    
    BaseActivation (BaseState *aactivator,
		    int aorder = 0) :
      xactivators(1),
      xstates(1,false) {
      
      xactivators[0] = aactivator;
      xon = true;
      xcount = xstates.size();
      xorder = aorder;
      xscheduled = false;
    }
    
    BaseActivation (int NumActivators = 0, 
		    int aorder = 0) {
      xactivators.reserve(NumActivators);
      xstates.reserve(NumActivators);
      
      xon = true;
      xcount = xstates.size();
      xorder = aorder;
      xscheduled = false;
    }
    
    virtual ~BaseActivation ();
    
    bool SetActivation (BaseState *aactivator, int aparallel = 0);
    
    void operator () () {proc();}
    
    void AddActivator (BaseState *aactivator);
    void AddActivator (ChangeableInlet *aactivator) {
      AddActivator ((BaseState *)(*aactivator));
    }
    
    int RemoveActivator (BaseState *aactivator, bool compress = false);
    void ReplaceActivator (int aindex, BaseState *aactivator);

    typedef stl::vector<BaseState *>::iterator ActivatorIterator;

    ActivatorIterator BeginActivators () {
      return xactivators.begin();
    }

    bool EndActivators (const ActivatorIterator &toTest) {
      return toTest == xactivators.end();
    }   
    
    typedef stl::list<BaseState *>::iterator EffectsIterator;
    
    void AddEffect (BaseState *toAdd) {
      xeffects.push_back(toAdd);
    }

    void RemoveEffect (BaseState *toRemove) {
      xeffects.erase(remove(xeffects.begin(),xeffects.end(),toRemove),
		     xeffects.end());      
    }

    EffectsIterator BeginEffects () {
      return xeffects.begin();
    }

    bool EndEffects (const EffectsIterator &toTest) {
      return toTest == xeffects.end();
    }

    virtual Container *GetContainer () const = 0;

    int Order () const {
      return xorder;
    }
    
    void SetScheduled (bool ascheduled) {
      xscheduled = ascheduled;
    }
    bool IsScheduled () const {
      return xscheduled;
    }

  protected:
    
    stl::vector<BaseState *>    xactivators;
    stl::vector<bool>		xstates;
    stl::list<BaseState *>           xeffects;
    bool			xon,xscheduled;
    int				xcount,xorder;
    Semaphore                   xsemaphore;

    virtual void proc();
    
    void Reset () {
      xon = !xon;
      xcount = xstates.size();
    }

    friend class BaseState;
    void RemoveActivatorFast (BaseState *aactivator);
  };
  
  
  
  template<class T>
  struct void_mem_function {
    void_mem_function() :
      f(NULL) {
    }
    
    void_mem_function (void (T::*af)()) :
      f(af) {
    }
    
    void operator () (T *aobj) {
      (aobj->*f)();
    }
    
    void (T::*f)();
  };
  
  
  template<class T, class functor = void_mem_function<T> >
    class _oswexport Activation : public BaseActivation {
      
    public:
      
      Activation () :
	BaseActivation (0),
	xcontainer(NULL) {
      }
      
      typedef void (T::* activation_proc)();
      
      Activation (BaseState *aactivator,
		  T  * const acontainer , 
		  const functor &aproc) :
	BaseActivation(aactivator),
	xcontainer(acontainer),
	xproc(aproc) {
	  RegisterWithActivators();
      }
      
      Activation (BaseState *aactivator,
		  int aorder,
		  T  * const acontainer,
		  const functor &aproc) : 
	BaseActivation(aactivator,aorder),
	xcontainer(acontainer),
	xproc(aproc) {
	  RegisterWithActivators();
      }

      Activation (int NumActivators,
		  T  * const acontainer,
		  const functor &aproc) : 
	BaseActivation(NumActivators),
	xcontainer(acontainer),
	xproc(aproc) {
	  RegisterWithActivators();
      }

      Activation (int NumActivators,
		  int aorder,
		  T  * const acontainer, 
		  const functor &aproc) :
	BaseActivation(NumActivators,aorder),
	xcontainer(acontainer),
	xproc(aproc) {
	  RegisterWithActivators();
      }

      void RegisterWithActivators () {
  
	for (stl::vector<osw::BaseState *>::iterator p = xactivators.begin();
	     p != xactivators.end();
	     ++p) {
	  (*p)->RegisterActivation(this);
	}
      }

      Container *GetContainer () const {
	return dynamic_cast<Container *>(xcontainer);
      }

      //Container *GetContainerFast () const {
      //	return (Container *) xcontainer;
      //}
      
    protected:
      
      //! Wrapper for the activation-expression procedure
      /**
       *  This method calls the functor representing the activation-expression
       *  procedure.
       *  Definition appears in oswPatch.h due to reference to Patch methods
       *  @sa Patch Transform
       */
      void proc ();
      
      T			  * const xcontainer;
      functor             xproc;
    };


  /****************************/

  // State<T>::SetActivations must be defined here because it requires
  // defintion of BaseActivation

  template<class T>
    void State<T>::SetActivations() {
    
    for (stl::list<BaseActivation *>::iterator p = xactivations.begin(); p != xactivations.end(); ++p) {
      (*p)->SetActivation(this,xparallel);
      //(*p)->SetActivation(this,0);
    }
  }





}

#endif //  _OSW_ACTIVATION_H


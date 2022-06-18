
/* 
Copyright (c) 2001-2003 Amar Chaudhary. All rights reserved.
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
/* oswThis.cpp
   OSW Patch Self-Reference class
*/

#include "osw.h"

namespace osw {

  //! Tranform for self-referencing a patch

  class ThisTransform : public TimeDomainTransform {

  protected:

    //! Activation expression functor routes ThisTransform inlets to corresponding patch inlets
    struct InletRouter {
      BaseState *thisInlet;
      BaseState *patchInlet;

      InletRouter(BaseState *athisInlet, BaseState *apatchInlet) {
        thisInlet = athisInlet;
        patchInlet = apatchInlet;
      }

      void operator () (ThisTransform *theThisTransform) {
        theThisTransform->CloneAndExecute(*this);
      }
    };

    //! Activation expression functor routes patch outlets to corresponding ThisTransform outlets
    struct OutletRouter {
      BaseState *thisOutlet;
      BaseState *patchOutlet;

      OutletRouter(BaseState *athisOutlet, BaseState *apatchOutlet) {
        thisOutlet = athisOutlet;
        patchOutlet = apatchOutlet;
      }

      void operator () (ThisTransform *theThisTransform) {
        //osw::Alert("Leaving This clone");
        *thisOutlet = *patchOutlet;
      }
    };

  public:

    //! Standard OSW Transform constructor
    /**
     * @param aname string name of patch
     * @param acontainer pointer to Container for patch
     * @param argc number of additional arguments to patch constuctor
     * @param argv array of strings for additional arguments to patch constructor
     * @sa TimeDomainTransform, Transform, Container
     */
      
    ThisTransform (const string &aname,
                   Container *acontainer,
                   int argc, char *argv[]) 
      : TimeDomainTransform(aname,acontainer,argc,argv) {
      inOverride = false;
      xclone = NULL;
      xthis = dynamic_cast<Patch *>(acontainer);
      if (xthis == NULL) {
        // Probably will never happen
        Error("\"This\" transform must exist inside a patch!");
      } else {
        xthis->AddSelfReference(this);
        for (Transform::InletIterator inlets = xthis->BeginInlets();
             !xthis->EndInlets(inlets);
             xthis->NextInlet(inlets)) {
          AddInlet(xthis->InletSelf(inlets));

        }
        for (Transform::OutletIterator outlets = xthis->BeginOutlets();
             !xthis->EndOutlets(outlets);
             xthis->NextOutlet(outlets)) {
          AddOutlet(xthis->OutletSelf(outlets));
        }
      }
    }

    //! ThisTransform destructor
    ~ThisTransform () {
      for (std::map<BaseState *,Activation<ThisTransform,OutletRouter> *>
           ::iterator p = outletActivations.begin();
           p != outletActivations.end();
           ++p) {
        if (p->second) {
          delete p->second;
        }
      }
      if (xthis) {
        try {
          xthis->RemoveSelfReference(this);
        } catch (...) {

        }
      }
      if (xclone) {
        try {
          delete xclone;
        } catch (...) {
          // xclone may have already been deleted by a recursive instance of This
        }
      }
    }


    //! Override of AddInlet method routes incoming values to corresponding patch inlets
    virtual void AddInlet (BaseState *toAdd) {
      if (inOverride) {
        Transform::AddInlet(toAdd);
      } else {
        inOverride = true;
        BaseState *newInlet = toAdd->CloneInlet(toAdd->Name(),
                                                this,
                                                toAdd->Description());
        inOverride = false;
        inletActivations[toAdd] = new Activation<ThisTransform,InletRouter>
          (newInlet,this,InletRouter(newInlet,toAdd));      
      }
    }

    //! Override of AddInlet method routes patch outlets to ThisTransform outlets
    virtual void AddOutlet (BaseOutlet *toAdd) {
      if (inOverride) {
        Transform::AddOutlet(toAdd);
      } else {
        inOverride = true;
        BaseState *toAddState = dynamic_cast<BaseState *>(toAdd);
        BaseState *newOutlet = toAddState->CloneOutlet(toAddState->Name(),
                                                this,
                                                       toAddState->Description());
        inOverride = false;
        //outletActivations[toAddState] = new Activation<ThisTransform,OutletRouter>
        //(toAddState,this,OutletRouter(newOutlet,toAddState));      
      }
    }

    //! Override of RemoveInlet to unroute corresponding inlets
    virtual void RemoveInlet (BaseState *toRemove) {
      BaseActivation *toDelete = 
        inletActivations[toRemove];
      inletActivations.erase(toRemove);
      delete toDelete;
      Transform::RemoveInlet(toRemove);
    }
    
    //! Override of RemoveOutlet to unroute corresponding outets
    virtual void RemoveOutlet (BaseOutlet *toRemove) {
      BaseState *toRemoveState = dynamic_cast<BaseState *>(toRemove);
      //BaseActivation *toDelete = 
      //  outletActivations[toRemoveState];
      //outletActivations.erase(toRemoveState);
      //delete toDelete;
      Transform::RemoveOutlet(toRemove);
    }

    //! Execute an instance of the self-referenced patch, cloning it if necessary
    void CloneAndExecute(InletRouter &arouter) {
      if (!xclone) {
        //Alert("Cloning " + xthis->PathName());
        xclone = xthis->Clone(xthis->Name(),this);
        for (OutletIterator q = xclone->BeginOutlets();
             !xclone->EndOutlets(q);
             xclone->NextOutlet(q)) {
          BaseState *toAddState = dynamic_cast<BaseState *>
            (xclone->OutletSelf(q));
          //Alert(toAddState->PathName());
          for (OutletIterator p = BeginOutlets();
               !EndOutlets(p);
               NextOutlet(p)) {
            BaseState *myState = dynamic_cast<BaseState *>(OutletSelf(p));
            //Alert(myState->PathName());
            if (myState->Name() == toAddState->Name()) {
              //Alert("Adding Activation for " + toAddState->PathName());
              outletActivations[toAddState] = new Activation<ThisTransform,OutletRouter>
                (toAddState,this,OutletRouter(myState,toAddState));                    
            }
          }
        }
      }

      for (InletIterator p = xclone->BeginInlets();
           !xclone->EndInlets(p);
           xclone->NextInlet(p)) {
        if (xclone->InletSelf(p)->Name() == arouter.patchInlet->Name()) {
          //Alert("Showtime: " + arouter.thisInlet->PathName());
          *(xclone->InletSelf(p)) = *(arouter.thisInlet);
          break;
        }
      }
    }

  protected:

    //! Map of patch inlets to router activations
    std::map<BaseState *,Activation<ThisTransform,InletRouter> *> inletActivations;
    //! Map of patch outlets to router activations
    std::map<BaseState *,Activation<ThisTransform,OutletRouter> *> outletActivations;

    //! The patch being self referenced
    Patch *xthis;

    //! The clone of the self-referenced patched, used for execution
    Patch *xclone;

    //! Special flag to make sure Inlet/Outlet cloning does not get stuck in an infinite loop
    bool inOverride;

  };

  //! ThisTransform Factory
  static TransformFactory<ThisTransform> ThisTransformFactory("This");
  OSW_DECLARE_USE(ThisTransformFactory);
}

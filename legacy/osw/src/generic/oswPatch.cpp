
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
/* oswPatch.cpp
   OSW Patch class
*/

#ifdef _USING_TCL
#include "oswTcl.h"
#else
#include "osw.h"
#endif
// oswPatch.h included by either of the above files

namespace osw {


  Patch::~Patch () {
#ifdef _USING_TCL
    oswTclEvalString(("catch {delete object " + window + "}\n").c_str());
#endif
  };
  
  
  void Patch::AddArgument (const string &aname, 
			   const string &atype, 
			   const string &adesc) {
    AddNameable(MakeState(atype,aname,this,adesc));
  }
  void Patch::RemoveArgument (const string &aname) {
    
    BaseState *toRemove = dynamic_cast<BaseState *>(Find(aname,this));
    if (toRemove != NULL) {
      RemoveNameable(toRemove);
    }
  }
  
  bool Patch::PropagateSampleRate (float aSampleRate,
				   int aNumberOfSamples,
				   int alabel,
				   BaseState *aterminal) {
    
    // propigating from a patch inlet/outlet means passing the responsibility
    // on to the inlet/outlet transform. Find those transforms.
    
    // also, if we got here, we know the terminal is of type Samples or
    // IntSamples, so no need to waste time checking it.  
    // labelling - that happens in the inlet/outlet transform.

    // First treat this like an ordinary transform
    if (!TimeDomainTransform::PropagateSampleRate(aSampleRate,
						  aNumberOfSamples,
						  alabel,
						  aterminal)) {
      return false;
    }
    
    // Set local values (if we haven't already)
    
    SampleRate.SetWithoutActivating(aSampleRate);
    
    if (aterminal->IsInlet()) {
      for (BaseState::ActivationIterator activation = aterminal->BeginActivations();
	   !aterminal->EndActivations(activation); ++activation) {
	TimeDomainTransform *transform = 
	  dynamic_cast<TimeDomainTransform *>((*activation)->GetContainer());
	if (transform != NULL) {
	  transform->PropagateSampleRate(aSampleRate,aNumberOfSamples,
					 alabel,aterminal);
	}
      }
    } else {
      TimeDomainTransform *transform = FindOutletTransform(aterminal);
      if (transform != NULL) {
	transform->PropagateSampleRate(aSampleRate,aNumberOfSamples,
				       alabel,aterminal);
      }
    }    
    return true;
  }
  
  bool Patch::PropagateNumberOfSamples (float aSampleRate,
					int aNumberOfSamples,
					int alabel,
					BaseState *aterminal) {
    
    // propigating from a patch inlet/outlet means passing the responsibility
    // on to the inlet/outlet transform. Find those transforms.
    
    // also, if we got here, we know the terminal is of type Samples or
    // IntSamples, so no need to waste time checking it.  Nor do we bother
    // labelling - that happens in the inlet/outlet transform.

    if (!TimeDomainTransform::PropagateNumberOfSamples(aSampleRate,
						       aNumberOfSamples,
						       alabel,
						       aterminal)) {
      return false;
    }
    // Set local values (if we haven't already)
    if (aSampleRate == (float) SampleRate) {
      NumberOfSamples.SetWithoutActivating(aNumberOfSamples);
    } else {
      NumberOfSamples.SetWithoutActivating
        ((unsigned int)(float(aNumberOfSamples) / aSampleRate * SampleRate));
    }
    
    if (aterminal->IsInlet()) {
      for (BaseState::ActivationIterator activation = aterminal->BeginActivations();
	   !aterminal->EndActivations(activation); ++activation) {
	TimeDomainTransform *transform = 
	  dynamic_cast<TimeDomainTransform *>((*activation)->GetContainer());
	if (transform != NULL) {
	  transform->PropagateNumberOfSamples(aSampleRate,aNumberOfSamples,
					      alabel,aterminal);
	}
      }
    } else {
      TimeDomainTransform *transform = FindOutletTransform(aterminal);
      if (transform != NULL) {
	transform->PropagateNumberOfSamples(aSampleRate,aNumberOfSamples,
					    alabel,aterminal);
      }
    }    
    return true;
  }
  

  void Patch::FixSelfReferences () {
    for (SelfReferenceList::iterator p = xself_references.begin();
         p != xself_references.end();
         ++p) {
      ostringstream os;
      os << "oswFixTransformTerminals " << (*p)->PathName() << endl;
      oswTclEvalString(os.str().c_str());
    }
  }

  void Patch::AddOutletToSelfReferences(BaseState *toAdd) {
    for (SelfReferenceList::iterator p = xself_references.begin();
         p != xself_references.end();
         ++p) {
      (*p)->AddOutlet(dynamic_cast<BaseOutlet *>(toAdd));
    }
    FixSelfReferences();
  }

  void Patch::RemoveOutletFromSelfReferences(BaseState *toRemove) {
    for (SelfReferenceList::iterator p = xself_references.begin();
         p != xself_references.end();
         ++p) {
      (*p)->RemoveOutlet(dynamic_cast<BaseOutlet *>(toRemove));
    }
    FixSelfReferences();
  }

  void Patch::AddInletToSelfReferences(BaseState *toAdd) {
    for (SelfReferenceList::iterator p = xself_references.begin();
         p != xself_references.end();
         ++p) {
      (*p)->AddInlet(toAdd);
    }
    FixSelfReferences();
  }

  void Patch::RemoveInletFromSelfReferences(BaseState *toRemove) {
    for (SelfReferenceList::iterator p = xself_references.begin();
         p != xself_references.end();
         ++p) {
      (*p)->RemoveInlet(toRemove);
    }
    FixSelfReferences();
  }


  extern BaseState *GetOutletOutlet(Transform *t);
  extern BaseState *GetInletInlet(Transform *t);
  extern bool CheckInletTransform(Transform *t);
  extern bool CheckOutletTransform(Transform *t);

  Patch *Patch::Clone (const string &aname, Container *acontainer) {
    string name = aname;
    if (name == "") {
      name = Name()+"_clone";
    }
    if (acontainer == NULL) {
      acontainer = this;
    }
    Patch *clone = new Patch(aname,acontainer);
    for (MemberIterator p = BeginMembers();
         !EndMembers(p);
         NextMember(p)) {
      Nameable *member = MemberSelf(p);
      //Alert("Copying "+(*p)->PathName());
      BaseState *state = dynamic_cast<BaseState *>(member);
      if (state) {
        if (state->IsInlet()) {
          MakeInlet(state->TypeOf().name(),state->Name(),clone,state->Description());
        } else if (dynamic_cast<BaseOutlet *>(state) != NULL) {
          MakeInlet(state->TypeOf().name(),state->Name(),clone,state->Description());
        } else {
          MakeState(state->TypeOf().name(),state->Name(),clone,state->Description());
        }
      } else {
        Transform *t = dynamic_cast<Transform *>(member);
        if (t) {
          //Alert(string("Attempting to clone transform of type ") + t->ClassName());
          Transform *cloneTransform = NULL;
          if (CheckInletTransform(t)) {
            BaseState *inletinlet = GetInletInlet(t);
            cloneTransform = MakeTransformRTTI(t->ClassName(),
                                               inletinlet->Name(),
                                               clone->PathName(),
                                               t->GetArgc(),t->GetArgv());
          } else if (CheckOutletTransform(t)) {
            BaseState *outletoutlet = GetOutletOutlet(t);
            cloneTransform = MakeTransformRTTI(t->ClassName(), 
                                               outletoutlet->Name(),
                                               clone->PathName(),
                                               t->GetArgc(),t->GetArgv());
          } else {
            cloneTransform = MakeTransformRTTI(t->ClassName(),
                                               t->Name(),
                                               clone->PathName(),
                                               t->GetArgc(),t->GetArgv());
          }
          if (cloneTransform) {
            // do nothing for now
            //Alert("Added "+cloneTransform->PathName());            
          } else {
            Error("Could not clone transform " + t->PathName());
          }
        } else {
          Error("Non-state, non-transform member of patch: " + member->PathName());
        }
      }
    }
    // now do the connections
    for (MemberIterator q = BeginMembers();
         !EndMembers(q);
         NextMember(q)) {
      Transform *t = dynamic_cast<Transform *>(MemberSelf(q));
      if (t) {
        for (OutletIterator outlets = t->BeginOutlets();
             !t->EndOutlets(outlets);
             t->NextOutlet(outlets)) {
          BaseOutlet *outlet = OutletSelf(outlets);
          BaseState  *outletState = dynamic_cast<BaseState *>(outlet);
          BaseState *xconnection = outlet->GetConnection();
          if (xconnection != NULL) {
            //Alert(outletState->PathName()+"->"+xconnection->PathName());
            
            BaseState *newinlet = dynamic_cast<BaseState *>
              (Nameable::Find(clone->PathName()+"/"+xconnection->GetContainer()->Name()+"/"+xconnection->Name()));
            BaseOutlet *newoutlet = dynamic_cast<BaseOutlet *>
              (Nameable::Find(clone->PathName()+"/"+t->Name()+"/"+outletState->Name()));
            if (newinlet) {
              //Alert("Inlet "+newinlet->PathName());
            }
            if (newoutlet) {
              //Alert("Found outlet");
            }
            if (newinlet && newoutlet) {
              //Alert(outletState->PathName()+" -> "+newinlet->PathName());
              dynamic_cast<Transform *>(newinlet->GetContainer())->Connect(newoutlet,newinlet);
            }
          }
        }

      }
    }
    return clone;
  }

  /***********************************/
  // Cleanup any outstanding patches

  BEGIN_CLEANUP_HANDLER_ORDER(Patch,100)
  {
    // Have to be careful here because the last one out the door
    //  also destroys the list

    int notAPatch = 0;
    Container::enumerator e = Container::BeginTopLevelEnumerate ();
    while (Container::NumTopLevelMembers() > notAPatch 
	   && !Container::EndTopLevelEnumerate(e)) {
      Patch *patch = dynamic_cast<Patch *>
	(Container::GetContainerFromTopLevelEnumerator(e));
      if (patch != NULL) {
	delete patch;
	notAPatch = 0;
	e = Container::BeginTopLevelEnumerate ();
      } else {
	++notAPatch;
	++e;
      }
    }
  }
  END_CLEANUP_HANDLER(Patch)

} // namespace osw

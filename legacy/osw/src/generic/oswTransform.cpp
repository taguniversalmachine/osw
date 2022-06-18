
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
/* oswTransform.cpp
   Method implementations for
   OSW Transform class
   TimeDomainTransform subclass
   and Transform Factory classes

   Includes connection rules and automatic type casting between transforms
*/


#include "osw.h" // includes oswTransform.h and whatever else we need
#include "oswCast.h"
#include "oswTime.h"

namespace osw {

  // From oswAudio.h
  Float _oswexport GetDefaultSampleRate ();
  Integer _oswexport GetDefaultBufferSize ();

  static map<string,string> *s_RTTItoOSW = NULL;

  static int s_xfactoryChecker = 666;
  
  BaseTransformFactory::FactoryMap *(BaseTransformFactory::xfactories) = NULL;
  
  BaseTransformFactory::BaseTransformFactory (const string &aname,
						      const string &ahello,
						      bool aautonameable) {
    if (xfactories == NULL || s_xfactoryChecker != 666) {
      xfactories = new BaseTransformFactory::FactoryMap;
      s_xfactoryChecker = 666;
    }
    (*xfactories)[aname] = this;
    xautonameable = aautonameable;
    if (ahello != "") {
      std::cout << ahello.c_str() << endl;
    }
  }
  
  void BaseTransformFactory::MapRTTIToOSWName(const string &rttiName,
                                              const string &oswName) {
    if (s_RTTItoOSW == NULL) {
      s_RTTItoOSW = new std::map<string,string>;
    }
    (*s_RTTItoOSW)[rttiName]=oswName;
  }

  
  Transform *(MakeTransform) (const string &axformType, 
			      const string &aname,
			      const string &acontainerName,
			      int argc, char *argv[]) {
    
    if (BaseTransformFactory::xfactories == NULL) {
      BaseTransformFactory::xfactories = 
	new BaseTransformFactory::FactoryMap;
    }
    BaseTransformFactory::FactoryMap::iterator foundFactory =
      BaseTransformFactory::xfactories->find(axformType);
    
    if (foundFactory != BaseTransformFactory::xfactories->end()) {
      
      return
	(*foundFactory).second->Construct(aname,acontainerName,argc,argv);
    } else {
      // MSVC seems to use "class name" as the RTTI name, but only sometimes...
      // How very very annoying!
      
      foundFactory = BaseTransformFactory::xfactories->find("class "
							    + axformType);
      if (foundFactory != BaseTransformFactory::xfactories->end()) {
	return (*foundFactory).second->Construct(aname,acontainerName,
						     argc,argv);
      } else {
	return NULL;
      }
    }
  }
  
  Transform *MakeTransformRTTI (const string &axformType, 
                                const string &aname,
                                const string &acontainerName,
                                int argc, char *argv[]) {
    //Alert(axformType+"->"+s_RTTItoOSW[axformType]);
    return MakeTransform((*s_RTTItoOSW)[axformType],aname,acontainerName,argc,argv);
  }


  bool AutoNameable(const string &axformType) {
    BaseTransformFactory::FactoryMap::iterator foundFactory =
      BaseTransformFactory::xfactories->find(axformType);
    
    if (foundFactory != BaseTransformFactory::xfactories->end()) {
      
      return (*foundFactory).second->xautonameable;
    } else {
      return false;
    }
  }
  
  BaseTransformFactory::FactoryMap *GetFactories () {
    
    return BaseTransformFactory::xfactories;
  }
  
  /******************/
  
  Transform::~Transform () {
    if (xargv) {
      for (int i = 0; i < xargc; ++i) {
        free(xargv[i]);
      } 
      delete [] xargv;
    }
  }
  
  bool Transform::Connect (BaseOutlet *source, BaseState *&sink) {
    
    // sink is part of this, and source is the thing connecting to this
    if (source != NULL) {
      BaseState *sourceState= dynamic_cast<BaseState *>(source);
      Transform *sourceTransform = 
	dynamic_cast<Transform *>(sourceState->GetContainer());
      if (sink->IsChangeable() && sourceState->TypeOf() != sink->TypeOf()) {
	sink = sink->Change(sourceState);
      }
      xinlets[sink] = source;
      return sourceTransform->BackConnect(source,sink);
    }
    return false;
  }  
  
  bool Transform::BackConnect (BaseOutlet *source, BaseState *sink) {
    xoutlets[source] = sink;
    //cerr << "Connecting to " << sink->PathName() << endl;
    sink->Connected(source);
    return source->Connect(sink);
  }
  
  // For everyone's convenience, Disconnect and BackDisconnect are the
  //  opposite of Connect and BackConnect ;)
  
  
  bool Transform::Disconnect (BaseOutlet *source) {
    BaseState *toDisconnect = xoutlets[source];
    if (toDisconnect != NULL) {
      dynamic_cast<Transform *>(toDisconnect->GetContainer()) 
	->BackDisconnect(toDisconnect);
      xoutlets[source] = NULL;
      //cerr << "Disconnecting from " << toDisconnect->PathName() << endl;
      toDisconnect->Disconnected(source);
      source->Connect(NULL);
    }
    return true;
  }
  
  bool Transform::BackDisconnect (BaseState *sink) {
    xinlets[sink] = NULL;
    return true;
  }
  
  
  void Transform::RemoveInlet (BaseState *toRemove) {
    BaseState *outletState;
    try {
      outletState = dynamic_cast<BaseState *>(xinlets[toRemove]);
    } catch (...) {
      outletState = NULL;
    }
    if (outletState != NULL) {
      xinlets[toRemove]->Connect(NULL);
      Transform *transform = dynamic_cast<Transform *>(outletState->GetContainer());
      if (transform != NULL) {
	transform->xoutlets[xinlets[toRemove]] = NULL;
      }
    }
    xinlets.erase(remove_if(xinlets.begin(),xinlets.end(),MatchesInlet(toRemove)));
  }
  
  void Transform::RemoveOutlet (BaseOutlet *toRemove) {
    //cerr << "Removing Outlet " << dynamic_cast<Nameable *>(toRemove)->PathName() << endl;
    if (xoutlets[toRemove] != NULL) {
      // ThisTransform may trigger exceptions here
      Transform *transform;
      try {
        transform = dynamic_cast<Transform *>(xoutlets[toRemove]->GetContainer());
      } catch (...) {
        transform = NULL;
      }
      if (transform != NULL) {
	transform->xinlets[xoutlets[toRemove]] = NULL;
      }
    }
    xoutlets.erase(remove_if(xoutlets.begin(),xoutlets.end(),MatchesOutlet(toRemove)));
  }
  
  void Transform::InletTypeHasChanged (BaseState *changedInlet) {
  }
  
  
  
  Patch * Transform::GetPatch () {
    // not all transform containers are patches
    // (for example, the transform could be part of an array
    
    Transform *containerTransform = dynamic_cast<Transform *>(GetContainer());
    if (containerTransform == NULL) {
      return NULL;
    }
    Patch *patch = dynamic_cast<Patch *>(containerTransform);
    if (patch == NULL) {
      return containerTransform->GetPatch();
    }
    return patch;
  }
  
  
  
  /******************/
  
  
  Transform *TheCastContainer = NULL;
  
  BaseState *(BaseOutlet::CheckCast)(BaseState *toCheck) {
    if (toCheck == NULL) {
      return NULL;
    }
    if (TheCastContainer == NULL) {
      TheCastContainer = new Transform("casts",NULL);
    }

    BaseState *thisState = dynamic_cast<BaseState *>(this);
    BaseRegisterCast *cast = FindCast(thisState->TypeOf(),toCheck->TypeOf());
    if (cast != NULL) {
      Debug (thisState->PathName() + ": casting from " + thisState->TypeOf().name() + " to " +
	     toCheck->TypeOf().name());
      BaseState *castInlet = thisState->CloneInlet(thisState->PathName(),
						   TheCastContainer,"");
      // Special check for Inlet<Time>
      //  need to get rid of implicit time machine that creeps into
      //  cast operations - yuk
      Inlet<Time> *timeInlet = dynamic_cast<Inlet<Time> *>(castInlet);
      if (timeInlet) {
        _TimeExplicit(timeInlet);
      }

      castInlet->AddImplicitCast(cast->MakeCast(*castInlet,*toCheck));
      return castInlet;
    }
    return NULL;
  }

  /******************/

  TimeDomainTransform::TimeDomainTransform (const string& aname,
					    Container *acontainer,
					    float aSampleRate,
					    int aNumberOfSamples) : 
    Transform(aname,acontainer),
    SampleRate("sample_rate",this,"Sample Rate.",
	       (aSampleRate == 0) ? GetDefaultSampleRate() : aSampleRate),
    NumberOfSamples("num_samples",this,"Number of samples processed at once.",
		    (aNumberOfSamples == 0) ? GetDefaultBufferSize() : aNumberOfSamples),
    SampleRateChanged(&SampleRate,this,
		      &TimeDomainTransform::SampleRateChangedExpr),
    NumberOfSamplesChanged(&NumberOfSamples,this,
			     &TimeDomainTransform::NumberOfSamplesChangedExpr) {
    }

  TimeDomainTransform::TimeDomainTransform (const string &aname, 
					    Container *acontainer,
					    int argc, char *argv[])
    : Transform (aname,acontainer,argc,argv),
      SampleRate("sample_rate",this,"Sample Rate.",
		 ScanArguments<float>("-sample_rate",GetDefaultSampleRate(),this,argc,argv)),
      NumberOfSamples("num_samples",this,"Number of samples processed at once.",
		      ScanArguments<int>("-num_samples",GetDefaultBufferSize(),this,argc,argv)),
      SampleRateChanged(&SampleRate,this,
			&TimeDomainTransform::SampleRateChangedExpr),
      NumberOfSamplesChanged(&NumberOfSamples,this,
			     &TimeDomainTransform::NumberOfSamplesChangedExpr) {
  }
  
  
  bool TimeDomainTransform::PropagateSampleRate (float aSampleRate,
						 int aNumberOfSamples,
						 int alabel,
						 BaseState *startingPoint) {
    
    Debug("Propagating SampleRate thru "+ PathName());
    
    if (label == alabel) {
      return false;
      // returning false tells derived class that overload this method
      // to stop propagating.
    }
    
    label = alabel;
    // If this isn't the first node and it's properties are already
    // the correct values, then we don't need to continue
    if (startingPoint != &SampleRate && aSampleRate == (float)SampleRate) {
      return false;
    }
    
    // Set local values (if we haven't already)
    
    SampleRate.SetWithoutActivating(aSampleRate);
    SampleRateHasChanged();
    
    
    for (InletIterator inletIndex = BeginInlets(); !EndInlets(inletIndex);
	 ++inletIndex) {
      
      if (InletConnection(inletIndex) == NULL) {
	continue;
      }
      TimeDomainTransform *transform = 
	dynamic_cast<TimeDomainTransform *>(dynamic_cast<BaseState *>
					    (InletConnection(inletIndex))
					    ->GetContainer());
      if (IsTimeDomainType(InletSelf(inletIndex)->TypeOf()) &&
	  transform != NULL) {
	transform	-> PropagateSampleRate(aSampleRate,aNumberOfSamples,
					       label,
					       dynamic_cast<BaseState *>
					       (InletConnection(inletIndex)));
      }
  }
  
    for (OutletIterator outletIndex = BeginOutlets(); !EndOutlets(outletIndex);
	 ++outletIndex) {
      if (OutletConnection(outletIndex) == NULL) {
	continue;
      }
      TimeDomainTransform *transform =     
	dynamic_cast<TimeDomainTransform *>(OutletConnection(outletIndex)
					    ->GetContainer());
      
      if (IsTimeDomainType(OutletConnection(outletIndex)->TypeOf()) &&
	  transform != NULL) {
	transform	-> PropagateSampleRate(aSampleRate,aNumberOfSamples,
					       label,
					       OutletConnection(outletIndex));
      } 
    }
    
    
    return true;
  }
  
  bool TimeDomainTransform::PropagateNumberOfSamples (float aSampleRate,
						      int aNumberOfSamples,
						      int alabel,
						      BaseState *startingPoint) {
    
#ifdef OSW_DEBUG_MESSAGE
    ostringstream os;
    os << "Propagating NumberOfSamples " << aNumberOfSamples << " thru " << PathName ();
    Debug(os.str());
#endif
    
    if (label == alabel) {
      return false;
      // returning false tells derived class that overload this method
      // to stop propagating.
    }
    
    label = alabel;
    // If this isn't the first node and it's properties are already
    // the correct values, then we don't need to continue
    if (startingPoint != &NumberOfSamples && aNumberOfSamples == (int) NumberOfSamples) {
      return false;
    }
    
    // Set local values (if we haven't already)
    
    if (aSampleRate == (float) SampleRate) {
      NumberOfSamples.SetWithoutActivating(aNumberOfSamples);
    } else {
      NumberOfSamples.SetWithoutActivating
        ((unsigned int)(float(aNumberOfSamples) / aSampleRate * SampleRate));
    }
    NumberOfSamplesHasChanged();
    
    for (InletIterator inletIndex = BeginInlets(); !EndInlets(inletIndex);
	 ++inletIndex) {
      
      if (InletConnection(inletIndex) == NULL) {
	continue;
      }
      TimeDomainTransform *transform = 
	dynamic_cast<TimeDomainTransform *>(dynamic_cast<BaseState *>
					    (InletConnection(inletIndex))
					    ->GetContainer());
      if (IsTimeDomainType(InletSelf(inletIndex)->TypeOf()) &&
	  transform != NULL) {
	transform	-> PropagateNumberOfSamples(aSampleRate,aNumberOfSamples,
						    label,
						    dynamic_cast<BaseState *>
						    (InletConnection(inletIndex)));
      }
    }
    
    for (OutletIterator outletIndex = BeginOutlets(); !EndOutlets(outletIndex);
	 ++outletIndex) {
      if (OutletConnection(outletIndex) == NULL) {
	continue;
      }
      TimeDomainTransform *transform =     
	dynamic_cast<TimeDomainTransform *>(OutletConnection(outletIndex)
					    ->GetContainer());
      
      if (IsTimeDomainType(OutletConnection(outletIndex)->TypeOf()) &&
	  transform != NULL) {
	transform	-> PropagateNumberOfSamples(aSampleRate,aNumberOfSamples,
						    label,
						    OutletConnection(outletIndex));
      } 
    }
    
    return true;
}
  
  void TimeDomainTransform::SampleRateHasChanged () {
  }
  
  void TimeDomainTransform::NumberOfSamplesHasChanged () {
  }
  
  bool TimeDomainTransform::Connect (BaseOutlet *source, BaseState *&sink) {
    bool result = Transform::Connect(source,sink);
    /*
    if (result && IsTimeDomainType(sink->TypeOf())) {
      TimeDomainTransform *transform = dynamic_cast<TimeDomainTransform *>
	(dynamic_cast<BaseState *>(source)->GetContainer());
      bool dirtyParams = false;
      if (transform != NULL) {
	if ((float) transform->SampleRate != (float) SampleRate) {
	  Alert ("Adjusting sample rate...");
	  SampleRate = transform->SampleRate;
	}
	if ((int) transform->NumberOfSamples != (int) NumberOfSamples) {
	  Alert ("Adjusting number of samples...");
	  NumberOfSamples = transform->NumberOfSamples;
	}
      }
    }
    */
    return result;
  }

  /***************/

  //! This cleanup handler removes s_RTTIToOSW and the FactoryMap

  BEGIN_CLEANUP_HANDLER(RTTI)
  {
    delete s_RTTItoOSW;
    delete GetFactories();
  }
  END_CLEANUP_HANDLER(RTTI)

  
  //! This cleanup handler removes the implicit cast variables
  BEGIN_CLEANUP_HANDLER_ORDER(CastContainer,101)
  {
    delete TheCastContainer;
  }
  END_CLEANUP_HANDLER(CastContainer)

}  // namespace osw

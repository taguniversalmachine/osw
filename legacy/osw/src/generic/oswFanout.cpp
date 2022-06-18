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
   fanout.cpp
   fanout and switch transforms
   Amar Chaudhary
	1/29/99 Version
*/

#include "oswTcl.h"

using namespace std;

namespace osw {


  class FanOut : public TimeDomainTransform {
    
  public:
    
    BaseState                 *input;
    std::vector<BaseState *>  outputs;
    State<bool>                noparallel;
    
    FanOut (const string &aname, Container *acontainer,
	    int argc, char *argv[]) :
      TimeDomainTransform (aname,acontainer,argc,argv),
      input(new Inlet<ChangeableInlet::Changeable>("in",this,"Input to fan out.")),
      noparallel("noparallel",this,"Do not parallelize outputs",
		 ScanArguments<bool>("-noparallel",false,acontainer,argc,argv)),
      activation1 (input,this,&FanOut::activation1Expr) {
	
	int NumOutputs = ScanArguments<int>("-outputs",2,acontainer,argc,argv);
	outputs.reserve(NumOutputs);
	
	char buffer[32];
	for (int i = 0; i < NumOutputs; ++i) {
	  sprintf (buffer,"out%d",i+1);
	  outputs.push_back(new Outlet<ChangeableInlet::Changeable>(buffer,this));
	  activation1.AddEffect(outputs[i]);
	  outputs[i]->SetOrder(i);
	  outputs[i]->NoParallel();
	}

	input->SetOrder(0);
	numconnected = 0;
    }
    
    ~FanOut() {
      for (int i = 0; i < outputs.size(); ++i) {
	delete outputs[i];
      }
      delete input;
    }
    
    virtual bool Connect (BaseOutlet *source, BaseState *&sink) {
      if (sink == input) {
	BaseState *sourceAsState = dynamic_cast<BaseState *>(source);
	if (sourceAsState->TypeOf() != sink->TypeOf()) {
	  
	  int varIndex = activation1.RemoveActivator(input);
	  delete input;
	  input = sourceAsState->CloneInlet("in",this,"Input to fan out.");
	  activation1.ReplaceActivator(varIndex,input);
	  input->SetOrder(0);

	  for (int i = 0; i < outputs.size(); ++i) {
	    const string outputName = outputs[i]->Name();
	    BaseState *connection = 
	      dynamic_cast<BaseOutlet *>(outputs[i])->GetConnection();
	    activation1.RemoveEffect(outputs[i]);
	    delete outputs[i];
	    outputs[i] = input->CloneOutlet(outputName,this);
	    activation1.AddEffect(outputs[i]);
	    outputs[i]->SetOrder(i);
	    outputs[i]->NoParallel();
	    // gotta reconnect the new outlet to its sink
	    dynamic_cast<BaseOutlet *>(outputs[i])->Connect(connection);
	  } 
	}
	return TimeDomainTransform::Connect(source,input);
      } else {
	return TimeDomainTransform::Connect(source,sink);
      }
    }

    virtual bool BackConnect (BaseOutlet *source, BaseState *sink) {
      ++numconnected;
      if (numconnected >= outputs.size()) {
	char buffer[32];
	sprintf(buffer,"out%d",numconnected+1);
	outputs.push_back(input->CloneOutlet(buffer,this));
	outputs[numconnected]->SetOrder(numconnected);
	ostringstream os;
	os << "oswFixTransformTerminals " << PathName() << endl;
	oswTclEvalString(os.str().c_str());
      }
      if (!TimeDomainTransform::BackConnect(source,sink)) {
	--numconnected;
	return false;
      }
      return true;
    }

    virtual bool Disconnect (BaseOutlet *source) {
      --numconnected;
      return TimeDomainTransform::Disconnect(source);
    }

    const char *Description () const {
      return "Passes data to all outlets.";
    }

  protected:

    int numconnected;
    Activation<FanOut>  activation1;
    
    void activation1Expr () {

      for (int i = 0; i < outputs.size(); ++i) {
	*outputs[i] = *input;
      }	  
    
    }


  };


  class Switch : public TimeDomainTransform {
    
  public:
    
    BaseState                 *input;
    std::vector<BaseState *>  outputs;
    Inlet<int>	        switchSelection;
    
    Switch (const string &aname, Container *acontainer,
	    int argc, char *argv[]) :
      TimeDomainTransform (aname,acontainer,argc,argv),
      input(new Inlet<ChangeableInlet::Changeable>("in",this,"Input to switch.")),
      switchSelection("switch",this,"Send input to this outlet.",
		      ScanArguments<int>("-switch",0,acontainer,argc,argv)),
      activation1 (input,this,&Switch::activation1Expr),
      activation2 (&switchSelection,this,&Switch::activation2Expr) {
	
	int NumOutputs = ScanArguments<int>("-outputs",2,acontainer,argc,argv);
	outputs.reserve(NumOutputs);
	
	char buffer[32];
	for (int i = 0; i < NumOutputs; ++i) {
	  sprintf (buffer,"out%d",i+1);
	  outputs.push_back(new Outlet<ChangeableInlet::Changeable>(buffer,this));
	  activation1.AddEffect(outputs[i]);
	  outputs[i]->SetOrder(i);
	  outputs[i]->NoParallel();
	}

	input->SetOrder(0);
	switchSelection.SetOrder(1);
	numconnected = 0;
    }
    
    ~Switch() {
      for (int i = 0; i < outputs.size(); ++i) {
	delete outputs[i];
      }
      delete input;
    }
    
    virtual bool Connect (BaseOutlet *source, BaseState *&sink) {
      if (sink == input) {
	BaseState *sourceAsState = dynamic_cast<BaseState *>(source);
	if (sourceAsState->TypeOf() != sink->TypeOf()) {
	  //cerr << Name() << ": Converting from type " <<
	  //sink->TypeOf().name() << 
	  //"to " << sourceAsState->TypeOf().name() << endl;
	  
	  int varIndex = activation1.RemoveActivator(input);
	  delete input;
	  input = sourceAsState->CloneInlet("in",this,"Input to fan out.");
	  activation1.ReplaceActivator(varIndex,input);
	  input->SetOrder(0);

	  for (int i = 0; i < outputs.size(); ++i) {
	    const string outputName = outputs[i]->Name();
	    BaseState *connection = 
	      dynamic_cast<BaseOutlet *>(outputs[i])->GetConnection();
	    activation1.RemoveEffect(outputs[i]);
	    delete outputs[i];
	    outputs[i] = input->CloneOutlet(outputName,this);
	    activation1.AddEffect(outputs[i]);
	    outputs[i]->SetOrder(i);
	    outputs[i]->NoParallel();
	    // gotta reconnect the new outlet to its sink
	    dynamic_cast<BaseOutlet *>(outputs[i])->Connect(connection);
	  } 
	}
	return Transform::Connect(source,input);
      } else {
	return Transform::Connect(source,sink);
      }
    }

    virtual bool BackConnect (BaseOutlet *source, BaseState *sink) {
      ++numconnected;
      //cout << "Connected << " << numconnected << endl;
      if (numconnected >= outputs.size()) {
	char buffer[32];
	sprintf(buffer,"out%d",numconnected+1);
	outputs.push_back(input->CloneOutlet(buffer,this));
	outputs[numconnected]->SetOrder(numconnected);
	ostringstream os;
	os << "oswFixTransformTerminals " << PathName() << endl;
	oswTclEvalString(os.str().c_str());
      }
      if (!TimeDomainTransform::BackConnect(source,sink)) {
	--numconnected;
	return false;
      }
      return true;
    }

    virtual bool Disconnect (BaseOutlet *source) {
      --numconnected;
      //cout << "Disconnected << " << numconnected << endl;
      return TimeDomainTransform::Disconnect(source);
    }

    const char *Description () const {
      return "Passes data to the outlet specified by "
	"the value of \"switch\".";
    }
    
  protected:
    
    Activation<Switch>  activation1, activation2;
    int   numconnected;
    
    void activation1Expr () {
      *outputs[switchSelection] = *input;
    }

    void activation2Expr () {
      if ((int) switchSelection < 0) {
	switchSelection.SetWithoutActivating(0);
      } else if ((int) switchSelection >= outputs.size()) {
	switchSelection.SetWithoutActivating(outputs.size()-1);
      }
    }
  };
  

  class SyncSwitch : public TimeDomainTransform {
    
  public:
    
    BaseState                 *input;
    std::vector<BaseState *>  outputs;
    Inlet<int>	        switchSelection;
    
    SyncSwitch (const string &aname, Container *acontainer,
	    int argc, char *argv[]) :
      TimeDomainTransform (aname,acontainer,argc,argv),
      input(new Inlet<ChangeableInlet::Changeable>("in",this,"Input to switch.")),
      switchSelection("switch",this,"Send input to this outlet.",
		      ScanArguments<int>("-switch",0,acontainer,argc,argv)),
      activation1 (2,this,&SyncSwitch::activation1Expr) {	
	int NumOutputs = ScanArguments<int>("-outputs",2,acontainer,argc,argv);
	outputs.reserve(NumOutputs);
	
	activation1.AddActivator(input);
	activation1.AddActivator(&switchSelection);

	char buffer[32];
	for (int i = 0; i < NumOutputs; ++i) {
	  sprintf (buffer,"out%d",i+1);
	  outputs.push_back(new Outlet<ChangeableInlet::Changeable>(buffer,this));
	  activation1.AddEffect(outputs[i]);
	  outputs[i]->SetOrder(i);
	  outputs[i]->NoParallel();
	}

	input->SetOrder(0);
	switchSelection.SetOrder(1);
	numconnected = 0;
    }
    
    ~SyncSwitch() {
      for (int i = 0; i < outputs.size(); ++i) {
	delete outputs[i];
      }
      delete input;
    }
    
    virtual bool Connect (BaseOutlet *source, BaseState *&sink) {
      if (sink == input) {
	BaseState *sourceAsState = dynamic_cast<BaseState *>(source);
	if (sourceAsState->TypeOf() != sink->TypeOf()) {
	  //cerr << Name() << ": Converting from type " <<
	  //sink->TypeOf().name() << 
	  //"to " << sourceAsState->TypeOf().name() << endl;
	  
	  int varIndex = activation1.RemoveActivator(input);
	  delete input;
	  input = sourceAsState->CloneInlet("in",this,"Input to fan out.");
	  activation1.ReplaceActivator(varIndex,input);
	  input->SetOrder(0);

	  for (int i = 0; i < outputs.size(); ++i) {
	    const string outputName = outputs[i]->Name();
	    BaseState *connection = 
	      dynamic_cast<BaseOutlet *>(outputs[i])->GetConnection();
	    activation1.RemoveEffect(outputs[i]);
	    delete outputs[i];
	    outputs[i] = input->CloneOutlet(outputName,this);
	    activation1.AddEffect(outputs[i]);
	    outputs[i]->SetOrder(i);
	    outputs[i]->NoParallel();
	    // gotta reconnect the new outlet to its sink
	    dynamic_cast<BaseOutlet *>(outputs[i])->Connect(connection);
	  } 
	}
	return Transform::Connect(source,input);
      } else {
	return Transform::Connect(source,sink);
      }
    }

     virtual bool BackConnect (BaseOutlet *source, BaseState *sink) {
      ++numconnected;
      if (numconnected >= outputs.size()) {
	char buffer[32];
	sprintf(buffer,"out%d",numconnected+1);
	outputs.push_back(input->CloneOutlet(buffer,this));
	outputs[numconnected]->SetOrder(numconnected);
	ostringstream os;
	os << "oswFixTransformTerminals " << PathName() << endl;
	oswTclEvalString(os.str().c_str());
      }
      return TimeDomainTransform::BackConnect(source,sink);
    }

    virtual bool Disconnect (BaseOutlet *source) {
      --numconnected;
      //cout << "Disconnected << " << numconnected << endl;
      return TimeDomainTransform::Disconnect(source);
    }

    const char *Description () const {
      return "Route each incoming value to the outlet specified by \"switch\".";
    }
    
  protected:
    
    Activation<SyncSwitch> activation1;
    int  numconnected;
    
    void activation1Expr () {
      if ((int)switchSelection < outputs.size()) {
	*outputs[switchSelection] = *input;
      }
    }

  };

  static TransformFactory<osw::FanOut >   FanOutFactory ("FanOut",true); 
  static TransformFactory<osw::Switch >   SwitchFactory ("Switch");  
  static TransformFactory<osw::SyncSwitch >   SyncSwitchFactory ("SyncSwitch");  
} // namespace osw












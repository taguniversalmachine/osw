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
   fanin.cpp
   fanin transform (output one of many inputs)
   gate transform (select one of many inputs for output)
   Amar Chaudhary
*/

#include "oswTcl.h"

using namespace std;

namespace osw {


  class FanIn : public TimeDomainTransform {

  private:

    struct IndexedExpression {

      IndexedExpression(int aindex = 0) :
	index (aindex) {
      }

      int index;

      void operator () (FanIn *container) {
	container->SetOutputTo(index);
      }
    };
    
    friend struct IndexedExpression;

  public:
    
    BaseState                 *output;
    std::vector<BaseState *>       inputs;
        
    FanIn (const string &aname, Container *acontainer,
	    int argc, char *argv[]) :
      TimeDomainTransform (aname,acontainer,argc,argv),
      output(new Outlet<ChangeableInlet::Changeable>("out",this,"")) {
    	
	int NumInputs = ScanArguments<int>("-inputs",2,acontainer,argc,argv);
	inputs.reserve(NumInputs);
	activation.reserve(NumInputs);
	char buffer[32];
	for (int i = 0; i < NumInputs; ++i) {
	  sprintf (buffer,"in%d",i+1);
	  inputs.push_back(new Inlet<ChangeableInlet::Changeable>(buffer,this,""));
	  inputs[i]->SetOrder(i);
	  activation.push_back(new Activation<FanIn,IndexedExpression>
	    (inputs[i],this,IndexedExpression(i)));
	  activation[i]->AddEffect(output);
	}

	output->SetOrder(0);

	xalreadydone = false;
	numconnected = 0;
    }
    
    ~FanIn() {
      for (int i = 0; i < inputs.size(); ++i) {
	delete inputs[i];
	delete activation[i];
      }
      delete output;
    }
    
    virtual bool Connect (BaseOutlet *source, BaseState *&sink) {
      BaseState *sourceAsState = dynamic_cast<BaseState *>(source);

      ++numconnected;
      //cout << "Connected << " << numconnected << endl;
      
      if (sourceAsState->TypeOf() != sink->TypeOf()) {
	for (InletIterator inletIndex = BeginInlets();
	     !EndInlets(inletIndex); ++inletIndex) {
	  if (InletConnection(inletIndex) != NULL &&
	      InletSelf(inletIndex) != sink &&
	      InletSelf(inletIndex)->TypeOf() != sourceAsState->TypeOf()) {
	    Error (string(sourceAsState->TypeOf().name()) +
		   " is not compatible with other connected inlets.");
	    return false;
	  }
	}
	
	int index;
	for (index = 0; index < inputs.size(); ++index) {
	  if (inputs[index] == sink) {
	    break;
	  }
	}
	int varIndex = activation[index]->RemoveActivator(sink);
	string inputName = sink->Name();
	delete sink;
	sink = inputs[index] = 
	  sourceAsState->CloneInlet(inputName,this,"");
	activation[index]->ReplaceActivator(varIndex,sink);
	sink->SetOrder(index);

	if (!xalreadydone) {
	  BaseState *connection = 
	    dynamic_cast<BaseOutlet *>(output)->GetConnection();
	  for (int i = 0; i < activation.size(); ++i) {
	    activation[i]->RemoveEffect(output);
	  }
	  delete output;
	  output = sink->CloneOutlet("out",this,"");
#ifdef OSW_LAME_FOR_LOOP
	 for (i = 0; i < activation.size(); ++i) {
#else
	 for (int i = 0; i < activation.size(); ++i) {
#endif
	    activation[i]->AddEffect(output);
	  }
	  dynamic_cast<BaseOutlet *>(output)->Connect(connection);
	  xalreadydone = true;
	}
	activation[index]->AddEffect(output);
	
      } 
      if (numconnected >= inputs.size()) {
        char buffer[32];
        sprintf(buffer,"in%d",numconnected+1);
        inputs.push_back(sink->CloneInlet(buffer,this));
        inputs[numconnected]->SetOrder(numconnected);
	activation.push_back(new Activation<FanIn,IndexedExpression>
	    (inputs[numconnected],this,IndexedExpression(numconnected)));
	activation[numconnected]->AddEffect(output);
	ostringstream os;
	os << "oswFixTransformTerminals " << PathName() << endl;
	oswTclEvalString(os.str().c_str());
      }
      if (!TimeDomainTransform::Connect(source,sink)) {
	--numconnected;
	return false;
      }
      return true;	  
    }

      bool BackDisconnect(BaseState *sink) {
	--numconnected;
	return TimeDomainTransform::BackDisconnect(sink);
      }  

    const char *Description () const {
      return "Passes data from any inlet to a single outlet";
    }

  protected:
    
    std::vector<Activation<FanIn,IndexedExpression> *>  activation;

    void SetOutputTo(int index) {
      *output = *inputs[index];
    }

    bool xalreadydone;
    int numconnected;
  };

  /*************/

  class Gate : public TimeDomainTransform {

  private:

    struct IndexedExpression {

      IndexedExpression(int aindex = 0) :
	index (aindex) {
      }

      int index;

      void operator () (Gate *container) {
	if (index == container->index) {
	  container->SetOutputTo(index);
	}
      }
    };
    
    friend struct IndexedExpression;

  public:
    
    BaseState                 *output;
    std::vector<BaseState *>       inputs;
    Inlet<int>                 index;
        
    Gate (const string &aname, Container *acontainer,
	    int argc, char *argv[]) :
      TimeDomainTransform (aname,acontainer,argc,argv),
      index("index",this,"Select input to use",0),
      output(new Outlet<ChangeableInlet::Changeable>("out",this,"")) {
      
      int NumInputs = ScanArguments<int>("-inputs",2,acontainer,argc,argv);
      inputs.reserve(NumInputs);
      activation.reserve(NumInputs);
      char buffer[32];
      for (int i = 0; i < NumInputs; ++i) {
	sprintf (buffer,"in%d",i+1);
	inputs.push_back(new Inlet<ChangeableInlet::Changeable>(buffer,this,""));
	inputs[i]->SetOrder(i+1);
	activation.push_back(new Activation<Gate,IndexedExpression>
			     (inputs[i],this,IndexedExpression(i)));
	activation[i]->AddEffect(output);
      }
      
      output->SetOrder(0);
      index.SetOrder(0);
      
      xalreadydone = false;
      numconnected = 0;
    }
    
    ~Gate() {
      for (int i = 0; i < inputs.size(); ++i) {
	delete inputs[i];
	delete activation[i];
      }
      delete output;
    }
    
    virtual bool Connect (BaseOutlet *source, BaseState *&sink) {
      BaseState *sourceAsState = dynamic_cast<BaseState *>(source);
      if (sink == &index) {
        return Transform::Connect(source,sink);
      }
      ++numconnected;
      //cout << "Connected << " << numconnected << endl;
      
      if (sourceAsState->TypeOf() != sink->TypeOf()) {
	for (InletIterator inletIndex = BeginInlets();
	     !EndInlets(inletIndex); ++inletIndex) {
	  if (InletConnection(inletIndex) != NULL &&
	      InletSelf(inletIndex) != sink &&
              InletSelf(inletIndex) != &index &&
	      InletSelf(inletIndex)->TypeOf() != sourceAsState->TypeOf()) {
	    Error (string(sourceAsState->TypeOf().name()) +
		   " is not compatible with other connected inlets.");
	    return false;
	  }
	}
	
	int index;
	for (index = 0; index < inputs.size(); ++index) {
	  if (inputs[index] == sink) {
	    break;
	  }
	}
	int varIndex = activation[index]->RemoveActivator(sink);
	string inputName = sink->Name();
	delete sink;
	sink = inputs[index] = 
	  sourceAsState->CloneInlet(inputName,this,"");
	activation[index]->ReplaceActivator(varIndex,sink);
	sink->SetOrder(index);

	if (!xalreadydone) {
	  BaseState *connection = 
	    dynamic_cast<BaseOutlet *>(output)->GetConnection();
	  for (int i = 0; i < activation.size(); ++i) {
	    activation[i]->RemoveEffect(output);
	  }
	  delete output;
	  output = sink->CloneOutlet("out",this,"");
#ifdef OSW_LAME_FOR_LOOP
	 for (i = 0; i < activation.size(); ++i) {
#else
	 for (int i = 0; i < activation.size(); ++i) {
#endif
	    activation[i]->AddEffect(output);
	  }
	  dynamic_cast<BaseOutlet *>(output)->Connect(connection);
	  xalreadydone = true;
	}
	activation[index]->AddEffect(output);
	
      } 
      if (numconnected >= inputs.size()) {
        char buffer[32];
        sprintf(buffer,"in%d",numconnected+1);
        inputs.push_back(sink->CloneInlet(buffer,this));
        inputs[numconnected]->SetOrder(numconnected);
	activation.push_back(new Activation<Gate,IndexedExpression>
	    (inputs[numconnected],this,IndexedExpression(numconnected)));
	activation[numconnected]->AddEffect(output);
	ostringstream os;
	os << "oswFixTransformTerminals " << PathName() << endl;
	oswTclEvalString(os.str().c_str());
      }
      if (!TimeDomainTransform::Connect(source,sink)) {
	--numconnected;
	return false;
      }
      return true;	  
    }

      bool BackDisconnect(BaseState *sink) {
	--numconnected;
	return TimeDomainTransform::BackDisconnect(sink);
      }  

    const char *Description () const {
      return "Select one of many inputs";
    }

  protected:
    
    std::vector<Activation<Gate,IndexedExpression> *>  activation;

    void SetOutputTo(int index) {
      *output = *inputs[index];
    }

    bool xalreadydone;
    int numconnected;
  };

  /*************/
  
  static TransformFactory<osw::FanIn> FanInFactory ("FanIn",true);  
  static TransformFactory<osw::Gate> GateFactory ("Gate",true);  

  OSW_DECLARE_USE(FanInFactory);
  OSW_DECLARE_USE(GateFactory);

} // namespace osw


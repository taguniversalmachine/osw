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
   oswBundle.cpp
   transforms for the Bundle data type
   Amar Chaudhary
*/

#include "oswTcl.h"
#include "oswBundle.h"
#include "oswList.h"

using namespace std;

namespace osw {



  class Unbundle : public TimeDomainTransform {
    
  public:
    
    Inlet<Bundle>             bundleIn;
    std::vector<Outlet<Any> *>  outputs;
    
    Unbundle (const string &aname, Container *acontainer,
	    int argc, char *argv[]) :
      TimeDomainTransform (aname,acontainer,argc,argv),
      bundleIn("bundleIn",this,"Bundled input."),
      activation1 (&bundleIn,this,&Unbundle::activation1Expr) {
	
	int NumOutputs = ScanArguments<int>("-size",1,acontainer,argc,argv);
	outputs.reserve(NumOutputs);
	
	char buffer[32];
	for (int i = 0; i < NumOutputs; ++i) {
	  sprintf (buffer,"out%d",i+1);
	  outputs.push_back(new Outlet<Any>(buffer,this));
	  activation1.AddEffect(outputs[i]);
	  outputs[i]->SetOrder(i);
	}

	bundleIn.SetOrder(0);
	numconnected = 0;
    }
    
    ~Unbundle() {
      for (int i = 0; i < outputs.size(); ++i) {
	delete outputs[i];
      }
    }
 
    virtual bool BackConnect (BaseOutlet *source, BaseState *sink) {
      ++numconnected;
      if (numconnected >= outputs.size()) {
	char buffer[32];
	sprintf(buffer,"out%d",numconnected+1);
	outputs.push_back(new Outlet<Any>(buffer,this));
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
      return "Converts a bundle back to standard outlets.";
    }

  protected:
    
    Activation<Unbundle>  activation1;
    int numconnected;

    void activation1Expr () {
      if (bundleIn->index < outputs.size()) {
	*outputs[bundleIn->index] = (const Any &)(bundleIn->state->MakeDatum());
      }
    }
  };

  /***************/

  class BundleMany : public TimeDomainTransform {

  private:

    struct IndexedExpression {

      IndexedExpression(int aindex = 0) :
	index (aindex) {
      }

      int index;

      void operator () (BundleMany *container) {
	container->SetOutputTo(index);
      }
    };
    
    friend struct IndexedExpression;

  public:
    
    std::vector<ChangeableInlet *>       inputs;
    Outlet<Bundle>   bundleOut;
    
    BundleMany (const string &aname, Container *acontainer,
	    int argc, char *argv[]) :
      TimeDomainTransform (aname,acontainer,argc,argv),
      bundleOut("bundleOut",this,"Bundled output.") {
    	
	int NumInputs = ScanArguments<int>("-inputs",2,acontainer,argc,argv);
	inputs.reserve(NumInputs);
	activation.reserve(NumInputs);
	char buffer[32];
	for (int i = 0; i < NumInputs; ++i) {
	  sprintf (buffer,"in%d",i+1);
	  inputs.push_back(new ChangeableInlet (buffer,this,"Input to bundle."));
	  inputs[i]->SetOrder(i);
	  activation.push_back(new Activation<BundleMany,IndexedExpression>
	    (*inputs[i],this,IndexedExpression(i)));
	  activation[i]->AddEffect(&bundleOut);
	}

	bundleOut.SetOrder(0);
	numconnected = 0;
    }
    
    ~BundleMany() {
      for (int i = 0; i < inputs.size(); ++i) {
	delete inputs[i];
	delete activation[i];
      }
    }

    virtual bool Connect (BaseOutlet *source, BaseState *&sink) {
      BaseState *sourceAsState = dynamic_cast<BaseState *>(source);

      ++numconnected;
    
      if (numconnected >= inputs.size()) {
        char buffer[32];
        sprintf(buffer,"in%d",numconnected+1);
        inputs.push_back(new ChangeableInlet(buffer,this,"Input to bundle."));
        inputs[numconnected]->SetOrder(numconnected);
	activation.push_back(new Activation<BundleMany,IndexedExpression>
	    (*inputs[numconnected],this,IndexedExpression(numconnected)));
	activation[numconnected]->AddEffect(&bundleOut);
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
      return "Combine several inputs into a bundle.";
    }

  protected:
    
    std::vector<Activation<BundleMany,IndexedExpression> *>  activation;
    int numconnected;

    void SetOutputTo(int index) {
      bundleOut = Bundle(*inputs[index],index);
    }
  };



  /***************/

  static TransformFactory<Unbundle> UnbundleFactory("Unbundle",true);
  static TransformFactory<BundleMany> BundleManyFactory("Bundle",true);

  OSW_DECLARE_USE(UnbundleFactory);
  OSW_DECLARE_USE(BundleManyFactory);

  REGISTER_TYPE(Bundle);
}











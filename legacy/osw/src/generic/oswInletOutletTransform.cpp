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
  oswInletOutletTransform.cpp
  Transforms for defining inlets and outlets of patches.
  Amar Chaudhary
  */


#include "oswTcl.h"

using namespace std;


namespace osw {

  class InletTransform : public TimeDomainTransform {
    
  public:

    BaseState *input;
    BaseState *output;
    State<string> label;
    State<string> type;

    InletTransform (const string &aname, Container *acontainer,
		    int argc, char *argv[]) :
      TimeDomainTransform (aname+"_inlet",acontainer,argc,argv),
      input(MakeInlet(ScanArguments<string>("-type","Float",
					    acontainer,argc,argv),
		      ScanArguments<string>("-name",aname.c_str(),
					    acontainer,argc,argv),
		      dynamic_cast<Patch *>(acontainer),
		      ScanArguments<string>("-desc","",acontainer,
					    argc,argv))),
      output(MakeOutlet(ScanArguments<string>("-type","Float",
					      this,argc,argv),
			"out",this,"")),
      label("label",this,"Name of inlet.",
	    ScanArguments<string>("-name",aname.c_str(),acontainer,argc,argv)),
      activation1 (input,this,&InletTransform::activation1Expr) {
	activation1.AddEffect(output);
	ostringstream os;
	os << "oswFixPatchTerminals " << GetPatch()->PathName() << endl;
	oswTclEvalString(os.str().c_str());
        GetPatch()->AddInletToSelfReferences(input);
    }

    ~InletTransform () {
      if (GetPatch()) {
        GetPatch()->RemoveInletFromSelfReferences(input);
      }
      delete output;
      delete input;
    }

    const char *Draw (void *DrawState) {
      return 
	"HideName\n"
	"label $drawingSpace.icon -image"
	" [oswLoadBitmap bitmap inletxfm.xbm] "
	"     -bg [$drawingSpace cget -bg] \n"
	"label $drawingSpace.name -text [oswGet $transform/label] "
	"     -bg [$drawingSpace cget -bg] \n"
	"pack $drawingSpace.icon $drawingSpace.name -side top\n";
    }

    const char *Description () const {
      return "Defines an inlet for this patch.";
    }
  
  protected:

    Activation<InletTransform> activation1;

    void activation1Expr () {
      *output = *input;
    }

    virtual bool PropagateSampleRate(float aSampleRate,
				     int aNumberOfSamples,
				     int alabel,
				     BaseState *aterminal) {
      if(TimeDomainTransform::PropagateSampleRate
	 (aSampleRate,aNumberOfSamples,alabel,aterminal)) {
	return GetPatch()->PropagateSampleRate
	  (aSampleRate,aNumberOfSamples,alabel,aterminal);
      } else {
	return false;
      }
    }

    virtual bool PropagateNumberOfSamples(float aSampleRate,
					  int aNumberOfSamples,
					  int alabel,
					  BaseState *aterminal) {
      if(TimeDomainTransform::PropagateNumberOfSamples
	 (aSampleRate,aNumberOfSamples,alabel,aterminal)) {
	return GetPatch()->PropagateNumberOfSamples
	  (aSampleRate,aNumberOfSamples,alabel,aterminal);
      } else {
	return false;
      }
    }
  };

  /**********************/

  class OutletTransform : public TimeDomainTransform {

public:

    BaseState *input;
    BaseState *output;
    State<string> label;

    OutletTransform (const string &aname, Container *acontainer,
		    int argc, char *argv[]) :
      TimeDomainTransform (aname+"_outlet",acontainer,argc,argv),
      output(MakeOutlet(ScanArguments<string>("-type","Float",
					      acontainer,argc,argv),
			ScanArguments<string>("-name",aname.c_str(),
					      acontainer,argc,argv),
			dynamic_cast<Patch *>(acontainer),
			ScanArguments<string>("-desc","",acontainer,
					      argc,argv))),
      input(MakeInlet(ScanArguments<string>("-type","Float",
					    this,argc,argv),
			"in",this,"")),
      label("label",this,"Name of outlet.",
	    ScanArguments<string>("-name",aname.c_str(),acontainer,argc,argv)),
      activation1 (input,this,&OutletTransform::activation1Expr) {
	activation1.AddEffect(output);
	GetPatch()->RegisterOutletTransform(output,this);

	ostringstream os;
	os << "oswFixPatchTerminals " << GetPatch()->PathName() << endl;
	oswTclEvalString(os.str().c_str());
        GetPatch()->FixSelfReferences();
        GetPatch()->AddOutletToSelfReferences(output);
    }

    ~OutletTransform () {
      Patch *patch = GetPatch();
      if (patch) {
        patch->RemoveOutletFromSelfReferences(output);
        patch->UnregisterOutletTransform(output);
      }
      delete output;
      delete input;
    }

    const char *Draw (void *DrawState) {
      return 
	"HideName\n"
	"label $drawingSpace.name -text [oswGet $transform/label] "
	"     -bg [$drawingSpace cget -bg] \n"
	"label $drawingSpace.icon -image"
	" [oswLoadBitmap bitmap outletxfm.xbm] "
	"     -bg [$drawingSpace cget -bg] \n"
	"pack $drawingSpace.icon $drawingSpace.name -side top\n";
    }

    const char *Description () const {
      return "Defines an outlet for this patch.";
    }

  protected:
    
    Activation<OutletTransform> activation1;
    
    void activation1Expr () {
      *output = *input;
    }

    virtual bool PropagateSampleRate(float aSampleRate,
				     int aNumberOfSamples,
				     int alabel,
				     BaseState *aterminal) {
      TimeDomainTransform::PropagateSampleRate
	(aSampleRate,aNumberOfSamples,alabel,aterminal);
      return GetPatch()->PropagateSampleRate
	(aSampleRate,aNumberOfSamples,alabel,output);
    }

    virtual bool PropagateNumberOfSamples(float aSampleRate,
				     int aNumberOfSamples,
				     int alabel,
				     BaseState *aterminal) {
      // first, exhaust all local connections
      TimeDomainTransform::PropagateNumberOfSamples
	(aSampleRate,aNumberOfSamples,alabel,aterminal);
      return GetPatch()->PropagateNumberOfSamples
	(aSampleRate,aNumberOfSamples,alabel,aterminal);

    }

  };

  /********************/

  static TransformConstructor<InletTransform> 
    InletTransformConstructor ("Inlet");
  static TransformConstructor<OutletTransform> 
    OutletTransformConstructor ("Outlet");

  /********************/

  //! Special function for retrieving the "outlet" associated with an OutletTransform
  BaseState *GetOutletOutlet(Transform *t) {
    return dynamic_cast<OutletTransform *>(t)->output;
  }

  //! Special function for retrieving the "inlet" associated with an InletTransform
  BaseState *GetInletInlet(Transform *t) {
    return dynamic_cast<InletTransform *>(t)->input;
  }

  //! Returns true if Transform is an InletTransform
  bool CheckInletTransform(Transform *t) {
    return dynamic_cast<InletTransform *>(t) != NULL;
  }

  //! Returns true if Transform is an OutletTransform
  bool CheckOutletTransform(Transform *t) {
    return dynamic_cast<OutletTransform *>(t) != NULL;
  }



} // namespace osw









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
   array.cpp
   Transform Arrays
   Amar Chaudhary
*/


#include "oswBundle.h"

using namespace std;

namespace osw {

  class _oswexport TransformArray : public TimeDomainTransform {

  protected:
    
    struct IndexedExpression {
      
      IndexedExpression(int aindex = 0) :
	index (aindex) {
      }

      int index;

      void operator () (TransformArray *container) {
	container->UnbundleAndActivate(index);
      }
    };

    
    struct OutletActivation {
      OutletActivation (int aindex = 0,
			int abundleIndex = 0,
			BaseState *aindividualOutlet = NULL) :
	index (aindex),
	bundleIndex (abundleIndex),
	individualOutlet (aindividualOutlet) {
      }

      int index, bundleIndex;
      BaseState *individualOutlet;

      void operator () (TransformArray *container) {
	container->BundleOutlet(index,bundleIndex,individualOutlet);
      }
    };
    
    struct BundledInlet {
      Inlet<Bundle> *inlet;
      std::vector<BaseState *> individualInlets;
    };

    struct BundledOutlet {
      Outlet<Bundle> *outlet;
      std::vector<Activation<TransformArray,OutletActivation> *> outletActivations;
    };


    std::vector<BundledInlet>  xinlets;
    std::vector<BundledOutlet> xoutlets;

    std::vector<Activation<TransformArray,IndexedExpression> * > xactivations;

    std::vector<Transform *> xtransforms;
    string xtransformClass;
    

  public:


    State<int> size;
    State<int> rows;
    TransformArray (const string &aname, Container *acontainer,
		    int argc, char *argv[]) :
      TimeDomainTransform(aname,acontainer,argc,argv),
      size("size",this,"Elements in array (do not change!)",
           (argc > 0) ? atoi(argv[0]) : 0),
      rows("rows",this,"Number of in which to display elements",
	   ScanArguments<int>("-rows",1,acontainer,argc,argv)) {
      
      if (argc > 1) {
	int xnumTransforms = size;
	xtransformClass = argv[1];
	xtransforms.reserve(xnumTransforms);
	
	char  indexStr[32];
	for (int i = 0; i < xnumTransforms; ++i) {
	  sprintf (indexStr,"%d",i);
	  xtransforms.push_back(MakeTransform(xtransformClass,indexStr,
                                              PathName(),argc-2,argv+2));
	}
	if (xnumTransforms > 0) {
          
	  xinlets.reserve(xtransforms[0]->NumInlets());
	  xactivations.reserve(xtransforms[0]->NumInlets());
	  int i = 0;
	  for (Transform::InletIterator inletIndex = 
		 xtransforms[0]->BeginInlets();
	       !xtransforms[0]->EndInlets(inletIndex);
	       xtransforms[0]->NextInlet(inletIndex), 
		 ++i) {
	    BaseState *inlet = xtransforms[0]->InletSelf(inletIndex);
	    xinlets.push_back(BundledInlet());
	    xinlets[i].inlet = new Inlet<Bundle>(inlet->Name(),this,
						 inlet->Description());
	    xinlets[i].individualInlets.reserve(xnumTransforms);
	    xactivations.push_back(new Activation<TransformArray,IndexedExpression>
				   (xinlets[i].inlet,this,
				    IndexedExpression(i)));
	  }
	  for (i = 0; i < xnumTransforms; ++i) {
	    int j = 0;
	    for (Transform::InletIterator inletIndex =
		   xtransforms[i]->BeginInlets();
		 !xtransforms[i]->EndInlets(inletIndex);
		 xtransforms[i]->NextInlet(inletIndex),
		   ++j) {
	      xinlets[j].individualInlets[i] = 
		xtransforms[i]->InletSelf(inletIndex);
	    }
	  }
          
	  xoutlets.reserve(xtransforms[0]->NumOutlets());
	  i = 0;
	  for (Transform::OutletIterator outletIndex = 
		 xtransforms[0]->BeginOutlets();
	       !xtransforms[0]->EndOutlets(outletIndex);
	       xtransforms[0]->NextOutlet(outletIndex), 
		 ++i) {
	    BaseState *outlet = dynamic_cast<BaseState *>
	      (xtransforms[0]->OutletSelf(outletIndex));
	    xoutlets.push_back(BundledOutlet());
	    xoutlets[i].outlet = new Outlet<Bundle>(outlet->Name(),this,
                                                    outlet->Description());
	    xoutlets[i].outletActivations.reserve(xnumTransforms);
	  }
	  for (i = 0; i < xnumTransforms; ++i) {
	    int j = 0;
	    for (Transform::OutletIterator outletIndex =
		   xtransforms[i]->BeginOutlets();
		 !xtransforms[i]->EndOutlets(outletIndex);
		 xtransforms[i]->NextOutlet(outletIndex),
		   ++j) {
	      BaseState *outletAsState = 
		dynamic_cast<BaseState *>(xtransforms[i]->OutletSelf(outletIndex));
	      xoutlets[j].outletActivations.push_back(
                                                      new Activation<TransformArray, OutletActivation>
                                                      (outletAsState,
                                                       this,
                                                       OutletActivation(j,i,outletAsState)));
	    }
	  }
	}
      }	 
    }

    ~TransformArray () {
      int i;
      for (i = 0; i < xactivations.size(); ++i) {
	delete xactivations[i];
	delete xinlets[i].inlet;
      }

      for (i = 0; i < xoutlets.size(); ++i) {
	delete xoutlets[i].outlet;
	for (int j = 0; j < xoutlets[i].outletActivations.size(); ++j) {
	  delete xoutlets[i].outletActivations[j];
	}
      }

      for (i = 0; i < xtransforms.size(); ++i) {
	delete xtransforms[i];
      }
    }


    void UnbundleAndActivate (int index) {
      if (index < xinlets.size()) {
	int bundleIndex = (*(xinlets[index].inlet))->index;
	*(xinlets[index].individualInlets[bundleIndex]) =
	  *((*(xinlets[index].inlet))->state);
      }
    }

    void BundleOutlet(int index, int bundleIndex, 
		      BaseState *individualOutlet) {
      *(xoutlets[index].outlet) = Bundle(individualOutlet,bundleIndex);
    }

    const char *Description() const {
      return ((const int &)size > 0) ? xtransforms[0]->Description() : "";
    }

    const char *Draw(void *DrawState) {
      return 
	"HideName \n"
	"iwidgets::scrolledframe $drawingSpace.array "
	"  -vscrollmode none -hscrollmode none \n"
	"update idletasks \n"
	"set arraySpace [$drawingSpace.array childsite]\n"
	"set savedSpace $drawingSpace\n"
	"set arraySize [oswGet $transform/size]\n"
	"set arrayRows [oswGet $transform/rows]\n"
	"set arrayCols [expr $arraySize / $arrayRows]\n"
	"set savedTransform $transform\n"
	"set row 0\n"
	"set column 0\n"
	"for {set i 0} {$i < $arraySize} {incr i} {\n"
	"  if {$i % $arrayCols == 0} {\n"
	"     incr row\n"
	"     set column 0\n"
	"  } else {\n"
	"     incr column\n"
	"  }\n"
	"  frame $arraySpace.f$i \n"
	"  update idletasks\n"
	"  set drawingSpace $arraySpace.f$i \n"
	"  set transform $savedTransform/$i \n"
	"  grid $arraySpace.f$i -row $row -column $column -sticky nsew\n"
	"  eval [oswDrawTransform $transform $drawingSpace]\n"
	"  update idletasks\n"
	"}\n"
	"pack $savedSpace.array -fill both -expand yes -side left \n"
        "$savedSpace.array configure -hscrollmode dynamic \n"
	;
    }

  };


  /*************/

  static TransformFactory<TransformArray> ArrayFactory ("Array");

  OSW_DECLARE_USE(ArrayFactory);
};



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
  getput.cpp
  Get, Put and Glob transforms.
  Used to send data between patches.
  Amar Chaudhary
  */


#include "osw.h"
#include "oswPatternMatch.h"
#include "oswCast.h"

using namespace std;

namespace osw {

  /******************/

  class _oswexport InternalPutTransform : public TimeDomainTransform {

  public:
    
    BaseState *toPut;
    ChangeableInlet input;
    
    InternalPutTransform (const string &aname, Container *acontainer,
		  int argc, char *argv[]) :
      TimeDomainTransform (aname,acontainer,argc,argv),
      input ("input",this,""),
      activation1(input,this,&InternalPutTransform::activation1Expr) {
	
	if (argc >= 1) {
	  toPut = dynamic_cast<BaseState *>
	    (Nameable::Find(argv[0],GetContainer()));

	  if (toPut == NULL) {
	    toPut = new State<Any>((argv[0][0] == '/') ? argv[0] + 1 : argv[0],
				   (argv[0][0] == '/') ? NULL : GetContainer(),
				   "");
	  } else {
	    if (toPut->TypeOf() != typeid(Any)) {
	      input.Change(toPut);
	    }
	  }
          if (toPut != NULL) {
            activation1.AddEffect(toPut);
          }
	}
    }

    ~InternalPutTransform () {
      // once a put variable is defined, it stays defined
      // (who knows what other transforms might use it!)
    }

  protected:

    Activation<InternalPutTransform> activation1;

    void activation1Expr () {
      if (toPut->TypeOf() == typeid(Any)) {
	*((State<Any> *)toPut) = (const Any &)(input->MakeDatum());
      } else if (toPut->TypeOf() == input.TypeOf()) {
	*toPut = *((BaseState *)input);
      } else if (input.TypeOf() == typeid(Any) &&
		 Any(((State<Any>&)(*input))).TypeOf()==toPut->TypeOf()) {
	toPut->SetFromDatum(Any(((State<Any>&)(*input))).GetDatum());
      } else {
	BaseRegisterCast *cast = NULL;
	if (input.TypeOf() == typeid(Any)) {
	  cast = FindCast(Any(((State<Any>&)(*input))).TypeOf(),
					    toPut->TypeOf());
	  if (cast == NULL) {
	    Error(string("Can't convert type ") + 
		  Any(((State<Any>&)(*input))).TypeOf().name() 
		  + " to " + toPut->TypeOf().name());
	  } else {
	    cast->CastFromDatum(*(Any(((State<Any>&)(*input))).GetDatum()),
				*toPut);
	  }
	} else {
	  cast = FindCast(input.TypeOf(),toPut->TypeOf());
	  if (cast == NULL) {
	    Error(string("Can't convert type ") + input.TypeOf().name() 
		  + " to " + toPut->TypeOf().name());
	  } else {
	    cast->CastFromDatum(*(input->MakeDatum()),*toPut);
	  }
	}

      }
    }

    void InletTypeHasChanged (BaseState *changedInlet) {
      // it doesn't matter :)
    }

  };

  static TransformFactory<InternalPutTransform> 
    InternalPutTransformFactory ("Put",true);
  OSW_DECLARE_USE(InternalPutTransformFactory);

  /****************/

  class _oswexport InternalGetTransform : public TimeDomainTransform {

  public:

    BaseState   *toGet;
    Outlet<Any> value;
    Inlet<string> getvar;
    

    InternalGetTransform (const string &aname, Container *acontainer,
		  int argc, char *argv[]) :
      TimeDomainTransform (aname,acontainer,argc,argv),
      value("value",this,"Value of variable"),
      getvar("getvar",this,"Change the variable being monitored"),
      getvarChanged(&getvar,this,&InternalGetTransform::getvarChangedExpr),
      activation1(0,ScanArguments<int>("-order",0,acontainer,
						argc,argv),
		  this,&InternalGetTransform::activation1Expr) {
      toGet = NULL;
      if (argc >= 1) {
	toGet = dynamic_cast<BaseState *>
	  (Nameable::Find(argv[0],GetContainer()));
	
	
	if (toGet == NULL) {
	  toGet = new State<Any>((argv[0][0] == '/') ? argv[0]+1 : argv[0],
				 (argv[0][0] == '/') ? NULL : GetContainer(),
				 "");
	} 
      
        if (toGet != NULL) {
          activation1.AddActivator(toGet);
        } else {
          Error ("could not find or create variable " + string(argv[0]));
        }
      }
      
      activation1.AddEffect(&value);
    }

    ~InternalGetTransform () {
    }
  
  protected:

    Activation<InternalGetTransform> activation1,getvarChanged;
    
    void activation1Expr () {
      if (toGet->TypeOf() == typeid(Any)) {
	value = (const Any &)(*((State<Any> *)toGet));
      } else {
	value = (const Any &)(toGet->MakeDatum());
      }
    }

    void getvarChangedExpr () {
      int varIndex = -1;
      if (toGet != NULL) {
	varIndex = activation1.RemoveActivator(toGet);
      }
      const char *varname = getvar->c_str();
      toGet = dynamic_cast<BaseState *>
	(Nameable::Find(varname,GetContainer()));
      
      if (toGet == NULL) {
	toGet = new State<Any>((varname[0] == '/') ? varname+1 : varname,
			       (varname[0] == '/') ? NULL : GetContainer(),
			       "");
      } 
      if (toGet != NULL) {
	if (varIndex >= 0) {
	  activation1.ReplaceActivator(varIndex,toGet);
	} else {
	  activation1.AddActivator(toGet);
	}
      } else {
	Error ("could not find or create variable " + string(varname));
      }	
    }
    
  };

  static TransformFactory<InternalGetTransform> 
  InternalGetTransformFactory ("Get",true);

  OSW_DECLARE_USE(InternalGetTransformFactory);

  /*****************/
  

  class _oswexport GlobTransform : public TimeDomainTransform {
    
  public:
    
    ChangeableInlet input;
    State<string> pattern;

    GlobTransform (const string &aname, Container *acontainer,
		  int argc, char *argv[]) :
      TimeDomainTransform (aname,acontainer,argc,argv),
      input ("input",this,""),
      pattern ("pattern",this,"pattern to be matched.",
	       ScanArguments<string>("-pattern",
                                     (argc > 0) ? argv[0] : "",
				     acontainer,
				     argc,argv)),
      activation1(input,this,&GlobTransform::activation1Expr) {
      if (argc > 0 && pattern == string("") && argv[0][0] != '-') {
	pattern = argv[0];
      }
    }
    
    
    ~GlobTransform () {
    }

  protected:

    Activation<GlobTransform> activation1;

    void activation1Expr () {
      char *cpattern = const_cast<char *>(pattern->c_str());
      StateList statesToPut;
      if (cpattern[0] == '/') {
	MatchStates(cpattern,NULL,statesToPut);
      } else {
	MatchStates(cpattern,GetContainer(),statesToPut);
      }
      for (StateList::iterator toPut = statesToPut.begin();
	   toPut != statesToPut.end();
	   ++toPut) {
	if ((*toPut)->TypeOf() == typeid(Any)) {
	  *((State<Any> *)(*toPut)) = (const Any &)(input->MakeDatum());
	} else if ((*toPut)->TypeOf() == input.TypeOf()) {
	  **toPut = *((BaseState *)input);
	} else if (input.TypeOf() == typeid(Any) &&
		   Any(((State<Any>&)(*input))).TypeOf()==(*toPut)->TypeOf()) {
	  (*toPut)->SetFromDatum(Any(((State<Any>&)(*input))).GetDatum());
	} else {
	  BaseRegisterCast *cast = NULL;
	  if (input.TypeOf() == typeid(Any)) {
	    cast = FindCast(Any(((State<Any>&)(*input))).TypeOf(),
			    (*toPut)->TypeOf());
	    if (cast == NULL) {
	      Error(string("Can't convert type ") 
		    + Any(((State<Any>&)(*input))).TypeOf().name() 
		    + " to " + (*toPut)->TypeOf().name());
	    } else {
	      cast->CastFromDatum(*(Any(((State<Any>&)(*input))).GetDatum()),
				  **toPut);
	    }
	  } else {
	    cast = FindCast(input.TypeOf(),(*toPut)->TypeOf());
	    if (cast == NULL) {
	      Error(string("Can't convert type ") + input.TypeOf().name() 
		    + " to " + (*toPut)->TypeOf().name());
	    } else {
	      cast->CastFromDatum(*(input->MakeDatum()),**toPut);
	    }
	  }
	  
	}
      }
    }

    void InletTypeHasChanged (BaseState *changedInlet) {
      // it doesn't matter :)
    }

  };

  static TransformFactory<GlobTransform> 
    GlobTransformFactory ("Put*",true);

  OSW_DECLARE_USE(GlobTransformFactory);
  
  /****************/


}

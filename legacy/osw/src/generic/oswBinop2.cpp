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
   oswbinop2.cpp
   More transforms for the basic arithmetic operators (+,-,*,/,%,etc.)
*/

#include "oswBinop.h"
#include "oswLex.h"

using namespace std;

namespace osw {


  template<int opnum> 
    class BinopTransformBothActive : public TimeDomainTransform {

    public:

      ChangeableInlet   param1,param2;
      BaseState         *result,*internal;

      BinopTransformBothActive (const string &aname, Container *acontainer,
				int argc, char *argv[]) :
	TimeDomainTransform (aname,acontainer,argc,argv),
	param1("param1",this,""),
	param2("param2",this,""),
	result(NULL),
	activation1(2,this,&
		    BinopTransformBothActive<opnum>::activation1Expr) {

	  result = new Outlet<Any>("result",this,"");
	  xop = NULL;
	  xlock = false;
	  activation1.AddActivator(param1);
	  activation1.AddActivator(param2);
	  activation1.AddEffect(result);
          xvariableType = false;
      }
    
      ~BinopTransformBothActive () {
	if (result != NULL) {
	  delete result;
	}
      }

    protected:

      BaseBinop  *xop;
      bool      xlock;
      short     xvariableType;

      BaseBinop *LookupBinop (const type_info &type1, const type_info &type2) {
	BinopMap::iterator found = TheBinopMap[opnum].find(type1.name());
	if (found != TheBinopMap[opnum].end()) {
	  stl::map<string, BaseBinop *, less<string> >::iterator reallyFound
	    = (*found).second.find(type2.name());
	  if (reallyFound != (*found).second.end()) {
	    return (*reallyFound).second;
	  }
	}
	//Error (Name() + ": no defintion for " + type1.name() + 
	//       " and " + type2.name());
	return NULL;
      }

      void InletTypeHasChanged (BaseState *changedInlet) {

        xvariableType = false;
	xop = LookupBinop(param1->TypeOf(),param2->TypeOf());

	if (xop != NULL) {
	  if (xop->ResultType() != result->TypeOf()) {
	    BaseOutlet *resultAsOutlet = dynamic_cast<BaseOutlet *>(result);
	    BaseState *connection = resultAsOutlet->GetConnection();
	    Disconnect(resultAsOutlet);
	    activation1.RemoveEffect(result);
	    delete result;
	    result = xop->MakeOutlet("result",this,"");
	    activation1.AddEffect(result);
	    if (connection != NULL) {
	      dynamic_cast<Transform *>(connection->GetContainer())
		->Connect(dynamic_cast<BaseOutlet *>(result),connection);
	    }
	  }
	} else {
	  if (param1->TypeOf() == typeid(Any)) {
	    xvariableType |= 1;
	  }
	  if (param2->TypeOf() == typeid(Any)) {
	    xvariableType |= 2;
	  }
          if (xvariableType == 0) {
            Error(String("no defintion for ") + param1->TypeOf().name() + 
                  " and " + param2->TypeOf().name());
          } 
        }
      }

      Activation<BinopTransformBothActive<opnum> > activation1;

      void activation1Expr () {
	if (xop != NULL) {
	  xop->eval(*((BaseState *)param1),*((BaseState *)param2),*result);
	} else {
          Untyped untypedResult;
          switch (xvariableType) {
          case 1:
            xop = LookupBinop((*((State<Any> *)(BaseState *)param1))->TypeOf(),
                              param2->TypeOf());
            if (xop) {
              untypedResult
                = xop->eval_from_data(*(*((State<Any> *)(BaseState *)param1))->GetDatum(),
                                      *param2->MakeDatum());
            }
            break;
          case 2:
            xop = LookupBinop(param1->TypeOf(),
                              (*((State<Any> *)(BaseState *)param2))->TypeOf());
            if (xop) {
              untypedResult
                = xop->eval_from_data(*param1->MakeDatum(),
                                      *(*((State<Any> *)(BaseState *)param2))->GetDatum());
            }
            break;       
          case 3:
            xop = LookupBinop((*((State<Any> *)(BaseState *)param1))->TypeOf(),
                              (*((State<Any> *)(BaseState *)param2))->TypeOf());
            if (xop) {
              untypedResult
                = xop->eval_from_data(*(*((State<Any> *)(BaseState *)param1))->GetDatum(),
                                      *(*((State<Any> *)(BaseState *)param2))->GetDatum());
            }
            break;                            
          }
          // have to reset for next time
          xop = NULL;
          (State<Any> &)*result = untypedResult;
        }
      }

    };

  // BothActive - i.e. '-transforms
  static TransformFactory<BinopTransformBothActive<ADD> > Add2AFactory("'+",true);
  static TransformFactory<BinopTransformBothActive<SUBTRACT> > Subtract2AFactory("'-",true);
  static TransformFactory<BinopTransformBothActive<MULTIPLY> > Multiply2AFactory("'*",true);
  static TransformFactory<BinopTransformBothActive<DIVIDE> > Divide2AFactory("'/",true);
  static TransformFactory<BinopTransformBothActive<MODULUS> > Modulus2AFactory("'%",true);
  static TransformFactory<BinopTransformBothActive<LESS> > Less2AFactory("'<",true);
  static TransformFactory<BinopTransformBothActive<GREATER> > Greater2AFactory("'>",true);
  static TransformFactory<BinopTransformBothActive<LTEQ> > Lteq2AFactory("'<=",true);
  static TransformFactory<BinopTransformBothActive<GTEQ> > Gteq2AFactory("'>=",true);
  static TransformFactory<BinopTransformBothActive<EQUAL> > Equal2AFactory("'==",true);
  static TransformFactory<BinopTransformBothActive<NOTEQUAL> > NotEqual2AFactory("'!=",true);
  static TransformFactory<BinopTransformBothActive<LOGOR> > LogOr2AFactory("'||",true);
  static TransformFactory<BinopTransformBothActive<LOGAND> > LogAnd2AFactory("'&&",true);
  static TransformFactory<BinopTransformBothActive<EXPONENT> > ExponentFactory("'^",true);
  static TransformFactory<BinopTransformBothActive<SHIFTR> > RShiftFactory("'>>",true);
  static TransformFactory<BinopTransformBothActive<SHIFTL> > LShiftFactory("'<<",true);
  static TransformFactory<BinopTransformBothActive<BITAND> > BitAndFactory("'?&",true);
  static TransformFactory<BinopTransformBothActive<BITOR> > BitOrFactory("'?|",true);
  static TransformFactory<BinopTransformBothActive<BITXOR> > BitXorFactory("'?^",true);
  
  
  OSW_DECLARE_USE(Add2AFactory);
  OSW_DECLARE_USE(Subtract2AFactory);
  OSW_DECLARE_USE(Multiply2AFactory);
  OSW_DECLARE_USE(Divide2AFactory);
  OSW_DECLARE_USE(Modulus2AFactory);
  OSW_DECLARE_USE(Less2AFactory);
  OSW_DECLARE_USE(Greater2AFactory);
  OSW_DECLARE_USE(Lteq2AFactory);
  OSW_DECLARE_USE(Gteq2AFactory);
  OSW_DECLARE_USE(Equal2AFactory);
  OSW_DECLARE_USE(NotEqual2AFactory);
  OSW_DECLARE_USE(LogOr2AFactory);
  OSW_DECLARE_USE(LogAnd2AFactory);
  OSW_DECLARE_USE(ExponentFactory);
  OSW_DECLARE_USE(RShiftFactory);
  OSW_DECLARE_USE(LShiftFactory);
  OSW_DECLARE_USE(BitAndFactory);
  OSW_DECLARE_USE(BitOrFactory);
  OSW_DECLARE_USE(BitXorFactory);

}








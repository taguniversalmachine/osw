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
   oswbinop.cpp
   Transforms for the basic arithmetic operators (+,-,*,/,%,etc.)
*/

#include "oswBinop.h"
#include "oswLex.h"

#include "osw.h"
#include <map>

using namespace std;

namespace osw {

  _oswexport BinopMap  *TheBinopMap = NULL;

  _oswexport BaseBinop *LookupBinop(int opnum, const type_info &type1, const type_info &type2) {
    BinopMap::const_iterator found = TheBinopMap[opnum].find(type1.name());
    if (found != TheBinopMap[opnum].end()) {
      stl::map<string, BaseBinop *, less<string> >::const_iterator reallyFound
	= (*found).second.find(type2.name());
      if (reallyFound != (*found).second.end()) {
	return (*reallyFound).second;
      }
    }
    //error (Name() + ": no defintion for " + type1.name() + 
    //     " and " + type2.name());
    return NULL;
  }

  /**********************/

  BaseBinop::BaseBinop () {
    if (TheBinopMap == NULL) {
      TheBinopMap = new BinopMap [LASTOP+1];
    }
  }

  void BaseBinop::Register (int opnum, const string &type1,
			    const string &type2, BaseBinop *toRegister) {
    TheBinopMap[opnum][type1][type2] = toRegister;
  }

  /**********************/

  template<int opnum> 
    class BinopTransform : public TimeDomainTransform {

    public:

      ChangeableInlet   param1,param2;
      BaseState         *result;

      BinopTransform (const string &aname, Container *acontainer,
		      int argc, char *argv[]) :
	TimeDomainTransform (aname,acontainer,argc,argv),
	param1("param1",this,""),
	param2("param2",this,""),
	result(NULL),
	activation1(param1,this,&BinopTransform<opnum>::activation1Expr) {
	if (argc > 0) {
	    Token *token = GetToken(argv[0]);
	    switch (token->type()) {
	    case TOKEN_INT :
	      param2.Change(new State<int>("param2",this,"",
					   ((IntToken *)(token))->val));
	      break;
	    case TOKEN_FLOAT :
	      param2.Change(new State<float>("param2",this,"",
					((FloatToken *)(token))->val));
	      break;
	    case TOKEN_DOUBLE :
	      param2.Change(new State<double>("param2",this,"",
					((DoubleToken *)(token))->val));
	      break;
	    case TOKEN_COMPLEX :
	      param2.Change(new State<complex<float> >("param2",this,"",
					((ComplexToken *)(token))->val));
	      break;
	    case TOKEN_DOUBLECOMPLEX :
	      param2.Change(new State<complex<double> >("param2",this,"",
					((DoubleComplexToken *)(token))->val));
	      break;
	    default:
	      Error (Name() + ": not a valid number.");
	    }
	  }
	  
	  result = new Outlet<Any>("result",this,"");
	  xop = NULL;
	  xlock = false;
	  xvariableType = false;
	  activation1.AddEffect(result);
      }
    
      ~BinopTransform () {
	if (result != NULL) {
	  delete result;
	}
      }

    protected:

      BaseBinop  *xop;
      bool       xlock;
      short      xvariableType;

      BaseBinop *LookupBinop (const type_info &type1, const type_info &type2) {
	BinopMap::iterator found = TheBinopMap[opnum].find(type1.name());
	if (found != TheBinopMap[opnum].end()) {
	  stl::map<string, BaseBinop *, less<string> >::iterator reallyFound
	    = (*found).second.find(type2.name());
	  if (reallyFound != (*found).second.end()) {
	    return (*reallyFound).second;
	  }
	}
	//error (Name() + ": no defintion for " + type1.name() + 
	//     " and " + type2.name());
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
    
    
      Activation<BinopTransform<opnum> > activation1;

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


  

  static TransformConstructor<BinopTransform<ADD> > AddConstructor("+",true);
  static TransformConstructor<BinopTransform<SUBTRACT> > SubtractConstructor("-",true);
  static TransformConstructor<BinopTransform<MULTIPLY> > MultiplyConstructor("*",true);
  static TransformConstructor<BinopTransform<DIVIDE> > DivideConstructor("/",true);
  static TransformConstructor<BinopTransform<MODULUS> > ModulusConstructor("%",true);
  static TransformConstructor<BinopTransform<LESS> > LessConstructor("<",true);
  static TransformConstructor<BinopTransform<GREATER> > GreaterConstructor(">",true);
  static TransformConstructor<BinopTransform<LTEQ> > LteqConstructor("<=",true);
  static TransformConstructor<BinopTransform<GTEQ> > GteqConstructor(">=",true);
  static TransformConstructor<BinopTransform<EQUAL> > EqualConstructor("==",true);
  static TransformConstructor<BinopTransform<NOTEQUAL> > NotEqualConstructor("!=",true);
  static TransformConstructor<BinopTransform<LOGAND> > LogAndConstructor("&&",true);
  static TransformConstructor<BinopTransform<LOGOR> > LogOrConstructor("||",true);
  static TransformConstructor<BinopTransform<EXPONENT> > ExponentConstructor("^",true);
  static TransformConstructor<BinopTransform<SHIFTR> > RShiftConstructor(">>",true);
  static TransformConstructor<BinopTransform<SHIFTL> > LShiftConstructor("<<",true);
  static TransformConstructor<BinopTransform<BITAND> > BitAndConstructor("?&",true);
  static TransformConstructor<BinopTransform<BITOR> > BitOrConstructor("?|",true);
  static TransformConstructor<BinopTransform<BITXOR> > BitXorConstructor("?^",true);



  typedef vector<complex<double> > DoubleComplexSamples;
  typedef vector<std::complex<float> > ComplexSamples;

  //ostream & operator << (ostream &os, const float_complex &toPrint) {
  //  return os << real(toPrint)<<'+'<<imag(toPrint)<<'i';
  //}
  REGISTER_TYPE(Complex);
  REGISTER_TYPE(DoubleComplex);
  REGISTER_TYPE(DoubleComplexSamples);
  REGISTER_TYPE(ComplexSamples);

  /*************/

  //! This cleanup handler removes the Binop map
  BEGIN_CLEANUP_HANDLER(BinopMap)
  {
    delete [] TheBinopMap;
  }
  END_CLEANUP_HANDLER(BinopMap)

}




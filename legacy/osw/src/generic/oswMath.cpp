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
  oswmath.cpp
  Transforms for standard math functions

  Amar Chaudhary
  */

#include "oswMath.h"


#if !defined(__GNUC__) && !defined(__sgi) && !defined(__APPLE__)
#include "oswFloatify.h"
#endif 

namespace osw {

  typedef stl::map<string, BaseDeclareMathFunc *, stl::less<string> > MathFuncMap;
  stl::map<string, MathFuncMap, stl::less<string> >  TheMathFuncMap;

  
  BaseDeclareMathFunc::BaseDeclareMathFunc(const string &aname,
					   const string &type_name) {
    TheMathFuncMap[aname][type_name] = this;
  }

  BaseDeclareMathFunc _oswexport *LookupMathFunc(const string &aname,
						 const string &type_name) {
    if (TheMathFuncMap.find(aname) != TheMathFuncMap.end()) {
      return TheMathFuncMap[aname][type_name];
    }
    if (islower(aname[0])) {
      string uname = aname;
      uname[0] = toupper(uname[0]);
      if (TheMathFuncMap.find(uname) != TheMathFuncMap.end()) {
	return TheMathFuncMap[uname][type_name];
      }
    }    
    return NULL;
  }

  /********************/

  class _oswexport MathTransform : public TimeDomainTransform {

  public:

    ChangeableInlet  in;
    BaseState        *out;
    
    MathTransform (const string &aname, Container *acontainer,
		   int argc, char *argv[],
		   const string &afunc_name) :
      TimeDomainTransform (aname,acontainer,argc,argv),
      in("in",this,"function input"),
      out(NULL),
      activation1(in,this,&MathTransform::activation1Expr),
      xfunc_name(afunc_name) {       
      out = new Outlet<Any>("out",this,"function output");
      activation1.AddEffect(out);
      xfunc = NULL;
      xany = true;
      xanyfunc = NULL;
      xlastDatum = NULL;
    }
    
    ~MathTransform () {
      if (out != NULL) {
	delete out;
      }
    }
    
    
  protected:
    
    string xfunc_name;
    BaseDeclareMathFunc  *xfunc;
    BaseDeclareMathFunc  *xanyfunc;
    bool                 xany;
    BaseDatum            *xlastDatum;
    
    void InletTypeHasChanged (BaseState *changedInlet) {
      MathFuncMap::iterator found =
	TheMathFuncMap[xfunc_name].find(in.TypeOf().name());
      if (found != TheMathFuncMap[xfunc_name].end()) {
	xfunc = found->second;
	xany = false;
	if (xlastDatum) {
	  xlastDatum->RemoveRef();
	  xlastDatum = NULL;
	}
	if (out->TypeOf() != in->TypeOf()) {
	  activation1.RemoveEffect(out);
	  delete out;
	  out = xfunc->MakeOutlet("out",this,"function output.");
	  activation1.AddEffect(out);
	}
      } else if (in->TypeOf() == typeid(Any)) {
	xfunc = NULL;
	xany = true;
	if (out->TypeOf() != typeid(Any)) {
	  activation1.RemoveEffect(out);
	  delete out;
	  out = in->CloneOutlet("out",this,"function output.");
	  activation1.AddEffect(out);
	}
	Debug("Dynamic Typing");
      } else { 
	xfunc = NULL;
	xany = false;
	Error (" no definition of " + xfunc_name + " for type " 
	       + in.TypeOf().name());
      }      
    }
    
    Activation<MathTransform> activation1;
    
    void activation1Expr () {
      if (xfunc != NULL) {
	xfunc->eval(*((BaseState *)in),*out);
      } else if (xany) {
	State<Any> &inAsAny = dynamic_cast<State<Any> &>(*in);
	if (!xlastDatum || inAsAny->GetDatum()->TypeOf() != xlastDatum->TypeOf()) {
	  MathFuncMap::iterator found =
	    TheMathFuncMap[xfunc_name].find(inAsAny->GetDatum()->TypeOf().name());
	  if (found != TheMathFuncMap[xfunc_name].end()) {
	    xanyfunc = found->second;
	  } else {
	    Error(" no definition of " + xfunc_name + " for type "
		  + inAsAny->GetDatum()->TypeOf().name());
	    xanyfunc = NULL;
	  }
	  if (xlastDatum) {
	    xlastDatum->RemoveRef();
	    xlastDatum = inAsAny->GetDatum();
	  }
	}
	if (xanyfunc) {
	  *((State<Any> *)out) = xanyfunc->evalDatum(inAsAny->GetDatum());
	}
      }
    }
  };

  
  /******************/

  Transform *(MathTransformFactory::Construct) (const string &aname,
						const string &acontainerName,
						int argc, char *argv[]) {
    return new MathTransform (aname,
			      dynamic_cast<Container *>(Nameable::Find(acontainerName)),
			      argc,argv,xfunc_name);
  }

  /****************************/

  static MathTransformFactory  SinFactory("Sin");
  static MathTransformFactory  CosFactory("Cos");
  static MathTransformFactory  TanFactory("Tan");
  static MathTransformFactory  AbsFactory("Abs");
  static MathTransformFactory  LogFactory("Log");
  static MathTransformFactory  ExpFactory("Exp");
  static MathTransformFactory  Log10Factory("Log10");
  static MathTransformFactory  AtanFactory("Atan");
  static MathTransformFactory  AcosFactory("Acos");
  static MathTransformFactory  AsinFactory("Asin");
  static MathTransformFactory  SqrtFactory("Sqrt");
  static MathTransformFactory  SinhFactory("Sinh");  
  static MathTransformFactory  CoshFactory("Cosh");
  static MathTransformFactory  TanhFactory("Tanh");
  static MathTransformFactory  AtanhFactory("Atanh");
  static MathTransformFactory  AcoshFactory("Acosh");
  static MathTransformFactory  AsinhFactory("Asinh");
  static MathTransformFactory  SgnFactory("Sgn");
  static MathTransformFactory  FracFactory("Frac");  
  static MathTransformFactory  FloorFactory("Floor");  
  static MathTransformFactory  CeilFactory("Ceil");  
  static MathTransformFactory  SecFactory("Sec");
  static MathTransformFactory  CscFactory("Csc");
  static MathTransformFactory  CotFactory("Cot");
  static MathTransformFactory  AsecFactory("Asec");
  static MathTransformFactory  AcscFactory("Acsc");
  static MathTransformFactory  AcotFactory("Acot");
  static MathTransformFactory  SechFactory("Sech");
  static MathTransformFactory  CschFactory("Csch");
  static MathTransformFactory  CothFactory("Coth");
  static MathTransformFactory  AsechFactory("Asech");
  static MathTransformFactory  AcschFactory("Acsch");
  static MathTransformFactory  AcothFactory("Acoth");
  static MathTransformFactory  AngleFactory("Angle");
  static MathTransformFactory  ConjFactory("Conj");
  static MathTransformFactory  NormFactory("Norm");
  static MathTransformFactory  RealFactory("Real");
  static MathTransformFactory  ImagFactory("Imag");
  static MathTransformFactory  SincFactory("Sinc");
  static MathTransformFactory  SinhcFactory("Sinhc");
  static MathTransformFactory  FactFactory("Fact");
  static MathTransformFactory  GammaFactory("Gamma");

  /****************************/


  DECLARE_MATH_FUNC(Abs,int,abs);
  DECLARE_MATH_FUNC(Sgn,int,sgn);
  DECLARE_MATH_FUNC(Fact,int,Factorial);
  DECLARE_MATH_FUNC(Gamma,int,Gamma);
 
  DECLARE_MATH_VFUNC(Abs,int,abs);
  DECLARE_MATH_VFUNC(Sgn,int,sgn);
  DECLARE_MATH_VFUNC(Fact,int,Factorial);
  DECLARE_MATH_VFUNC(Gamma,int,Gamma);
  
#ifdef OSW_NEEDS_MATHF_FUNCTIONS
  DECLARE_MATH_FUNC(Sin,float,sin);
  DECLARE_MATH_FUNC(Cos,float,cos);
  DECLARE_MATH_FUNC(Tan,float,tan);
  DECLARE_MATH_FUNC(Log,float,log);
  DECLARE_MATH_FUNC(Exp,float,exp);
  DECLARE_MATH_FUNC(Log10,float,log10);
  DECLARE_MATH_FUNC(Abs,float,fabs);
  DECLARE_MATH_FUNC(Asin,float,asin);
  DECLARE_MATH_FUNC(Acos,float,acos);
  DECLARE_MATH_FUNC(Atan,float,atan);
  DECLARE_MATH_FUNC(Sinh,float,sinh);
  DECLARE_MATH_FUNC(Cosh,float,cosh);
  DECLARE_MATH_FUNC(Tanh,float,tanh);
  DECLARE_MATH_FUNC(Sqrt,float,sqrt);
  DECLARE_MATH_FUNC(Ceil,float,ceil);
  DECLARE_MATH_FUNC(Floor,float,floor);
#else
  DECLARE_MATH_FUNC(Sin,float,sinf);
  DECLARE_MATH_FUNC(Cos,float,cosf);
  DECLARE_MATH_FUNC(Tan,float,tanf);
  DECLARE_MATH_FUNC(Log,float,logf);
  DECLARE_MATH_FUNC(Exp,float,expf);
  DECLARE_MATH_FUNC(Log10,float,log10f);
  DECLARE_MATH_FUNC(Abs,float,fabsf);
  DECLARE_MATH_FUNC(Asin,float,asinf);
  DECLARE_MATH_FUNC(Acos,float,acosf);
  DECLARE_MATH_FUNC(Atan,float,atanf);
  DECLARE_MATH_FUNC(Sinh,float,sinhf);
  DECLARE_MATH_FUNC(Cosh,float,coshf);
  DECLARE_MATH_FUNC(Tanh,float,tanhf);
  DECLARE_MATH_FUNC(Sqrt,float,sqrtf);
  DECLARE_MATH_FUNC(Ceil,float,ceilf);
  DECLARE_MATH_FUNC(Floor,float,floorf);
#endif
  DECLARE_MATH_TFUNC(Sec,float,sec);
  DECLARE_MATH_TFUNC(Csc,float,csc);
  DECLARE_MATH_TFUNC(Cot,float,cot);
  DECLARE_MATH_TFUNC(Asec,float,asec);
  DECLARE_MATH_TFUNC(Acsc,float,acsc);
  DECLARE_MATH_TFUNC(Acot,float,acot);
  DECLARE_MATH_TFUNC(Sech,float,sech);
  DECLARE_MATH_TFUNC(Csch,float,csch);
  DECLARE_MATH_TFUNC(Coth,float,coth);
  DECLARE_MATH_FUNC(Asinh,float,asinh);
  DECLARE_MATH_FUNC(Acosh,float,acosh);
  DECLARE_MATH_FUNC(Atanh,float,atanh);
  DECLARE_MATH_TFUNC(Asech,float,asech);
  DECLARE_MATH_TFUNC(Acsch,float,acsch);
  DECLARE_MATH_TFUNC(Acoth,float,acoth);
  DECLARE_MATH_FUNC(Sgn,float,sgn);
  DECLARE_MATH_FUNC(Frac,float,frac);
  DECLARE_MATH_TFUNC(Sinc,float,sinc);
  DECLARE_MATH_TFUNC(Sinhc,float,sinhc);
  DECLARE_MATH_FUNC(Gamma,float,Gamma);

  DECLARE_MATH_FUNC(Sin,double,sin);
  DECLARE_MATH_FUNC(Cos,double,cos);
  DECLARE_MATH_FUNC(Tan,double,tan);
  DECLARE_MATH_FUNC(Log,double,log);
  DECLARE_MATH_FUNC(Exp,double,exp);
  DECLARE_MATH_FUNC(Log10,double,log10);
  DECLARE_MATH_FUNC(Abs,double,fabs);
  DECLARE_MATH_FUNC(Asin,double,asin);
  DECLARE_MATH_FUNC(Acos,double,acos);
  DECLARE_MATH_FUNC(Atan,double,atan);
  DECLARE_MATH_FUNC(Sinh,double,sinh);
  DECLARE_MATH_FUNC(Cosh,double,cosh);
  DECLARE_MATH_FUNC(Tanh,double,tanh);
  DECLARE_MATH_FUNC(Sgn,double,sgn);
  DECLARE_MATH_FUNC(Frac,double,frac);
  DECLARE_MATH_FUNC(Sqrt,double,sqrt);
  DECLARE_MATH_FUNC(Ceil,double,ceil);
  DECLARE_MATH_FUNC(Floor,double,floor);
  DECLARE_MATH_TFUNC(Sec,double,sec);
  DECLARE_MATH_TFUNC(Csc,double,csc);
  DECLARE_MATH_TFUNC(Cot,double,cot);
  DECLARE_MATH_TFUNC(Asec,double,asec);
  DECLARE_MATH_TFUNC(Acsc,double,acsc);
  DECLARE_MATH_TFUNC(Acot,double,acot);
  DECLARE_MATH_TFUNC(Sech,double,sech);
  DECLARE_MATH_TFUNC(Csch,double,csch);
  DECLARE_MATH_TFUNC(Coth,double,coth);
  DECLARE_MATH_TFUNC(Asinh,double,asinh);
  DECLARE_MATH_TFUNC(Acosh,double,acosh);
  DECLARE_MATH_TFUNC(Atanh,double,atanh);
  DECLARE_MATH_TFUNC(Asech,double,asech);
  DECLARE_MATH_TFUNC(Acsch,double,acsch);
  DECLARE_MATH_TFUNC(Acoth,double,acoth);
  DECLARE_MATH_TFUNC(Sinc,double,sinc);
  DECLARE_MATH_TFUNC(Sinhc,double,sinhc);
  DECLARE_MATH_FUNC(Gamma,double,Gamma);

  DECLARE_MATH_VFUNC(Sin,float,sin);
  DECLARE_MATH_VFUNC(Cos,float,cos);
  DECLARE_MATH_VFUNC(Tan,float,tan);
  DECLARE_MATH_VFUNC(Log,float,log);
  DECLARE_MATH_VFUNC(Exp,float,exp);
  DECLARE_MATH_VFUNC(Log10,float,log10);
  DECLARE_MATH_VFUNC(Abs,float,fabs);
  DECLARE_MATH_VFUNC(Asin,float,asin);
  DECLARE_MATH_VFUNC(Acos,float,acos);
  DECLARE_MATH_VFUNC(Atan,float,atan);
  DECLARE_MATH_VFUNC(Asinh,float,asinh);
  DECLARE_MATH_VFUNC(Acosh,float,acosh);
  DECLARE_MATH_VFUNC(Atanh,float,atanh);
  DECLARE_MATH_VFUNC(Sinh,float,sinh);
  DECLARE_MATH_VFUNC(Cosh,float,cosh);
  DECLARE_MATH_VFUNC(Tanh,float,tanh);
  DECLARE_MATH_VFUNC(Sqrt,float,sqrt);
  DECLARE_MATH_VFUNC(Sgn,float,sgn);
  DECLARE_MATH_VFUNC(Frac,float,frac);
  DECLARE_MATH_VFUNC(Ceil,float,ceil);
  DECLARE_MATH_VFUNC(Floor,float,floor);
  DECLARE_MATH_VFUNC(Sec,float,sec);
  DECLARE_MATH_VFUNC(Csc,float,csc);
  DECLARE_MATH_VFUNC(Cot,float,cot);
  DECLARE_MATH_VFUNC(Asec,float,asec);
  DECLARE_MATH_VFUNC(Acsc,float,acsc);
  DECLARE_MATH_VFUNC(Acot,float,acot);
  DECLARE_MATH_VFUNC(Sech,float,sech);
  DECLARE_MATH_VFUNC(Csch,float,csch);
  DECLARE_MATH_VFUNC(Coth,float,coth);
  DECLARE_MATH_VFUNC(Asech,float,asech);
  DECLARE_MATH_VFUNC(Acsch,float,acsch);
  DECLARE_MATH_VFUNC(Acoth,float,acoth);
  DECLARE_MATH_VFUNC(Sinc,float,sinc);
  DECLARE_MATH_VFUNC(Sinhc,float,sinhc);
  DECLARE_MATH_VFUNC(Gamma,float,Gamma);


  DECLARE_MATH_VFUNC(Sin,double,sin);
  DECLARE_MATH_VFUNC(Cos,double,cos);
  DECLARE_MATH_VFUNC(Tan,double,tan);
  DECLARE_MATH_VFUNC(Log,double,log);
  DECLARE_MATH_VFUNC(Exp,double,exp);
  DECLARE_MATH_VFUNC(Log10,double,log10);
  DECLARE_MATH_VFUNC(Abs,double,fabs);
  DECLARE_MATH_VFUNC(Asin,double,asin);
  DECLARE_MATH_VFUNC(Acos,double,acos);
  DECLARE_MATH_VFUNC(Atan,double,atan);
  DECLARE_MATH_VFUNC(Sinh,double,sinh);
  DECLARE_MATH_VFUNC(Cosh,double,cosh);
  DECLARE_MATH_VFUNC(Tanh,double,tanh);
  DECLARE_MATH_VFUNC(Asinh,double,asinh);
  DECLARE_MATH_VFUNC(Acosh,double,acosh);
  DECLARE_MATH_VFUNC(Aanh,double,atanh);
  DECLARE_MATH_VFUNC(Sgn,double,sgn);
  DECLARE_MATH_VFUNC(Frac,double,frac);
  DECLARE_MATH_VFUNC(Sqrt,double,sqrt);
  DECLARE_MATH_VFUNC(Ceil,double,ceil);
  DECLARE_MATH_VFUNC(Floor,double,floor);
  DECLARE_MATH_VFUNC(Sec,double,sec);
  DECLARE_MATH_VFUNC(Csc,double,csc);
  DECLARE_MATH_VFUNC(Cot,double,cot);
  DECLARE_MATH_VFUNC(Asec,double,asec);
  DECLARE_MATH_VFUNC(Acsc,double,acsc);
  DECLARE_MATH_VFUNC(Acot,double,acot);
  DECLARE_MATH_VFUNC(Sech,double,sech);
  DECLARE_MATH_VFUNC(Csch,double,csch);
  DECLARE_MATH_VFUNC(Coth,double,coth);
  DECLARE_MATH_VFUNC(Asech,double,asech);
  DECLARE_MATH_VFUNC(Acsch,double,acsch);
  DECLARE_MATH_VFUNC(Acoth,double,acoth);
  DECLARE_MATH_VFUNC(Sinc,double,sinc);
  DECLARE_MATH_VFUNC(Sinhc,double,sinhc);
  DECLARE_MATH_VFUNC(Gamma,double,Gamma);
 
  DECLARE_MATH_RFUNC(Sin,Complex,std::sin);
  DECLARE_MATH_RFUNC(Cos,Complex,std::cos);
  DECLARE_MATH_RFUNC(Tan,Complex,std::tan);
  DECLARE_MATH_RFUNC(Log,Complex,std::log);
  DECLARE_MATH_RFUNC(Exp,Complex,std::exp);
  DECLARE_MATH_RFUNC(Log10,Complex,std::log10);  
  DECLARE_MATH_DIFFERENT_TYPES(Abs,Complex,Float,std::abs);
  DECLARE_MATH_RFUNC(Asin,Complex,asin);
  DECLARE_MATH_RFUNC(Acos,Complex,acos);
  DECLARE_MATH_RFUNC(Atan,Complex,atan);
  DECLARE_MATH_FUNC(Asinh,Complex,asinh);
  DECLARE_MATH_FUNC(Acosh,Complex,acosh);
  DECLARE_MATH_FUNC(Atanh,Complex,atanh);
  DECLARE_MATH_RFUNC(Sinh,Complex,std::sinh);
  DECLARE_MATH_RFUNC(Cosh,Complex,std::cosh);
  DECLARE_MATH_RFUNC(Tanh,Complex,std::tanh);
  DECLARE_MATH_RFUNC(Sqrt,Complex,std::sqrt);
  DECLARE_MATH_FUNC(Sec,Complex,sec);
  DECLARE_MATH_FUNC(Csc,Complex,csc);
  DECLARE_MATH_FUNC(Cot,Complex,cot);
  DECLARE_MATH_FUNC(Asec,Complex,asec);
  DECLARE_MATH_FUNC(Acsc,Complex,acsc);
  DECLARE_MATH_FUNC(Acot,Complex,acot);
  DECLARE_MATH_FUNC(Sech,Complex,sech);
  DECLARE_MATH_FUNC(Csch,Complex,csch);
  DECLARE_MATH_FUNC(Coth,Complex,coth);
  DECLARE_MATH_FUNC(Asech,Complex,asech);
  DECLARE_MATH_FUNC(Acsch,Complex,acsch);
  DECLARE_MATH_FUNC(Acoth,Complex,acoth);
  DECLARE_MATH_FUNC(Sinc,Complex,sinc);
  DECLARE_MATH_FUNC(Sinhc,Complex,sinhc);

  /* Double Complex */
  DECLARE_MATH_RFUNC(Sin,DoubleComplex,std::sin);
  DECLARE_MATH_RFUNC(Cos,DoubleComplex,std::cos);
  DECLARE_MATH_RFUNC(Tan,DoubleComplex,std::tan);
  DECLARE_MATH_RFUNC(Log,DoubleComplex,std::log);
  DECLARE_MATH_RFUNC(Exp,DoubleComplex,std::exp);
  DECLARE_MATH_RFUNC(Log10,DoubleComplex,std::log10);
  DECLARE_MATH_DIFFERENT_TYPES(Abs,DoubleComplex,Double,std::abs);
#if 0
  // These guys don't seem to be available everywhere.
  // TODO - add formulae to m1.h (AC)
  DECLARE_MATH_RFUNC(Asin,DoubleComplex,asin);
  DECLARE_MATH_RFUNC(Acos,DoubleComplex,acos);
  DECLARE_MATH_RFUNC(Atan,DoubleComplex,atan);
#endif
  DECLARE_MATH_FUNC(Asinh,DoubleComplex,asinh);
  DECLARE_MATH_FUNC(Acosh,DoubleComplex,acosh);
  DECLARE_MATH_FUNC(Atanh,DoubleComplex,atanh);
  DECLARE_MATH_RFUNC(Sinh,DoubleComplex,std::sinh);
  DECLARE_MATH_RFUNC(Cosh,DoubleComplex,std::cosh);
  DECLARE_MATH_RFUNC(Tanh,DoubleComplex,std::tanh);
  DECLARE_MATH_RFUNC(Sqrt,DoubleComplex,std::sqrt);
  DECLARE_MATH_FUNC(Sec,DoubleComplex,sec);
  DECLARE_MATH_FUNC(Csc,DoubleComplex,csc);
  DECLARE_MATH_FUNC(Cot,DoubleComplex,cot);
#if 0
  DECLARE_MATH_FUNC(Asec,DoubleComplex,asec);
  DECLARE_MATH_FUNC(Acsc,DoubleComplex,acsc);
  DECLARE_MATH_FUNC(Acot,DoubleComplex,acot);
#endif 
  DECLARE_MATH_FUNC(Sech,DoubleComplex,sech);
  DECLARE_MATH_FUNC(Csch,DoubleComplex,csch);
  DECLARE_MATH_FUNC(Coth,DoubleComplex,coth);
  DECLARE_MATH_FUNC(Asech,DoubleComplex,asech);
  DECLARE_MATH_FUNC(Acsch,DoubleComplex,acsch);
  DECLARE_MATH_FUNC(Acoth,DoubleComplex,acoth);
  DECLARE_MATH_FUNC(Sinc,DoubleComplex,sinc);
  DECLARE_MATH_FUNC(Sinhc,DoubleComplex,sinhc);
 
  /**********************/

  // These functions only make sense with complex numbers

  //! conjugate of a complex number
  Complex conj (const Complex &z) {
    return ::std::conj((const complex<float> &)z);
  }

  //! conjugate of double-precision complex number
  DoubleComplex conj (const DoubleComplex &z) {
    return ::std::conj((const complex<double> &)z);
  }

  DECLARE_MATH_RFUNC(Conj,Complex,conj);  
  DECLARE_MATH_DIFFERENT_TYPES(Real,Complex,Float,real);
  DECLARE_MATH_DIFFERENT_TYPES(Imag,Complex,Float,imag);
  DECLARE_MATH_DIFFERENT_TYPES(Norm,Complex,Float,norm);
  DECLARE_MATH_DIFFERENT_TYPES(Angle,Complex,Float,arg);

  DECLARE_MATH_RFUNC(Conj,DoubleComplex,conj);  
  DECLARE_MATH_DIFFERENT_TYPES(Real,DoubleComplex,Double,real);
  DECLARE_MATH_DIFFERENT_TYPES(Imag,DoubleComplex,Double,imag);
  DECLARE_MATH_DIFFERENT_TYPES(Norm,DoubleComplex,Double,norm);
  DECLARE_MATH_DIFFERENT_TYPES(Angle,DoubleComplex,Double,arg);

  /******************************/

  //! GCD Transform
  /**
   *  Transform to compute the greatest-common divisor for several types
   */

  class GCDTransform : public Transform {

  public:
    ChangeableInlet a,b;
    BaseState       *out;

    GCDTransform (const string &aname,
		  Container *acontainer,
		  int argc, char *argv[]) :
      Transform (aname,acontainer,argc,argv),
      a("a",this,"First GCD parameter"),
      b("b",this,"Second GCD parameter"),
      out(NULL),
      activation1(2,this,&GCDTransform::activation1_expr) {

      out = new Outlet<Any>("out",this,"GCD Result");
      activation1.AddActivator(a);
      activation1.AddActivator(b);
      activation1.AddEffect(out);
    }

    ~GCDTransform () {
      if (out != NULL) {
	delete out;
      }
    }

    const char *Description () const {
      return "Returns the GCD (greatest-common divisor) of two numbers";
    }

  protected:

    Activation<GCDTransform> activation1;

    void InletTypeHasChanged (BaseState *changedInlet) {
      if (out->TypeOf() != changedInlet->TypeOf()) {
	activation1.RemoveEffect(out);
	delete out;
	out = changedInlet->CloneOutlet("out",this,"GCD Result");
	activation1.AddEffect(out);
      }
    }

    void activation1_expr () {
      if (a->TypeOf() == typeid(Integer)) {
	if (b->TypeOf() == typeid(Integer)) {
	  *((State<Integer> *)out) = Gcd(*((State<Integer> *)(BaseState *)a),
		     *((State<Integer> *)(BaseState *)b));
	} 
      } else if (a->TypeOf() == typeid(Float)) {
	if (b->TypeOf() == typeid(Float)) {
	  *((State<Float> *)out) = Gcd(*((State<Float> *)(BaseState *)a),
		     *((State<Float> *)(BaseState *)b));
	}
      } else if (a->TypeOf() == typeid(Double)) {
	if (b->TypeOf() == typeid(Double)) {
	  *((State<Double> *)out) = Gcd(*((State<Double> *)(BaseState *)a),
		     *((State<Double> *)(BaseState *)b));
	}
      } else if (a->TypeOf() == typeid(Complex)) {
	if (b->TypeOf() == typeid(Complex)) {
	  *((State<Complex> *)out) = Gcd(*((State<Complex> *)(BaseState *)a),
					 *((State<Complex> *)(BaseState *)b));
	}
      } else if (a->TypeOf() == typeid(DoubleComplex)) {
	if (b->TypeOf() == typeid(DoubleComplex)) {
	  *((State<DoubleComplex> *)out) = Gcd(*((State<DoubleComplex> *)(BaseState *)a),
					*((State<DoubleComplex> *)(BaseState *)b));
	}
      }      
    }
  };

  static TransformFactory<GCDTransform> GCDTransformFactory("Gcd",true);

  /******************************/

  //! Implementation of special math functions

  Integer Gcd (Integer a, Integer b) {

    // make sure b > a
  start:
    if (a > b) {
      Integer t = b;
      b = a;
      a = t;
    }
    if (b % a == 0) {
      return a;
    } else {
      Integer t = b - a;
      b = a;
      a = t;
      goto start;
    }
    return a; // should never happen
  }
 
  Double Gcd (Double a, Double b) {
  start:
    if (a > b) {
      Double t = b;
      b = a;
      a = t;
    }
    if (fmod(b,a) == 0.0) {
      return a;
    } else {
      Double t = b - a;
      b = a;
      a = t;
      goto start;
    }
    return a;
  }

  DoubleComplex Gcd (DoubleComplex a, DoubleComplex b) {
  start:
    if (norm(a) > norm(b)) {
      DoubleComplex t = b;
      b = a;
      a = t;
    }
    DoubleComplex q = b/a;
    if (imag(q) == 0.0 && floor(real(q)) == 0.0) {
      return a;
    } else {
      DoubleComplex t = b - a;
      b = a;
      a = t;
      goto start;
    }
  }

  Integer Factorial (Integer n) {
    int a = 1;
  start:
    if (n == 0 || n == 1) {
      return a;
    } else {
      a *= n;
      --n;
      goto start;
    }
    return a; // should never happen
  }

  /*
   Natural log of the gamma function (xx > 0)
   Derived from "Numerical Receipes in C"
  */
  double LnGamma(double xx)
  {
    int j;
    double x,y,tmp,ser;
    double cof[6] = {
      76.18009172947146,    -86.50532032941677,
      24.01409824083091,    -1.231739572450155,
      0.1208650973866179e-2,-0.5395239384953e-5
    };
    
    y = x = xx;
    tmp = x + 5.5 - (x + 0.5) * log(x + 5.5);
    ser = 1.000000000190015;
    for (j=0;j<=5;j++)
      ser += (cof[j] / ++y);
    return(log(2.5066282746310005 * ser / x) - tmp);
  }

  Double Gamma (double x) {
    return exp(LnGamma(x));
  }



  /***********************************/


}


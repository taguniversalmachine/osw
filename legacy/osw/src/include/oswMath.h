
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
  oswMath.h
  Declarations of types and transforms for standard math functions

  Amar Chaudhary
*/

#ifndef _OSW_MATH
#define _OSW_MATH

#include "osw.h"
#include <complex>

namespace osw {
  
  //! Base class for OSW math functions
  struct _oswexport BaseDeclareMathFunc {
    
    //! Base constructor for OSW functions
    /**
     * @param aname string name of math function
     * @param type_name string representation of function's return type
     */
    BaseDeclareMathFunc(const string &aname, const string &type_name);

    //! evaluate function using state variables for argument and return value
    /**
     * @param in reference to BaseState containing function argument
     * @param out reference to BaseState to receive function result
     */
    virtual void eval (BaseState &in, BaseState &out) = 0;

    //! evaluate function using Datums for argument and return value
    /**
     * @param in pointer to a Datum to use as function argument
     * @return a new Datum containing function result
     * @sa Datum, BaseDatum
     */
    virtual BaseDatum *evalDatum (BaseDatum *in) = 0;

    //! create a new Outlet of the appropriate type to hold the return value
    /**
     * @param aname name of output
     * @param atransform transform in which outlet will be created
     * @param adesc destriptive string for outlet
     * @return a new outlet object of the appropriate type
     */

    virtual BaseState *MakeOutlet (const string &aname,
                                   Transform *atransform,
                                   const string &adesc) = 0;

    //! return type of the math function
    /**
     * @return return type of math function as RTTI object
     */

    virtual const type_info &ReturnType() const = 0;

  };
  

  template<class T>
    struct DeclareMathFunc : public BaseDeclareMathFunc {

      DeclareMathFunc(const string &aname, 
		      T (* afunc) (T)) :
	BaseDeclareMathFunc(aname,typeid(T).name()),
	xfunc(afunc) {
      }

      virtual void eval (BaseState &in, BaseState &out) {
	State<T> &typed_in = dynamic_cast<State<T> &>(in);
	State<T> &typed_out = dynamic_cast<State<T> &>(out);
	typed_in.Lock();
	T result = xfunc(T(typed_in));
	typed_in.Unlock();
	typed_out = result;
      }

      virtual BaseDatum *evalDatum (BaseDatum *in) {
	Datum<T> *typed_datum = (Datum<T>*)(in);
	return new Datum<T>(xfunc(T(*typed_datum)));
      }

      virtual BaseState *MakeOutlet (const string &aname,
                                     Transform *atransform,
                                     const string &adesc) {
        return new Outlet<T>(aname,atransform,adesc);
      }
      
      virtual const type_info &ReturnType () const {
        return typeid(T);
      }

      std::pointer_to_unary_function<T,T> xfunc;
    };

  template<class S, class T>
  struct DeclareMathFuncDifferentTypes : public BaseDeclareMathFunc {

     DeclareMathFuncDifferentTypes(const string &aname, 
				   T (* afunc) (const S &)) :
	BaseDeclareMathFunc(aname,typeid(S).name()),
	xfunc(afunc) {
      }

      virtual void eval (BaseState &in, BaseState &out) {
	State<S> &typed_in = dynamic_cast<State<S> &>(in);
	State<T> &typed_out = dynamic_cast<State<T> &>(out);
	typed_in.Lock();
	T result = xfunc(S(typed_in));
	typed_in.Unlock();
	typed_out = result;
      }

      virtual BaseDatum *evalDatum (BaseDatum *in) {
	Datum<S> *typed_datum = (Datum<S>*)(in);
	return new Datum<T>(xfunc(S(*typed_datum)));
      }

      virtual BaseState *MakeOutlet (const string &aname,
                                     Transform *atransform,
                                     const string &adesc) {
        return new Outlet<T>(aname,atransform,adesc);
      }

      virtual const type_info &ReturnType () const {
        return typeid(T);
      }

      std::pointer_to_unary_function<const S &,T> xfunc;
  };

  template<class T>
    struct DeclareMathVFunc : public BaseDeclareMathFunc {

      DeclareMathVFunc(const string &aname, 
		       unopvector<vector<T>, std::pointer_to_unary_function<T,T>  > (*afunc)(const vector<T> &)) :
	BaseDeclareMathFunc(aname,typeid(vector<T>).name()),
	xfunc(afunc) {
      }

      virtual void eval (BaseState &in, BaseState &out) {
	State<vector<T> > &typed_in = dynamic_cast<State<vector<T> > &>(in);
	State<vector<T> > &typed_out = dynamic_cast<State<vector<T> > &>(out);
	typed_in.Lock();
	vector<T> result = xfunc(typed_in);
	typed_in.Unlock();
	typed_out = result;
      }

      virtual BaseDatum *evalDatum (BaseDatum *in) {
	Datum<vector<T> > *typed_datum = (Datum<vector<T> >*)(in);
	return new Datum<vector<T> >(xfunc(vector<T>(*typed_datum)));
      }

      virtual BaseState *MakeOutlet (const string &aname,
                                     Transform *atransform,
                                     const string &adesc) {
        return new Outlet<vector<T> >(aname,atransform,adesc);
      }

      virtual const type_info &ReturnType () const {
        return typeid(vector<T>);
      }

      unopvector<vector<T>, std::pointer_to_unary_function<T,T> >  (*xfunc)(const vector<T> &);
    };

  template<class T>
    struct DeclareMathRFunc : public BaseDeclareMathFunc {

      DeclareMathRFunc(const string &aname, 
		       T (* afunc)(const T &)) :
	BaseDeclareMathFunc(aname,typeid(T).name()),
	xfunc(afunc) {
      }

      virtual void eval (BaseState &in, BaseState &out) {
	State<T> &typed_in = dynamic_cast<State<T> &>(in);
	State<T> &typed_out = dynamic_cast<State<T> &>(out);
	typed_in.Lock();
	T result = xfunc(T(typed_in));
	typed_in.Unlock();
	typed_out = result;
      }

      virtual BaseDatum *evalDatum (BaseDatum *in) {
	Datum<T> *typed_datum = (Datum<T>*)(in);
	return new Datum<T>(xfunc(*typed_datum));
      }

      virtual BaseState *MakeOutlet (const string &aname,
                                     Transform *atransform,
                                     const string &adesc) {
        return new Outlet<T>(aname,atransform,adesc);
      }

      virtual const type_info &ReturnType () const {
        return typeid(T);
      }

      std::pointer_to_unary_function<const T &,T> xfunc;
    };

  

  struct MathTransformFactory 
    : public BaseTransformFactory {
      
      MathTransformFactory (const string &afunc_name) :
	BaseTransformFactory(afunc_name,"",true),
	xfunc_name(afunc_name) {
      }

      Transform *Construct (const string &nanme,
			    const string &acontainerName,
			    int argc, char *argv[]);
  private:

      string xfunc_name;
  };
  
  BaseDeclareMathFunc _oswexport *LookupMathFunc(const string &aname,
						 const string &type_name);



  // Some useful math functions for OSW

  //! returns the factorial of an integer n
  Integer Factorial (Integer n);

  //! returns the extended-definition GCD (greatest-common divisor) of two Double-precision numbers a and b
  /**
   * the values of a and b need not be integral.
   */
  Double Gcd (Double a, Double b);
  
  //! returns the extended-defintion GCD (greatest-common divisor of two Double-precision complex numbers a and b
  DoubleComplex Gcd( DoubleComplex a,  DoubleComplex b);
     
  //! returns the extended-definition GCD (greatest-common divisor of two single-precision complex numbers a and b
  inline Complex Gcd( Complex a,  Complex b) {
    DoubleComplex result = Gcd(DoubleComplex(double(real(a)),double(imag(b))),
			       DoubleComplex(double(real(b)),double(imag(b))));
    return Complex(Float(real(result)),Float(imag(result)));
  }

  //! Float overload of GcdX
  /**
   * @see GcdX
   */
  inline Float Gcd(Float a, Float b) { return Gcd(Double(a),Double(b));}
  
  //! Integer implementation of GCD (greatest-command divisor function)
  Integer Gcd (Integer a, Integer b);

  //! LCM (least-common-multiple) of two real numbers a and b
  /**
   * LCM(a,b) == (a * b) / GCD(a,b)
   */

  template<class T> 
  T Lcm (T a, T b) {
    return a * b / Gcd(a,b);
  }



  //! Gamma function (real-valued factorial)
  Double Gamma (Double x);

  //! Overload of Gamma for Float
  inline Float Gamma (Float x) { return Gamma(Double(x));}

  //! Overload of Gamma for Integers
  inline Integer Gamma (Integer x) {return (Integer) Gamma(Double(x));}

  STDUNFUNC(Gamma);
}



#define DECLARE_MATH_FUNC(name,type,func) \
static DeclareMathFunc<type > DMF##name##type (#name,(type (*)(type)) func);

#define DECLARE_MATH_RFUNC(name,type,func) \
static DeclareMathRFunc<type > DMF##name##type (#name,(type (*)(const type &)) func);

#define DECLARE_MATH_VFUNC(name,type,func) \
static DeclareMathVFunc<type> \
  DMF##name##vector##type (#name,(unopvector<osw::vector<type>,std::pointer_to_unary_function<type,type> > (*)(const osw::vector<type> &)) &func);

#define DECLARE_MATH_TFUNC(name,type,func) \
static DeclareMathFunc<type > DMF##name##type (#name,(type (*)(type)) &func<type>);

#define DECLARE_MATH_TRFUNC(name,type,func) \
static DeclareMathFunc<type > DMF##name##type (#name,(type (*)(const type &)) &func<type>);

#define DECLARE_MATH_DIFFERENT_TYPES(name,intype,outtype,func) \
static DeclareMathFuncDifferentTypes<intype,outtype> DMF##name##intype##outtype (#name,(outtype (*)(const intype &)) func);

#endif // _OSW_MATH


/*
Copyright (c) 2001 Amar Chaudhary
Copyright (c) 1999-2001 Regents of the University of California.
All rights reserved.

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
  oswBinop.h
  Headers for the standard binary operators (+,-,*,/) supporting
  several different scalar and vector types.  The supported types
  can be extended using declarations in this header.

  Amar Chaudhary

*/

#ifndef _OSW_BINOP
#define _OSW_BINOP

#include "osw.h"

// used for array indexing and template parameters
#define ADD 0
#define SUBTRACT 1
#define MULTIPLY 2
#define DIVIDE 3
#define LESS 4
#define GREATER 5
#define LTEQ 6
#define GTEQ 7
#define EQUAL 8
#define MODULUS 9
#define LOGAND 10
#define LOGOR 11
#define EXPONENT 12
#define SHIFTL 13
#define SHIFTR 14
#define BITAND 15
#define BITOR  16
#define BITXOR 17
#define NOTEQUAL 18

#define LASTOP NOTEQUAL

using namespace std;



namespace osw {

  /***********************/
  
  // relational operator for complex numbers
  template<class T>
    bool operator < (const complex<T> &a, const complex<T> &b) {
    return norm(a) < norm(b);
}
  template<class T>
    bool operator > (const complex<T> &a, const complex<T> &b) {
    return norm(a) > norm(b);
  }
  template<class T>
    bool operator <= (const complex<T> &a, const complex<T> &b) {
    return !(norm(a) > norm(b));
  }
  template<class T>
    bool operator >= (const complex<T> &a, const complex<T> &b) {
    return !(norm(a) < norm(b));
  }
  
  /***********************/


  struct _oswexport BaseBinop;

  typedef stl::map<string, stl::map<string, BaseBinop *, less<string> >, less<string> > BinopMap;

  extern _oswexport BinopMap *TheBinopMap;

  _oswexport BaseBinop *LookupBinop(int opnum, const type_info &type1, const type_info &type2);

  /******************/

  struct _oswexport BaseBinop {

    static void Register (int opnum, const string &type1,
			  const string &type2, BaseBinop *toRegister);
    
    BaseBinop ();
    
    virtual void eval (BaseState &arg1, BaseState &arg2, 
		       BaseState &result) const = 0;
    virtual BaseDatum *eval_from_data (BaseDatum &arg1, BaseDatum &arg2) const = 0;
    virtual BaseState *MakeOutlet (const string &aname,
				   Transform *atransform,
				   const string &adesc = "") const = 0;
    virtual const type_info &ResultType () const = 0;
  };


  /***************/

  template <class A1, class A2, class Result>
  struct add {
    Result operator () (const A1 &arg1, const A2 &arg2) {
      return Result(arg1 + arg2);
    }
    
    typedef A1 Arg1Type;
    typedef A2 Arg2Type;
    typedef Result ResultType;
  };
  
  template <class A1, class A2, class Result>
  struct subtract {
    Result operator () (const A1 &arg1, const A2 &arg2) {
      return Result(arg1 - arg2);
    }
    
    typedef A1 Arg1Type;
    typedef A2 Arg2Type;
    typedef Result ResultType;
  };
  
  template <class A1, class A2, class Result>
  struct multiply {
    Result operator () (const A1 &arg1, const A2 &arg2) {
      return Result(arg1 * arg2);
    }
    
    typedef A1 Arg1Type;
    typedef A2 Arg2Type;
    typedef Result ResultType;
  };
  
  template <class A1, class A2, class Result>
  struct divide {
    Result operator () (const A1 &arg1, const A2 &arg2) {
      return Result(arg1 / arg2);
    }
    
    typedef A1 Arg1Type;
    typedef A2 Arg2Type;
    typedef Result ResultType;
  };
  
  template <class A1, class A2, class Result>
  struct modulus {
    Result operator () (const A1 &arg1, const A2 &arg2) {
      return Result(arg1 % arg2);
    }
    
    typedef A1 Arg1Type;
    typedef A2 Arg2Type;
    typedef Result ResultType; 
  };
  
  template <class A1, class A2, class Result>
  struct fmodulus {
    Result operator () (const A1 &arg1, const A2 &arg2) {
      return Result(fmod(arg1,arg2));
    }
    
    typedef A1 Arg1Type;
    typedef A2 Arg2Type;
    typedef Result ResultType; 
  };
  
#ifdef _MSC_VER
  inline float pow(float x, float y) {return powf(x,y);}
  inline float pow(int x, float y) {return powf(x,y);}
  inline float pow(float x, int y) {return powf(x,y);}
  inline float pow(short x, float y) {return powf(x,y);}
  inline float pow(float x, short y) {return powf(x,y);}
  inline float pow(int x, int y) {return powf(x,y);}
  inline float pow(short x, int y) {return powf(x,y);}
  inline float pow(int x, short y) {return powf(x,y);}
  inline float pow(short x, short y) {return powf(x,y);}
  inline double pow (double x, double y) {return ::pow(x,y);}
  inline double pow (double x, float y) {return ::pow(x,y);}
  inline double pow (float x, double y) {return ::pow(x,y);}
  inline double pow (int x, double y) {return ::pow(x,y);}
  inline double pow (double x, int y) {return ::pow(x,y);}
  
  //template<class T>
  //inline complex<float> pow (const complex<float> &x, const T &y) {return stl::pow(x,complex<float>(y));}
  //template<class T>
  //inline complex<float> pow (const T &x, const complex<float> &y) {return stl::pow(complex<float>(x),y);}

  inline complex<float> pow (const complex<float> &x, const complex<float> &y) {return stl::pow(x,y);}
  inline complex<double> pow (const complex<double> &x, const complex<double> &y) {return stl::pow(x,y);}
#endif  

#if defined(__GNUC__)
#if __GNUC__ >= 3  
  inline float pow(short x, float y) {return ::pow((float)x,(float)y);}
  inline float pow(float x, short y) {return ::pow((float)x,(float)y);}
  inline float pow(int x, float y) {return ::pow((float)x,(float)y);}
  inline float pow(float x, int y) {return ::pow((float)x,(float)y);}
  inline double pow(int x, double y) {return ::pow((double)x,(double)y);}
  inline double pow(double x, int y) {return ::pow((double)x,(double)y);}
  inline double pow(double x, short y) {return ::pow((double)x,(double)y);}
  inline double pow(short x, double y) {return ::pow((double)x,(double)y);}
  inline int pow(int x, int y)  {return (int)::pow((float)x,(float)y);}
  inline int pow(short x, int y)  {return (int)::pow((float)x,(float)y);}
  inline int pow(int x, short y)  {return (int)::pow((float)x,(float)y);}
  inline int pow(short x, short y)  {return (int)::pow((float)x,(float)y);}
  inline float pow(float x, float y) {return ::pow(x,y);}
  inline double pow (double x, double y) {return ::pow(x,y);}
  inline double pow (double x, float y) {return ::pow((double)x,(double)y);}
  inline double pow (float x, double y) {return ::pow((double)x,(double)y);}
#if __GNUC_MINOR__ > 1
  inline double fmod(float x, double y) {return ::fmod((double)x,(double)y);}
  inline double fmod(double x, float y) {return ::fmod((double)x,(double)y);}
  inline double fmod(double x, double y) {return ::fmod(x,y);}
  inline double fmod(float x, float y) {return ::fmod(x,y);}
  inline float fmod(short x, float y) {return ::fmod((float)x,(float)y);}
  inline float fmod(float x, short y) {return ::fmod((float)x,(float)y);}
  inline float fmod(int x, float y) {return ::fmod((float)x,(float)y);}
  inline float fmod(float x, int y) {return ::fmod((float)x,(float)y);}
  inline double fmod(int x, double y) {return ::fmod((double)x,(double)y);}
  inline double fmod(double x, int y) {return ::fmod((double)x,(double)y);}
  inline double fmod(double x, short y) {return ::fmod((double)x,(double)y);}
  inline double fmod(short x, double y) {return ::fmod((double)x,(double)y);}
#endif
#else // __GNUC__ >= 3
 #ifdef _STLP_DEBUG
 #define _STL _STLD
 #endif
  inline float pow(short x, float y) {return _STL::pow((float)x,(float)y);}
  inline double pow(int x, double y) {return _STL::pow((double)x,(double)y);}
  inline double pow(double x, int y) {return _STL::pow((double)x,(double)y);}
  inline double pow(double x, short y) {return _STL::pow((double)x,(double)y);}
  inline double pow(short x, double y) {return _STL::pow((double)x,(double)y);}
  inline int pow(int x, int y)  {return (int)_STL::pow((float)x,(float)y);}
  inline int pow(short x, int y)  {return (int)_STL::pow((float)x,(float)y);}
  inline int pow(int x, short y)  {return (int)_STL::pow((float)x,(float)y);}
  inline int pow(short x, short y)  {return (int)_STL::pow((float)x,(float)y);}
  inline float pow(float x, float y) {return _STL::pow(x,y);}
  inline double pow (double x, double y) {return _STL::pow(x,y);}
  inline double pow (double x, float y) {return _STL::pow(x,y);}
  inline double pow (float x, double y) {return _STL::pow(x,y);}
#endif // __APPLE__
#endif // __GNUC__

#if defined(__sgi) 
  inline float pow(short x, float y) {return ::powf((float)x,(float)y);}
  inline float pow(int x, float y) {return ::powf((float)x,(float)y);}
  inline float pow(float x, int y) {return ::powf((float)x,(float)y);}
  inline double pow(int x, double y) {return ::pow((double)x,(double)y);}
  inline double pow(double x, int y) {return ::pow((double)x,(double)y);}
  inline double pow(double x, short y) {return ::pow((double)x,(double)y);}
  inline double pow(short x, double y) {return ::pow((double)x,(double)y);}
  inline int pow(int x, int y)  {return (int)::powf((float)x,(float)y);}
  inline int pow(short x, int y)  {return (int)::powf((float)x,(float)y);}
  inline int pow(int x, short y)  {return (int)::powf((float)x,(float)y);}
  inline int pow(short x, short y)  {return (int)::powf((float)x,(float)y);}
  inline float pow(float x, float y) {return ::powf(x,y);}
  inline double pow (double x, double y) {return ::pow(x,y);}
  inline double pow (double x, float y) {return ::pow((double)x,(double)y);}
  inline double pow (float x, double y) {return ::pow((double)x,(double)y);}
  inline complex<float> pow (const complex<float> &x, 
			     const complex<float> &y) {
    return std::pow(x,y);
  }
  inline complex<double> pow (const complex<double> &x, 
			      const complex<double> &y) {
    return std::pow(x,y);
  }
  inline double fmod(double x, float y) {return ::fmod((double)x, double(y));}
  inline double fmod(float x, double y) {return ::fmod((double)x, double(y));}
  inline double fmod(double x, double y) {return ::fmod((double)x, double(y));}
  inline double fmod(float x, float y) {return ::fmodf((float)x, float(y));}
  inline double fmod(double x, int y) {return ::fmod((double)x, double(y));}
  inline double fmod(int x, double y) {return ::fmod((double)x, double(y));}
  inline double fmod(float x, int y) {return ::fmodf((float)x, float(y));}
  inline double fmod(int x, float y) {return ::fmodf((float)x, float(y));}
#endif

  BINOP(pow,_pow);
  BINOPSCALAR(pow,_pow);


  template <class A1, class A2, class Result>
  struct exponent {
    Result operator () (const A1 &arg1, const A2 &arg2) {
      return Result(pow(arg1,arg2));
    }
    
    typedef A1 Arg1Type;
    typedef A2 Arg2Type;
    typedef Result ResultType; 
  };
  
  template <class A1, class A2, class Result>
  struct logand {
    Result operator () (const A1 &arg1, const A2 &arg2) {
      return Result(arg1 && arg2);
    }
    
    typedef A1 Arg1Type;
    typedef A2 Arg2Type;
    typedef Result ResultType; 
  };
  
  template <class A1, class A2, class Result>
  struct logor {
    Result operator () (const A1 &arg1, const A2 &arg2) {
      return Result(arg1 || arg2);
    }
    
    typedef A1 Arg1Type;
    typedef A2 Arg2Type;
    typedef Result ResultType; 
  };
  
  template <class A1, class A2, class Result>
  struct binop_less {
    Result operator () (const A1 &arg1, const A2 &arg2) {
      return Result(arg1 < arg2);
    }
    typedef A1 Arg1Type;
    typedef A2 Arg2Type;
    typedef Result ResultType;
  };
  
  template <class A1, class A2, class Result>
  struct binop_greater {
    Result operator () (const A1 &arg1, const A2 &arg2) {
      return Result(arg1 > arg2);
    }
    typedef A1 Arg1Type;
    typedef A2 Arg2Type;
    typedef Result ResultType;
  };
  
  template <class A1, class A2, class Result>
  struct binop_less_eq {
    Result operator () (const A1 &arg1, const A2 &arg2) {
      return Result(arg1 <= arg2);
    }
    typedef A1 Arg1Type;
    typedef A2 Arg2Type;
    typedef Result ResultType;
  };
  
  template <class A1, class A2, class Result>
  struct binop_greater_eq {
    Result operator () (const A1 &arg1, const A2 &arg2) {
      return Result(arg1 >= arg2);
    }
    typedef A1 Arg1Type;
    typedef A2 Arg2Type;
    typedef Result ResultType;
  };
  
  template <class A1, class A2, class Result>
  struct binop_eq {
    Result operator () (const A1 &arg1, const A2 &arg2) {
      return Result(arg1 == arg2);
    }
    typedef A1 Arg1Type;
    typedef A2 Arg2Type;
    typedef Result ResultType;
  };

  template <class A1, class A2, class Result>
  struct binop_neq {
    Result operator () (const A1 &arg1, const A2 &arg2) {
      return Result((A1) arg1 != (A2) arg2);
    }
    typedef A1 Arg1Type;
    typedef A2 Arg2Type;
    typedef Result ResultType;
  };
  
  template <class A1, class A2, class Result>
  struct lshift {
    Result operator () (const A1 &arg1, const A2 &arg2) {
      return Result(arg1 << arg2);
    }
    typedef A1 Arg1Type;
    typedef A2 Arg2Type;
    typedef Result ResultType;
  };

  template <class A1, class A2, class Result>
  struct rshift {
    Result operator () (const A1 &arg1, const A2 &arg2) {
      return Result(arg1 >> arg2);
    }
    typedef A1 Arg1Type;
    typedef A2 Arg2Type;
    typedef Result ResultType;
  };

  template <class A1, class A2, class Result>
  struct binop_bitand {
    Result operator () (const A1 &arg1, const A2 &arg2) {
      return Result(arg1 & arg2);
    }
    typedef A1 Arg1Type;
    typedef A2 Arg2Type;
    typedef Result ResultType;
  };

  template <class A1, class A2, class Result>
  struct binop_bitor {
    Result operator () (const A1 &arg1, const A2 &arg2) {
      return Result(arg1 | arg2);
    }
    typedef A1 Arg1Type;
    typedef A2 Arg2Type;
    typedef Result ResultType;
  };

  template <class A1, class A2, class Result>
  struct bitxor {
    Result operator () (const A1 &arg1, const A2 &arg2) {
      return Result(arg1 ^ arg2);
    }
    typedef A1 Arg1Type;
    typedef A2 Arg2Type;
    typedef Result ResultType;
  };

  /***************/

  template <class Op, int opnum> 
    struct DeclareBinop : public BaseBinop {
      
      DeclareBinop () {
	Register(opnum,typeid(_TYPENAME Op::Arg1Type).name(),
		 typeid(_TYPENAME Op::Arg2Type).name(),this);
      }

      void eval (BaseState &arg1, BaseState &arg2, 
		 BaseState &result) const {
	State<_TYPENAME Op::Arg1Type> &typed_arg1 =
	  dynamic_cast<State<_TYPENAME Op::Arg1Type> &>(arg1);
	State<_TYPENAME Op::Arg2Type> &typed_arg2 =
	  dynamic_cast<State<_TYPENAME Op::Arg2Type> &>(arg2);
	typed_arg1.Lock();
	typed_arg2.Lock();
	_TYPENAME Op::ResultType tempResult = Op()(typed_arg1,typed_arg2);
	typed_arg1.Unlock();
	typed_arg2.Unlock();
	dynamic_cast<State<_TYPENAME Op::ResultType> &>(result) = tempResult;	
      }

      BaseDatum *eval_from_data (BaseDatum &arg1, BaseDatum &arg2) const {
	Datum<_TYPENAME Op::Arg1Type> &typed_arg1 = 
	  dynamic_cast<Datum<_TYPENAME Op::Arg1Type> &>(arg1);
	Datum<_TYPENAME Op::Arg2Type> &typed_arg2 = 
	  dynamic_cast<Datum<_TYPENAME Op::Arg2Type> &>(arg2);
	return new Datum<_TYPENAME Op::ResultType>(Op()(typed_arg1,typed_arg2));
      }
						    
      BaseState *MakeOutlet (const string &aname,
			     Transform *atransform,
			     const string &adesc = "") const {
	return osw::MakeOutlet (typeid(_TYPENAME Op::ResultType).name(),aname,
				atransform,adesc);
      }

      const type_info &ResultType () const {
	return typeid(_TYPENAME Op::ResultType);
      }
    };

  template <class Op, class CastType, int opnum> 
    struct CastLeftBinop : public BaseBinop {
      
      CastLeftBinop () {
	Register(opnum,typeid(_TYPENAME Op::Arg1Type).name(),
		 typeid(_TYPENAME Op::Arg2Type).name(),this);
      }

      void eval (BaseState &arg1, BaseState &arg2, 
		 BaseState &result) const {

	State<CastType> &typed_arg1 =
	  dynamic_cast<State<CastType> &>(arg1);
	State<_TYPENAME Op::Arg2Type> &typed_arg2 =
	  dynamic_cast<State<_TYPENAME Op::Arg2Type> &>(arg2);
	typed_arg1.Lock();
	typed_arg2.Lock();
	_TYPENAME Op::ResultType tempResult = Op()(_TYPENAME Op::Arg1Type(CastType(typed_arg1)),
						   typed_arg2);
	typed_arg1.Unlock();
	typed_arg2.Unlock();
	dynamic_cast<State<_TYPENAME Op::ResultType> &>(result) = tempResult;	
      }

      BaseDatum *eval_from_data (BaseDatum &arg1, BaseDatum &arg2) const {
	Datum<_TYPENAME Op::Arg1Type> &typed_arg1 = 
	  dynamic_cast<Datum<_TYPENAME Op::Arg1Type> &>(arg1);
	Datum<_TYPENAME Op::Arg2Type> &typed_arg2 = 
	  dynamic_cast<Datum<_TYPENAME Op::Arg2Type> &>(arg2);
	return new Datum<_TYPENAME Op::ResultType>(Op()(_TYPENAME Op::Arg1Type(CastType(typed_arg1)),
							typed_arg2));
      }
	       
      BaseState *MakeOutlet (const string &aname,
			     Transform *atransform,
			     const string &adesc = "") const {
	return osw::MakeOutlet (typeid(_TYPENAME Op::ResultType).name(),aname,
				atransform,adesc);
      }

      const type_info &ResultType () const {
	return typeid(_TYPENAME Op::ResultType);
      }
    };

  template <class Op, class CastType, int opnum> 
    struct CastRightBinop : public BaseBinop {
      
      CastRightBinop () {
	Register(opnum,typeid(_TYPENAME Op::Arg1Type).name(),
		 typeid(_TYPENAME Op::Arg2Type).name(),this);
      }

      void eval (BaseState &arg1, BaseState &arg2, 
		 BaseState &result) const {
	State<_TYPENAME Op::Arg1Type> &typed_arg1 =
	  dynamic_cast<State<_TYPENAME Op::Arg1Type> &>(arg1);
	State<CastType> &typed_arg2 =
	  dynamic_cast<State<CastType> &>(arg2);
	typed_arg1.Lock();
	typed_arg2.Lock();
	_TYPENAME Op::ResultType tempResult = Op()(typed_arg1,
					 (_TYPENAME Op::Arg2Type)(CastType(typed_arg2)));
	typed_arg1.Unlock();
	typed_arg2.Unlock();
	dynamic_cast<State<_TYPENAME Op::ResultType> &>(result) = tempResult;	
      }

      BaseDatum *eval_from_data (BaseDatum &arg1, BaseDatum &arg2) const {
	Datum<_TYPENAME Op::Arg1Type> &typed_arg1 = 
	  dynamic_cast<Datum<_TYPENAME Op::Arg1Type> &>(arg1);
	Datum<_TYPENAME Op::Arg2Type> &typed_arg2 = 
	  dynamic_cast<Datum<_TYPENAME Op::Arg2Type> &>(arg2);
	return new Datum<_TYPENAME Op::ResultType>(Op()(typed_arg1,
							(_TYPENAME Op::Arg2Type)(CastType(typed_arg2))));
      }	
       
      BaseState *MakeOutlet (const string &aname,
			     Transform *atransform,
			     const string &adesc = "") const {
	return osw::MakeOutlet (typeid(_TYPENAME Op::ResultType).name(),aname,
				atransform,adesc);
      }

      const type_info &ResultType () const {
	return typeid(_TYPENAME Op::ResultType);
      }
    };
  
  template <class A1, class A2, class Result>
    struct DeclareScalarBinops {
      DeclareBinop <add<A1,A2,Result>,ADD > add1;
      DeclareBinop <add<A2,A1,Result>,ADD > add2;
      DeclareBinop <subtract<A1,A2,Result>,SUBTRACT > subtract1;
      DeclareBinop <subtract<A2,A1,Result>,SUBTRACT > subtract2;
      DeclareBinop <multiply<A1,A2,Result>,MULTIPLY > multiply1;
      DeclareBinop <multiply<A2,A1,Result>,MULTIPLY > multiply2;
      DeclareBinop <divide<A1,A2,Result>,DIVIDE > divide1;
      DeclareBinop <divide<A2,A1,Result>,DIVIDE > divide2;
      DeclareBinop <exponent<A1,A2,Result>,EXPONENT > exponent1;
      DeclareBinop <exponent<A2,A1,Result>,EXPONENT > exponent2;
    };

  template <class A1, class A2> 
    struct DeclareScalarRelops {
      DeclareBinop <binop_less<A1,A2,bool>,LESS > less1;
      DeclareBinop <binop_less<A2,A1,bool>,LESS > less2;
      DeclareBinop <binop_greater<A1,A2,bool>,GREATER > greater1;
      DeclareBinop <binop_greater<A2,A1,bool>,GREATER > greater2;
      DeclareBinop <binop_less_eq<A1,A2,bool>,LTEQ > lteq1;
      DeclareBinop <binop_less_eq<A2,A1,bool>,LTEQ > lteq2;
      DeclareBinop <binop_greater_eq<A1,A2,bool>,GTEQ > gteq1;
      DeclareBinop <binop_greater_eq<A2,A1,bool>,GTEQ > gteq2;
      DeclareBinop <binop_eq<A2,A1,bool>,EQUAL > eq1;
      DeclareBinop <binop_eq<A1,A2,bool>,EQUAL > eq2;
      DeclareBinop <binop_neq<A2,A1,bool>,NOTEQUAL > neq1;
      DeclareBinop <binop_neq<A1,A2,bool>,NOTEQUAL > neq2;
    };

  template <class A>
    struct DeclareSymmetricBinops {
      DeclareBinop <add<A,A,A>, ADD> add1;
      DeclareBinop <subtract<A,A,A>, SUBTRACT> subtract1;
      DeclareBinop <multiply<A,A,A>, MULTIPLY> multiply1;
      DeclareBinop <divide<A,A,A>, DIVIDE> divide1;
      DeclareBinop <exponent<A,A,A>,EXPONENT > exponent1;
      DeclareBinop <binop_less<A,A,bool>,LESS > less1;
      DeclareBinop <binop_greater<A,A,bool>,GREATER > greater1;
      DeclareBinop <binop_less_eq<A,A,bool>,LTEQ > lteq1;
      DeclareBinop <binop_greater_eq<A,A,bool>,GTEQ > gteq1;
      DeclareBinop <binop_eq<A,A,bool>,EQUAL > eq1;      
      DeclareBinop <binop_neq<A,A,bool>,NOTEQUAL > neq1;      
    };


  template <class A1, class A2, class Result>
    struct DeclareModulus {
      DeclareBinop <modulus<A1,A2,Result>, MODULUS> mod1;
    };

  template <class A1, class A2, class Result>
    struct DeclareFModulus {
      DeclareBinop <fmodulus<A1,A2,Result>, MODULUS> mod1;
      DeclareBinop <fmodulus<A2,A1,Result>, MODULUS> mod2;
    };

  template <class A1, class A2, class Result>
  struct DeclareExponent {
      DeclareBinop <exponent<A1,A2,Result>,EXPONENT > exponent1;
      DeclareBinop <exponent<A2,A1,Result>,EXPONENT > exponent2;
  };    

  template <class A1, class A2, class Result>
  struct DeclareLogicalOps {
    DeclareBinop <logand<A1,A2,Result>, LOGAND> land1;
    DeclareBinop <logor<A1,A2,Result>, LOGOR> lor1;
  };

  // need to add modulus on vectors! AC 12/8/99

  template<class A>
    struct DeclareSymmetricVectorBinops {
      DeclareBinop <add<A,A,A>, ADD> add1;
      DeclareBinop <subtract<A,A,A>, SUBTRACT> subtract1;
      DeclareBinop <multiply<A,A,A>, MULTIPLY> multiply1;
      DeclareBinop <divide<A,A,A>, DIVIDE> divide1;
      DeclareBinop <exponent<A,A,A>, EXPONENT> exponent1;
      DeclareBinop <binop_less<A,A,A>,LESS > less1;
      DeclareBinop <binop_greater<A,A,A>,GREATER > greater1;
      DeclareBinop <binop_less_eq<A,A,A>,LTEQ > lteq1;
      DeclareBinop <binop_greater_eq<A,A,A>,GTEQ > gteq1;
    };


  template <class V, class S> 
    struct DeclareVectorBinops {
      CastLeftBinop<add<V,osw::vector<V>,osw::vector<V> >, S, ADD > add2;
      CastRightBinop<add<osw::vector<V>,V,osw::vector<V> >, S, ADD > add1;
      
      CastRightBinop<subtract<osw::vector<V>,V,osw::vector<V> >, S, SUBTRACT > subtract1;
      CastLeftBinop<subtract<V,osw::vector<V>,osw::vector<V> >, S, SUBTRACT > subtract2;
      CastRightBinop<multiply<osw::vector<V>,V,osw::vector<V> >, S, MULTIPLY > multiply1;
      CastLeftBinop<multiply<V,osw::vector<V>,osw::vector<V> >, S, MULTIPLY > multiply2;
      CastRightBinop<divide<osw::vector<V>,V,osw::vector<V> >, S, DIVIDE > divide1;
      CastLeftBinop<divide<V,osw::vector<V>,osw::vector<V> >, S, DIVIDE > divide2;
      CastRightBinop<exponent<osw::vector<V>,V,osw::vector<V> >, S, EXPONENT > exponent1;
      CastLeftBinop<exponent<V,osw::vector<V>,osw::vector<V> >, S, EXPONENT > exponent2;
      CastRightBinop<binop_less<osw::vector<V>,V,osw::vector<V> >, S, LESS > less1;
      CastLeftBinop<binop_less<V,osw::vector<V>,osw::vector<V> >, S, LESS > less2;
      CastRightBinop<binop_greater<osw::vector<V>,V,osw::vector<V> >, S, GREATER > greater1;
      CastLeftBinop<binop_greater<V,osw::vector<V>,osw::vector<V> >, S, GREATER > greater2;
      CastRightBinop<binop_less_eq<osw::vector<V>,V,osw::vector<V> >, S, LTEQ > lteq1;
      CastLeftBinop<binop_less_eq<V,osw::vector<V>,osw::vector<V> >, S, LTEQ > lteq2;
      CastRightBinop<binop_greater_eq<osw::vector<V>,V,osw::vector<V> >, S, GTEQ > gteq1;
      CastLeftBinop<binop_greater_eq<V,osw::vector<V>,osw::vector<V> >, S, GTEQ > gteq2;
      CastRightBinop<binop_eq<osw::vector<V>,V,osw::vector<V> >, S, EQUAL > eq1;
      CastLeftBinop<binop_eq<V,osw::vector<V>,osw::vector<V> >, S, EQUAL > eq2;
      CastRightBinop<binop_eq<osw::vector<V>,V,osw::vector<V> >, S, NOTEQUAL > neq1;
      CastLeftBinop<binop_eq<V,osw::vector<V>,osw::vector<V> >, S, NOTEQUAL > neq2;
      
    };

}

#endif // _OSW_BINOP


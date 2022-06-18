
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
   oswvect.h
   The OSW vector library
   A numeric vector with efficient memory-management and optimized arithmetic
   functions

   Amar Chaudhary
*/

#ifndef _OSWVECT
#define _OSWVECT

#ifdef OSW_NO_STDC_HEADERS
#include <stdlib.h>
#else
#include <cstdlib>
#endif
#include "m1.h"
#include <vector>
#ifdef OSW_EXT_HASH_MAP
#include <ext/functional>
#include <ext/iterator>
#else
#include <functional>
#include <iterator>
#endif

#ifndef IOSTREAM_INCLUDED
#  include <iostream>
#endif
#include "oswMem.h"

using namespace std;

namespace osw {

  template <class InputVector,class UnaryOperator,class Type = _TYPENAME InputVector::value_type>
    class unopvector;
  
  template <class S, class T,class BinaryOperator>
    class binopvector;

  /**********************/
  
  // this macro defines all the overloads for each assignment operater
      
#define ASSIGNOP(op,opfunc) \
  vector<T> & operator op (const T &operand) { \
    return apply_unary_self (binder2nd<opfunc<T> >(opfunc<T>(),operand));\
  }\
  vector<T> & operator op (const vector<T> &opvect) {\
    return apply_vector_self(opfunc<T>(),opvect);\
  }\
  template <class InputVector, class Unop>\
  vector<T> & operator op (const unopvector<InputVector,Unop> &opvect) {\
    return apply_vector_self(opfunc<T>(),opvect);\
  }\
  template <class InputVector1, class InputVector2, class Binop>\
  vector<T> & operator op (const binopvector<InputVector1,InputVector2,Binop> &opvect) {\
    return apply_vector_self(opfunc<T>(),opvect);\
  }

  //! implementation of reference-counted shared data for OSW vectors
  /**
   * vector_share is for use by the osw::vector class and should not be called directly
   */
  template<class T>
    class vector_share {
      
    public:
      
      typedef T value_type;
      typedef T *pointer;
      typedef const T *const_pointer;
      typedef T &reference;
      typedef const T &const_reference;

      typedef pointer iterator;
      typedef const_pointer const_iterator;
#ifdef REVERSE_ITERATOR_2_ARGS
      typedef _TYPENAME2 std::reverse_iterator<iterator,value_type> reverse_iterator;
      typedef _TYPENAME2 std::reverse_iterator<const_iterator,value_type> const_reverse_iterator;
#else
      typedef _TYPENAME2 std::reverse_iterator<iterator> reverse_iterator;
      typedef _TYPENAME2 std::reverse_iterator<const_iterator> const_reverse_iterator;
#endif
 
      vector_share (const size_t asize = 0) :
	xsize(asize),
	xvect((T *) xalloc.allocate (xsize*sizeof(T),NULL)) {
	  xref = 1;
      }
      
      vector_share (const size_t asize, const T& aval) :
	xsize(asize),
	xvect((T *) xalloc.allocate (xsize*sizeof(T),NULL)) {
	  xref = 1;
	  for (int i = 0; i < xsize; ++i) {
	    xvect[i] = aval;
	  }
      }

      vector_share(const T *ptr, size_t asize) :
	xsize(asize),
	xvect((T *) xalloc.allocate (xsize*sizeof(T),NULL)) {
	memcpy(xvect,ptr,xsize*sizeof(T));
	xref = 1;
      }

      ~vector_share() {
	xalloc.deallocate(xvect,xsize*sizeof(T));
      }
      
      void AddRef () const {++xref;}
      
      const vector_share<T> *RemoveRef () const {
	--xref;
	if (xref == 0) {
	  delete const_cast<vector_share<T> *>(this);
	}
	return this;
      }
      
      
      T & operator [] (size_t index)  {
	return xvect[index];
      }
      
      T & operator [] (size_t index) const {
	return xvect[index];
      }
      
      size_t size () const {return xsize;}
      
      iterator begin() {return xvect;}
      iterator end() {return xvect+xsize;}
      reverse_iterator rbegin() {return reverse_iterator(end());}
      reverse_iterator rend() {return reverse_iterator(begin());}
      
      const_iterator begin() const {return xvect;}
      const_iterator end() const {return xvect+xsize;}
      const_reverse_iterator rbegin() const {return const_reverse_iterator(begin());}
      const_reverse_iterator rend() const {return const_reverse_iterator(end());}


    private:
      
      mutable size_t      xref,xsize;
      mutable T           *xvect;
      osw::allocator<T>   xalloc;
  };
  
  //! The OSW vector class
  /**
   *  Used for efficient vector operations and representation of signal types
   */
  template<class T>
    class vector  {
      
    public:

      //! The type of values in the vector (same as class template argument T)
      typedef T value_type;

      //! vector iterator
      typedef _TYPENAME vector_share<T>::iterator iterator;

      //! vector reverse iterator
      typedef _TYPENAME vector_share<T>::reverse_iterator reverse_iterator;

      //! iterator for constant vectors (or constant references to vectors)
      typedef _TYPENAME vector_share<T>::const_iterator const_iterator;

      //! reverse iterator for constant vectors (or constant references to vectors)
      typedef _TYPENAME vector_share<T>::const_reverse_iterator const_reverse_iterator;
      
      //! construct a new vector of a given size
      /**
       *  @param asize a size_t representing the size of the new vector
       */
      vector (const size_t asize = 0) :
	xshare (new vector_share<T>(asize)) {
      }
      
      //! construct a new vector of a given size with each element initialized to a given value
      /**
       *  @param asize a size_t representing the size of the new vector
       *  @param aval a value_type representing the initialization value
       */
      vector (const size_t asize, const T& aval) :
	xshare (new vector_share<T>(asize,aval)) {
      }
      
      //! construct a copy of an existing vector (standard copy constructor)
      vector (const vector<T>& toCopy) :
	xshare (toCopy.xshare) {
	  if (xshare != NULL) {
	    xshare->AddRef();
	  }
      }
      
      //! construct a vector from a C-style array
      /**
       * @param ptr a pointer to block of memory containing an array of values of type T
       * @param asize the size of the array (measured in T elements, not bytes)
       */
      vector (const T *ptr, size_t asize) :
	xshare(new vector_share<T>(ptr,asize)) {
      }

      //! construct a vector from the result of a unary operation
      template <class InputVector, class UnaryOperator, class Type>
      inline vector (const unopvector<InputVector,UnaryOperator,Type>& toCopy) :
	xshare (new vector_share<T>(toCopy.size())) {
	  for (int i = 0; i < xshare->size();++i) {
	    (*xshare)[i] = T(toCopy[i]);
	  }
      }
      
      //! construct a vector from the result of a binary operation
      template <class InputVector1, class InputVector2, class BinaryOperator>
      inline vector (const binopvector<InputVector1,InputVector2,BinaryOperator>& toCopy) :
	xshare (new vector_share<T>(toCopy.size())) {
	  for (int i = 0; i < xshare->size(); ++i) {
	    (*xshare)[i] = T(toCopy[i]);
	  }
      }
      
      //! destroy a vector
      ~vector () {
	xshare->RemoveRef();
      }
      
      //! assign a vector variable
      inline vector<T> & operator = (const vector<T>& toAssign) {
	if (xshare != NULL) {
	  xshare->RemoveRef();
	}
	xshare = toAssign.xshare;
	if (xshare != NULL) {
	  xshare->AddRef();
	}
	return *this;
      }
      
      //! assign a vector variable from the result of a unary operation
      template <class InputVector, class UnaryOperator>
      inline vector<T> & operator = (const unopvector<InputVector,UnaryOperator>& toAssign) {
	if (xshare != NULL) {
	  xshare->RemoveRef();
	}
	xshare = toAssign.xshare;
	if (xshare != NULL) {
	  xshare->AddRef();
	  for (int i = 0; i < xshare->size(); ++i) {
	    (*xshare)[i] = toAssign[i];
	  }    
	}
	return *this;
      }
      
      //! assign a vector variable from the result of a binary operation
      template <class InputVector1, class InputVector2, class BinaryOperator>
      inline vector<T> & operator = (const binopvector<InputVector1,InputVector2,BinaryOperator>& toAssign) {
	vector_share<T> *newshare = new vector_share<T>(xshare->size());
	for (int i = 0; i < newshare->size(); ++i) {
	  (*newshare)[i] = toAssign[i];
	}
	if (xshare != NULL) {
	  xshare->RemoveRef();
	}	
	xshare = newshare;
	return *this;
      }
      
      
      //! return an element from a vector
      T& operator [] (size_t index) const {
	return (*xshare)[index];
      }
      
      //! return the size of a vector (number of elements)
      size_t size () const {return xshare->size();}
      
      //! return an iterator initialized to the beginning of the vector
      iterator begin() {return xshare->begin();}

      //! return an iterator that represents the end of the vector
      iterator end() {return xshare->end();}

      //! return the type (C++ RTTI type) of the vector elements
      const type_info &type_of() const { return typeid(T);}
      //! return the size of a vector element
      unsigned int    &size_of() const { return sizeof(T);}

#ifndef WEIRD_REVERSE_ITERATOR

      //! return a reverse_iterator initialized to the end of the vector
      reverse_iterator rbegin() {return xshare->rbegin();}

      //! return a reverse_iterator representing the beginning (end of a reverse traversal) of the vector
      reverse_iterator rend() {return xshare->rend();}
#endif

      //! return a constant iterator initialized to the beginning of the vector
      const_iterator begin() const {return xshare->begin();}

      //! return a constant iterator that represents the end of the vector
      const_iterator end() const {return xshare->end();}
      //! return a constant reverse_iterator initialized to the end of the vector
      const_reverse_iterator rbegin() const {return xshare->rbegin();}
      //! return a constant reverse_iterator representing the beginning (end of a reverse traversal) of the vector
      const_reverse_iterator rend() const {return xshare->rend();}
      
      //! increment the reference count of the vector
      void AddRef () const {xshare->AddRef();}

      //! decrement the reference count of the vector
      void RemoveRef () const {xshare->RemoveRef();}

      //! compare to vectors
      /**
       * @param toCompare a vector to compare
       * @return true if both vectors are of the same size and all elements are equal
       */
      bool operator == (const vector<T>& toCompare) const {
	bool result = true;
	if (size() != toCompare.size()) {
	  result = false;
	} else {
	  for (int i = 0; i < xshare->size(); ++i) {
	    if ((*xshare)[i] != toCompare[i]) {
	      result = false;
	      break;
	    }
	  }
	}
	return result;
      }

      //! apply an in-place unary operator to this vector
      /**
       * @param unop a C++ representation (function pointer or functor) of the unary operator
       */
      template<class Unop>
      vector<T> &apply_unary_self(const Unop &unop) {
	for (iterator p = begin(); p != end(); ++p) {
	  *p = unop(*p);
	}
	return *this;
      }
      
      //! apply an in-place binary operator to this vector
      /**
       * @param binop a C++ representation (function pointer or functor) of the binary operator
       * @param avect a vector to use as the second argument to the binary operator
       */
      template<class Binop, class OpVector>
      vector<T> &apply_vector_self(const Binop &binop, const OpVector &avect) {
	iterator p = begin();
	for (int i = 0; p != end() && i < avect.size(); ++p, ++i) {
	  *p = binop(*p,avect[i]);
	}
	return *this;
      }
      
     
      //! overloaded += adds a constant or another vector to this vector
      ASSIGNOP(+=,plus);
      //! overloaded -= subtracts a constant or another vector from this vector
      ASSIGNOP(-=,minus);
      //! overloaded *= multiplies a constant or another vector by this vector
      ASSIGNOP(*=,multiplies);
      //! overloaded /= divides a constant or another vector into this vector
      ASSIGNOP(/=,divides);
      //! overloaded %= stores the modulus of division a constant or another vector in this vector
      ASSIGNOP(%=,modulus);
      
    protected:
      
      //! the internal shared representation of the vector data
      vector_share<T> *xshare;
    };

  //! output the vector to a C++ stream
  template<class T>
    std::ostream& operator << (std::ostream &os, const vector<T>& toPrint) {
    copy (toPrint.begin(),toPrint.end(),std::ostream_iterator<T>(os," "));
    return os;
  }
  
  //! input the vector from a C++ stream
  template<class T>
    std::istream& operator >> (std::istream &is, vector<T>& toPrint) {
    int n = 0;
    for (std::istream_iterator<T> i = is; i != std::istream_iterator<T>(); ++i) {
      if (n >= toPrint.size()) {
	break;
      }
      toPrint[n] = *i;
      ++n;
    };
    return is;
  }
  
/**********************/

  template <class InputVector,class UnaryOperator, class Type>
    class unopvector {
      
    public:
      
      typedef Type value_type;
      
      unopvector (const InputVector &avect, const UnaryOperator &aunop) :
	xvect (avect), xunop(aunop) {
	  xvect.AddRef();   
      }
      
      unopvector (const unopvector &toCopy) :
	xvect(toCopy.xvect), xunop(toCopy.xunop) {
	xvect.AddRef();
      }

      ~unopvector() {
	xvect.RemoveRef();
      }
      
      inline typename InputVector::value_type operator [] (size_t index) const {
	return (_TYPENAME InputVector::value_type)(xunop(xvect[index]));
      }
      
      size_t size () const {return xvect.size();}
      
      void AddRef () const {
	xvect.AddRef();
      }
      
      void RemoveRef () const {
	xvect.RemoveRef();
      }
      
    protected:
      
      const InputVector    &xvect;
      UnaryOperator  xunop;
    };
  
  
  template <class InputVector1, class InputVector2, class BinaryOperator>
     class binopvector {
      
    public:
      
      // by decree, first type is value type
      typedef typename InputVector1::value_type value_type;
      
      binopvector (const InputVector1 &avect1, const InputVector2 &avect2, const BinaryOperator &abinop) :
	xvect1(avect1), xvect2(avect2), xbinop(abinop) {
	  xvect1.AddRef();
	  xvect2.AddRef();
      }
      
      ~binopvector() {
	xvect1.RemoveRef();
	xvect2.RemoveRef();
      }
      
      value_type operator [] (size_t index) const {
	return value_type(xbinop(xvect1[index],xvect2[index]));
      }
      
      size_t size () const {return xvect1.size();}
      
      void AddRef () const {
	xvect1.AddRef();
	xvect2.AddRef();
      }
      
      void RemoveRef () const {
	xvect1.RemoveRef();
	xvect2.RemoveRef();
      }
      
    protected:
      
      const InputVector1 &xvect1;
      const InputVector2 &xvect2;
      BinaryOperator     xbinop;
      
    };
  

  template<class T>
    class slice {
    
    public:
      
      class iterator {
	
	_TYPENAME vector<T>::iterator  xiterator;
	int                  xstride;
	
      public:
	
	iterator () :
	  xstride(1) {
	}
	
	iterator (_TYPENAME vector<T>::iterator aiterator, int astride) :
	  xiterator (aiterator),
	  xstride (astride) {
	}
	
	iterator (const iterator &aiterator, int astride) :
	  xiterator ((_TYPENAME vector<T>::iterator)(aiterator)),
	  xstride (astride * aiterator.stride()) {
	}
	
	T & operator * () const {
	  return (*xiterator);
	}
	
	bool operator == (const iterator &toTest) const {
	  return xiterator == toTest.xiterator;
	}
	
	iterator & operator ++ () {
	  xiterator += xstride;
	  return *this;
	}
	
	iterator & operator -- () {
	  xiterator -= xstride;
	  return *this;
	}
	
	int  stride () const {
	  return xstride;
	}
	
	operator _TYPENAME vector<T>::iterator () const {
	  return xiterator;
	}
      };
      
      typedef const iterator const_iterator;
      typedef iterator reverse_iterator;
      typedef const iterator const_reverse_iterator;

      typedef T value_type;
      
      slice (vector<T> &asource, int afirst, int alast, int astride = 1) :
	xsource (asource),
	xfirst (afirst),
	xlast (alast),
	xstride (astride) {
	  xsource.AddRef();
	  calculate_size();
	  adjust_last();
      }
      
      slice (vector<T> &asource, int astride) :
	xsource (asource),
	xfirst (0),
	xlast (asource.size()),
	xstride (astride) {
	  xsource.AddRef();
	  calculate_size();
	  adjust_last();
      }
      
      slice (slice<T> &asource, int afirst, int alast, int astride = 1) :
	xsource (asource.source()),
	xfirst (afirst + asource.first() * asource.stride()),
	xlast (alast + asource.first() * asource.stride()),
	xstride (astride * asource.stride()) {
	  xsource.AddRef();
	  calculate_size();
	  adjust_last();
      }
      
      slice (slice<T> &asource, int astride) :
	xsource (asource.source()),
	xfirst (asource.first()),
	xlast (asource.last()),
	xstride (astride * asource.stride()) {
	  xsource.AddRef();
	  calculate_size();
	  adjust_last();
      }
      
      ~slice() {
	xsource.RemoveRef();
      }
      
      T & operator [] (size_t index)  {
	return xsource[xfirst + index * xstride];
      }
      
      T & operator [] (size_t index) const {

	return xsource[xfirst + index * xstride];
      }



      size_t size () const {return xsize;}
      
      iterator begin() {return iterator(xsource.begin()+xfirst,xstride);}
      iterator end() {return iterator(xsource.begin()+xlast,xstride);} 
      const_iterator begin() const {return const_iterator(xsource.begin()+xfirst,xstride);}
      const_iterator end() const {return const_iterator(xsource.begin()+xlast,xstride);} 
      
      // For now, slices don't have reverse iterators
      //  If you *really* need them, use a negative stride
      
      void AddRef () const {xsource.AddRef();}
      void RemoveRef () const {xsource.RemoveRef();}

      template<class Unop>
      vector<T> &apply_unary_self(const Unop &unop) {
	for (iterator p = begin(); p != end(); ++p) {
	  *p = unop(*p);
	}
	return *this;
      }
      
      template<class Binop, class OpVector>
      vector<T> &apply_vector_self(const Binop &binop, const OpVector &avect) {
	iterator p = begin();
	
	for (int i = 0; p != end() && i < avect.size(); ++p, ++i) {
	  *p = binop(*p,avect[i]);
	}
	return *this;
      }
      
      
      ASSIGNOP(+=,plus);
      ASSIGNOP(-=,minus);
      ASSIGNOP(*=,multiplies);
      ASSIGNOP(/=,divides);
      ASSIGNOP(%=,modulus);

    protected:
      
      vector<T>    xsource;
      int          xfirst, xlast, xstride, xsize;
      
      void calculate_size () {
	xsize = xsource.size() / xstride;
	if (xsource.size() % xstride > 0) {
	  ++xsize;
	}
      }

      void adjust_last () {
	if ((xlast - xfirst) % xstride != 0) {
	  xlast += xstride - (xlast - xfirst) % xstride;
	}
      }
    };
  
  template<class T>
    std::ostream& operator << (std::ostream &os, slice<T>& toPrint) {
    copy (toPrint.begin(),toPrint.end(),std::ostream_iterator<T>(os," "));
    return os;
  }
      
  template<class T>
    std::istream& operator >> (std::istream &is, slice<T>& toPrint) {
    
    _TYPENAME slice<T>::iterator p = toPrint.begin();
    for (std::istream_iterator<T> i = is; i != std::istream_iterator<T>(); ++i) {
      if (p >= toPrint.end()) {
	break;
      }
      *p = *i;
      ++p;
    };
    return is;
  }
      


  
/*******************************/

  // for the moment, this is just linear

  template<class T>
    class interpolate {
    public:

      typedef T  value_type;

      interpolate(int an, T afrom, T ato) :
	xfrom(afrom),
	xto(ato),
	xn (an) {
	  xdelta = (ato - afrom) / T(an);
	  xval = afrom;
      }

      interpolate<T> & operator = (const interpolate<T> &toAssign) {
      }

      T operator [] (int notused) const {
	T local = xval;
	xval += xdelta;
	return local;
      }

      void AddRef () const {}
      void RemoveRef () const {}

      int size () const {return xn;}

      operator const vector<T> & () const {
	vector<T> result = vector<T>(xn);
	for (int i = 0; i < xn; ++i) {
	  result = (*this)[i];
	}
	return result;
      }

    private:

      T xfrom, xto;
      int xn;
      mutable T xdelta,xval;
    };

  template<class T>
    inline interpolate<T> extrapolate (int n, T from, T delta) {
    return interpolate<T>(n,from,from + T(n) * delta);
  }
  
/*******************************/

/* Operator overloads for vector,unopvector and binopvector */
/* This is really ugly stuff, but necessary */

// cast operation


  template <class S, class T>
    struct cast_functor {
      T operator () (const S &x) const {
	return T(x);
      }
    };

  template<class T>
  vector<T> concat(const vector<T> &first,
			      const vector<T> &second) {
	vector<T> result = vector<T>(first.size() + second.size());
	memcpy (result.begin(),first.begin(),sizeof(T)*first.size());
	memcpy (result.begin()+first.size(),second.begin(),
		sizeof(T)*second.size());
	return result;
      }



  template<class OutputType, class InputVector> 
    inline unopvector<InputVector,cast_functor<_TYPENAME InputVector::value_type,OutputType>, OutputType>
    vector_cast (const InputVector &toCast) {
    return unopvector<InputVector,cast_functor<_TYPENAME InputVector::value_type,OutputType>, OutputType>
      (toCast,cast_functor<_TYPENAME InputVector::value_type,OutputType>());
  }

  

// Thess are here because macros don't handle commas inside <>'s very well

#define TEMPLATE1(t1) template<t1>
#define TEMPLATE2(t1,t2) template<t1,t2>
#define TEMPLATE3(t1,t2,t3) template<t1,t2,t3>
#define TEMPLATE4(t1,t2,t3,t4) template<t1,t2,t3,t4>
#define TEMPLATE5(t1,t2,t3,t4,t5) template<t1,t2,t3,t4,t5>
#define TEMPLATE6(t1,t2,t3,t4,t5,t6) template<t1,t2,t3,t4,t5,t6>

#define DECLUNOPVECT(t1,t2) unopvector<t1,t2>
#define DECLBINOPVECT(t1,t2,t3) binopvector<t1,t2,t3>


#define UNOP(op,opfunc) \
template<class T> \
unopvector <vector<T>,opfunc<T> > \
  op (const vector<T> &avect) { \
  return unopvector<vector<T>,opfunc<T> > (avect,opfunc<T>()); \
} \
  \
template<class T> \
unopvector <slice<T>,opfunc<T> > \
  op (const slice<T> &avect) { \
  return unopvector<slice<T>,opfunc<T> > (avect,opfunc<T>()); \
} \
  \
template<class T> \
unopvector <interpolate<T>,opfunc<T> > \
  op (const interpolate<T> &avect) { \
  return unopvector<interpolate<T>,opfunc<T> > (avect,opfunc<T>()); \
} \
  \
template<class InputVector, class Unop, class Type> \
unopvector <unopvector<InputVector,Unop,Type>,opfunc<Type> >\
  op (const unopvector<InputVector,Unop,Type> &avect) { \
  return unopvector<unopvector<InputVector,Unop>, \
	            opfunc<Type> > \
	   (avect,opfunc<Type>()); \
} \
  \
template<class InputVector1, class InputVector2, class Binop> \
unopvector <binopvector<InputVector1,InputVector2,Binop>, \
            opfunc<_TYPENAME binopvector<InputVector1,InputVector2,Binop>::value_type> >\
  op (const binopvector<InputVector1,InputVector2,Binop> &avect) { \
  return unopvector<unopvector<InputVector1,InputVector2,Binop>, \
	            opfunc<_TYPENAME binopvector<InputVector1,InputVector2,Binop>::value_type> > \
	   (avect,opfunc<_TYPENAME binopvector<InputVector1,InputVector2,Binop>::value_type>()); \
} \
  


UNOP(operator -,std::negate);
UNOP(operator !,std::logical_not);


#define BINOPINT(op,opfunc,template_decl,vectype1,vectype2) \
template_decl \
binopvector <vectype1,vectype2,opfunc<_TYPENAME vectype1::value_type> > \
inline op (const vectype1 &avect1, const vectype2 &avect2) { \
  return binopvector<vectype1,vectype2,opfunc<_TYPENAME vectype1::value_type> > \
    (avect1,avect2,opfunc<_TYPENAME vectype1::value_type>()); \
} 

#define BINOP(op,opfunc) \
BINOPINT(op,opfunc,TEMPLATE1(class T),vector<T>,vector<T>);\
BINOPINT(op,opfunc,TEMPLATE1(class T),vector<T>,slice<T>);\
BINOPINT(op,opfunc,TEMPLATE1(class T),slice<T>,slice<T>);\
BINOPINT(op,opfunc,TEMPLATE1(class T),slice<T>,vector<T>);\
BINOPINT(op,opfunc,TEMPLATE1(class T),vector<T>,interpolate<T>);\
BINOPINT(op,opfunc,TEMPLATE1(class T),interpolate<T>,slice<T>);\
BINOPINT(op,opfunc,TEMPLATE1(class T),slice<T>,interpolate<T>);\
BINOPINT(op,opfunc,TEMPLATE1(class T),interpolate<T>,vector<T>);\
BINOPINT(op,opfunc,TEMPLATE1(class T),interpolate<T>,interpolate<T>);\
BINOPINT(op,opfunc,TEMPLATE3(class T,class InputVector,class Unop),\
	 vector<T>,\
	 DECLUNOPVECT(InputVector,Unop));\
BINOPINT(op,opfunc,TEMPLATE3(class T,class InputVector,class Unop),\
	 DECLUNOPVECT(InputVector,Unop),\
	 vector<T>);\
BINOPINT(op,opfunc,TEMPLATE3(class T,class InputVector,class Unop),\
	 slice<T>,\
	 DECLUNOPVECT(InputVector,Unop));\
BINOPINT(op,opfunc,TEMPLATE3(class T,class InputVector,class Unop),\
	 DECLUNOPVECT(InputVector,Unop),\
	 slice<T>);\
BINOPINT(op,opfunc,TEMPLATE3(class T,class InputVector,class Unop),\
	 interpolate<T>,\
	 DECLUNOPVECT(InputVector,Unop));\
BINOPINT(op,opfunc,TEMPLATE3(class T,class InputVector,class Unop),\
	 DECLUNOPVECT(InputVector,Unop),\
	 interpolate<T>);\
BINOPINT(op,opfunc,TEMPLATE4(class T,class InputVect1,class InputVect2,class Binop),\
	 vector<T>,\
	 DECLBINOPVECT(InputVect1,InputVect2,Binop));\
BINOPINT(op,opfunc,TEMPLATE4(class T,class InputVect1,class InputVect2,class Binop),\
	 DECLBINOPVECT(InputVect1,InputVect2,Binop),\
	 vector<T>);\
BINOPINT(op,opfunc,TEMPLATE4(class T,class InputVect1,class InputVect2,class Binop),\
	 slice<T>,\
	 DECLBINOPVECT(InputVect1,InputVect2,Binop));\
BINOPINT(op,opfunc,TEMPLATE4(class T,class InputVect1,class InputVect2,class Binop),\
	 DECLBINOPVECT(InputVect1,InputVect2,Binop),\
	 slice<T>);\
BINOPINT(op,opfunc,TEMPLATE4(class T,class InputVect1,class InputVect2,class Binop),\
	 interpolate<T>,\
	 DECLBINOPVECT(InputVect1,InputVect2,Binop));\
BINOPINT(op,opfunc,TEMPLATE4(class T,class InputVect1,class InputVect2,class Binop),\
	 DECLBINOPVECT(InputVect1,InputVect2,Binop),\
	 interpolate<T>);\
BINOPINT(op,opfunc,TEMPLATE4(class InputVect1,class Unop1,class InputVect2, class Unop2),\
	 DECLUNOPVECT(InputVect1,Unop1),\
	 DECLUNOPVECT(InputVect2,Unop2));\
BINOPINT(op,opfunc,TEMPLATE5(class InputVect1,class Unop1,class InputVect2, class InputVect3, class Binop2),\
	 DECLUNOPVECT(InputVect1,Unop1),\
	 DECLBINOPVECT(InputVect2,InputVect3,Binop2));\
BINOPINT(op,opfunc,TEMPLATE5(class InputVect1,class Unop1,class InputVect2, class InputVect3, class Binop2),\
	 DECLBINOPVECT(InputVect2,InputVect3,Binop2),\
         DECLUNOPVECT(InputVect1,Unop1));\
BINOPINT(op,opfunc,TEMPLATE6(class InputVect1,class InputVect2,class Binop1,class InputVect3,class InputVect4,class Binop2),\
	 DECLBINOPVECT(InputVect1,InputVect2,Binop1),\
	 DECLBINOPVECT(InputVect3,InputVect4,Binop2));



BINOP(operator +,std::plus);
BINOP(operator -,std::minus);
BINOP(operator *,std::multiplies);
BINOP(operator /,std::divides);
BINOP(operator %,std::modulus);



#define BINOPSCALARINT(op,opfunc,template_decl,vectype) \
template_decl \
unopvector<vectype,std::binder2nd<opfunc<_TYPENAME vectype::value_type> > > \
inline op (const vectype &avect1, const _TYPENAME vectype::value_type ascalar2) { \
  return unopvector<vectype,std::binder2nd<opfunc<_TYPENAME vectype::value_type> > > \
    (avect1,std::binder2nd<opfunc<_TYPENAME vectype::value_type> > \
     (opfunc<_TYPENAME vectype::value_type>(),ascalar2)); \
} \
template_decl  \
unopvector<vectype,std::binder1st<opfunc<_TYPENAME vectype::value_type> > > \
inline op (_TYPENAME vectype::value_type ascalar1 ,const vectype &avect2) { \
  return unopvector<vectype,std::binder1st<opfunc<_TYPENAME vectype::value_type> > > \
    (avect2,std::binder1st<opfunc<_TYPENAME vectype::value_type> > \
     (opfunc<_TYPENAME vectype::value_type>(),ascalar1)); \
} \

#define BINOPSCALAR(op,opfunc) \
BINOPSCALARINT(op,opfunc,TEMPLATE1(class T),vector<T>); \
BINOPSCALARINT(op,opfunc,TEMPLATE1(class T),slice<T>); \
BINOPSCALARINT(op,opfunc,TEMPLATE1(class T),interpolate<T>); \
BINOPSCALARINT(op,opfunc,TEMPLATE2(class InputVector,class Unop),\
	       DECLUNOPVECT(InputVector,Unop)); \
BINOPSCALARINT(op,opfunc,TEMPLATE3(class InputVect1,class InputVect2, class Binop),\
	       DECLBINOPVECT(InputVect1,InputVect2,Binop)); 


BINOPSCALAR(operator +,std::plus);
BINOPSCALAR(operator -,std::minus);
BINOPSCALAR(operator *,std::multiplies);
BINOPSCALAR(operator /,std::divides);
BINOPSCALAR(operator %,std::modulus);


template<class T> 
struct _less : public std::binary_function<T,T,T> {
  T operator () (const T& a, const T& b) const {
    return T(a < b);
  }
};

template<class T> 
struct _greater : public std::binary_function<T,T,T> {
  T operator () (const T& a, const T& b) const {
    return T(a > b);
  }
};

template<class T> 
struct _less_eq : public std::binary_function<T,T,T> {
  T operator () (const T& a, const T& b) const {
    return T(a <= b);
  }
};

template<class T> 
struct _greater_eq : public std::binary_function<T,T,T> {
  T operator () (const T& a, const T& b) const {
    return T(a >= b);
  }
};

template<class T> 
struct _eq : public std::binary_function<T,T,T> {
  T operator () (const T& a, const T& b) const {
    return T(a == b);
  }
};

template<class T>
struct _bitwise_and : public std::binary_function<T,T,T> {
  T operator () (const T& a, const T& b) const {
    return T(a & b);
  }
};

template<class T>
struct _bitwise_or : public std::binary_function<T,T,T> {
  T operator () (const T& a, const T& b) const {
    return T(a | b);
  }
};

template<class T>
struct _bitwise_xor : public std::binary_function<T,T,T> {
  T operator () (const T& a, const T& b) const {
    return T(a ^ b);
  }
};

template<class T>
struct _shift_left : public std::binary_function<T,T,T> {
  T operator () (const T& a, const T& b) const {
    return T(a << b);
  }
};

template<class T>
struct _shift_right : public std::binary_function<T,T,T> {
  T operator () (const T& a, const T& b) const {
    return T(a >> b);
  }
};



BINOP(operator <,_less);
BINOP(operator >,_greater);
BINOP(operator <=,_less_eq);
BINOP(operator >=,_greater_eq);
BINOPSCALAR(operator <,_less);
BINOPSCALAR(operator >,_greater);
BINOPSCALAR(operator <=,_less_eq);
BINOPSCALAR(operator >=,_greater_eq);
BINOPSCALAR(operator ==,_eq);

BINOP(operator &,_bitwise_and);
BINOP(operator |,_bitwise_or);
BINOP(operator ^,_bitwise_xor);
BINOP(operator <<,_shift_left);
BINOP(operator >>,_shift_right);
BINOPSCALAR(operator &,_bitwise_and);
BINOPSCALAR(operator |,_bitwise_or);
BINOPSCALAR(operator ^,_bitwise_xor);
BINOPSCALAR(operator <<,_shift_left);
BINOPSCALAR(operator >>,_shift_right);
      
} // namespace osw


#define DECLUNOPVECTEX(t1,t2) osw::unopvector<t1,t2>
#define DECLBINOPVECTEX(t1,t2,t3) osw::binopvector<t1,t2,t3>

#define BINOPINTEX(op,opfunc,template_decl,vectype1,vectype2) \
template_decl \
osw::binopvector <vectype1,vectype2,opfunc<_TYPENAME vectype1::value_type> > \
inline op (const vectype1 &avect1, const vectype2 &avect2) { \
  return osw::binopvector<vectype1,vectype2,opfunc<_TYPENAME vectype1::value_type> > \
    (avect1,avect2,opfunc<_TYPENAME vectype1::value_type>()); \
} 

#define BINOPEX(op,opfunc) \
BINOPINTEX(op,opfunc,TEMPLATE1(class T),osw::vector<T>,osw::vector<T>);\
BINOPINTEX(op,opfunc,TEMPLATE1(class T),osw::vector<T>,osw::slice<T>);\
BINOPINTEX(op,opfunc,TEMPLATE1(class T),osw::slice<T>,osw::slice<T>);\
BINOPINTEX(op,opfunc,TEMPLATE1(class T),osw::slice<T>,osw::vector<T>);\
BINOPINTEX(op,opfunc,TEMPLATE1(class T),osw::vector<T>,osw::interpolate<T>);\
BINOPINTEX(op,opfunc,TEMPLATE1(class T),osw::interpolate<T>,osw::slice<T>);\
BINOPINTEX(op,opfunc,TEMPLATE1(class T),osw::slice<T>,osw::interpolate<T>);\
BINOPINTEX(op,opfunc,TEMPLATE1(class T),osw::interpolate<T>,osw::vector<T>);\
BINOPINTEX(op,opfunc,TEMPLATE1(class T),osw::interpolate<T>,osw::interpolate<T>);\
BINOPINTEX(op,opfunc,TEMPLATE3(class T,class InputVector,class Unop),\
	 osw::vector<T>,\
	 DECLUNOPVECTEX(InputVector,Unop));\
BINOPINTEX(op,opfunc,TEMPLATE3(class T,class InputVector,class Unop),\
	 DECLUNOPVECTEX(InputVector,Unop),\
	 osw::vector<T>);\
BINOPINTEX(op,opfunc,TEMPLATE3(class T,class InputVector,class Unop),\
	 osw::slice<T>,\
	 DECLUNOPVECTEX(InputVector,Unop));\
BINOPINTEX(op,opfunc,TEMPLATE3(class T,class InputVector,class Unop),\
	 DECLUNOPVECTEX(InputVector,Unop),\
	 osw::slice<T>);\
BINOPINTEX(op,opfunc,TEMPLATE3(class T,class InputVector,class Unop),\
	 osw::interpolate<T>,\
	 DECLUNOPVECTEX(InputVector,Unop));\
BINOPINTEX(op,opfunc,TEMPLATE3(class T,class InputVector,class Unop),\
	 DECLUNOPVECTEX(InputVector,Unop),\
	 osw::interpolate<T>);\
BINOPINTEX(op,opfunc,TEMPLATE4(class T,class InputVect1,class InputVect2,class Binop),\
	 osw::vector<T>,\
	 DECLBINOPVECTEX(InputVect1,InputVect2,Binop));\
BINOPINTEX(op,opfunc,TEMPLATE4(class T,class InputVect1,class InputVect2,class Binop),\
	 DECLBINOPVECTEX(InputVect1,InputVect2,Binop),\
	 osw::vector<T>);\
BINOPINTEX(op,opfunc,TEMPLATE4(class T,class InputVect1,class InputVect2,class Binop),\
	 osw::slice<T>,\
	 DECLBINOPVECTEX(InputVect1,InputVect2,Binop));\
BINOPINTEX(op,opfunc,TEMPLATE4(class T,class InputVect1,class InputVect2,class Binop),\
	 DECLBINOPVECTEX(InputVect1,InputVect2,Binop),\
	 osw::slice<T>);\
BINOPINTEX(op,opfunc,TEMPLATE4(class T,class InputVect1,class InputVect2,class Binop),\
	 osw::interpolate<T>,\
	 DECLBINOPVECTEX(InputVect1,InputVect2,Binop));\
BINOPINTEX(op,opfunc,TEMPLATE4(class T,class InputVect1,class InputVect2,class Binop),\
	 DECLBINOPVECTEX(InputVect1,InputVect2,Binop),\
	 osw::interpolate<T>);\
BINOPINTEX(op,opfunc,TEMPLATE4(class InputVect1,class Unop1,class InputVect2, class Unop2),\
	 DECLUNOPVECTEX(InputVect1,Unop1),\
	 DECLUNOPVECTEX(InputVect2,Unop2));\
BINOPINTEX(op,opfunc,TEMPLATE5(class InputVect1,class Unop1,class InputVect2, class InputVect3, class Binop2),\
	 DECLUNOPVECTEX(InputVect1,Unop1),\
	 DECLBINOPVECTEX(InputVect2,InputVect3,Binop2));\
BINOPINTEX(op,opfunc,TEMPLATE5(class InputVect1,class Unop1,class InputVect2, class InputVect3, class Binop2),\
	 DECLBINOPVECTEX(InputVect2,InputVect3,Binop2),\
         DECLUNOPVECTEX(InputVect1,Unop1));\
BINOPINTEX(op,opfunc,TEMPLATE6(class InputVect1,class InputVect2,class Binop1,class InputVect3,class InputVect4,class Binop2),\
	 DECLBINOPVECTEX(InputVect1,InputVect2,Binop1),\
	 DECLBINOPVECTEX(InputVect3,InputVect4,Binop2));



#define BINOPSCALARINTEX(op,opfunc,template_decl,vectype) \
template_decl \
osw::unopvector<vectype,std::binder2nd<opfunc<_TYPENAME vectype::value_type> > > \
inline op (const vectype &avect1, const _TYPENAME vectype::value_type ascalar2) { \
  return osw::unopvector<vectype,std::binder2nd<opfunc<_TYPENAME vectype::value_type> > > \
    (avect1,std::binder2nd<opfunc<_TYPENAME vectype::value_type> > \
     (opfunc<_TYPENAME vectype::value_type>(),ascalar2)); \
} \
template_decl  \
osw::unopvector<vectype,std::binder1st<opfunc<_TYPENAME vectype::value_type> > > \
inline op (_TYPENAME vectype::value_type ascalar1 ,const vectype &avect2) { \
  return osw::unopvector<vectype,std::binder1st<opfunc<_TYPENAME vectype::value_type> > > \
    (avect2,std::binder1st<opfunc<_TYPENAME vectype::value_type> > \
     (opfunc<_TYPENAME vectype::value_type>(),ascalar1)); \
} \

#define BINOPSCALAREX(op,opfunc) \
BINOPSCALARINTEX(op,opfunc,TEMPLATE1(class T),osw::vector<T>); \
BINOPSCALARINTEX(op,opfunc,TEMPLATE1(class T),osw::slice<T>); \
BINOPSCALARINTEX(op,opfunc,TEMPLATE1(class T),osw::interpolate<T>); \
BINOPSCALARINTEX(op,opfunc,TEMPLATE2(class InputVector,class Unop),\
	       DECLUNOPVECTEX(InputVector,Unop)); \
BINOPSCALARINTEX(op,opfunc,TEMPLATE3(class InputVect1,class InputVect2, class Binop),\
	       DECLBINOPVECTEX(InputVect1,InputVect2,Binop)); 


template<class T>
struct _pow : public std::binary_function<T,T,T> {
  T operator () (const T& a, const T& b) const {
    return T(std::pow(a,b));
  }
};


#ifdef OSW_NEED_INT_POW
_STLP_TEMPLATE_NULL
struct _pow<int> : public std::binary_function<int,int,int> {
  int operator () (const int &a, const int &b) const {
    return int(std::pow(float(a),float(b)));
  }
};

_STLP_TEMPLATE_NULL
struct _pow<short> : public std::binary_function<short,short,short> {
  short operator () (const short &a, const short &b) const {
    return short(std::pow(float(a),float(b)));
  }
};

#endif

BINOPEX(pow,_pow);
BINOPSCALAREX(pow,_pow);





// Math functions need to be defined outside the osw namespace
// to make them look like "normal" math functions

#define UNFUNC(op,opfunc,opinit) \
template<class T> \
osw::unopvector <osw::vector<T>,opfunc<T,T> > \
  op (const osw::vector<T> &avect) { \
  return osw::unopvector<osw::vector<T>,opfunc<T,T> > (avect,opfunc<T,T>(opinit)); \
} \
  \
template<class T> \
osw::unopvector <osw::slice<T>,opfunc<T,T> > \
  op (const osw::slice<T> &avect) { \
  return osw::unopvector<osw::slice<T>,opfunc<T,T> > (avect,opfunc<T,T>(opinit)); \
} \
template<class T> \
osw::unopvector <osw::interpolate<T>,opfunc<T,T> > \
  op (const osw::interpolate<T> &avect) { \
  return osw::unopvector<osw::interpolate<T>,opfunc<T,T> > (avect,opfunc<T,T>(opinit)); \
} \

#define UNFUNC1(op,opfunc,opinit) \
template<class InputVector, class Unop, class Type> \
osw::unopvector <osw::unopvector<InputVector,Unop,Type> ,opfunc<Type,Type>,Type >\
  op (const osw::unopvector<InputVector,Unop,Type> &avect) { \
  return osw::unopvector<osw::unopvector<InputVector,Unop,Type>, \
	            opfunc<Type,Type>,Type > \
	   (avect,opfunc<Type,Type>(opinit)); \
} \

#define UNFUNC2(op,opfunc,opinit) \
template<class InputVector1, class InputVector2, class Binop> \
osw::unopvector <osw::binopvector<InputVector1,InputVector2,Binop>, \
            opfunc<typename osw::binopvector<InputVector1,InputVector2,Binop>::value_type, typename osw::binopvector<InputVector1,InputVector2,Binop>::value_type>  >\
  op (const osw::binopvector<InputVector1,InputVector2,Binop> &avect) { \
  return osw::unopvector<osw::unopvector<InputVector1,InputVector2,Binop>, \
	            opfunc<typename osw::binopvector<InputVector1,InputVector2,Binop>::value_type, typename osw::binopvector<InputVector1,InputVector2,Binop>::value_type> > \
	   (avect,opfunc<typename osw::binopvector<InputVector1,InputVector2,Binop>::value_type, typename osw::binopvector<InputVector1,InputVector2,Binop>::value_type >(opinit)); \
} \
  

#define UNFUNCTOR(op,opfunc,opinit) \
UNFUNC(op,opfunc,opinit) \
UNFUNC1(op,opfunc,opinit) \
UNFUNC2(op,opfunc,opinit) \

#define STDUNFUNC(op) \
UNFUNC(op,std::pointer_to_unary_function,((T (*)(T)) op)) \
UNFUNC1(op,std::pointer_to_unary_function,((Type (*)(Type)) op)) \
UNFUNC2(op,std::pointer_to_unary_function,((typename osw::binopvector<InputVector1,InputVector2,Binop>::value_type (*)(typename osw::binopvector<InputVector1,InputVector2,Binop>::value_type)) op)) \

#define TMPLFUNC(op) \
UNFUNC(op,std::pointer_to_unary_function,((T (*) (T)) &op<T>)) \
UNFUNC1(op,std::pointer_to_unary_function,((Type (*) (Type))  &op<Type>)) \
UNFUNC2(op,std::pointer_to_unary_function,((typename osw::binopvector<InputVector1,InputVector2,Binop>::value_type (*)(typename osw::binopvector<InputVector1,InputVector2,Binop>::value_type)) &op<typename osw::binopvector<InputVector1,InputVector2,Binop>::valueType>)) \


STDUNFUNC(abs);
STDUNFUNC(asin);
STDUNFUNC(acos);
STDUNFUNC(atan);
STDUNFUNC(cos);
STDUNFUNC(cosh);
STDUNFUNC(exp);
STDUNFUNC(fabs);
STDUNFUNC(log);
STDUNFUNC(log10);
STDUNFUNC(sin);
STDUNFUNC(sinh);
STDUNFUNC(tan);
STDUNFUNC(tanh);
STDUNFUNC(sqrt);
STDUNFUNC(floor);
STDUNFUNC(ceil);
STDUNFUNC(sgn);
STDUNFUNC(frac);
TMPLFUNC(sec);
TMPLFUNC(csc);
TMPLFUNC(cot);
TMPLFUNC(asec);
TMPLFUNC(acsc);
TMPLFUNC(acot);
TMPLFUNC(sech);
TMPLFUNC(csch);
TMPLFUNC(coth);
TMPLFUNC(asinh);
TMPLFUNC(acosh);
TMPLFUNC(atanh);
TMPLFUNC(asech);
TMPLFUNC(acsch);
TMPLFUNC(acoth);
TMPLFUNC(sinc);
TMPLFUNC(sinhc);

#endif // _OSWVECT

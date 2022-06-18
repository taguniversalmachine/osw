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


/*  m1.h
    Some helpful math functions
*/


#ifndef _M1_H
#define _M1_H

#ifdef OSW_NO_STDC_HEADERS
#include <math.h>
#else
#include <cmath>
#endif

#include <complex>

#ifndef M_PI
#define M_PI        3.14159265358979323846
#endif


//template<class T>
//  T sgn (const T &x) {
//  return (x > T(0)) ? 1 : ((x < T(0)) ? -1 : 0);
//}


//! returns the signum (positive, negative, zero) of a number  
/**
 * @param x an integer argument
 * @returns -1 if x is negative, 1 if x is positive, 0 if x is zero
 */
inline int sgn (int x) {
  return (x > 0) ? 1 : ((x < 0) ? -1 : 0);
}

//! returns the signum (positive, negative, zero) of a number  
/**
 * @param x a float argument
 * @returns -1 if x is negative, 1 if x is positive, 0 if x is zero
 */
inline float sgn (float x) {
  return (x > 0.0f) ? 1.0f : ((x < 0.0f) ? -1.0f : 0.0f);
}

//! returns the signum (positive, negative, zero) of a number  
/**
 * @param x a double argument
 * @returns -1 if x is negative, 1 if x is positive, 0 if x is zero
 */
inline double sgn (double x) {
  return (x > 0.0) ? 1.0 : ((x < 0.0) ? -1.0 : 0.0);
}

#if 0
#if !defined(__GNUC__)
  template <class T>
    struct identity {
      const T& operator () (const T &x) {return x;}
    };
#endif
#endif

#ifndef _GLIBCPP_HAVE_FLOORF
inline float frac (float x) {
  return x - floor(static_cast<double>(x));
}
#else
inline float frac (float x) {
  return x - floorf(x);
}
#endif

//! returns to the fractional (non-integer) portion of a number
inline double frac (double x) {
  return x - floor(x);
}

// maybe not the most efficient implementations, but easy to
// understand and portable (AC)

//! secant function
template<class T>
inline T sec (T x) {
  return T(1) / cos(x);
}

//! cosecant function
template<class T>
inline T csc (T x) {
  return T(1) / sin(x);
}

//! cotangent function
template<class T>
inline T cot (T x) {
  return T(1) / tan(x);
}

//! arcsecant (inverse secant) function
template<class T>
inline T asec(T x) {
  return acos(T(1) / x);
}


//! arccosecant (inverse cosecant) function
template<class T>
inline T acsc(T x) {
  return asin(T(1) / x);
}

//! arccotangent (inverse cotangent) function
template<class T>
inline T acot(T x) {
  return atan(T(1) / x);
}

//! hyperbolic secant function
template<class T>
inline T sech (T x) {
  return T(1) / cosh(x);
}

//! hyperbolic cosecant function
template<class T>
inline T csch (T x) {
  return T(1) / sinh(x);
}

//! hyperbolic cotangent function
template<class T>
inline T coth (T x) {
  return T(1) / tanh(x);
}

//! inverse hyperbolic sine function
template<class T>
inline T asinh(T x) {
  return -log(sqrt(T(1) + x * x) - x);
}

//! inverse hyperbolic tangent function
template<class T>
inline T atanh(T x) {
  return T(0.5) * log((T(1) + x) / (T(1) - x));
}

//! inverse hyperbolic cosine function
template<class T>
inline T acosh(T x) {
  return asinh(sqrt(x * x - T(1)));
}

//! inverse hyperbolic secant function
template<class T>
inline T asech(T x) {
  return acosh(T(1) / x);
}

//! inverse hyperbolic cosecant function
template<class T>
inline T acsch(T x) {
  return asinh(T(1) / x);
}

//! inverse hyperbolic cotangent function
template<class T>
inline T acoth(T x) {
  return atanh(T(1) / x);
}

//! sinc function: sin(x) / x
template<class T>
inline T sinc(T x) {
  return (x == T(0)) ? T(1) : sin(x) / x;
}

//! hyperbolic sinc function: sinh(x) / x
template<class T>
inline T sinhc(T x) {
  return (x == T(0)) ? T(1) : sinh(x) / x;
}

//! inverse cosine of a complex number
template<class T>
inline std::complex<T> acos(const std::complex<T> &x) {
  return acosh(x) * std::complex<T>(T(0),T(1));
}

//! inverse sine of a complex number
template<class T>
inline std::complex<T> asin(const std::complex<T> &x) {
  return acos(x) + T(M_PI) * T(0.5);
}

//! inverse tangent of a complex number
template<class T>
inline std::complex<T> atan(const std::complex<T> &x) {
  return atanh(x) * std::complex<T>(T(0),T(1));
}



#endif //_ M1_H




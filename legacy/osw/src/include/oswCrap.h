
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
   oswcrap.h
   The OSW crap library
   Includes functions that might be missing from one or another implementation
*/

#ifndef _OSW_CRAP_H
#define _OSW_CRAP_H

#include <complex>
#ifdef OSW_NEEDS_MATHF_FUNCTIONS
#include <cmath>
#endif

using namespace std;

namespace osw {

  using stl::cout;
  using stl::cin;
  using stl::cerr;
  using stl::clog;


#ifdef OSW_NEEDS_COPY_N
  template<typename T1, typename Size, typename T2>
  inline T2 copy_n(T1 first, Size count, T2 result) {
    return copy(first,first+count,result);
  }
#endif

#ifdef OSW_NEEDS_MINMAX

  // These cases are needed for cygwin gcc
  #ifdef min
   #undef min
  #endif
  #ifdef max
   #undef max
  #endif
  template<class T>
  inline const T &min (const T &a, const T &b) {
    return stl::min(a,b);
  }

  template<class T>
  inline const T &max (const T &a, const T &b) {
    return stl::max(a,b);
  }
#endif

#if defined(OSW_NEEDS_COMPLEX_INEQUALITY)
  
 template<class T>
    bool operator != (const std::complex<T> &a, const std::complex<T> &b) {
    return !(a == b);
  }
#endif


#ifdef OSW_NEEDS_CRAP_HASH_STRING
}
# ifdef OSW_NEEDS_CRAP_HASH_STRING_FUNC
  inline size_t __stl_hash_string(const char* __s)
  {
   unsigned long __h = 0; 
   for ( ; *__s; ++__s)
     __h = 5*__h + *__s;
  
   return size_t(__h);
  }
# endif

# ifdef OSW_USES_HASH_COMPARE
 _STLP_TEMPLATE_NULL
   struct std::hash_compare<osw::string>  {
     enum
		{	// parameters for hash table
		bucket_size = 4,	// 0 < bucket_size
		min_buckets = 8};	// min_buckets = 2 ^^ N, 0 < N

     size_t operator() (const osw::string &s) const {
       return __stl_hash_string(s.c_str());
     }

     bool operator () (const osw::string &s1, const osw::string &s2) const {
       return s1 < s2;
     }
   };
# else
 _STLP_TEMPLATE_NULL
   struct std::hash<osw::string>  {
     size_t operator() (const osw::string &s) {
       return __stl_hash_string(s.c_str());
     }
   };
# endif

namespace osw {

#else
#ifdef OSW_IMPORT_FROM_GNU_CXX
}
namespace __gnu_cxx {

  _STLP_TEMPLATE_NULL
  struct hash<osw::String> {
     size_t operator() (const osw::String &s) const {
       return __stl_hash_string(s.c_str());
     }
   };
} // __gnu_cxx
namespace osw {
#endif
#endif


#ifdef OSW_NEEDS_COMPLEX_OVERRIDE
  inline Float real (const Complex &z) {
    return std::real((const std::complex<float> &) z);
  }

  inline Float imag (const Complex &z) {
    return std::imag((const std::complex<float> &) z);
  }

  inline Double real (const DoubleComplex &z) {
    return std::real((const std::complex<double> &) z);
  }

  inline Double imag (const DoubleComplex &z) {
    return std::imag((const std::complex<double> &) z);
  }

#endif

}


#endif // _OSW_CRAP_H


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
   oswcfg.h
   Contains definitions for platform-specific configuration

   Amar Chaudhary

*/

#ifndef _OSWCFG_H
#define _OSWCFG_H

#ifndef OSW_MAJOR_VERSION
#  define OSW_MAJOR_VERSION 0
#endif
#ifndef OSW_MINOR_VERSION
#  define OSW_MINOR_VERSION 0
#endif
#ifndef OSW_SUB_VERSION
#  define OSW_SUB_VERSION 0
#endif

// Debug stuff
#ifdef OSW_DEBUG
#  define OSW_DEBUG_MESSAGES
#endif


// Windows stuff, possibly compiler-independent
#if defined(_WINDOWS) || defined (__WIN32__)

# define LITTLE_ENDIAN 1

#define OSW_NO_LOCK_ON_SCHEDULE

#define strcasecmp stricmp

#endif


// Microsoft Visual C++
#if defined(_MSC_VER)
# ifdef _OSWCORE
#  define _oswexport __declspec(dllexport) 
# else
#  define _oswexport __declspec(dllexport)
# endif
#define OSW_LAME_FOR_LOOP
#define _TYPENAME
#define _TYPENAME2
#define OSW_SUPPORTS_WCHAR_T
#define OSW_NEEDS_MINMAX 1
#define OSW_NO_EXPLICIT_MEMBER_TEMPLATES
#define OSW_USE_ITOA

#if _MSC_VER >= 1300  // VS .NET
#  define OSW_USE_OWN_STL 1
#  define OSW_STL_IS_STD 1
#  define OSW_NEEDS_COPY_N 1
#  define OSW_HASH_MAP_TRAITS 1
#  define OSW_NEEDS_CRAP_HASH_STRING 1
#  define OSW_NEEDS_CRAP_HASH_STRING_FUNC 1
#  define OSW_USES_HASH_COMPARE 1 
#else
#  define REVERSE_ITERATOR_2_ARGS 1
#endif 

#undef max
#undef min
//#define max _cpp_max
//#define min _cpp_min



#pragma inline_depth(255)
 
// This warnings are quite irritating.  Turn them off!

#pragma warning (disable : 4355)
#pragma warning (disable : 4503)


#endif


// GCC (for linux and eventually other Unixes)
#if defined(__GNUC__)


#if __GNUC__ == 3
#if __GNUC_MINOR__ < 2
#define OSW_NEEDS_CRAP_HASH_STRING
#endif
#ifndef OSW_IMPORT_FROM_GNU_CXX
   #define OSW_IMPORT_FROM_GNU_CXX 1
#endif

#endif

#ifdef __APPLE__
   #define OSW_NEEDS_MATHF_FUNCTIONS
   #define OSW_NEEDS_EXPLICIT_CLEANUP 1
#endif

#define _TYPENAME typename
#define _TYPENAME2 typename

#define OSW_USE_OWN_STL 1
#define OSW_NO_LIMITS

#define OSW_NEED_INT_POW

#if  __GNUC_MINOR__ > 3
#define OSW_NEEDS_COMPLEX_OVERRIDE 1
#endif

#ifdef _OSWOSX
#define _NEEDS_STRING_IO 1
#endif

#ifdef __linux__
   #define OSW_NEEDS_EXPLICIT_CLEANUP 1
#endif

#ifdef _WINDOWS
#define _oswexport 
#else
#define _oswexport
#endif

#define OSW_NEEDS_MINMAX 1

// not sure who's responsible for this stroke of genius!
#define stricmp strcasecmp
#define strnicmp strncasecmp

#ifdef _WINDOWS
#define _M_IX86
#endif

#endif // __GNUC__


// SGI MipsPro C++
#if defined(__sgi) && !defined(__GNUC__)

#define _TYPENAME typename
#define _TYPENAME2 

#define _oswexport

#define OSW_USE_OWN_STL 1
#define OSW_STL_IS_STD 1
#define OSW_NO_STDC_HEADERS


#define stricmp strcasecmp
#define strnicmp strncasecmp

#endif  // __sgi 

#ifndef _STLP_OWN_IOSTREAMS
  #define _STLP_TEMPLATE_NULL  template<>
#endif

// Common Comfiguration


#endif // _OSWCFG_H






/* 
Copyright (c) 2001-2003 Amar Chaudhary. All rights reserved.
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

*/

/*
  oswTypes.h
  definitions for some basic types used in OSW
*/

#include <string>
#include <complex>

using namespace std;

namespace osw {
   
  // lower case versions for backward compatibility with early code
  typedef unsigned char       byte;
  typedef unsigned char       Byte;
  typedef stl::string         string;
  typedef stl::string         String;
  typedef bool                Boolean;
  typedef int                 Integer;
  typedef short               Integer16;
  typedef unsigned int        UInteger;
  typedef unsigned short      UInteger16;
  typedef float               Float;
  typedef double              Double;
  typedef char                Char;
#ifdef Complex
#undef Complex
#endif
  typedef stl::complex<float>      Complex;
  typedef stl::complex<double>     DoubleComplex;
}








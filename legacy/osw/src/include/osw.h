
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
   osw.h
   basic class definitions for Open Sound World
*/

#ifndef _OSW_H
#define _OSW_H


#include "oswCfg.h"

// standard stuff
#include <iostream>
#include <sstream>
#include <typeinfo>
#ifdef OSW_EXT_HASH_MAP
#include <ext/hash_map>
#else
#include <hash_map>
#endif
#include <list>
#include <map>
#ifdef OSW_EXT_HASH_MAP
#include <ext/algorithm>
#else
#include <algorithm>
#endif
#include <string>
#include "m1.h"
#include "base64.h"
#ifdef OSW_NO_STDC_HEADERS
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#else
#include <cstdlib>
#include <cstdio>
#include <cstring>
#endif
#include "oswUseDecl.h"

#ifndef OSW_USE_OWN_STL
  namespace stl=stlport;
#else
 #ifdef OSW_STL_IS_STD
   namespace stl=std;
 #else
   #define stl
 #endif
#endif

#ifdef OSW_IMPORT_FROM_GNU_CXX
namespace std {
  using namespace __gnu_cxx;
}
#endif

#include "oswTypes.h"
#include "oswMisc.h"
#include "oswCrap.h"
#include "oswVect.h"
#include "oswMutex.h"
#include "oswExcept.h"
#include "oswCleanup.h"

using namespace std;

namespace osw {

  _oswexport void Error (const string &message);
  _oswexport void Alert (const string &message);
  _oswexport void Debug (const string &message);
  _oswexport string &UserQuery (const string &message, const string &options);
  
  inline int GetMajorVersion () {
    return OSW_MAJOR_VERSION;
  }
  inline int GetMinorVersion () {
    return OSW_MINOR_VERSION;
  }

  _oswexport const char * GetVersionString ();


  typedef osw::vector<float>  Samples;
  typedef osw::vector<short>  Integer16Samples;
  typedef osw::vector<int>    IntegerSamples;
  typedef osw::vector<char>   ByteSamples;
  typedef osw::vector<double> DoubleSamples;


}

#include "oswEnumeratedString.h"
#include "oswRaw.h"
#include "oswDatum.h"
#include "oswUnit.h"
#include "oswAny.h"
#include "oswNameable.h"
#include "oswContainer.h"
#include "oswState.h"
#include "oswScan.h"
#include "oswTransform.h"
#include "oswInletOutlet.h"
#include "oswActivation.h"
#include "oswTimeDomainTransform.h"
#include "oswPatch.h"
#include "oswTypeRegistry.h"



#endif // _OSW_H




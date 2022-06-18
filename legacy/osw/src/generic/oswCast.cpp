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
   oswcast.h
   Conversions between different registered data types

   Amar Chaudhary
   
*/

#include "osw.h"
#include "oswCast.h"
#include "oswList.h"
#include "oswTime.h"
#include <complex>

using namespace std;

namespace osw {

  typedef stl::map<string, stl::map<string, BaseRegisterCast *, less<string> >, less<string> > CastMap;
  
  static CastMap *TheCastMap = NULL;

  BaseRegisterCast::BaseRegisterCast (const string &sourcetype,
				      const string &sinktype) {
    if (TheCastMap == NULL) {
      TheCastMap = new CastMap;
    }
    //cout << "Casting " << sourcetype << " to " << sinktype << endl;
    (*TheCastMap)[sourcetype][sinktype] = this;
  }

  _oswexport BaseRegisterCast *FindCast (const type_info &sourcetype,
					 const type_info &sinktype) {
    //cout << "Checking cast of " << sourcetype.name() << " to " << sinktype.name() << endl;
    CastMap::iterator p = TheCastMap->find(sourcetype.name());
    if (p != TheCastMap->end()) {
      stl::map<string, BaseRegisterCast *, less<string> >::iterator q =
	p->second.find(sinktype.name());
      if (q != p->second.end()) {
	return q->second;
      }
    }
    return NULL;
  }

  REGISTER_CAST(int,Float);
  REGISTER_CAST(Float,int);
  REGISTER_CAST(Double,Float);
  REGISTER_CAST(Float,Double);
  REGISTER_CAST(int,Double);
  REGISTER_CAST(Double,int);
  REGISTER_CAST(byte,int);
  REGISTER_CAST(int,byte);
  REGISTER_CAST(int,char);
  REGISTER_CAST(char,int);
  REGISTER_CAST(int,bool);
  REGISTER_CAST(bool,int);
  REGISTER_CAST(bool,Float);
  REGISTER_CAST(bool,Double);
  REGISTER_CAST(Time,Double);

  /******************************/

  // Cast from Unit to "Changeable"
  typedef ChangeableInlet::Changeable  ChangeableType;
  struct UnitToChangeable {
    ChangeableType operator () (const Unit &toCast) {
      return ChangeableType();
    }
  };

  REGISTER_CAST3(Unit,ChangeableType,UnitToChangeable);


  /******************************/

  // Cast from Raw to basic types

  template <class T>
  struct RawCast {
    T operator () (const Raw &toCast) {
      return *((const T *)(toCast.begin()));
    }
  };

  template <> 
  struct RawCast<string> {
    string operator () (const Raw &toCast) {
      return string((const char *)toCast.begin(),
		    (const char *)toCast.end());
    }
  };

  REGISTER_CAST3(Raw,int,RawCast<int> );
  REGISTER_CAST3(Raw,float,RawCast<float> );
  REGISTER_CAST3(Raw,double,RawCast<double> );
  REGISTER_CAST3(Raw,bool,RawCast<bool> );
  REGISTER_CAST3(Raw,Unit,RawCast<Unit> );
  REGISTER_CAST3(Raw,string,RawCast<string> );
  REGISTER_CAST3(Raw,Byte,RawCast<Byte> );

  typedef complex<float> FloatComplex;
  typedef complex<double> DoubleComplex;
  REGISTER_CAST3(Raw,FloatComplex,RawCast<FloatComplex> );
  REGISTER_CAST3(Raw,DoubleComplex,RawCast<DoubleComplex> );

  /*****************************/
  
  // Cast from String to List

  struct StringToList {

    List operator () (const String &toCast) {

      istringstream is(toCast);
      List alist;
      is >> alist;
      return alist;
    }
    
  };
  
  REGISTER_CAST3(String,List,StringToList);

  /*******************/

  //! This cleanup handler removes TheCastMap

  BEGIN_CLEANUP_HANDLER(TheCastMap)
  {
    delete TheCastMap;
  }
  END_CLEANUP_HANDLER(TheCastMap)
}



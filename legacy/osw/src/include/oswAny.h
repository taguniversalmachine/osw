
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
*/
/* oswAny.h
   The "Any" variant type for OSW variables
*/

#ifndef _OSW_ANY_H
#define _OSW_ANY_H
#include "oswDatum.h"

namespace osw {

  struct _oswexport Any {

  protected:

    BaseDatum *xdatum;

    // Unassigned type for Anys that haven't yet been explicitly assigned
    // a datum.

    struct Unassigned {
      
      // always equal
      bool operator == (const Unassigned &toCompare) const {
	return true;
      }
      
      // unordered, so always false
      bool operator < (const Unassigned &toCompare) const {
	return false;
      }
      
      friend _oswexport ostream & operator << (ostream &os, 
					       const Unassigned &toPrint) {
	return os << "<Unassigned>";
      }
      
      friend _oswexport istream &operator >> (istream &is, 
					      Unassigned &toInput) {
	// unassigneds don't take input
	return is;
      }
      
    };

  public:

    Any (const BaseDatum *axdatum = NULL) {
      xdatum = const_cast<BaseDatum *>(axdatum);
      if (xdatum) {
	xdatum->AddRef();
      } else {
	xdatum = new Datum<Unassigned>;
        xdatum->AddRef();
      }
    }

    Any (const Any &toCopy) {
      xdatum = toCopy.xdatum;
      if (xdatum) {
	xdatum->AddRef();
      }
    }

    ~Any () {
      if (xdatum) {
	xdatum->RemoveRef();
      }
    }

    Any &  operator = (const Any &toCopy) {
      if (xdatum) {
	xdatum->RemoveRef();
      }
      xdatum = toCopy.xdatum;
      if (xdatum) {
	xdatum->AddRef();
      }
      return *this;
    }

    bool operator == (const Any &toCompare) {
      return *xdatum == *toCompare.xdatum;
    }

    bool operator < (const Any &toCompare) {
      return false;
    }


    const type_info & TypeOf () const { return xdatum->TypeOf(); }
    int Size () const {return xdatum->Size();}
    const char * Bytes () const {return xdatum->Bytes();}

    friend _oswexport ostream & operator << (ostream &os, const Any &toPrint) {
      return os << *(toPrint.xdatum);
    }

    friend _oswexport istream &operator >> (istream &is, Any &toInput);
    

    template<class T>
    inline operator const T & () const {
      return ((Datum<T>*)xdatum)->Reference();
    }

    BaseDatum *GetDatum () const {
      return xdatum;
    }

    operator BaseDatum * () const {
      return xdatum;
    }

    operator BaseDatum & () const {
      return *xdatum;
    }
  };

  _STLP_TEMPLATE_NULL inline int sizefunc<>(const Any &x) {
    return x.Size();
  }

  _STLP_TEMPLATE_NULL inline const char * ptrfunc(Any &x) {
    return x.Bytes();
  }

  // legacy type from pre-1.0 code, should go away eventually
  typedef BaseDatum *Untyped;

}

#endif //  _OSW_ANY_H

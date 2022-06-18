
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
/* oswDatum.h
   dynamically typed, reference-counted data elements
 */

#ifndef _OSW_DATUM_H
#define _OSW_DATUM_H

namespace osw {

  struct BaseDatum {

    friend _oswexport ostream & operator << (ostream &os, const BaseDatum &toPrint) {
      return toPrint.Print(os);
    }
    
    //! in debug mode, keep track of # of datums
#if OSW_DEBUG
    friend void _oswexport IncrDatumCount();
    friend void _oswexport DecrDatumCount();
#endif

    BaseDatum () {
      xref = 0;
#if OSW_DEBUG
      IncrDatumCount();
#endif
    }
    BaseDatum (const BaseDatum &toCopy) {
      xref = toCopy.xref;
    }

    virtual ~BaseDatum () {
#if OSW_DEBUG
      DecrDatumCount();
#endif
    }
    
    short AddRef ()  {
      return ++xref;
    }

    short Ref () const {
      return xref;
    }

    void RemoveRef () {
       --xref;
       if (xref <= 0) {
	  delete this;
       }
    }

    virtual ostream & Print (ostream &os) const = 0;
    virtual const type_info & TypeOf () const = 0;

    virtual int Size () const  = 0;
    virtual const char *Bytes () = 0;
    virtual void SetFromBytes(const char *ptr, int size) = 0;

    bool operator == (const BaseDatum &toCompare) const {
      return EqualVirtual (toCompare);
    };

  protected:

    short xref;

    virtual bool EqualVirtual (const BaseDatum &toCompare) const = 0;
  };

  template<class T>
    struct Datum : public BaseDatum {
      
      Datum (const T& aval = T()) :
	xval (aval) {
      }
      
      ostream & Print (ostream &os) const {
	return os << xval;
      }
      
      operator T () const {
	return xval;
      }

      T & Reference () {
	return xval;
      }      

      //void * operator new (size_t asize) {
      //  return FindRecycledBlock(asize);
      //}

      //void operator delete (void *toDelete) {
      //  RecycleBlock(toDelete,sizeof(Datum<T>));
      //}

      virtual const type_info & TypeOf () const {return typeid(T);}

      virtual int Size () const {return sizefunc(xval);}
      virtual const char *Bytes() {return ptrfunc(xval);}
      virtual void SetFromBytes(const char *ptr, int size) {
	setfromptr(ptr,size,xval);
      }



    private:

      virtual bool EqualVirtual (const BaseDatum &toCompare) const {	
	try {
	  return xval == dynamic_cast<const Datum<T> &>(toCompare).xval;
	} catch (...) {
	  return false;
	}
      }

      T xval;
      //osw::allocator<Datum> xalloc;
    };

}

#endif // _OSW_DATUM_H

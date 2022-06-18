
/* 
Copyright (c) 2001 Amar Chaudhary. All rights reserved.
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
   oswlist.h
   Dynamic polymorphic list type
*/

#ifndef _OSWLIST_H
#define _OSWLIST_H

#include "osw.h"
#include <deque>

using namespace std;

namespace osw {


  template<class T>
  inline T Extract(Untyped source) {
    return *((Datum<T> *)source);
  }

  class _oswexport List {

  protected:

    struct ListNode {

      BaseDatum *datum;
      ListNode (const BaseDatum *adatum = NULL) :
        datum(const_cast<BaseDatum *>(adatum)) {
	  if (datum) {
	    datum->AddRef();
	  }
	  //cout << datum << " AddRef " << (int)datum->Ref() << endl;
      }
      
      ListNode (const ListNode &toCopy) :
	datum(toCopy.datum) {
	  if (datum) {
	    datum->AddRef();
	  }
	  //cout << datum << " AddRef " << (int)datum->Ref() << endl;
      }
      ~ListNode () {
	if (datum) {
	  //cout << datum << " RemoveRef " << (datum->Ref()-1) << endl;
	  datum->RemoveRef();
	}
      }

      operator Untyped () const {
	return datum;
      }

    };

  public:

    typedef stl::deque<ListNode> implementation;
    typedef implementation::iterator iterator;
    typedef implementation::reverse_iterator reverse_iterator;
    typedef implementation::const_reverse_iterator const_reverse_iterator;
    typedef implementation::const_iterator const_iterator;


    List () {
    }

    List (const List &toCopy);
    ~List();

    List & operator = (const List &toCopy);

    bool operator == (const List &toCompare) const ;

    BaseDatum *nth (int index)  {
      return ximp[index].datum;
    }

    template<class T> 
    T typed_nth (int index) {
      return T(*((Datum<T> *)(ximp[index].datum)));
    }

    template<class T>
    T GetElement (int index) {
      return T(*((Datum<T> *)(ximp[index].datum)));
    }

    template<class T>
    bool IsElementType(int index) {
      return dynamic_cast<Datum<T> *>(ximp[index].datum) != NULL;
    }

    BaseDatum *GetUntypedElement (int index) const {
      return ximp[index].datum;
    }

    iterator begin () { return ximp.begin();}
    iterator end () { return ximp.end();}
    reverse_iterator rbegin () { return ximp.rbegin();}
    reverse_iterator rend () { return ximp.rend();}
    const_iterator begin () const { return ximp.begin();}
    const_iterator end () const { return ximp.end();}
    const_reverse_iterator rbegin () const { return ximp.rbegin();}
    const_reverse_iterator rend () const { return ximp.rend();}


    bool empty () const {
      return ximp.empty();
    }

    int size () {
      return ximp.size();
    }

    void pop_front() {
      ximp.pop_front();
    }
    
    void pop_back() {
      ximp.pop_back();
    }
    
    List &append (const BaseDatum *toAppend) {
      ximp.push_back(toAppend);
      return *this;
    }
    List &prepend (const BaseDatum *toPrepend) {
      ximp.push_front(toPrepend);
      return *this;
    }

    iterator erase (iterator first, iterator last) {
      return ximp.erase(first,last);
    }
    
    iterator erase (iterator at) {
      return ximp.erase(at);
    }

    void clear () {erase(begin(),end());}
    
    friend _oswexport ostream & operator << (ostream &os, const List &toPrint);
    friend _oswexport istream & operator >> (istream &is, List &toInput);

  protected:

    implementation  ximp;

  };

  /**********************/

  // helper function to access GetElement<T> for platforms
  //  that don't do a good job with member-template support (e.g. MSVC)

  template<class T, bool zero = false>
  struct GetElementFromList {

    explicit GetElementFromList(const List &alist, int aindex) 
      : xlist(alist), xindex(aindex) {      
    }
    
    explicit GetElementFromList(const List *alist, int aindex)
      : xlist(*alist), xindex(aindex) {
    }

    operator T () const {
      if (zero) {
        if (dynamic_cast<Datum<T> *>(xlist.GetUntypedElement(xindex))) {
          return *((Datum<T> *) xlist.GetUntypedElement(xindex));
        } else {
          return T(0);
        }
      } else {
        return *((Datum<T> *) xlist.GetUntypedElement(xindex));
      }
    }

  private:
    const List &xlist;
    const int xindex;
  };

}


#endif


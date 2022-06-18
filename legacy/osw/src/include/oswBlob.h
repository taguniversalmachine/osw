//T Blob
//X Aggregate data type


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
   oswBlob.h
   Simple aggregate data type for OSW

   Amar Chaudhary
*/


#ifndef _BLOB_H
#define _BLOB_H

#include "osw.h"
#include <iterator>



namespace osw {


  class _oswexport Blob {

    typedef map<string,BaseDatum *,less<string> > container_type;

  public:

    typedef container_type::iterator  iterator;
    typedef container_type::const_iterator const_iterator;

    Blob () {
    }

    Blob (const Blob &toCopy) {
      for (container_type::const_iterator p = toCopy.xdata.begin(); p != toCopy.xdata.end(); ++p) {
	xdata.insert(*p);
	p->second->AddRef();
      }
    }

    ~Blob() {
      clear();
    }

    Blob & operator = (const Blob &toAssign) {
      clear();
      for (container_type::const_iterator p = toAssign.xdata.begin(); p != toAssign.xdata.end(); ++p) {
	xdata.insert(*p);
	p->second->AddRef();
      }
      return *this;
    }

    void add (const string &afield, BaseDatum *value) {
      if (xdata[afield] != NULL) {
	xdata[afield]->RemoveRef();
      }
      xdata[afield] = value;
      value->AddRef();
    }

    void remove (const string &afield) {
      container_type::iterator found = xdata.find(afield);
      if (found != xdata.end()) {
	found->second->RemoveRef();
	xdata.erase(afield);
      }
    }

    void clear () {
      for (container_type::iterator p = xdata.begin(); p != xdata.end(); ++p) {
	p->second->RemoveRef();
      }
      xdata.erase(xdata.begin(),xdata.end());
    }

    BaseDatum *get(const string &afield) const {
      container_type::const_iterator found = xdata.find(afield);
      return (found != xdata.end()) ? found->second : NULL;
    }

    template<class T>
    BaseDatum *get_typed(const string &afield) const {
      return dynamic_cast<T*>(get(afield));
    }

    bool operator == (const Blob &toCompare) const {
      return xdata == toCompare.xdata;
    }

    iterator begin() {return xdata.begin();}
    iterator end() {return xdata.end();}
    const_iterator begin() const {return xdata.begin();}
    const_iterator end() const {return xdata.end();}

    friend ostream & operator << (ostream &os, const Blob &toOutput) {
      os << "<Blob ";
      toOutput.print_stuff(os);
      return os;
    }

    friend istream & operator >> (istream &is, Blob &toInput) {
      return is;
    }

    ostream & print_stuff(ostream &os) const {
      for (container_type::const_iterator p = xdata.begin(); p != xdata.end(); ++p) {
	os << p->first << ':' << *(p->second) << ' ';
      }
      return os;
    }
  protected:
    container_type xdata;
  };

}

#endif 



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
Copyright (c) 1998,1999 Regents of the University of California.
All rights reserved.

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
   oswtable.h
   Lookup tables
   Amar Chaudhary
*/

#ifndef _OSWTABLE_H
#define _OSWTABLE_H

#include "osw.h"

using namespace std;

namespace osw {
  
template<class T>
  class Table {

    osw::vector<T> xvect;

  public:

    typedef _TYPENAME osw::vector<T>::iterator iterator;
    typedef _TYPENAME osw::vector<T>::reverse_iterator reverse_iterator;
    typedef _TYPENAME osw::vector<T>::const_iterator const_iterator;
    typedef _TYPENAME osw::vector<T>::const_reverse_iterator const_reverse_iterator;

    
    Table (int asize = 0, const T &astart = T(0), const T &afinish = T(1)) :
      xvect(asize) {
	xstart = astart;
	xfinish = afinish;
	xone_over_range = T(1) / (afinish - astart);
    } 

    Table(const osw::vector<T> &avect,
	  const T &astart = T(0), const T &afinish = T(1)) :
      xvect(avect) {
	xstart = astart;
	xfinish = afinish;
	xone_over_range = T(1) / (afinish - astart);
    }

    Table (const Table &toCopy) :
      xvect(toCopy.xvect) {
	xstart = toCopy.xstart;
	xfinish = toCopy.xfinish;
	xone_over_range = toCopy.xone_over_range;
    }
    
    template<class Function>
    Table(Function f, int asize, const T &astart, const T &afinish) :
      xvect(asize) {
	xstart = astart;
	xfinish = afinish;
	xone_over_range = T(1) / (afinish - astart);
	T val = xstart;
	T delta = T(asize) / xone_over_range;
	for (int i = 0; i < asize; ++i, val += delta) {
	  xvect[i] = f(val);
	}
    }
    
    Table(T (*f)(T), int asize, const T &astart, const T &afinish) :
      xvect(asize) {
      	xstart = astart;
	xfinish = afinish;
	xone_over_range = T(1) / (afinish - astart);
	T val = xstart;
	T delta = T(1) / T(asize) / xone_over_range;
	for (int i = 0; i < asize; ++i, val += delta) {
	  xvect[i] = f(val);
	}
    }
    

    template<class Iterator>
    Table (Iterator first, Iterator last, const T &astart, const T &afinish) :
      xvect(last - first) {
	xstart = astart;
	xfinish = afinish;
	xone_over_range = T(1) / (afinish - astart);
        int i = 0;
	for (Iterator index = first; index != last; ++index, ++i) {
	  xvect[i] = *index;
	}
    }

    T& operator [] (int index) {
      return xvect[index];
    }

    T operator () (const T& val) {
      return xvect[int((val - xstart) * xone_over_range * T(size())) % size()];
    }

    T operator () (const T& val, const bool unused) {
      int index = int((val - xstart) * xone_over_range * T(size()-1)) % (size()-1);
      float findex = fmod((val - xstart) * xone_over_range * T(size()-1),size()-1);
      float weight = findex - float(index);
      return (1.0f - weight) * xvect[index] + weight * xvect[index+1];
    }
      
    
    int size() {return xvect.size();}

    T start () {
      return xstart;
    }

    T finish () {
      return xfinish;
    }

    bool operator == (const Table &toCompare) const {
      return xvect == toCompare.xvect;
    }

    iterator begin() {return xvect.begin();}
    iterator end() {return xvect.end();}
    reverse_iterator rbegin() {return xvect.rbegin();}
    reverse_iterator rend() {return xvect.rend();}
    const_iterator begin() const {return xvect.begin();}
    const_iterator end() const {return xvect.end();}
    const_reverse_iterator rbegin() const {return xvect.rbegin();}
    const_reverse_iterator rend() const {return xvect.rend();}

    ostream & Print (ostream &os) const {
      return os << xvect;
    }

    istream & Input (istream &is) {
      return is >> xvect;
    }

  protected:
    
    T	xstart, xone_over_range, xfinish;
  };

 template<class T>
   inline ostream & operator << (ostream &os, const Table<T> &toPrint) {
   return toPrint.Print(os);
 }

 template<class T>
   inline istream & operator >> (istream &is, Table<T> &toInput) {
   return toInput.Input(is);
 }

   typedef Table<float> FloatTable;
   typedef Table<int>   IntegerTable;
   
   extern _oswexport Table<float> SineTable, CosineTable, ExpTable, SincTable;
   
   _oswexport Table<float> *GetTable (const string &aname, 
                                      Container *acontainer = NULL);
   _oswexport State<Unit> *GetTableChangedVariable (const string &aname, 
                                                    Container *acontainer = NULL);
} // namespace osw

#endif // _OSWTABLE_H





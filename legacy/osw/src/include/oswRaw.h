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
/* oswRaw.h
   rawbytes class and associated functions for handling arbitrary
   binary data
*/

#ifndef _OSW_RAW_H
#define _OSW_RAW_H

namespace osw {
  template<class T>
  inline int sizefunc(const T &x) {
    return sizeof(x);
  }

  template<class T>
  inline const char * ptrfunc(T &x) {
    return (const char *) &x;
  }

  template<class T>
  inline void setfromptr(const char *ptr, int size, T &toSet) {
    toSet = *((const T *)ptr);
  }

  _STLP_TEMPLATE_NULL inline int sizefunc<string>(const string &x) {
    return x.size();
  }

  _STLP_TEMPLATE_NULL inline const char * ptrfunc<string>(string &x) {
    return x.c_str();
  }

  _STLP_TEMPLATE_NULL inline void setfromptr (const char *ptr, 
					      int size, 
					      string &toSet) {
    toSet = string(ptr,size);
  }

  _STLP_TEMPLATE_NULL inline int sizefunc(const osw::vector<float> &v) {
    return sizeof(float) * v.size();
  }

  _STLP_TEMPLATE_NULL inline const char * ptrfunc(osw::vector<float> &v) {
    return (const char *) v.begin();
  }

  _STLP_TEMPLATE_NULL inline void setfromptr (const char *ptr,
					      int size,
					      vector<float> &toSet) {
    toSet = vector<float>((float *)ptr,size / sizeof(float));
  }


  _STLP_TEMPLATE_NULL inline int sizefunc(const osw::vector<double> &v) {
    return sizeof(double) * v.size();
  }

  _STLP_TEMPLATE_NULL inline const char * ptrfunc(osw::vector<double> &v) {
    return (const char *) v.begin();
  }

  _STLP_TEMPLATE_NULL inline void setfromptr (const char *ptr,
					      int size,
					      vector<double> &toSet) {
    toSet = vector<double>((double *)ptr,size / sizeof(double));
  }

  _STLP_TEMPLATE_NULL inline int sizefunc(const osw::vector<short> &v) {
    return sizeof(short) * v.size();
  }

  _STLP_TEMPLATE_NULL inline const char * ptrfunc(osw::vector<short> &v) {
    return (const char *) v.begin();
  }

  _STLP_TEMPLATE_NULL inline void setfromptr (const char *ptr,
					      int size,
					      vector<short> &toSet) {
    toSet = vector<short>((short *)ptr,size / sizeof(short));
  }

  _STLP_TEMPLATE_NULL inline int sizefunc(const osw::vector<int> &v) {
    return sizeof(int) * v.size();
  }

  _STLP_TEMPLATE_NULL inline const char * ptrfunc(osw::vector<int> &v) {
    return (const char *) v.begin();
  }

  _STLP_TEMPLATE_NULL inline void setfromptr (const char *ptr,
					      int size,
					      vector<int> &toSet) {
    toSet = vector<int>((int *)ptr,size / sizeof(int));
  }

  struct rawbytes {

    typedef byte *iterator;
    typedef const byte *const_iterator;

    rawbytes(int n = 1) :
      xdata(new byte[n]) {
      xsize = n;
      xref = 1;
    }

    rawbytes(int n, byte c) :
      xdata(new byte[n]) {
      memset((void *) xdata, c, n);
      xsize = n;
      xref = 1;
    }

    rawbytes(const rawbytes &toCopy) :
      xdata (toCopy.xdata) {
      xsize = toCopy.xsize;
      xref = ++toCopy.xref;
    }

    rawbytes(int n, void *adata) :
      xdata(new byte[n]) {
      byte *adata1 = (byte *)(adata);
      copy_n(adata1,n,xdata);
      xsize = n;
      xref = 1;
    }

    ~rawbytes() {
      --xref;
      if (xdata && xref == 0) {
	delete [] xdata;
      }
    }

    rawbytes & operator = (const rawbytes &toCopy) {
      --xref;
      if (xdata && xref == 0) {
	delete [] xdata;
      }
      xdata = toCopy.xdata;
      xsize = toCopy.xsize;
      xref = ++toCopy.xref;
      return *this;
    }

    int size () const {return xsize;}

    iterator begin() {return xdata;}
    iterator end () {return xdata+xsize;}
    const_iterator begin() const {return xdata;}
    const_iterator end () const {return xdata+xsize;}

    byte & operator [] (int index) {
      return xdata[index];
    }

    const byte operator [] (int index) const {
      return xdata[index];
    }

    friend ostream & operator << (ostream &os, const rawbytes &toPrint) {
      return os.write((const char *)toPrint.xdata,toPrint.xsize);
    }

    friend istream & operator >> (istream &is, rawbytes &toInput) {
      return is.read((char *)toInput.xdata,toInput.xsize);
    }

    bool operator == (const rawbytes &toCompare) const {
      return false;
    }

  private:

    byte *xdata;
    size_t xsize;
    mutable short xref;
  };

  typedef rawbytes Raw;

}

#endif // _OSW_RAW_H

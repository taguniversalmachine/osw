//T Spectrum
//X Frequency-domain spectrum, with real and imaginary components

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
   Spectrum.h
   spectrum data type for OSW
   (unlike core source files, external source files do not have an "osw"
    prefix)
*/

#ifndef _SPECTRUM
#define _SPECTRUM

#include "osw.h"

namespace osw {

  struct _oswexport Spectrum {
    
    Spectrum (int NumBins = 1) :
      interleaved (2*NumBins),
      real (interleaved,0,2*NumBins,2),
      imag (interleaved,1,2*NumBins,2) {
    }
    
    Spectrum  (const osw::vector<float> &areal,
	       const osw::vector<float> &aimag) :
      interleaved (2*areal.size()),
      real (interleaved,0,2*areal.size(),2),
      imag (interleaved,1,2*areal.size(),2) {
	for (int i = 0; i < areal.size(); ++i) {
	  real[i] = areal[i];
	  imag[i] = aimag[i];
	}
      }

    Spectrum (const Spectrum &toCopy) :
      interleaved(toCopy.interleaved),
      real (interleaved,0,interleaved.size(),2),
      imag (interleaved,1,interleaved.size(),2) {
    }

    osw::vector<float> interleaved;
    osw::slice<float>  real, imag;

    bool operator == (const Spectrum &toCompare) const {
      return interleaved == toCompare.interleaved;
    }

    bool operator != (const Spectrum &toCompare) const {
      return !(interleaved == toCompare.interleaved);
    }
    
    friend ostream & operator << (ostream &os, const Spectrum &toPrint);
    friend istream & operator >> (istream &is, Spectrum &toInput);
    
    int size () const {
      return real.size();
    }

  };

  
  inline ostream& operator << (ostream& os, const Spectrum &toPrint) {
    
    for (int i = 0; i < toPrint.real.size(); ++i) {
      os << toPrint.real[i] << ' ' << toPrint.imag[i] << ' ';
    }
    return os;
  }
  
  inline istream& operator >> (istream& is, Spectrum &toInput) {
    
    for (int i = 0; i < toInput.real.size(); ++i) {
      is >> toInput.real[i] >> toInput.imag[i];
    }
    return is;
  }


} // namespace osw


#endif // _SPECTRUM




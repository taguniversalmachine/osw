//T Resonances
//X Frequency, amplitude and decay-rate values of resonance partials
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
   Resonances.h
   data type and base transform class for working with resonance models

   Amar Chaudhary
   5/11/99 Version
*/

#ifndef _RESONANCES_H
#define _RESONANCES_H

#include "osw.h"

namespace osw {


  struct _oswexport Resonances {
    
    Resonances (int NumPartials = 1) :
      freq (NumPartials,1.0f),
      amp  (NumPartials),
      decay(NumPartials) {
    }
    
    Resonances (const Samples &afreq,
	       const Samples &aamp,
	       const Samples &adecay) :
      freq(afreq), amp (aamp), decay (adecay) {
    }
    
    Samples	freq, amp, decay;

    bool operator == (const Resonances &toCompare) const {
      return freq == toCompare.freq && amp == toCompare.amp 
	&& decay == toCompare.decay;
    }

    int size() {return freq.size();}
    
  };

  ostream& operator << (ostream& os, const Resonances &toPrint) {
    
    for (int i = 0; i < toPrint.freq.size(); ++i) {
      os << i << ' ' << toPrint.freq[i]
	 << ' ' << toPrint.amp[i]
	 << ' ' << toPrint.decay[i] 
	 << ' ';
    }
    return os;
  }
  
  istream& operator >> (istream& is, Resonances &toInput) {
    
    int numPartials;
    is >> numPartials;
    
    for (int i = 0; i < numPartials; ++i) {
      int index;
      is >> index;
      is >> toInput.freq[index]
	 >> toInput.amp[index]
	 >> toInput.decay[index];
    }
    return is;
  }


 REGISTER_TYPE(Resonances);

  
  
} // namespace osw


#endif // _RESONANCES_H




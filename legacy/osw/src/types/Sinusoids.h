//T Sinusoids
//X Frequency, amplitude and phase values of sinusoidal partials

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
   sinusoids.h
   data type and base transform class for working with sinusoids
   (i.e., "BYOs")

   Amar Chaudhary
*/

#ifndef _SINUSOIDS_H
#define _SINUSOIDS_H

#include "osw.h"

namespace osw {


  struct _oswexport Sinusoids {
    
    Sinusoids (int NumPartials = 1) :
      index (NumPartials),
      freq (NumPartials,1.0f),
      amp  (NumPartials),
      phase(NumPartials) {
    }
    
    Sinusoids (const Samples &aindex,
	       const Samples &afreq,
	       const Samples &aamp,
	       const Samples &aphase) :
      index(aindex), freq(afreq), amp (aamp), phase (aphase) {
    }

    Samples	  index, freq, amp, phase;

    int size () {return freq.size();}

    bool operator == (const Sinusoids &toCompare) const {
      return freq == toCompare.freq && amp == toCompare.amp 
	&& phase == toCompare.phase;
    }
    
    Sinusoids operator + (const Sinusoids &toConcat) const {
      return Sinusoids (concat(index,toConcat.index),
			concat(freq,toConcat.freq),
			concat(amp,toConcat.amp),
			concat(phase,toConcat.phase));
    }
    
  };

  inline ostream& operator << (ostream& os, const Sinusoids &toPrint) {
    
    for (int i = 0; i < toPrint.freq.size(); ++i) {
      os << toPrint.index[i] << ' ' << toPrint.freq[i]
	 << ' ' << toPrint.amp[i]
	 << ' ' << toPrint.phase[i] << ' ';
    }
    return os;
  }
  
  inline istream& operator >> (istream& is, Sinusoids &toInput) {
    
    int numPartials;
    is >> numPartials;
    
    for (int i = 0; i < numPartials; ++i) {
      int index;
      is >> index;
      is >> toInput.freq[index]
	 >> toInput.amp[index]
	 >> toInput.phase[index];
    }
    return is;
  }


  REGISTER_TYPE(Sinusoids);


} // namespace osw


#endif // _SINUSOIDS_H




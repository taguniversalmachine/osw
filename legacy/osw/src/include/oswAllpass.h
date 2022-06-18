
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
  oswAllpass.h
  A unit-delay allpass filter for use in OSW transforms

  Amar Chaudhary
*/

#ifndef _OSW_ALLPASS_H
#define _OSW_ALLPASS_H

#include "osw.h"

namespace osw {

  //! Single-sample allpass filter
  /** An allpass filter with a single sample of delay.  The filter has 
      feed-forward and feedback paths scaled by a gain factor and its
      negation, respectively.  This filter has a flat frequency response,
      but frequency-dependent phase response.  It is used as a building
      block for a variety of algorithms, including phase shifters and
      reverberators.
  */

  class Allpass {

  private:

    Float xgain;
    mutable Float xprev;

  public:

    //! Constructor for Allpass
    /** initializes gain to zero
     */
    Allpass()
      : xgain(0.0f), xprev(0.0f) {
    }


    //! get current gain parameter
    Float Gain () const {
      return xgain;
    }

    //! set gain parameter 
    /**
     * @param again new gain value
     */
    void SetGain (Float again) {
      xgain = again;
    }

    //! set gain parameter based on a delay value
    /**
     * @param adelay a value: (# of samples) / (sample rate)
     */
    void SetDelay( Float adelay ){      
      xgain = (1.f - adelay) / (1.f + adelay);
    }


    //! operator ()
    /**
     * @param sample a floating-point sample to be processed
     * @return sample representing output of allpass filter
     * Unit-delay, feedback and feed-forward operations are performed
     * on incoming sample. Feedback value is stored to be added to next
     * incoming value
     */

    Float operator () (Float sample) const {
      Float y = sample * -xgain + xprev;
      xprev = y * xgain + sample;
      return y;
    }

    //! operator () on vectors (e.g., Samples)
    /**
     * @param vect input vector 
     * Sequentially applies the allpass filter to each sample in a vector
     */
    
#ifdef _MSC_VER    
    // Micro$oft VC++ can't seem to handle the member template here :(
    unopvector<vector<Float>, Allpass> 
    operator () (const vector<Float> &vect) const {
      return unopvector<vector<Float>,Allpass> (vect,*this);
    }
#else
    template <class InputVector> 
    unopvector<InputVector,Allpass,typename InputVector::value_type> 
    operator () (const InputVector &vect) const {
      return unopvector<InputVector,Allpass> (vect,*this);
    }
#endif

  };

  


}



#endif // _OSW_ALLPASS_H


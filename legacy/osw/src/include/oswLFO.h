
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
  oswLFO.h
  LFO functions for use OSW transfors

  Amar Chaudhary
*/

#ifndef _OSW_LFO_H
#define _OSW_LFO_H

#include "osw.h"

namespace osw {

  //! LFO (low-frequency oscillator) class
  
  template<typename T = Float>
  struct LFO {

    //! enumeration of LFO functions supported by the LFO class

    typedef enum {
      RANDOM, 
      SINE,
      SAW,
      SQUARE,
      TRIANGLE,
      PULSE,
      PULSE25,
      ABSSINE,
      FURRY,
      SINE1_2
    } ShapeType;

    //! Constructor for LFO
    /**
     * @param ashape inital shape (function) to use, default is SINE
     */

    LFO (ShapeType ashape = SINE) :
      xshape(ashape),
      xlastphase(T(0)),
      xlastoutput(T(0)) {
    }

    //! return the current LFO function being used
    ShapeType GetShape () const { return xshape; }

    //! set the LFO function
    /**
     * @param ashape new function to use
     */

    void SetShape (ShapeType ashape) {
      xshape = ashape;
    }

    //! operator (), evaluate LFO function at a given phase value
    /** 
     * @param phase phase value for LFO
     * @return value of current LFO function at given phase
     */

    T operator () (T phase) {
      T result;

      switch (xshape) {
       
      case RANDOM:
        phase = frac(phase);
        if (phase >= xlastphase) {
          result = xlastoutput;
        } else {
          xlastoutput = result = Random();
        }
        xlastphase = phase;
        break;
      case SINE:
        result = T(0.5) - T(0.5) * cos(T(2) * T(M_PI) * phase);
        break;
      case ABSSINE:
        result = fabs(sin(T(2) * T(M_PI) * phase));
        break;
      case SAW:
        result = frac(phase);
        break;
      case SQUARE:
        result = T(frac(phase) < T(0.5));
        break;
      case TRIANGLE:
        phase = frac(phase);
        result = (phase < T(0.5)) ? T(2) * phase : T(2) - T(2) * phase;
        break;
      case PULSE:
        phase = frac(phase);
        result = T(phase < xlastphase);      
        xlastphase = phase;
        break;
      case PULSE25:
        phase = frac(phase);
        result = T(phase < 0.25);
        break;
      case SINE1_2:
        result = T(0.5) 
          - T(0.25) * cos(T(2) * T(M_PI) * phase)
          - T(0.25) * cos(T(4) * T(M_PI) * phase);
        break;
      case FURRY:
        result = T(0.5) - T(0.4) * cos(T(2) * T(M_PI) * phase)
          + T(0.2) * Random();
        break;
      default:
        result = T(0);
        break;
      }
      return result;
    }

  private:

        
    T Random () {
      return rand() / T(RAND_MAX);
    }

    ShapeType xshape;
    T         xlastphase, xlastoutput;
  };

  _STLP_TEMPLATE_NULL
  Integer LFO<Integer>::Random () {
    return rand();
  }




}



#endif // _OSW_LFO_H



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
  delay.h
  A delay-line object for use in transforms

  Amar Chaudhary
  10/23/99 Version
*/

#ifndef _OSW_DELAY_H
#define _OSW_DELAY_H

#include "osw.h"

namespace osw {

  class DelayLine {

  protected:

    stl::vector<float> xbuffer;
    int             xin,xout;
    float           xweight;

  public:

    DelayLine (int max_length = 1024) :
      xbuffer(max_length,0.0f) {
       xin = 0;
       xout = max_length - 1;
       xweight = 0;
    }

    void clear () {
      for (int i = 0; i < xbuffer.size(); ++i) {
	xbuffer[i] = 0.0f;
      }
    }

    void set_delay (float adelay) {
      if (adelay > xbuffer.size() - 1) {
	Alert ("delay value exceeds length.");
        cout << adelay << " > " << xbuffer.size() << endl;
	xout = xin + 1;        
      } else {
	xout = xin - int(adelay);
        xweight = frac(adelay);
      }
      while (xout < 0) {
	xout += xbuffer.size();
      }
    }

    void set_max (int amax) {
      if (amax > xbuffer.size()) {
	xbuffer.reserve(amax);
	xbuffer.insert(xbuffer.end(),amax-xbuffer.size(),0.0f);
      }
      clear();
    }

    int size () const {
      return xbuffer.size();
    }
    
    float operator () (float sample) {
      float result;
      xbuffer[xin] = sample;
      ++xin;
      xin %= xbuffer.size();
      result = xbuffer[xout];
      ++xout;
      xout %= xbuffer.size();
      return result;
    }      

    float interpolate (float sample) {
      float result;
      xbuffer[xin] = sample;
      ++xin;
      xin %= xbuffer.size();
      result = xbuffer[xout];
      ++xout;
      xout %= xbuffer.size();
      result = (1.0 - xweight) * result + xweight * xbuffer[xout];
      return result;
    }

    float previous_out () const {
      return xbuffer[xout];
    }

    float & operator [] (int index) {
      return xbuffer[index];
    }
  };
}



#endif _OSW_DELAY_H


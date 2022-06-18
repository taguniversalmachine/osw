//T Biquad
//X a biquad filter with variable coefficients
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
  Biquad.h
  A combined function and data type representing a biquad filter,
  with variable coefficients and single-sample computation.

  Amar Chaudhary
  10/23/99 Version
*/


#ifndef _BIQUAD_H
#define _BIQUAD_H

#include "osw.h"

namespace osw {

  class Biquad {

    mutable float i1,i2,o1,o2,output_result;

  public:

    float a0, a1, a2, b1, b2;


    Biquad () {
      i1 = i2 = o1 = o2 = 0.0f;
      a0 = a1 = a2 = b1 = b2 = 0.0f;
    }

    Biquad (float aa0, float aa1, float aa2, float ab1, float ab2) {
      i1 = i2 = o1 = o2 = 0.0f;
      a0 = aa0;
      a1 = aa1;
      a2 = aa2;
      b1 = ab1;
      b2 = ab2;
    }

    Biquad (const Biquad &toCopy) {
      a0 = toCopy.a0;
      a1 = toCopy.a1;
      a2 = toCopy.a2;
      b1 = toCopy.b1;
      b2 = toCopy.b2;
      i1 = toCopy.i1;
      i2 = toCopy.i2;
      o1 = toCopy.o1;
      o2 = toCopy.o2;
    }

    Biquad & operator = (const Biquad &toAssign) {
      a0 = toAssign.a0;
      a1 = toAssign.a1;
      a2 = toAssign.a2;
      b1 = toAssign.b1;
      b2 = toAssign.b2;
      return *this;
    }

    bool operator == (const Biquad &toCompare) const {
      return a0 == toCompare.a0 
	&& a1 == toCompare.a1 && a2 == toCompare.a2
	&& b1 == toCompare.b1 && b2 == toCompare.b2;
    }

    void clear () {
      i1 = i2 = o1 = o2 = 0.0f;
    }

    void set_poles (float ab1, float ab2) {
      b1 = ab1;
      b2 = ab2;
    }

    void set_zeroes (float aa1, float aa2) {
      a1 = aa1;
      a2 = aa2;
    }
    
    void set_freq_reson (float afreq, float areson, float srate) {
      b2 = -areson * areson;
      b1 = 2.0f * areson * cos(2.0f * float(M_PI) * afreq / srate);
      a0 = 1 - areson;
    }

    void set_freq_bw (float afreq, float abw, float srate) {
      set_freq_reson (afreq, exp(-float(M_PI) * abw / srate), srate);
    }

    void set_equal_gain_zeroes () {
      a2 = -1.0f;
      a1 = 0.0f;
    }

    void set_gain (float aa0) {
      a0 = aa0;
    }

    float operator () (float input) const {
      float input_result = input * a0;
      input_result += i1 * a1;
      input_result += i2 * a2;
      
      output_result = input_result;
      output_result += o1 * b1;
      output_result += o2 * b2;
      i2 = i1;
      i1 = input;
      o2 = o1;
      o1 = output_result;
      
      return output_result;
    }

    float last_output () const {
      return output_result;
    }

#ifdef _MSC_VER    
    // Micro$soft VC++ can't seem to handle the member template here :(
    unopvector<vector<float>, Biquad, float> operator () (const vector<float> &vect) const {
      return unopvector<vector<float>,Biquad> (vect,*this);
    }
#else
    // this version lets us integrate Biquads into any optimized vector expression.
    template <class InputVector> 
    unopvector<InputVector,Biquad, float> operator () (const InputVector &vect) const {
      return unopvector<InputVector,Biquad, float> (vect,*this);
    }
#endif

    friend ostream & operator << (ostream &os, const Biquad &toPrint) {
      os << "<Biquad " << toPrint.a0 
	 << ' ' << toPrint.a1 << ' ' << toPrint.a2 
	 << ' ' << toPrint.b1 << ' ' << toPrint.b2 << '>';
      return os;
    }

    friend istream & operator >> (istream &is, Biquad &toInput) {
      // does not accept input
      return is;
    }
  };

}

#endif

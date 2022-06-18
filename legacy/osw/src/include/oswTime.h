
/* 
Copyright (c) 2001-2003 Amar Chaudhary. All rights reserved.
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
   oswTime.h
   OSW's diabolical Time data type

   Amar Chaudhary

*/

#ifndef _OSWTIME_H
#define _OSWTIME_H

#include "osw.h"

using namespace std;

namespace osw {


  struct Time {

    Double time,prevtime;
    Float  sample_rate, sampling_interval;
    Float  scale_factor;
    mutable Float prev_scale_factor;
    int    num_samples;
    mutable stl::vector<Double> local_states;
    mutable int    local_state_index;

    explicit Time (Double atime = 0.0, Float asample_rate = 44100.0f,
	  int anum_samples = 64)
      : local_states(8,0.0) {
	time = atime;
	prevtime = atime;
	sample_rate = asample_rate;
	sampling_interval = 1.0f / sample_rate;
	num_samples = anum_samples;
	scale_factor = 1.0f;
	prev_scale_factor = scale_factor;
	local_state_index = 0;
    }
    
    Time (Double atime, Float asample_rate, int anum_samples,
	  Float ascale_factor) 
      : local_states(8,0.0) {
	prevtime = time = atime;
	sample_rate = asample_rate;
	sampling_interval = 1.0f / sample_rate;
	num_samples = anum_samples;
	prev_scale_factor = scale_factor = ascale_factor;      
	local_state_index = 0;
    }
    
    Time (Double atime, Double aprevtime,
	  Float asample_rate, Float asampling_interval, int anum_samples,
	  Float aprev_scale_factor, Float ascale_factor) 
      : local_states(8,0.0) {
	time = atime;
	prevtime = aprevtime;
	sample_rate = asample_rate;
	sampling_interval = asampling_interval;
	num_samples = anum_samples;
	scale_factor = ascale_factor;
	prev_scale_factor = aprev_scale_factor;
	local_state_index = 0;
    }
    
    Time (const Time &toCopy) {
      time = toCopy.time;
      prevtime = toCopy.prevtime;
      sample_rate = toCopy.sample_rate;
      sampling_interval = toCopy.sampling_interval;
      num_samples = toCopy.num_samples;
      scale_factor = toCopy.scale_factor;
      prev_scale_factor = toCopy.prev_scale_factor;
      local_state_index = toCopy.local_state_index;
      local_states = toCopy.local_states;
    }
    
    /*Time & operator = (const Double toSet) {
      cout << "E\n";
      prevtime = time;
      time = toSet;
    }*/

    Time & operator = (const Time &toCopy) {
      prevtime = toCopy.prevtime;
      time = toCopy.time;
      sample_rate = toCopy.sample_rate;
      sampling_interval = toCopy.sampling_interval;
      num_samples = toCopy.num_samples;
      scale_factor = toCopy.scale_factor;
      // other state variables are local, so don't update them
      // but reset state index to 0
      local_state_index = 0;
      return *this;
    }

    Time & SetTime (const Double toSet) {
      prevtime = time;
      time = toSet;
      return *this;
    }

    operator Double () const {return time;}


    bool operator == (const Time &toCompare) const {
      return time == toCompare.time && scale_factor == toCompare.scale_factor;
    }

    bool operator < (const Time &toCompare) const {
      return Double(*this) < Double(toCompare);
    }

    /*
    inline friend const Time & operator + (const Time &time, const Float offset) {
      return Time(time.time + offset, time.prevtime, time.sample_rate,
		  time.sampling_interval, time.num_samples, time.prev_scale_factor,
		  time.scale_factor);
    }

    inline friend const Time & operator + (const Float offset, const Time &time) {
      return Time(offset + time.time, time.prevtime, time.sample_rate,
		  time.sampling_interval, time.num_samples, time.prev_scale_factor,
		  time.scale_factor);
    }

    inline friend const Time & operator - (const Time &time, const Float offset) {
      return Time(time.time - offset, time.prevtime, time.sample_rate,
		  time.sampling_interval, time.num_samples, 
		  time.prev_scale_factor, time.prev_scale_factor);
    }

    inline friend const Time & operator - (const Float offset, const Time &time) {
      return Time(offset - time.time, time.prevtime, time.sample_rate,
		  time.sampling_interval, time.num_samples, 
		  time.prev_scale_factor, time.scale_factor);
    }

    */

    struct defmul {
      const Time &time;
      const Float scale_factor;
      
      defmul(const Time &atime, const Float ascale_factor) :
	time(atime), scale_factor(ascale_factor) {
      }
    };

    inline friend defmul operator * (const Time &time, const Float factor) {
      return defmul (time,factor);
    }

    inline friend defmul operator * (const Float factor, const Time &time) {
      return defmul (time,factor);
    }

    inline friend defmul operator / (const Time &time, const Float factor) {
      return defmul (time,1.0f / factor);
    }

    Double operator - (const Time &toSubtract) const {
      return Double(*this) - Double(toSubtract);
    }

    Double operator / (const Time &toDivide) const {
      return Double(*this) / Double(toDivide);
    }

    void set_sample_rate (Float asample_rate) {
      sample_rate = asample_rate;
      sampling_interval = 1.0f / asample_rate;
    }

    void scale (Float ascale_factor) {
      prev_scale_factor = scale_factor;
      scale_factor = ascale_factor;
    }

    void reset_scale () {
      prev_scale_factor = scale_factor;
      scale_factor = 1.0f;
    }

    Time & operator ++ () {
      prevtime = time;
      time += scale_factor * num_samples * sampling_interval;
      return *this;
    }

    friend _oswexport ostream & operator << (ostream &os, const Time &toPrint) {
      return os << toPrint.time ;
    }

    friend _oswexport istream & operator >> (istream &is, Time &toInput) {
      return is >> toInput.time;
    }

    
    Double & current_local_state () const {
      return local_states[local_state_index];
    }

    Double & next_local_state () const {
      ++local_state_index;
      return local_states[local_state_index];
    }

    void reserve (int toReserve) const {
      if (local_state_index + toReserve > local_states.capacity()) {
	local_states.resize(local_state_index + toReserve);
      }
    }


    interpolate<Float> InterpolateFloat (int asize) const {
      return interpolate<Float>(asize,(Float)prevtime,(Float)time);
    }
    interpolate<Double> InterpolateDouble (int asize) const {
      return interpolate<Double>(asize,(Double)prevtime,(Double)time);
    }
  
  };
  
  inline vector<Float> saw (const Time::defmul &dm) {
    vector<Float> result(dm.time.num_samples);
    Double dt = 2.0 * dm.scale_factor * dm.time.scale_factor 
      * dm.time.sampling_interval;
    Double t = dm.time.local_states[dm.time.local_state_index];
    for (int i = 0; i < dm.time.num_samples; ++i) {
      t += dt;
      if (t >= 2.0) {
	t -= 2.0;
      }
      result[i] = t - 1.0f;
    }
    dm.time.local_states[dm.time.local_state_index] = t;
    ++dm.time.local_state_index;
    return result;
  }
  
  inline vector<Float> saw (const Time &time) {
    return saw(time * 1.0f);
  }

}
  
// define these guys outside of the osw namespace
using osw::Float;
using osw::Double;
using osw::DoubleComplex;

inline osw::vector<Float> exp (const osw::Time::defmul &dm) {
  osw::vector<Float> result(dm.time.num_samples);
  int index = dm.time.local_state_index;
  dm.time.reserve(3);
  Double newfactor = dm.scale_factor * dm.time.scale_factor * dm.time.sampling_interval;
  Double xvalue,xfactor;
  if (dm.time.local_states[index] != newfactor) {
    dm.time.local_states[index] = newfactor;
    xfactor = exp(newfactor);
    dm.time.local_states[index+1] = xfactor;
  } else {
    xfactor = dm.time.local_states[index+1];
  }
  dm.time.local_states[index+2] = exp(newfactor * dm.time.sample_rate * dm.time.time);
  xvalue = dm.time.local_states[index+2];
  for (int i = 0; i <  dm.time.num_samples; ++i) {
    xvalue *= xfactor;
    result[i] = 0.97 * xvalue;
  }
  dm.time.local_states[index+2] = xvalue;
  dm.time.local_state_index += 3;
  return result;
}

inline osw::vector<Float> sin (const osw::Time::defmul &dm) {
  osw::vector<Float> result(dm.time.num_samples);
  int index = dm.time.local_state_index;
  dm.time.reserve(5);
  Double newfactor = dm.scale_factor * dm.time.scale_factor * dm.time.sampling_interval;
  DoubleComplex xvalue,xfactor;
  if (dm.time.local_states[index] != newfactor) {
    dm.time.local_states[index] = newfactor;
    xfactor =	exp(DoubleComplex(0,newfactor));
    dm.time.local_states[index+1] = real(xfactor);
    dm.time.local_states[index+2] = imag(xfactor);
  } else {
    xfactor = DoubleComplex(dm.time.local_states[index+1],
			    dm.time.local_states[index+2]);
  }
  if (dm.time.local_states[index+4] == 0.0) {
    dm.time.local_states[index+4] = 1.0;
  }
  xvalue = DoubleComplex(dm.time.local_states[index+3],
			 dm.time.local_states[index+4]);
  //xvalue = exp(complex<Float>(0,newfactor * dm.time.sample_rate * dm.time.time));
  for (int i = 0; i <  dm.time.num_samples; ++i) {
    xvalue *= xfactor;
    result[i] = Float(0.97 * imag(xvalue));
  }
  dm.time.local_states[index+3] = real(xvalue);
  dm.time.local_states[index+4] = imag(xvalue);
  dm.time.local_state_index += 5;
  return result;
}

inline osw::vector<Float> cos (const osw::Time::defmul &dm) {
  osw::vector<Float> result(dm.time.num_samples);
  int index = dm.time.local_state_index;
  dm.time.reserve(5);
  Double newfactor = dm.scale_factor * dm.time.scale_factor * dm.time.sampling_interval;
  DoubleComplex xvalue,xfactor;
  if (dm.time.local_states[index] != newfactor) {
    dm.time.local_states[index] = newfactor;
    xfactor =	exp(DoubleComplex(0,newfactor));
    dm.time.local_states[index+1] = real(xfactor);
    dm.time.local_states[index+2] = imag(xfactor);
  } else {
    xfactor = DoubleComplex(dm.time.local_states[index+1],
			    dm.time.local_states[index+2]);
  }
  if (dm.time.local_states[index+3] == 0.0) {
    dm.time.local_states[index+3] = 1.0;
  }
  xvalue = DoubleComplex(dm.time.local_states[index+3],
			 dm.time.local_states[index+4]);
  //xvalue = exp(DoubleComplex(0,newfactor * dm.time.sample_rate * dm.time.time));
  for (int i = 0; i <  dm.time.num_samples; ++i) {
    xvalue *= xfactor;
    result[i] = 0.97 * real(xvalue);
  }
  dm.time.local_states[index+3] = real(xvalue);
  dm.time.local_states[index+4] = imag(xvalue);
  dm.time.local_state_index += 5;
  return result;
}

inline osw::vector<Float> sin (const osw::Time &time) {
  return sin(time * 1.0f);
}

inline osw::vector<Float> cos (const osw::Time &time) {
  return cos(time * 1.0f);
}  

inline osw::vector<Float> exp (const osw::Time &time) {
  return exp(time * 1.0f);
}


namespace osw {
  /*****************/

  // helper functions
  extern _oswexport void StartSampleRatePropagation(State<Time> &clock);
  extern _oswexport void StartBufferSizePropagation(State<Time> &clock);
  extern _oswexport void _TimeExplicit(Inlet<Time> *atimeInlet);
  extern _oswexport void _TimeImplicit(Inlet<Time> *atimeInlet);
  

  // Special Inlet for Implicit Time Machines

  template<> class _oswexport Inlet<Time> : public State<Time> {
    
  public:
    
    Inlet<Time> (const string &aname = "", Transform *atransform = NULL, 
		 const string &adesc = "", const Time &aval = Time(0.0)) :
      State<Time> (aname,atransform,adesc,aval) {
      if (atransform != NULL) {
	atransform->AddInlet(this);
      }
      xcast = NULL;
      Disconnected(NULL);
    }
    
    ~Inlet<Time>() {
      Transform *atransform = dynamic_cast<Transform *>(GetContainer());
      if (atransform != NULL) {
	atransform->RemoveInlet(this);
      }
      _TimeExplicit(this);
    }
    
    // the assignment operator cannot be inherited, so it has
    // to be redefined here
    
    Inlet<Time>& operator = (const Time& aval) {
      SafeAssign(aval);
      return *this;
    }
    
    Inlet<Time>& operator = (const State<Time> &toAssign) {
      Assign(toAssign);
      return *this;
    }
    
    bool IsInlet() const {
      return true;
    }
    
    // essentially to support dynamic casting
    virtual void BackDisconnect () {
      if (xcast != NULL) {
	delete xcast;
	delete this;
      }
    }
    
    virtual void Connected(BaseOutlet *aconnection) {
      _TimeExplicit(this);
    }
    virtual void Disconnected(BaseOutlet *aoldConnection) {
      _TimeImplicit(this);
    }

  protected:
    
    BaseActivation *xcast;
    
    void AddImplicitCast (BaseActivation *acast) {
      xcast = acast;
    }
    
    friend class BaseOutlet;
  };
  

  // The all-important state variable for time
#if defined(_OSWOSX) && defined(_WINDOWS)
  __declspec(dllimport) State<Time> MainClock;
#else
  extern _oswexport State<Time> MainClock;
#endif

} // namespace osw  

#endif // _OSWTIME_H




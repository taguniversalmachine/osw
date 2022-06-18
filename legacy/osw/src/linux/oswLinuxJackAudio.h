/* 
Copyright (c) 2001-2002 Amar Chaudhary. All rights reserved.
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
  Audio driver for Jack Audio Server
*/

#ifdef JACK

#include "oswAudio.h"
#include <queue>
#include <deque>
#include <unistd.h>
#include <fcntl.h>
#include <sched.h>

extern "C" {
#include <jack/jack.h>
}

#define MAX_CHANNELS 64

namespace osw {

  jack_client_t* xjc;
  jack_port_t* jack_inputs[MAX_CHANNELS];
  jack_port_t* jack_outputs[MAX_CHANNELS];
  
  int            input_pipe[2];                   // socket pair for input port data
  int            output_pipe[2];                  // socket pair for output port data

  float*         input_buffer_0[MAX_CHANNELS];                  // buffer used to transmit audio via slink...
  float*         output_buffer_0[MAX_CHANNELS];                 // buffer used to send audio via slink...
  float*         input_buffer_1[MAX_CHANNELS];                  // buffer used to transmit audio via slink...
  float*         output_buffer_1[MAX_CHANNELS];                 // buffer used to send audio via slink...

  float *jack_input_buffer[MAX_CHANNELS];
  float *jack_output_buffer[MAX_CHANNELS];
    
  int jack_inports = 0;
  int jack_outports = 0;

  int  osw_jack_detached_process_callback(jack_nframes_t n, void* arg);
  int  osw_jack_inline_process_callback(jack_nframes_t n, void* arg);
  int  osw_jack_buffer_size_callback(jack_nframes_t n, void* arg);

  class _oswexport JackDriver : public AudioDriver {

  public:
    
    State<Integer> trigger;
    
    JackDriver (const string &aname, Container *acontainer);
    ~JackDriver ();
    
    virtual int WriteSamples (int achannel, float *asamples, int NumSamples);
    virtual int FlushSamples();
    virtual void Start();
    virtual void Stop ();
    virtual int  SamplesInBuffer();
    virtual void StartMainLoop ();
    virtual int ReadSamples (int achannel, float *samples, int NumSamples);
    virtual int GetInputSamples ();

  protected:
    
    virtual int AdjustToBufferSize ();
    virtual int AdjustToSampleRate ();
    
  private:

    int xoldBufferSize;  

  };

}

#endif JACK

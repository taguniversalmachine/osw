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
  oswSGIAudio.cpp
  Implementation of SGI audio driver for OSW
 */

#include "oswSGIAudio.h"
#include "oswTime.h"
#include "oswDelete.h"
#include "oswSched.h"
#include <errno.h>

using namespace std;



namespace osw {

  /******************/
 
  SGIAudioDriver::SGIAudioDriver (const string &aname,
			osw::Container *acontainer) 
    : AudioDriver (aname,acontainer) {

    ALconfig output_alc = alNewConfig();
    if (output_alc == NULL) {
      throw ("Unable to create audio ALconfig structure for output");
    }

    ALconfig input_alc = alNewConfig ();
    if (input_alc == NULL) {
      throw ("Unable to create audio ALconfig structure for input");
    }

    alSetSampFmt(output_alc,AL_SAMPFMT_FLOAT);
    alSetSampFmt(input_alc,AL_SAMPFMT_FLOAT);
    alSetFloatMax(output_alc,1.0);
    alSetFloatMax(input_alc,1.0);

    ALpv pv;
    pv.param = AL_CHANNELS;
    if (alGetParams(AL_DEFAULT_OUTPUT,&pv,1) < 0) {
      Error(PathName()+": "+alGetErrorString(oserror()));
      Error(PathName()+": setting 2 output channels");
      output_channels = 2;
    } else {
      cout << PathName() << ": " << pv.value.i << " output channels\n";
      output_channels = pv.value.i;
    }
    if (alGetParams(AL_DEFAULT_INPUT,&pv,1) < 0) {
      Error(PathName()+": "+alGetErrorString(oserror()));
      Error(PathName()+": setting 2 input channels");
      input_channels = 2;
    } else {
      cout << PathName() << ": " << pv.value.i << " input channels\n";
      input_channels = pv.value.i;
    }
    alSetChannels(output_alc,output_channels);
    alSetChannels(input_alc,input_channels);
    
    hiWater = 512;
    loWater = 256;
    xqsize = ((int)hiWater - (int)loWater);
    if (xqsize <= 0) {
      xqsize = 32;
    }
    if (alSetQueueSize(output_alc,(int)hiWater) < 0) {
      alFreeConfig(output_alc);
      alFreeConfig(input_alc);
      throw(PathName()+": unable to set output queue size");
    }
    if (alSetQueueSize(input_alc,(int)hiWater) < 0) {
      alFreeConfig(output_alc);
      alFreeConfig(input_alc);
      throw(PathName()+": unable to set input queue size");
    }


    xalpout = alOpenPort("output","w",output_alc);
    if (xalpout == NULL) {
      throw ("Unable to open output audio port");
    }
    xalpin = alOpenPort("input","r",input_alc);
    if (xalpin == NULL) {
      throw ("Unable to open input audio port");
    }
    alSetFillPoint(xalpout,xqsize);
    xhasInput = true;

    alFreeConfig(output_alc);
    alFreeConfig(input_alc);

    xfd = alGetFD(xalpout);

    xbuffer = xinputBuffer = NULL;
    xoldBufferSize = 0;

    AdjustToBufferSize();
    AdjustToSampleRate();

    RegisterInputChannels();
    RegisterOutputChannels();
    
    return;

  }


  SGIAudioDriver::~SGIAudioDriver () {

    alClosePort(xalpout);
    alClosePort(xalpin);
  }

  /********************/

  int SGIAudioDriver::AdjustToBufferSize () {

    if (xoldBufferSize == bufferSize) {
      cout << PathName() << ": keeping buffers\n";
      return bufferSize;
    }

    if (xbuffer != NULL) {
      delete [] xbuffer;
    }
    xbuffer = new float [(int)bufferSize * output_channels];
  
    if (xinputBuffer != NULL) {
      delete [] xinputBuffer;
    }
    xinputBuffer = new float [(int)bufferSize * input_channels];

    xoldBufferSize = bufferSize;  



    return bufferSize;
  }

  int SGIAudioDriver::AdjustToSampleRate () {
    ALpv pv;

    pv.param = AL_OUTPUT_RATE;
    switch (sampleRate) {
    case 8000 : pv.value.i = AL_RATE_8000; break;
    case 11025 : pv.value.i = AL_RATE_11025; break;
    case 16000 : pv.value.i = AL_RATE_16000; break;
    case 22050 : pv.value.i = AL_RATE_22050; break;
    case 32000 : pv.value.i = AL_RATE_32000; break;
    case 44100 : pv.value.i = AL_RATE_44100; break;
    case 48000 : pv.value.i = AL_RATE_48000; break;
    default : Error(PathName()+": could not set sample rate");
      return 0;
    }
    int result;
    if ((result = alSetParams(AL_DEFAULT_OUTPUT, &pv, 1)) < 0) {
      Error(string("could not set output sample rate: ")+alGetErrorString(oserror()));
      return 0;
    }
    pv.param = AL_INPUT_RATE;
    if ((result = alSetParams(AL_DEFAULT_INPUT, &pv, 1)) < 0) {
      Error(string("could not set input sample rate: ")+alGetErrorString(oserror()));
      return 0;
    }
    cout << PathName() << ": SampleRate is now " << sampleRate << endl;
    return 1;
  }

  int SGIAudioDriver::SamplesInBuffer() {
    // this function is not used by the SGI audio driver
    return -1;
  }

  void SGIAudioDriver::WaitUntilReady () {
    //FD_ZERO(&read_fds);
    FD_ZERO(&write_fds);
    FD_SET(xfd,&write_fds);
    int nfds = xfd+1;
    //alert("Waiting...");
    int r = select(nfds,(fd_set *)0,&write_fds,(fd_set *)0,(timeval *)0);
    if (r < 0) {
      Error("A select error occured");
    }
  }

  int SGIAudioDriver::WriteSamples (int achannel, float *samples, int NumSamples) {

    if (achannel >= output_channels || achannel < 0) {
      // error, return for now
      return DAC_WRITE_ERROR;
    }
    int i,j;

    for (i = 0, j = achannel; i < NumSamples; ++i, j += output_channels) {
	xbuffer[j] += samples[i];
    }

    return DAC_WRITE_OK;
  }


  int SGIAudioDriver::FlushSamples() {

    //alert("Flushing...");
    alWriteFrames(xalpout,xbuffer,bufferSize);
    alSetFillPoint(xalpout,xqsize);
    memset(xbuffer,0,bufferSize * sizeof(float) * output_channels);
  
    return DAC_WRITE_OK;
  }


  void SGIAudioDriver::Start () {

  }

  void SGIAudioDriver::Stop () {
    //fclose(foofile);
  }

  void SGIAudioDriver::StartMainLoop () {
    

    Start();
    cerr << "Starting dac " << PathName () << endl;
    
    owns_main_clock = true; // for the moment

    if (owns_main_clock) {
      MainClock = MainClock;
    }
    clock = clock;
  }
    
  int SGIAudioDriver::ReadSamples (int achannel, float *samples, int NumSamples) {
    int i,j;

    for (i = 0, j = achannel; i < NumSamples; ++i, j += input_channels) {
      samples[i] = xinputBuffer[j];
    }
      
    return DAC_WRITE_OK;
  }
  
  int SGIAudioDriver::GetInputSamples () {
    //alert("Reading...");
    alReadFrames(xalpin,xinputBuffer,bufferSize);
    return DAC_WRITE_OK;
  }
  /********************/

}

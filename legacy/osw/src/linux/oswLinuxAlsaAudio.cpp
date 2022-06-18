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
     
     Andrew W. Schmeder <andy@a2hd.com>
  
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
  oswLinuxALSAAudio.cpp
  Implementation of Linux ALSA/Lite audio driver for OSW

  Original version by Amar
  Rewritten by Andy (Sept. 2003)
 */

#ifdef ALSA

#include "oswLinuxAlsaAudio.h"
#include "oswTime.h"
#include "oswClock.h"
#include "oswDelete.h"
#include "oswSched.h"
#include "oswSelect.h"

using namespace std;

#define ALSA_DRIVER_DEVICE          "hw:0,0"

namespace osw {

  /******************/
 
  ALSADriver::ALSADriver (const string &aname,
			  osw::Container *acontainer) 
    : AudioDriver (aname,acontainer) {
    
    int err;
    int nchannels;
    snd_pcm_hw_params_t *hwparams = NULL;
    outhandle = inhandle = NULL;  // snd_pcm_t* 

    cout << "Using ALSA device " << GetDriverOption("device", ALSA_DRIVER_DEVICE) << endl;

    // This one we do in non-blocking mode since we don't want to wait for output data to flush...
    // This also means that we are depending on the input file handle to trigger the read/write cycle
    // i.e. this driver can't be used in half-duplex write-only mode as written.
    err = snd_pcm_open(&outhandle, 
		       GetDriverOption("device", ALSA_DRIVER_DEVICE).c_str(),
		       SND_PCM_STREAM_PLAYBACK,
		       0);
    if (err < 0) {
      Error(snd_strerror(err));
      goto bad;
    }
    // Init hardware, etc
    if (!SetOutputParameters()) {
      goto bad;
    }
    cout << PathName() << ": Device has " << output_channels << " output channels." << endl;
    //xhasOutput = true;

    // Now for input, we use blocking and get a file descriptor...
    err = snd_pcm_open(&inhandle, 
		       GetDriverOption("device", ALSA_DRIVER_DEVICE).c_str(),
		       SND_PCM_STREAM_CAPTURE, 
		       0);
    if (err < 0) {
      Error(snd_strerror(err));
      goto bad;
    }
    // Init hardware, etc
    if (!SetInputParameters()) {
      goto bad;
    }
    cout << PathName() << ": Device has " << input_channels << " input channels." << endl;
    xhasInput = true;
    if(err = snd_pcm_poll_descriptors(inhandle, &input_fd, 1) < 0) {
      Error(snd_strerror(err));
      Alert("Could not get poll descriptor for input...");
      goto bad;
    }

    RegisterInputChannels();
    RegisterOutputChannels();

    return;

    bad:

    throw(osw::Exception("ALSA driver initialization failed."));
  }

  ALSADriver::~ALSADriver () {
    if (outhandle) {
      snd_pcm_close(outhandle);
    }
    if (inhandle) {
      snd_pcm_close(inhandle);
    }
  }

  /********************/

  bool ALSADriver::SetOutputParameters () {
    int err;
    snd_pcm_hw_params_t *hwparams = NULL;
    snd_pcm_sw_params_t* sw_params = NULL;
    unsigned int nchannels;
    unsigned int  srateAsUInt = sampleRate;
    int periods = hiWater / bufferSize;
    snd_pcm_uframes_t uframes;
    int frag_size = bufferSize;

    snd_pcm_hw_params_malloc(&hwparams);

    if ((err = snd_pcm_hw_params_any(outhandle, hwparams)) < 0) {
      Error(snd_strerror(err));
      Error("Can not configure this PCM device.");
      goto bad;
    }
    
    // basic parameters
    if ((err = snd_pcm_hw_params_set_access(outhandle, hwparams, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
      snd_strerror(err);
      Error("Error setting access.");
      goto bad;
    }
    
    err = snd_pcm_hw_params_set_format(outhandle, hwparams, SND_PCM_FORMAT_S32_LE);
    if (err < 0) {
      err = snd_pcm_hw_params_set_format(outhandle, hwparams,SND_PCM_FORMAT_S16_LE);
      if (err < 0) {
	Error(snd_strerror(err));
	goto bad;
      }
      alsa_samplewidth = 2;
    } else {
      alsa_samplewidth = 4;
    }
    bitsPerSample = alsa_samplewidth * 8;
    cout << PathName() << ": " << bitsPerSample << " bits per sample." << endl;
    //snd_pcm_hw_params_set_subformat(outhandle, hwparams, SND_PCM_SUBFORMAT_STD);
   err = snd_pcm_hw_params_get_channels_max(hwparams,&nchannels);
    if (err < 0) {
      nchannels = 0;
    }
    output_channels = nchannels;
    xblockAlign = nchannels * alsa_samplewidth;
    if ((err = snd_pcm_hw_params_set_channels(outhandle,hwparams,nchannels)) < 0) {
      Error(snd_strerror(err));
      goto bad;
    }
    
    // sample rate
    if ((err = snd_pcm_hw_params_set_rate_min(outhandle,hwparams,&srateAsUInt,0)) < 0) {
      Error(snd_strerror(err));
      goto bad;
    }

    // buffer size / period size (i.e., hi-water mark, or target latency)
    if ((err = snd_pcm_hw_params_set_periods(outhandle,hwparams,periods,0)) < 0) {
      Error(snd_strerror(err));
      goto bad;
    }
    uframes = bufferSize*periods;
    if ((err = snd_pcm_hw_params_set_buffer_size_near(outhandle,hwparams,&uframes)) < 0) {
      Error(snd_strerror(err));
      goto bad;
    }
    if ((err = snd_pcm_hw_params(outhandle, hwparams)) < 0) {
      Error(snd_strerror(err));
      goto bad;
    }
    if (hwparams) {
      snd_pcm_hw_params_free(hwparams);
    }
    
    err = snd_pcm_sw_params_malloc(&sw_params);
    if (err < 0) {
      Error(snd_strerror(err));
      goto bad;
    }
    err = snd_pcm_sw_params_current(outhandle, sw_params);
    if (err < 0) {
      Error(snd_strerror(err));
      goto bad;
    }
    err = snd_pcm_sw_params_set_start_mode(outhandle, sw_params,
                                           SND_PCM_START_EXPLICIT);
    if (err < 0) {
      Error(snd_strerror(err));
      goto bad;
    }
    err = snd_pcm_sw_params_set_xrun_mode(outhandle, sw_params,
                                          SND_PCM_XRUN_NONE);
    if (err < 0) {
      Error(snd_strerror(err));
      goto bad;
    }
    err = snd_pcm_sw_params_set_avail_min(outhandle, sw_params,
                                          frag_size);
    if (err < 0) {
      Error(snd_strerror(err));
      goto bad;
    }
    err = snd_pcm_sw_params(outhandle, sw_params);
    if (err < 0) {
      Error(snd_strerror(err));
      goto bad;
    }
    
    snd_pcm_sw_params_free(sw_params);
#if OSW_DEBUG_MESSAGE
    {
      snd_output_t *out;
      snd_output_stdio_attach(&out,stderr,0);
      snd_pcm_dump_sw_setup(outhandle,out);
    }
#endif

    return true;
  bad:
    if (hwparams) {
      snd_pcm_hw_params_free(hwparams);
    }
    if (sw_params) {
      snd_pcm_sw_params_free(sw_params);
    }
    return false;
  }

  bool ALSADriver::SetInputParameters () {
    int err;
    snd_pcm_hw_params_t *hwparams = NULL;
    snd_pcm_sw_params_t *sw_params = NULL;
    unsigned int nchannels;
    unsigned int  srateAsUInt = sampleRate;
    unsigned int periods = hiWater / bufferSize;
    snd_pcm_uframes_t uframes;
    unsigned int frag_size = bufferSize;

    snd_pcm_hw_params_malloc(&hwparams);

    if ((err = snd_pcm_hw_params_any(inhandle, hwparams)) < 0) {
      Error(snd_strerror(err));
      Error("Can not configure this PCM Capture device.");
      goto bad;
    }    
    
    // basic parameters
    if ((err = snd_pcm_hw_params_set_access(inhandle, hwparams, SND_PCM_ACCESS_RW_INTERLEAVED))	< 0) {
      Error(snd_strerror(err));
      Error("Error setting access.");
      goto bad;
    }
    
    err = snd_pcm_hw_params_set_format(inhandle, hwparams, SND_PCM_FORMAT_S32_LE);
    if (err < 0) {
      err = snd_pcm_hw_params_set_format(inhandle, hwparams,SND_PCM_FORMAT_S16_LE);
      if (err < 0) {
	Error(snd_strerror(err));
	goto bad;
      }
      alsa_samplewidth = 2;
    } else {
      alsa_samplewidth = 4;
    }
    bitsPerSample = alsa_samplewidth * 8;
    cout << PathName() << ": " << bitsPerSample << " bits per input sample." << endl;
    //snd_pcm_hw_params_set_subformat(inhandle, hwparams, SND_PCM_SUBFORMAT_STD);
    err = snd_pcm_hw_params_get_channels_max(hwparams,&nchannels);
    if (err < 0) {
      nchannels = 0;
    }
    input_channels = nchannels; // nchannels for now...
    xinputBlockAlign = nchannels * alsa_samplewidth;
    if ((err = snd_pcm_hw_params_set_channels(inhandle,hwparams,nchannels)) < 0) {
      Error(snd_strerror(err));
      goto bad;
    }
    
    cout << PathName() << ": " << nchannels << " input channels." << endl;

    // sample rate
    if ((err = snd_pcm_hw_params_set_rate_min(inhandle,hwparams,&srateAsUInt,0)) < 0) {
      Error(snd_strerror(err));
      goto bad;
    }

    cout << PathName() << ": " << srateAsUInt << " input sample rate." << endl;

    uframes = bufferSize;
    
    // period size is the OSW buffer size
    if ((err = snd_pcm_hw_params_set_period_size_near(inhandle,hwparams,
                                                      &uframes,0)) < 0) {
      Error(snd_strerror(err));
      goto bad;
    }

    cout << PathName() << ": " << uframes << " period size." << endl;

    // buffer size / period size (i.e., hi-water mark, or target latency)
    if ((err = snd_pcm_hw_params_set_periods_near(inhandle,hwparams,&periods,0)) < 0) {
      Error(snd_strerror(err));
      goto bad;
    }
    
    cout << PathName() << ": " << periods << " input periods." << endl;

    uframes = bufferSize * periods;
    if ((err = snd_pcm_hw_params_set_buffer_size_near(inhandle,hwparams,&uframes)) < 0) {
      Error(snd_strerror(err));
      goto bad;
    }

    if (uframes / bufferSize > periods) {
      periods = uframes / bufferSize;
      cout << PathName() << ": " << periods << " revised input periods." << endl;
      if ((err = snd_pcm_hw_params_set_periods_near(inhandle,hwparams,&periods,0)) < 0) {
        Error(snd_strerror(err));
        goto bad;
      }
    }
    cout << PathName() << ": " << (bufferSize*periods) << " total buffer size." << endl;

    if ((err = snd_pcm_hw_params(inhandle, hwparams)) < 0) {
      Error(snd_strerror(err));
      goto bad;
    }
    if (hwparams) {
      snd_pcm_hw_params_free(hwparams);
    }

    err = snd_pcm_sw_params_malloc(&sw_params);
    if (err < 0) {
      Error(snd_strerror(err));
      goto bad;
    }
    err = snd_pcm_sw_params_current(inhandle, sw_params);
    if (err < 0) {
      Error(snd_strerror(err));
      goto bad;
    }
    err = snd_pcm_sw_params_set_start_mode(inhandle, sw_params,
                                           SND_PCM_START_EXPLICIT);
    if (err < 0) {
      Error(snd_strerror(err));
      goto bad;
    }
    err = snd_pcm_sw_params_set_xrun_mode(inhandle, sw_params,
                                          SND_PCM_XRUN_NONE);
    if (err < 0) {
      Error(snd_strerror(err));
      goto bad;
    }
    err = snd_pcm_sw_params_set_avail_min(inhandle, sw_params,
                                          frag_size);
    if (err < 0) {
      Error(snd_strerror(err));
      goto bad;
    }
    err = snd_pcm_sw_params(inhandle, sw_params);
    if (err < 0) {
      Error(snd_strerror(err));
      goto bad;
    }
    
    snd_pcm_sw_params_free(sw_params);
#ifdef OSW_DEBUG_MESSAGE
    {
      snd_output_t *out;
      snd_output_stdio_attach(&out,stderr,0);
      snd_pcm_dump_sw_setup(inhandle,out);
    }
#endif

    return true;
  bad:
    if (hwparams) {
      snd_pcm_hw_params_free(hwparams);
    }
    if (sw_params) {
      snd_pcm_sw_params_free(sw_params);
    }
    return false;
  }

  int ALSADriver::AdjustToBufferSize () {
    if (xoldBufferSize == bufferSize) {
      Alert("keeping buffers");
      return bufferSize;
    }
    if (xbuffer != NULL) {
      delete [] xbuffer;
    }
    xbuffer = new byte [bufferSize * xblockAlign];
    xinputBuffer = new byte [bufferSize * xinputBlockAlign];

    char buffer[64];
    if (SetInputParameters() && SetOutputParameters()) {
      sprintf(buffer,"Buffer size is %d.",(int) bufferSize);
      Alert(buffer);    
      sprintf(buffer,"Hi-water mark is %d.",(int) hiWater);
      Alert(buffer);
      return bufferSize;
    }
    return 0;
  }

  int ALSADriver::AdjustToSampleRate () {    
    char buffer[64];
    if (SetInputParameters() && SetOutputParameters()) {
      sprintf(buffer,"Sample rate is now %d Hz.",(int) sampleRate);
      Alert(buffer);
      return 1;
    }
    return 0;
  }

  int ALSADriver::AdjustToHiWater () {
    char buffer[64];
    if (SetOutputParameters() && SetInputParameters()) {
      sprintf(buffer,"Sample rate is now %d Hz.",(int) sampleRate);
      Alert(buffer);
      return 1;
    }
    return 0;
  }

  void ALSADriver::DoInputOutputThing() {
    ProfileOff(this);
    GetInputSamples();
    FlushSamples();
    ProfileOn(this);
    ActivateInputTransforms();
  }

  int ALSADriver::SamplesInBuffer() {
    // Not used by ALSA driver
    return 0;
  }

  extern void DoOneEvent();

  void ALSADriver::WaitUntilReady () {
    Alert("Boo!");
    return;
  }

  void ALSADriver::StartMainLoop () {
    Start();
    owns_main_clock = true; // for the moment
    if (owns_main_clock) {
      MainClock = MainClock;
    }
    clock = clock;
  }

  int ALSADriver::WriteSamples (int achannel, float *samples, int NumSamples) {

    if (achannel >= output_channels || achannel < 0) {
      // error, return for now
      return DAC_WRITE_ERROR;
    }
  
    int i,j;
    switch (int(bitsPerSample)) {
    case 16 :
      for (i = 0, j = achannel * 2; i < NumSamples; ++i, j += output_channels * 2) {
	short sampleVal = short(32760.0f * samples[i]);
	xbuffer[j] += sampleVal & 0xFF;
	xbuffer[j+1] += (sampleVal >> 8) & 0xFF;
      }
      break;
    case 32 :
      for (i = 0, j = achannel * 4; i < NumSamples; ++i, j += output_channels * 4) {
	long sampleVal = xbuffer[j] 
	  + (xbuffer[j+1] << 8)
	  + (xbuffer[j+2] << 16)
	  + (xbuffer[j+3] << 24)
	  + long(2147483648.0 * samples[i]);
	xbuffer[j] = sampleVal & 0xFF;
	xbuffer[j+1] = (sampleVal >> 8) & 0xFF;
	xbuffer[j+2] = (sampleVal >> 16) & 0xFF;
	xbuffer[j+3] = (sampleVal >> 24) & 0xFF;

      }
      break;
    case 24 :
      for (i = 0, j = achannel * 3; i < NumSamples; ++i, j += output_channels * 3) {
	long sampleVal = xbuffer[j] 
	  + (xbuffer[j+1] << 8)
	  + (xbuffer[j+2] << 16)
	  + long(8388608.0 * samples[i]);
	xbuffer[j] = sampleVal & 0xFF;
	xbuffer[j+1] = (sampleVal >> 8) & 0xFF;
	xbuffer[j+2] = (sampleVal >> 16) & 0xFF;
      }
      break;
    }

    return DAC_WRITE_OK;
  }
    
  int ALSADriver::ReadSamples (int achannel, float *samples, int NumSamples) {
    int i,j;
    
    switch (int(bitsPerSample)) {
    case 16 :
      for (i = 0, j = achannel * 2; i < NumSamples; ++i, j += input_channels * 2) {
	samples[i] = float(xinputBuffer[j] 
			   + ((signed char)xinputBuffer[j+1] << 8)) 
	  / 32768.0f;
      }
      
      break;
    case 32 :
      for (i = 0, j = achannel * 4; i < NumSamples; ++i, j += input_channels * 4) {
	samples[i] = float(xinputBuffer[j] 
			   + (xinputBuffer[j+1] << 8)
			   + (xinputBuffer[j+2] << 16)
			   + ((signed char)xinputBuffer[j+3] << 24))
	  / 2147483648.0f;
      }
      break;
    case 24 :
      for (i = 0, j = achannel * 3; i < NumSamples; ++i, j += input_channels * 3) {
	samples[i] = float(xinputBuffer[j] 
			   + (xinputBuffer[j+1] << 8)
			   + ((signed char)xinputBuffer[j+2] << 16))
	  / 8388608.0f;
      }
      break;
    }
    return DAC_WRITE_OK;
  }

  void ALSADriver::Start () {  
    int err;

    snd_pcm_status_malloc(&instatus);    
    snd_pcm_status_malloc(&outstatus);

    PrepareInput();
    PrepareOutput();

    // Clear buffer
    memset(xbuffer,0,bufferSize * xblockAlign);
    int periods = hiWater / bufferSize;

    if ((err = snd_pcm_start(inhandle)) < 0) {
      Error("Failed to start input.");
      Error(snd_strerror(err));
    }
    if ((err = snd_pcm_start(outhandle)) < 0) {
      Error("Failed to start output.");
      Error(snd_strerror(err));
    }

  }

  void ALSADriver::Stop () {
    snd_pcm_drop(outhandle);
  }

  int ALSADriver::FlushSamples() {
    int err;
    if ((err = snd_pcm_writei(outhandle,xbuffer,bufferSize)) != bufferSize) {
      if(err != -EPIPE) { // -EPIPE is a normal XRUN which is not a /big/ deal. 
	Alert("There is some kind of problem with the output handle.");
	Alert(snd_strerror(err));
      }
      snd_pcm_prepare(outhandle);
    }
    // Reset memory area...
    memset(xbuffer,0,bufferSize * xblockAlign);
    return DAC_WRITE_OK;
  }
  
  int ALSADriver::GetInputSamples () {
    int err;
    if ((err = snd_pcm_readi(inhandle,xinputBuffer,bufferSize)) < 0) {
      if (err != -EPIPE) { // don't make a printout on XRUN
	Alert("There is some kind of problem with the input handle...");
	Alert(snd_strerror(err));
      }
      snd_pcm_prepare(inhandle);
    }
    return DAC_WRITE_OK;
  }
 
  /********************/

  static osw::ALSADriver *ALSAdriver = NULL;

  static void ALSASetupProc (SelectEventManager &eventManager) {
    if (ALSAdriver->DriverIsOn()) {
      eventManager.AddRead((ALSAdriver->input_fd).fd);
    }
  }

  static void ALSADispatchProc (SelectEventManager &eventManager) {
    if(ALSAdriver->DriverIsOn() && eventManager.IsSetRead((ALSAdriver->input_fd).fd)) {
      ALSAdriver->AdvanceClock();
    }
  }

  struct ALSADeviceManager : public AudioDeviceManager {

    ALSADeviceManager ();
   
    bool Initialize () {
      cout << "Trying ALSA driver..." << endl;
      try {
	ALSAdriver = new ALSADriver ("audio/0",NULL);
      } catch (osw::Exception e) {
	cout << e.what() << endl;
	ALSAdriver = NULL;
	return false;
      }
      
      ALSAdriver->sampleRate = (int) GetDefaultSampleRate();
      ALSAdriver->bufferSize = GetDefaultBufferSize();
      AddSelectGroup(ALSASetupProc, ALSADispatchProc);
      return true;
    }

    bool Cleanup () {
      if (ALSAdriver) {
	delete ALSAdriver;
      }
      return true;
    }

    // @todo: ???
    int GetDriverNames(DriverNameList &driverNames) {
      //driverNames.push_back("hw:0");
      //return 1;
      return 0;
    }
    
    void PrintOptions() {
      cout << "    " << "device : Name of ALSA device to use, default=hw:0" << endl;
      //cout << "    " << "outchannels : Number of output channels, default=max" << endl;
      //cout << "    " << "inchannels : Number of input channels, defalut=max" << endl;
    }
    
  };

  ALSADeviceManager::ALSADeviceManager () : 
    AudioDeviceManager("alsa", PRIORITY_EXCLUSIVE) {
  }

  static ALSADeviceManager TheAlsaDeviceManager;

}
#endif // ALSA

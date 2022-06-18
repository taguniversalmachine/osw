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
  oswLinuxOSSAudio.cpp
  Implementation of Linux OSS/Lite audio driver for OSW
 */

#include "oswLinuxOSSAudio.h"
#include "oswTime.h"
#include "oswDelete.h"
#include "oswSched.h"
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <sched.h>
#include <sys/mman.h>
#include <linux/soundcard.h>
#ifdef ALSA
#include <sys/asoundlib.h>
#endif

using namespace std;

#define OSSDRIVER "/dev/dsp"
#define OSSADCDRIVER "/dev/dsp"
#define OSSADCDRIVER1 "/dev/dsp1"

#define BUFFERSIZE	64
#define SAMPLERATE	44100


namespace osw {

  /******************/
 
  OSSDriver::OSSDriver (const string &aname,
			osw::Container *acontainer) 
    : AudioDriver (aname,acontainer) {

    int caps, param;

    xifd = 0;
    if ((xfd = open(OSSDRIVER,O_RDWR)) == -1) {
      if ((xfd = open(OSSDRIVER,O_WRONLY)) == -1) {
	Error (" could not open audio output.");
	return;
      }
      
      if ((xifd = open(OSSADCDRIVER,O_RDONLY)) == -1) {
	if ((xifd = open(OSSADCDRIVER1,O_RDONLY)) == -1) {
	  Error (" could not open audio input.");
	}
      }
    } else {
      // if O_RDWR worked, then the same handle can both input and output
      xifd = xfd;
    }
      
    xhasInput = (xifd) ? true : false;

    if (ioctl(xfd,SNDCTL_DSP_GETCAPS,&caps) == -1) {
     Error (" could not get device capabilities.");      
    }	  

    for (param = 16; param > 0; --param) {
      if (ioctl(xfd,SNDCTL_DSP_CHANNELS,&param) != -1) {
	break;
      }
    }

    output_channels = param;
    char buffer[64];
    sprintf (buffer,"%d output_channels.",(int) output_channels);
    Alert(buffer);

    input_channels = 0;
    if (xhasInput) {
      if (xifd != xfd) {
	for (param = 16; param > 0; --param) {
	  if (ioctl(xifd,SNDCTL_DSP_CHANNELS,&param) != -1) {
	    break;
	  }
	}
      }
      input_channels = param;
      sprintf(buffer,"%d input channels.",(int) input_channels);
      Alert(buffer);
    }      

    // type casts deal with a problem in gcc 2.96
    xblockAlign = (int) bitsPerSample / 8 * (int) output_channels;
    xinputBlockAlign = (int) bitsPerSample / 8 * (int) input_channels;
    sprintf(buffer,"%d bytes per sample frame.",xblockAlign);
    Alert(buffer);
    xbuffer = xinputBuffer = NULL;
    
    
    //bufferSize = bufferSize; // trigger the event to set the fragment size

    switch ((int)bitsPerSample) {
    case 8:
      param = AFMT_S8; break;
    case 16:
      param = AFMT_S16_LE; break;
    default:
      Error ("desired sample resolution is not supported under OSS.  Sorry!");
      goto bad;
    }

    xoldBufferSize = 0;
    AdjustToBufferSize();

    if (ioctl(xfd,SNDCTL_DSP_SETFMT,&param) == -1) {
      Error ("could not set output sample resolution");
    }

    if (ioctl(xifd,SNDCTL_DSP_SETFMT,&param) == -1) {
      Error ("could not set input sample resolution");
    }

    //sampleRate = sampleRate;
    AdjustToSampleRate();

    RegisterInputChannels();
    RegisterOutputChannels();
    
    return;

    bad:
    
    if (close(xfd) == -1) {
      Error ("could not close audio output device.  What's up with that?");
    }
    throw(osw::Exception("could not create audio device"));
  }


  OSSDriver::~OSSDriver () {

    if (close(xfd) == -1) {
      Error ("could not close audio output device.  What's up with that?");
    }
    if (xifd != xfd) {
      if (close(xifd) == -1) {
	Error ("could not close audio input device.  What's up with that?");
      }
    }
  }

  /********************/

  int OSSDriver::AdjustToBufferSize () {

    if (xoldBufferSize == bufferSize) {
      Alert("keeping buffers");
      return bufferSize;
    }

    if (xbuffer != NULL) {
      delete [] xbuffer;
    }
    xbuffer = new byte [bufferSize * xblockAlign];
  
    if (xinputBuffer != NULL) {
      delete [] xinputBuffer;
    }
    xinputBuffer = new byte [bufferSize * xinputBlockAlign];

    // take the base-2 log of bufferSize

    int logified = bufferSize;
    int logBufferSize;
    for (logBufferSize = 0; logified > 1; logified >>= 1, ++logBufferSize);

    char buffer[64];
    sprintf(buffer,"buffer 2^%d samples (%d bytes).",logBufferSize,
	    bufferSize*xblockAlign);
    Alert(buffer);
    // that pesky gcc 2.96 error again!
    int numfrags = (int) hiWater / (int) bufferSize * xblockAlign;
    int param = (numfrags << 16) + logBufferSize;
    if (ioctl(xfd,SNDCTL_DSP_SETFRAGMENT,&param) == -1) {
      Error ("could not set new output buffer size.");
      return 0;
    }

    if (ioctl(xifd,SNDCTL_DSP_SETFRAGMENT,&param) == -1) {
      Error ("could not set new input buffer size.");
      return 0;
    }

    if (ioctl(xfd,SNDCTL_DSP_RESET) == -1) {
      Error ("could not reset output device.");
    }

    if (ioctl(xifd,SNDCTL_DSP_RESET) == -1) {
      Error ("could not reset input device.");
    }

    xoldBufferSize = bufferSize;  
    return bufferSize;
  }

  int OSSDriver::AdjustToSampleRate () {
    
    int param = sampleRate;

    if (ioctl(xfd,SNDCTL_DSP_SPEED,&param) == -1) {
      Error ("could not set output sample rate.");
      return 0;
    }

    if (ioctl(xifd,SNDCTL_DSP_SPEED,&param) == -1) {
      Error ("could not set input sample rate.");
      return 0;
    }
  
    char buffer[32];
    sprintf(buffer,"Sample rate is now %d Hz.",(int) sampleRate);
    Alert(buffer);
    return 1;
  }

  int OSSDriver::SamplesInBuffer() {

    count_info  info;

    ioctl(xfd,SNDCTL_DSP_GETOPTR,&info);
  
    xsamplesInDriver -= (info.bytes - xoldPos) / xblockAlign;
    xoldPos = info.bytes;

    //cout << xoldPos << endl;
  
    //cout << xsamplesInDriver << ' ' << xcurrentBuffer << endl;
    return xsamplesInDriver;
  }

  int OSSDriver::WriteSamples (int achannel, float *samples, int NumSamples) {

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


  int OSSDriver::FlushSamples() {

    //cout << ">> " << xcurrentBuffer << endl;
    write(xfd,xbuffer,bufferSize * xblockAlign);

    xsamplesInDriver += bufferSize;
    memset(xbuffer,0,bufferSize * xblockAlign);
  
    return DAC_WRITE_OK;
  }


  void OSSDriver::Start () {

  }

  void OSSDriver::Stop () {
    //fclose(foofile);
  }

  void OSSDriver::StartMainLoop () {
    

    Start();
    Alert("Starting dac");
    
    owns_main_clock = true; // for the moment

    if (owns_main_clock) {
      MainClock = MainClock;
    }
    clock = clock;
  }
    
  int OSSDriver::ReadSamples (int achannel, float *samples, int NumSamples) {
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
  
  int OSSDriver::GetInputSamples () {
    read(xifd,xinputBuffer,bufferSize * xinputBlockAlign);
    return DAC_WRITE_OK;
  }
  /********************/


  static osw::OSSDriver *ossdriver = NULL;

  struct OSSDeviceManager : public AudioDeviceManager {

    OSSDeviceManager () : AudioDeviceManager("oss",PRIORITY_EXCLUSIVE) {
    }
   
    bool Initialize () {
      Alert("Initializing OSS Drivers");
      try {
	ossdriver = new OSSDriver ("audio/0",NULL);
      } catch (osw::Exception e) {
	cout << e.what() << endl;
	ossdriver = NULL;
	return false;
      }
      
      ossdriver->sampleRate = (int) GetDefaultSampleRate();
      ossdriver->bufferSize = GetDefaultBufferSize();
      return true;
    }

    bool Cleanup () {
      if (ossdriver) {
	delete ossdriver;
      }
      return true;
    }

    int GetDriverNames (DriverNameList &driverNames) {
      driverNames.push_back("/dev/dsp");
      return 1;
    }

    
  };

  static OSSDeviceManager TheOSSDeviceManager;

}

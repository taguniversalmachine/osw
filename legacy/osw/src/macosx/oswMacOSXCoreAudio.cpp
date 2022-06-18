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
  oswMacOSXCoreAudio.cpp
  Implementation of OSW Drivers for Mac OSX Core Audio

  Amar Chaudhary
*/

#include "oswMacOSXCoreAudio.h"


namespace osw {


  static OSStatus IOProc (AudioDeviceID inDevice,
			  const AudioTimeStamp *inNow,
			  const AudioBufferList *inputData,
			  const AudioTimeStamp *inInputTime,
			  AudioBufferList *inOutputData,
			  const AudioTimeStamp *inOutputTime,
			  void *inClientData);
  
  CoreAudioDriver::CoreAudioDriver (const string &aname, Container *acontainer, AudioDeviceID aid) 
    : AudioDriver(aname,acontainer),
      xid(aid) {
    
    OSStatus				err = noErr;
    AudioStreamBasicDescription		format;
    UInt32                              bufferSizeLocal;
    UInt32				count = sizeof(format);

    err = AudioDeviceGetProperty(xid, 0, false, kAudioDevicePropertyStreamFormat, &count, &format);
    fprintf(stderr, "kAudioDevicePropertyStreamFormat %d\n", err);
    fprintf(stderr, "sampleRate %g\n", format.mSampleRate);
    fprintf(stderr, "mFormatFlags %08X\n", format.mFormatFlags);
    fprintf(stderr, "mBytesPerPacket %d\n", format.mBytesPerPacket);
    fprintf(stderr, "mFramesPerPacket %d\n", format.mFramesPerPacket);
    fprintf(stderr, "mChannelsPerFrame %d\n", format.mChannelsPerFrame);
    fprintf(stderr, "mBytesPerFrame %d\n", format.mBytesPerFrame);
    fprintf(stderr, "mBitsPerChannel %d\n", format.mBitsPerChannel);
    if (err != kAudioHardwareNoError) goto Bail;

    output_channels = format.mChannelsPerFrame;
    sampleRate.SetWithoutActivating(format.mSampleRate);

    count = sizeof(UInt32);
    err = AudioDeviceGetProperty(xid, 0, false, kAudioDevicePropertyBufferSize, &count, &bufferSizeLocal); 
    if (err != kAudioHardwareNoError) goto Bail;
    bufferSizeLocal /= format.mBytesPerFrame;
    cout << PathName() << ": Output Buffer Size is " << bufferSizeLocal << endl;
    bufferLocal = NULL;
    xoldBufferSize = 0;
    bufferSize = bufferSizeLocal;
    bufferSizeChangedExpr();

    count = sizeof(format);
    err = AudioDeviceGetProperty(xid, 0, true, kAudioDevicePropertyStreamFormat, &count, &format);
    fprintf(stderr, "kAudioDevicePropertyStreamFormat %d\n", err);
    fprintf(stderr, "sampleRate %g\n", format.mSampleRate);
    fprintf(stderr, "mFormatFlags %08X\n", format.mFormatFlags);
    fprintf(stderr, "mBytesPerPacket %d\n", format.mBytesPerPacket);
    fprintf(stderr, "mFramesPerPacket %d\n", format.mFramesPerPacket);
    fprintf(stderr, "mChannelsPerFrame %d\n", format.mChannelsPerFrame);
    fprintf(stderr, "mBytesPerFrame %d\n", format.mBytesPerFrame);
    fprintf(stderr, "mBitsPerChannel %d\n", format.mBitsPerChannel);
    if (err != kAudioHardwareNoError) goto Bail;

    AudioDeviceAddIOProc (xid,IOProc,static_cast<void *>(this));

    input_channels = format.mChannelsPerFrame;

    RegisterOutputChannels();
    RegisterInputChannels();

    xrealtime = false;

    return;
  Bail:
    throw (osw::Exception("Could not start audio device"));
  }

  CoreAudioDriver::~CoreAudioDriver () {
    Stop();
    AudioDeviceRemoveIOProc(xid,IOProc);
    if (bufferLocal) {
      delete bufferLocal;
    }
  }

  int CoreAudioDriver::AdjustToBufferSize () {
    
    if (xoldBufferSize == bufferSize) {
      Alert("keeping buffers");
      return bufferSize;
    }

    int i;
    
    UInt32 bufferSizeLocal = bufferSize * int(output_channels) * sizeof(Float);
    for (i = 0; i < output_channels; ++i) {
      int err = AudioDeviceSetProperty(xid,NULL,i,false,
				       kAudioDevicePropertyBufferSize,
				       sizeof(UInt32),
				       &bufferSizeLocal);
      if (err != kAudioHardwareNoError) {
	Error("Could not set new buffer size on output");
	break;
      }
    }

    bufferSizeLocal = bufferSize * int(input_channels) * sizeof(Float);
    for (i = 0; i < input_channels; ++i) {
      int err = AudioDeviceSetProperty(xid,NULL,i,true,
				       kAudioDevicePropertyBufferSize,
				       sizeof(UInt32),
				       &bufferSizeLocal);
      if (err != kAudioHardwareNoError) {
	Error("Could not set new buffer size on input");
	break;
      }
    }

    if (bufferLocal) {
      delete [] bufferLocal;
    } 
    bufferLocal = new float [bufferSize * (int) output_channels];
    xoldBufferSize = bufferSize;


    return 0;
  }

  int CoreAudioDriver::AdjustToSampleRate () {
    
    // real samplerate stuff
    return 0;
  }




  /*************/
  
  void CoreAudioDriver::Start () {
    AudioDeviceStart(xid,IOProc);
  }

  void CoreAudioDriver::Stop () {
    AudioDeviceStop(xid,IOProc);
  }

  int CoreAudioDriver::WriteSamples (int achannel, float *asamples, int NumSamples) {
    //xsemaphore.Acquire();
    //cout << NumSamples << endl;
    for (int i = achannel,j = 0; j < NumSamples; ++j, i+=output_channels) {
      bufferLocal[i] += asamples[j];
    }
    //xsemaphore.Release();
    return DAC_WRITE_OK;
  }

  int CoreAudioDriver::FlushSamples () {    
    //xsemaphore.Acquire();
    memcpy(xoutputBuffer->mData,bufferLocal,xoutputBuffer->mDataByteSize);
    memset(bufferLocal,0,sizeof(Float) * bufferSize * int(output_channels));
    //xsemaphore.Release();
    return DAC_WRITE_OK;
  }

  int  CoreAudioDriver::SamplesInBuffer() {
    return -1;
  }

  void CoreAudioDriver::StartMainLoop () {
    Start();
    Alert("Starting dac");
    
    owns_main_clock = true; // for the moment

    if (owns_main_clock) {
      MainClock = MainClock;
    }
    clock = clock;
  }

  int CoreAudioDriver::ReadSamples (int achannel, float *samples, int NumSamples) {
    for (int i = 0,j = achannel ; i < NumSamples; ++i,j += input_channels) {
      samples[i] = ((float *)(xinputBuffer->mData))[j];
    }
    return DAC_WRITE_OK;
  }

  int CoreAudioDriver::GetInputSamples () {
        return DAC_WRITE_OK;
  }


  void CoreAudioDriver::DoInputOutputThing () {
    // do nothing!
  }

  void CoreAudioDriver::DoCoreAudioThing () {
    FlushSamples();
    GetInputSamples();
    ActivateInputTransforms();

    AdvanceClock();
  }

  static OSStatus IOProc (AudioDeviceID inDevice,
			  const AudioTimeStamp *inNow,
			  const AudioBufferList *inputData,
			  const AudioTimeStamp *inInputTime,
			  AudioBufferList *inOutputData,
			  const AudioTimeStamp *inOutputTime,
			  void *inClientData) {
    CoreAudioDriver *driver = static_cast<CoreAudioDriver *>(inClientData);
    driver->SetInputBuffer((AudioBuffer *)inputData->mBuffers);
    driver->SetOutputBuffer((AudioBuffer *)inOutputData->mBuffers);
    driver->DoCoreAudioThing();
    //driver->WaitForEvent();
    return kAudioHardwareNoError;
  }
  

  /*******************/

  static AudioDeviceID *s_ids;
  static int s_theNumberDevices;
  static list<CoreAudioDriver *> s_coreAudioDrivers;

  struct CoreAudioDeviceManager : public AudioDeviceManager {

    explicit CoreAudioDeviceManager ();

    bool Initialize () {
      Alert("Initializing CoreAudio...");
      UInt32 theSize, theStatus, theNumberDevices;

      theStatus = AudioHardwareGetPropertyInfo(kAudioHardwarePropertyDevices,
					       &theSize,NULL);
      theNumberDevices = theSize / sizeof(AudioDeviceID);
      cout << theNumberDevices << " devices." << endl;

      s_ids = new AudioDeviceID [theNumberDevices];
      theStatus = AudioHardwareGetProperty(kAudioHardwarePropertyDevices,
					   &theSize,s_ids);
      for (int i = 0; i < theNumberDevices; ++i) {
	char buffer[32];
	sprintf (buffer,"audio/%d",i);
	CoreAudioDriver *audioDriver = new CoreAudioDriver(buffer,NULL,s_ids[i]); 
	// For now, just run whatever CoreAudio gives us

	//audioDriver->bufferSize = GetDefaultBufferSize();
	//audioDriver->sampleRate = GetDefaultSampleRate();

	s_coreAudioDrivers.push_back(audioDriver);
      }


      s_theNumberDevices = theNumberDevices;

      return true;
    }

    bool Cleanup () {
      delete [] s_ids;
      for (list<CoreAudioDriver *>::iterator p = s_coreAudioDrivers.begin();
	   p != s_coreAudioDrivers.end();
	   ++p) {
	delete *p;
      }      return true;
    }

    int GetDriverNames (DriverNameList &driverNames) {
      UInt32 theSize, theStatus, theNumberDevices;

      theStatus = AudioHardwareGetPropertyInfo(kAudioHardwarePropertyDevices,
					       &theSize,NULL);
      theNumberDevices = theSize / sizeof(AudioDeviceID);

      AudioDeviceID *ids = new AudioDeviceID [theNumberDevices];
      theStatus = AudioHardwareGetProperty(kAudioHardwarePropertyDevices,
					   &theSize,ids);
      for (int i = 0; i < theNumberDevices; ++i) {
	driverNames.push_back((char *) ids[i]);
      }
      delete [] ids;
      return driverNames.size();
    }
  };

  
  CoreAudioDeviceManager::CoreAudioDeviceManager () 
    : AudioDeviceManager("coreaudio",PRIORITY_NORMAL) {
  }
   
   

  static volatile CoreAudioDeviceManager *s_TheDeviceManager = NULL;
  
  void InitCoreAudio () {
    s_TheDeviceManager = new CoreAudioDeviceManager;
  }



} // namespace osw 

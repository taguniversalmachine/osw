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
  oswWinMME.cpp
  Windows MME (Wave) Driver, uses PortAudio
*/

#include "oswWinAsio.h"
#include "asiodrivers.h"

// Set FOOFILE to 1 for "foo file" debugging
#define FOOFILE 0
#define FOOFILE2 0

#define NOISETEST 0

/********************/

extern void HandleAudioInput ();


/*******************/

#define PA_MAX_DEVICE_INFO (32)

// Some external references
extern AsioDrivers* asioDrivers ;

/*******************/

namespace osw {

  static int PaCallback( void *inputBuffer, void *outputBuffer,
                         unsigned long framesPerBuffer,
                         PaTimestamp outTime, void *userData );

#if FOOFILE
  // foofile debugging
  FILE *foofile;
#endif  
  
  
  AsioDriver::AsioDriver (const string &aname, 
                              Container *acontainer,
                              const string &afriendlyName,
                              int ainputDeviceID,
                              int aoutputDeviceID) :
    AudioDriver (aname,acontainer) {
    
    xstream = NULL;
    xinputDeviceID = ainputDeviceID;
    xoutputDeviceID = aoutputDeviceID;
    
    friendlyName.SetWithoutActivating(afriendlyName);

    const PaDeviceInfo *pdi;

    if (xinputDeviceID) {
      pdi = Pa_GetDeviceInfo(xinputDeviceID);
      input_channels = pdi->maxInputChannels;
    }
    if (xoutputDeviceID) {
      pdi = Pa_GetDeviceInfo(xoutputDeviceID);
      output_channels = pdi->maxOutputChannels;
    }

    cout << PathName() << ": " << input_channels << " input channels." << endl;
    cout << PathName() << ": " << output_channels << " output channels." << endl;
    
    if (input_channels == 0) {
      xhasInput = false;
      xinputDeviceID = paNoDevice;
    }

    sampleRate.SetWithoutActivating(GetDefaultSampleRate());
    bufferSize.SetWithoutActivating(GetDefaultBufferSize(this));

    if (!SetupBuffer()) {
      throw(osw::Exception("Could not open device"));
    }

    hSyncEvent=CreateEvent(NULL,FALSE,FALSE,(PathName()+"/OSWSYNC").c_str());

    InitializeInternalClock();
    RegisterInputChannels();
    RegisterOutputChannels();

#if FOOFILE
    foofile = fopen("foo.raw","wb");
#endif
  }
  
  
  bool AsioDriver::SetupBuffer() {
    if (xstream) {
      Pa_CloseStream(xstream);
    }

    PaError err = Pa_OpenStream(&xstream,
                                xinputDeviceID,
                                input_channels,
                                paFloat32,
                                NULL,
                                xoutputDeviceID,
                                output_channels,
                                paFloat32,
                                NULL,
                                sampleRate,
                                bufferSize,
                                0,
                                0,
                                PaCallback,
                                (void *)this);
    if (err != paNoError) {
      Error(Pa_GetErrorText(err));
      xstream = NULL;
      return false;
    }

    return true;
  }
  
  AsioDriver::~AsioDriver () {
    if (xstream) {
      Stop();
      Pa_CloseStream(xstream);
    }
    CloseHandle(hSyncEvent);
#if FOOFILE
    fclose(foofile);
#endif
  }
  
  /***********************/

  int AsioDriver::GetPreferredBufferSize() { 
    return 512;
  }
  
  void AsioDriver::DoInputOutputThing() {
    ProfileOff(this);
    GetInputSamples();
    ProfileOn(this);
    ActivateInputTransforms();
    ProfileOff(this);
    FlushSamples();
    SetEvent(hSyncEvent);
    ProfileOn(this);    
  }

  void AsioDriver::Start () {
    if (xstream) {
      Pa_StartStream(xstream);
    }
  }
  
  void AsioDriver::Stop () {    
    if (xstream) {
      Pa_StopStream(xstream);
    }
  }
  
  int AsioDriver::FlushSamples () {
#if FOOFILE
#if !defined(FOOFILE2) || !FOOFILE2
    static short foobuffer[4096];
    int numSamples = int(output_channels) * bufferSize;
    for (int i = 0; i < numSamples; ++i) {
      //cout << (xoutput[i] * 32767) << ' ';
      foobuffer[i] = xoutput[i] * 32767;
    }
    //cout << endl;

    fwrite(foobuffer,numSamples,sizeof(short),foofile);
#endif
#endif
    return DAC_WRITE_OK;
  }
  
  
  int AsioDriver::WriteSamples (int achannel, float *samples, int NumSamples) {    
    int i,j;
    for (i = 0, j = achannel; i < NumSamples; ++i, j+=int(output_channels)) {
      xoutput[j] += samples[i];
    }
    return DAC_WRITE_OK;
  }
  
  int AsioDriver::ReadSamples (int achannel, float *samples, int NumSamples) {
    
    int i,j;
    for (i = 0, j = achannel; i < NumSamples; ++i, j += int(input_channels)) {
       samples[i] = xinput[j];
    }
    
    return DAC_WRITE_OK;
  }
  
  int AsioDriver::GetInputSamples () {
    
    return DAC_WRITE_OK;
  }
  
  int AsioDriver::SamplesInBuffer () {
    return 0;
  }
  
  int AsioDriver::AdjustToBufferSize () {   
    return (SetupBuffer()) ? int(bufferSize) : 0;
  }
  
  int AsioDriver::AdjustToSampleRate () {
    return SetupBuffer();
  }
  
  void AsioDriver::StartMainLoop () {
    
    owns_main_clock = true; // for the moment
    Start();
    Debug("Starting DAC");
    
   
    /* We don't need to do this because the PA callback handles the clock */
    /*
    if (owns_main_clock) {
      StartMainClock();
    }
    clock = clock;
    */
  }

  
  /********************************/

  static int PaCallback( void *inputBuffer, void *outputBuffer,
                         unsigned long framesPerBuffer,
                         PaTimestamp outTime, void *userData ) {
    AsioDriver *driver = (AsioDriver *) userData;
    driver->SetIOBuffers((float *)inputBuffer,(float *)outputBuffer);
    driver->AdvanceClock();
    HRESULT result = WaitForSingleObject(driver->hSyncEvent,2000);

#if FOOFILE && FOOFILE2
    static short foobuffer[4096];
    int numSamples = framesPerBuffer;
    for (int i = 0; i < numSamples; ++i) {
      //cout << (xoutput[i] * 32767) << ' ';
      foobuffer[i] = ((float *)outputBuffer)[i] * 32767;
    }
    //cout << endl;

    fwrite(foobuffer,numSamples,sizeof(short),foofile);
#endif

#if NOISETEST
    float *output = (float *) outputBuffer;
    for (int i = 0; i < framesPerBuffer * 2; ++i) {
      output[i] = float(rand()) / float(RAND_MAX);
    }
#endif
    return 0;
  }

  /********************************/

  static std::vector<AudioDriver *> s_drivers;
  
  struct ASIODeviceInfo {
    int input;
    int output;
    int devnum;
    
    ASIODeviceInfo () {
      input = output = devnum = 0;
    }
  };

  typedef std::map<String,ASIODeviceInfo> ASIODeviceMap;

  static ASIODeviceMap s_DeviceMap;
  
  struct AsioDeviceManager : public AudioDeviceManager {

    AsioDeviceManager () :
      AudioDeviceManager("asio",PRIORITY_EXCLUSIVE) {
    }
    
    bool Initialize () {
      int err;
      if ((err = Pa_Initialize()) != paNoError) {
        Error(Pa_GetErrorText(err));
        return false;
      }

      Alert("Detecting ASIO drivers");
      int numDevices = Pa_CountDevices();
      const PaDeviceInfo *pdi;

      char *names[PA_MAX_DEVICE_INFO];

      for (int j = 0; j < PA_MAX_DEVICE_INFO; ++j) {
        names[j] = new char[32];
      }

      /* Get names of all available ASIO drivers */
      asioDrivers->getDriverNames(names,PA_MAX_DEVICE_INFO);

      int numOSWDevices = 0;
      for(int i = 0; i < numDevices; i++ ) {
        if (!stricmp(names[i],"ASIO Multimedia Driver")) {
          continue;
        }
        if (!stricmp(names[i],"ASIO DirectX Full Duplex Driver")) {
          continue;
        }
        pdi = Pa_GetDeviceInfo( i );

        if (s_DeviceMap.find(pdi->name) == s_DeviceMap.end()) {
          s_DeviceMap[pdi->name].devnum = numOSWDevices;
          ++numOSWDevices;
        }
        if (pdi->maxInputChannels > 0 && !s_DeviceMap[pdi->name].input) {
          s_DeviceMap[pdi->name].input = i;
        }
        if (pdi->maxOutputChannels > 0 && !s_DeviceMap[pdi->name].output) {
          s_DeviceMap[pdi->name].output = i;
        }
      }

      for (int j = 0; j < PA_MAX_DEVICE_INFO; ++j) {
        delete [] names[j];
      }

      for (ASIODeviceMap::iterator p = s_DeviceMap.begin();
           p != s_DeviceMap.end();
           ++p) {
        Alert(p->first);
        try {
          static char buffer[32];
          sprintf (buffer,"audio/%d",p->second.devnum);
          s_drivers.push_back(new osw::AsioDriver(buffer,NULL,
                                                    p->first,
                                                    p->second.input,
                                                    p->second.output));
          return true;
        } catch (exception e) {
	  Error(e.what());
	  continue;          
        }
      }
      return false;
    }

    bool Cleanup () {
      for (int i = 0; i < s_drivers.size(); ++i) {
	if (s_drivers[i] != NULL) {
	  while (s_drivers[i]->DriverIsOn()) {
	    s_drivers[i]->StopTransform();
	  }
	  delete s_drivers[i];
	}
      }
      Pa_Terminate();
      return true;
    }

    int GetDriverNames (DriverNameList &driverNames) {
      Pa_Initialize();
      int numDevices = Pa_CountDevices();
      char *names[PA_MAX_DEVICE_INFO];

      for (int j = 0; j < PA_MAX_DEVICE_INFO; ++j) {
        names[j] = new char[32];
      }

      /* Get names of all available ASIO drivers */
      asioDrivers->getDriverNames(names,PA_MAX_DEVICE_INFO);

      for(int i = 0; i < numDevices; i++ ) {
        driverNames.push_back(names[i]);
        delete [] names[i];
      }
      Pa_Terminate();
      return driverNames.size();
    }

  };

  static AsioDeviceManager TheAsioDeviceManager;
  
}



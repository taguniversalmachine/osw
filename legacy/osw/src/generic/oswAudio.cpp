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
   oswaudio.cpp
   Generic Audio transforms and drivers
*/

#include "oswAudio.h"
#include "oswArgs.h"
#include "oswPrefs.h"

/*******************/

using namespace std;

namespace osw {

  stl::vector<AudioDriver::OutputChannel>  AudioDriver::outputChannelMap;
  stl::vector<AudioDriver::InputChannel>  AudioDriver::inputChannelMap;
  
  AudioDriver::AudioDriver (const string &aname, Container *acontainer)
    : ClockSource (aname,acontainer),
      input_channels ("input_channels",this,"Number of input channels.",0),
      output_channels ("output_channels",this,"Number of output channels.",1),
      sampleRate ("sample_rate",this,"Current sample rate.",44100),
      bufferSize ("buffer_size",this,"Sample buffer size.",DEFAULTBUFFERSIZE),
      bitsPerSample ("sample_bits",this,"Bits per sample.",16),
      loWater("lo_water",this,"Low-water mark.",LOWATER),
      hiWater("hi_water",this,"High-water mark (target latency in samples)",HIWATER), 
      friendlyName("friendlyName",this,
                   "Readable system-specific name for audio driver",
                   aname),
      checkStatus(&clock,stl::numeric_limits<int>::max(),this,
		  &AudioDriver::checkStatusExpr),
      sampleRateChanged (&sampleRate,this,
			 &AudioDriver::sampleRateChangedExpr),
      bufferSizeChanged (&bufferSize,this,
			 &AudioDriver::bufferSizeChangedExpr),
      hiWaterChanged (&hiWater,this,
		      &AudioDriver::hiWaterChangedExpr) {
    xoldSampleRate = (int) sampleRate;
    clock.SetParallel();
    clock->set_sample_rate(sampleRate);
    clock->num_samples = (int) bufferSize;
    owns_main_clock=true;
    xready = false;
    xdriverOn = 0;
    xnumWrites = 0;
  }

  AudioDriver::~AudioDriver () {
    for (OutputTransformList::iterator p = xoutputTransforms.begin();
	 p != xoutputTransforms.end();
	 ++p) {
      if (*p != NULL) {
	(*p)->Nullify ();
      }
    }
    for (InputTransformList::iterator p = xinputTransforms.begin();
	 p != xinputTransforms.end();
	 ++p) {
      if (*p != NULL) {
	(*p)->Nullify ();
      }
    }
  }
  
  void AudioDriver::DoInputOutputThing () {
    ProfileOff(this);
    WaitUntilReady();
    //xsemaphore.Acquire();
    FlushSamples();
    ProfileOn(this);
    //xsemaphore.Release();
    if (xhasInput) {
      ProfileOff(this);
      GetInputSamples();
      ProfileOn(this);
      ActivateInputTransforms();
    }
    //xready = false;
    AdvanceClock();
  }

  void AudioDriver::bufferSizeChangedExpr () {
    if (!AdjustToBufferSize()) {
      return;
    }
    if (owns_main_clock) {
      MainClock->num_samples = (int) bufferSize;;
    }
    clock->num_samples = (int) bufferSize;
    StartBufferSizePropagation();
  }
  
  void AudioDriver::sampleRateChangedExpr () {
    if (!AdjustToSampleRate()) {
      sampleRate.SetWithoutActivating(xoldSampleRate);
      return;
    }
    if (owns_main_clock) {
      MainClock->set_sample_rate(sampleRate);
    }
    clock->set_sample_rate(sampleRate);
    
    hiWater = 
      (Integer) (float(hiWater) * (float(sampleRate) / float(xoldSampleRate)));
    loWater = 
      (Integer) (float(loWater) * (float(sampleRate) / float(xoldSampleRate)));
    xoldSampleRate = (int) sampleRate;
    StartSampleRatePropagation();
  }
  
  void AudioDriver::hiWaterChangedExpr () {
    if(!AdjustToHiWater()) {
      hiWater.SetWithoutActivating(xoldHiWater);
      return;
    }
  }
  
  void AudioDriver::checkStatusExpr () {
    if (DriverIsOn()) {
      DoInputOutputThing();
    }
  }
  
  void AudioDriver::InitializeInternalClock () {
    clock->num_samples = (int) bufferSize;
    clock->sample_rate = (int) sampleRate;
  }

  extern void DoOneEvent();

  void AudioDriver::WaitUntilReady () {
    while (SamplesInBuffer() > hiWater) {
      //DoSomethingUseful();
      DoOneEvent();
    }
  }

  bool AudioDriver::NeedMoreSamples () {
    return SamplesInBuffer() < hiWater;
  }

  void AudioDriver::Lock(AudioOutputTransform *lockingTransform) {
  
    xsemaphore.Acquire();
    ++(lockingTransform->xnumWrites);
    ++xnumWrites;
    /*
      bool flag = false;
      if (lockingTransform->xnumWrites > 1) {
      flag = true;
      } 
      if (xnumWrites >= xtransforms.size()) {
      flag = true;
      } 
      
      if (flag) {
      for (TransformList::iterator transformIndex = xtransforms.begin();
      transformIndex != xtransforms.end();
      ++transformIndex) {
      (*transformIndex)->xnumWrites = 0;
      }
      xnumWrites = 0;
      xready = true;
      }
    */
    
  }
  
  void AudioDriver::Unlock(AudioOutputTransform *unlockingTransform) {
    xsemaphore.Release();
  }

  void AudioDriver::RegisterOutputChannels () {
    for (int i = 0; i < output_channels; ++i) {
      outputChannelMap.push_back(AudioDriver::OutputChannel(this,i));
    }
  }

  void AudioDriver::RegisterInputChannels () {
    for (int i = 0; i < input_channels; ++i) {
      inputChannelMap.push_back(AudioDriver::InputChannel(this,i));
    }
  }

  void AudioDriver::ActivateInputTransforms () {
    for (InputTransformList::iterator transformIndex = xinputTransforms.begin();
	 transformIndex != xinputTransforms.end();
	 ++transformIndex) {
      if ((*transformIndex)->onoff) {
	(*transformIndex)->SendInput();
      }
    }
  }

  void AudioDriver::StartSampleRatePropagation() {
    ClockSource::StartSampleRatePropagation();
    InputTransformList::iterator  inputIndex;
    for (inputIndex = xinputTransforms.begin(); 
	 inputIndex != xinputTransforms.end();
	 ++inputIndex) {
      (*inputIndex)->SampleRate = (float) sampleRate;
    }

  }

  void AudioDriver::StartBufferSizePropagation() {
    ClockSource::StartBufferSizePropagation();
    InputTransformList::iterator  inputIndex;
    for (inputIndex = xinputTransforms.begin(); 
	 inputIndex != xinputTransforms.end();
	 ++inputIndex) {
      (*inputIndex)->NumberOfSamples = (unsigned int) bufferSize;
    }

  }

  /*******************/

  AudioOutputTransform::AudioOutputTransform (const string &aname,
					      Container *acontainer,
					      int argc, char *argv[]) :
    TimeDomainTransform (aname,acontainer,argc,argv),
    
    onoff("onoff",this,"Turns DAC output on/off.",false),
    mix("mix",this,"Send samples to ALL channels."),
    activation1(argc,this,&AudioOutputTransform::activation1Expr),
    activation3(&onoff,this,&AudioOutputTransform::activation3Expr) {
    
    mix.SetOrder(1000); // we wouldn't have THAT MANY channels!
    
    samplesIn.reserve(argc);
    channel_index.reserve(argc);
    
    char buffer[32];

    int bufferSize = 0;
    int index = 0;
    for (int i = 0; i < argc; ++i) {

      int channelnum = atoi(argv[i]) - 1;
      /*
      cout << PathName()  << ' ' 
	   << channelnum << ' ' 
	   << AudioDriver::outputChannelMap.size()
	   << endl;
      */
      if (channelnum >= AudioDriver::outputChannelMap.size() || channelnum < 0) {
	Error (string(argv[i])+": Invalid audio output channel.");
	continue;
      }
      sprintf (buffer,"samplesIn%d",channelnum);
      samplesIn.push_back(new Inlet<Samples> (buffer,this,"Input samples for DAC."));
      channel_index.push_back(channelnum);
      //activation1.AddActivator(samplesIn[i]);
      samplesIn[index]->SetOrder(index+1);
      AudioDriver::outputChannelMap[channelnum].AddTransform(this);
      ++index;
      if (!bufferSize) {
	bufferSize = (int) AudioDriver::outputChannelMap[channelnum].GetDriver()->bufferSize;
      } else if (bufferSize != 
		 AudioDriver::outputChannelMap[channelnum].GetDriver()->bufferSize) {
	char errbuf[64];
	sprintf(errbuf,"Sample vector size for channel %d does not agree",channelnum);
	Error(errbuf);
      }    
    }
    NumberOfSamples.SetWithoutActivating(bufferSize);
    onoff.SetOrder(0);
    
    xnumWrites = 0;
  }
  
  AudioOutputTransform::~AudioOutputTransform () {
    for (int i = 0; i < channel_index.size(); ++i) {
      delete samplesIn[i];
      if (channel_index[i] != NO_CHANNEL) {
	AudioDriver::outputChannelMap[channel_index[i]].RemoveTransform(this);
      }
    }
  }

  const char *(AudioOutputTransform::Draw)(void *DrawState) {
    return
      "oswLoadScript toggleandlabel.tcl\n"
      "HideName\n"
      "makeToggleAndLabel $transform $drawingSpace \"\" "
      "  [oswLoadBitmap photo speaker.gif]\n";
    //"toggle $drawingSpace.t -command [list oswSet $transform/onoff]"
    //" -onimage [oswLoadBitmap photo speaker_on.gif] "
    //" -offimage [oswLoadBitmap photo speaker_off.gif]\n"
    //"pack $drawingSpace.t -fill both -expand yes\n";
  }

  bool AudioOutputTransform::Connect (BaseOutlet *source, BaseState *&sink) {
    if (TimeDomainTransform::Connect(source,sink)) {
      if (sink != &onoff) {
	activation1.AddActivator(sink);
      }
      return true;
    }
    return false;
  }
 
  bool AudioOutputTransform::BackDisconnect(BaseState *sink) {
    activation1.RemoveActivator(sink,true);
    return TimeDomainTransform::BackDisconnect(sink);
  }
  
  void AudioOutputTransform::activation1Expr () {
    if ((bool)onoff) {
      for (int i = 0; i < channel_index.size(); ++i) {
	if (samplesIn[i]->IsActivator()) {
	  //AudioDriver::outputChannelMap[channel_index[i]].Lock(this);
	  AudioDriver::outputChannelMap[channel_index[i]].WriteSamples 
	    ((*samplesIn[i])->begin(),NumberOfSamples);
	  //AudioDriver::outputChannelMap[channel_index[i]].Unlock(this); 
	}
      }
      if (mix.IsActivator()) {
	for (int i = 0; i < channel_index.size(); ++i) {
	  //AudioDriver::outputChannelMap[channel_index[i]].Lock(this);
	  AudioDriver::outputChannelMap[channel_index[i]].WriteSamples
	    (mix->begin(),NumberOfSamples);
	  //AudioDriver::outputChannelMap[channel_index[i]].Unlock(this); 
	}
      }
    }
  }

  void AudioOutputTransform::activation3Expr () {
    if (bool(onoff)) {
      for (int i = 0; i < channel_index.size(); ++i) {
	AudioDriver::outputChannelMap[channel_index[i]].StartTransform();
      }
    } else {
      for (int i = 0; i < channel_index.size(); ++i) {
	AudioDriver::outputChannelMap[channel_index[i]].StopTransform();
      }
    }
  }


/*****************/

AudioInputTransform::AudioInputTransform (const string &aname,
					  Container *acontainer,
					  int argc, char *argv[]) :
  TimeDomainTransform (aname,acontainer,argc,argv),
  onoff("onoff",this,"Turns audio device on/off.",false),
  activation1(&onoff,this,&AudioInputTransform::activation1Expr) {

    
    samplesOut.reserve(argc);
    channel_index.reserve(argc);
    
    char buffer[32];
    int index = 0;
    for (int i = 0; i < argc; ++i) {
      int channelnum = atoi(argv[i]) - 1;
      if (channelnum >= AudioDriver::inputChannelMap.size() || channelnum < 0) {
	Error (string(argv[i])+": Invalid audio input channel.");
	continue;
      }
      sprintf (buffer,"samplesOut%d",channelnum);
      samplesOut.push_back(new Outlet<Samples> (buffer,this,"Input samples from device."));
      channel_index.push_back(channelnum);
      samplesOut[index]->SetOrder(index+1);
      samplesOut[index]->NoParallel();
      AudioDriver::inputChannelMap[channelnum].AddTransform(this);
      //SampleRate.SetWithoutActivating(AudioDriver::inputChannelMap[channelnum].driver->sampleRate);
      //NumberOfSamples.SetWithoutActivating(AudioDriver::inputChannelMap[channelnum].driver->bufferSize);
      ++index;
    }
    onoff.SetOrder(0);
}

AudioInputTransform::~AudioInputTransform () {
  for (int i = 0; i < channel_index.size(); ++i) {
    delete samplesOut[i];
    if (channel_index[i] != NO_CHANNEL) {
      AudioDriver::inputChannelMap[channel_index[i]].RemoveTransform(this);
    }
  }
}


void AudioInputTransform::activation1Expr () {
  if (bool(onoff)) {
    for (int i = 0; i < channel_index.size(); ++i) {
      AudioDriver::inputChannelMap[channel_index[i]].StartTransform();
    }
  } else {
    for (int i = 0; i < channel_index.size(); ++i) {
      AudioDriver::inputChannelMap[channel_index[i]].StopTransform();
    }
  }
}

void AudioInputTransform::SendInput () {
  Samples nextInput;
  for (int i = 0; i < channel_index.size(); ++i) {
    nextInput = Samples(NumberOfSamples);
    AudioDriver::inputChannelMap[channel_index[i]].ReadSamples
      (nextInput.begin(),NumberOfSamples);
    *(samplesOut[i]) = nextInput;
  }
}

const char *(AudioInputTransform::Draw)(void *DrawState) {
  return
    "oswLoadScript toggleandlabel.tcl\n"
    "HideName\n"
    "makeToggleAndLabel $transform $drawingSpace \"\" "
    "  [oswLoadBitmap photo mike1a.gif]\n";
  //"toggle $drawingSpace.t -command [list oswSet $transform/onoff]"
  //" -onimage [oswLoadBitmap photo audioin_on.gif] "
  //" -offimage [oswLoadBitmap photo audioin_off.gif]\n"
  //"pack $drawingSpace.t -fill both -expand yes\n";
}

/*****************/

static TransformConstructor<osw::AudioOutputTransform> 
    AudioOutputTransformConstructor ("AudioOutput");

static TransformConstructor<osw::AudioInputTransform> 
    AudioInputTransformConstructor ("AudioInput");

/*****************/
  
  typedef list<AudioDeviceManager *>::iterator DriverEnumerator;
  static list<AudioDeviceManager *> *exclusiveDrivers = NULL;
  static list<AudioDeviceManager *> *normalDrivers = NULL;
  static list<AudioDeviceManager *> inUseDrivers;
  static int s_initialized = 666;

  AudioDeviceManager::AudioDeviceManager(const string &aname,
					 PriorityType apriority) 
    : Container(aname,NULL) {
    if (s_initialized != 666) {
      exclusiveDrivers = normalDrivers = NULL;
    }
    if (!exclusiveDrivers) {
      exclusiveDrivers = new list<AudioDeviceManager *>;
    }
    if (!normalDrivers) {
      normalDrivers = new list<AudioDeviceManager *>;
    }
    if (apriority == PRIORITY_EXCLUSIVE) {
      exclusiveDrivers->push_back(this);
    } else {
      normalDrivers->push_back(this);
    }
  }

  void _oswexport InitAudioDrivers() {
    String ud = GetDefaultDriver();
    if(ud != "N/A") {
      cout << "User requested driver " << ud << "." << endl;
    }
    if(ud == "none") {
      return;
    } 
    DriverEnumerator drivers;
    if (exclusiveDrivers) {
      for (drivers = exclusiveDrivers->begin();
	   drivers != exclusiveDrivers->end();
	   ++drivers) 
      {
        if(ud == "N/A") {
          if ((*drivers)->Initialize()) {
            inUseDrivers.push_back(*drivers);
            return;
          }
        } else {
          if ((*drivers)->Name() == ud) {
            if((*drivers)->Initialize()) {
              inUseDrivers.push_back(*drivers);
              cout << "Enabled " << ud << " driver." << endl;
              return;
            } else {
              cout << "Failed to initialize " << ud << " driver." << endl;
              exit(1);
              return;
            }
          }
        }
      }
    }
    if (normalDrivers) {
      for (drivers = normalDrivers->begin();
       drivers != normalDrivers->end();
       ++drivers) 
      {
        if ((*drivers)->Initialize()) {
          inUseDrivers.push_back(*drivers);
        }
      }
    }
  }

  void _oswexport CleanupAudioDrivers() {
    for (DriverEnumerator drivers = inUseDrivers.begin();
	 drivers != inUseDrivers.end();
	 ++drivers) {
      (*drivers)->Cleanup();
    }
    if (normalDrivers) {
      delete normalDrivers;
    }
    if (exclusiveDrivers) {
      delete exclusiveDrivers;
    }
  }

  void _oswexport PrintDriverInfo () {
    DriverEnumerator drivers;
    cout << "Available Audio Drivers:" << endl;
    if (exclusiveDrivers) {
      for (drivers = exclusiveDrivers->begin();
	   drivers != exclusiveDrivers->end();
	   ++drivers) {
	DriverNameList driverNames;
	if ((*drivers)->GetDriverNames(driverNames) >= 0) {
	  int i = 1;
          cout << "Driver: " << (*drivers)->Name() << endl;
	  for (DriverNameList::iterator p = driverNames.begin();
	       p != driverNames.end();
	       ++p) {
	    cout << "  Device " << i << ": " << *p << endl;
	    ++i;
	  }
          cout << "  Options:" << endl;
          (*drivers)->PrintOptions();
	}
      }
    }
    if (normalDrivers) {
      for (drivers = normalDrivers->begin();
	   drivers != normalDrivers->end();
	   ++drivers) {
	DriverNameList driverNames;
	if ((*drivers)->GetDriverNames(driverNames) >= 0) {
	  int i = 1;
          cout << "Driver: " << (*drivers)->Name() << endl;
	  for (DriverNameList::iterator p = driverNames.begin();
	       p != driverNames.end();
	       ++p) {
	    cout << "  Device " << i << ": " << *p << endl;
	    ++i;
	  }
          cout << "  Options:" << endl;
          (*drivers)->PrintOptions();
	}
      }
    }
  }

  /******************/
  // Driver configuration stuff
  
  static Float    s_defaultSampleRate = 0.0f;
  static Integer  s_defaultBufferSize = 0;
  static Integer  s_defaultHiWater = 0;
  static String   s_defaultDriver = "N/A";
  static map<String,String,NoCaseStringCompare> s_driverOptions;

  Float _oswexport GetDefaultSampleRate () {
    if (s_defaultSampleRate == 0.0f) {
      String value;
      GetPreference("SampleRate",value,"44100");
      s_defaultSampleRate = atof(value.c_str());
      if (s_defaultSampleRate == 0) {
	s_defaultSampleRate = 44100.0f;
      }
    }
    return s_defaultSampleRate;
  }

  Integer _oswexport GetDefaultBufferSize () {
    if (s_defaultBufferSize == 0) {
      String value;
      GetPreference("BufferSize",value,"0");
      s_defaultBufferSize = atoi(value.c_str());
      if (s_defaultBufferSize == 0) {
        s_defaultBufferSize = 128;
      }
    }
    return s_defaultBufferSize;
  }

  Integer _oswexport GetDefaultBufferSize (AudioDriver *driver) {
    if (s_defaultBufferSize == 0) {
      String value;
      GetPreference("BufferSize",value,"0");
      s_defaultBufferSize = atoi(value.c_str());
      if (s_defaultBufferSize == 0) {
        s_defaultBufferSize = driver->GetPreferredBufferSize();
      }
    }
    return s_defaultBufferSize;
  }

  Integer _oswexport GetDefaultHiWater() {
    if (s_defaultHiWater == 0) {
      String value;
      GetPreference("HiWater",value,"512");
      s_defaultHiWater = atoi(value.c_str());
      if(s_defaultHiWater == 0) {
	s_defaultHiWater = 512;
      }
    }
    return s_defaultHiWater;
  }
  
  /// Return default driver or "N/A" if none chosen.
  String _oswexport GetDefaultDriver() {
    if (s_defaultDriver != "N/A") {
      return s_defaultDriver;
    }
    else {
      String value;
      GetPreference("Driver",value,"N/A");
      return value;
    }
  }
  
  /// Return option as a String value, or return default if not specified.
  String _oswexport GetDriverOption(const String& key, const String& def) {
    String value;
    if (s_driverOptions[key] != String()) {
      return s_driverOptions[key];
    } else {
      GetPreference(key, value, def);
      return value;
    }
  }
  
  /// Return option as an Integer, or return default if not specified.
  Integer _oswexport GetDriverIntegerOption(const String& key, const Integer def) {
    String value = GetDriverOption(key, "");
    if(value == "") {
      return def;
    }
    else {
      return Integer(atoi(value.c_str()));
    }
  }

  /// Return option as a Float type, or return default if not found.
  Float _oswexport GetDriverFloatOption(const String& key, const Float def) {
    String dstr = "";
    String value = GetDriverOption(key, "");
    if(value == "") {
      return def;
    }
    else {
      return Float(atof(value.c_str()));
    }
  }
  
  /******************/
  // Audio-related command-line args

  // List drivers, devices and options
  static bool ListDriversProc (int argc, char *argv[], int &index) {
    PrintDriverInfo();
    return true;
  }
  static CommandLineArg s_listDrivers("-l","--list-drivers","List available audio drivers",ListDriversProc);

  // Sample Rate (default 44100)
  static bool SetSampleRateProc (int argc, char *argv[], int &index) {
    ++index;
    if (index == argc) {
      Error("-r: please supply a sample rate (e.g., 44100)");
      return true;
    }
    s_defaultSampleRate = atof(argv[index]);
    return false;
  }
  static CommandLineArg s_setSampleRate("-r","--set-sample-rate","Set default sample rate", SetSampleRateProc);

  // Buffer size
  static bool SetBufferSizeProc (int argc, char *argv[], int &index) {
    ++index;
    if (index == argc) {
      Error("-b: please supply a valid buffer size (e.g., 64, 128, 256)");
      return true;
    }
    s_defaultBufferSize = (Integer) atof(argv[index]);
    return false;
  }
  static bool SetHiWaterProc (int argc, char *argv[], int &index) {
    ++index;
    if (index == argc) {
      Error("-h: please supply number of samples (e.g. 512)");
      return true;
    }
    s_defaultHiWater = atoi(argv[index]);
    return false;
  }
  static CommandLineArg s_setBufferSize("-b","--set-buffer-size", "Set default sample buffer size (vector size)", SetBufferSizeProc);
  static CommandLineArg s_setHiWater("-h","--set-hi-water", "Set Hi-Water (target latency in samples)", SetHiWaterProc);
  
  // Driver choice  
  static bool SetDriverProc (int argc, char* argv[], int &index) {
    ++index;
    if (index == argc) {
      Error("-dr: please supply a valid driver name.");
      return true;
    }
    s_defaultDriver = String(argv[index]);
    return false;
  }
  static CommandLineArg s_setDriver("-dr", "--driver", "Choose which driver to use", SetDriverProc);
  
  // Driver options
  static bool SetDriverOptionProc (int argc, char* argv[], int &index) {
    ++index;
    ++index;
    if (index >= argc) {
      Error("-o: bad option specification.");
      return true;
    }
    //cout << "Driver option " << String(argv[index-1]) << " : " << String(argv[index]) << endl;
    s_driverOptions[String(argv[index-1])] = String(argv[index]);
    return false;
  }
  static CommandLineArg s_setDriverOption("-o", "--driver-option", "Set driver option (e.g. -o option-name value)", SetDriverOptionProc);

} // namespace osw


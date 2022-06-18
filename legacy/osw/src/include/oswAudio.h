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
   oswaudio.h
   Interface to Audio I/O transforms and drivers
*/

#ifndef _OSWAUDIO_H
#define _OSWAUDIO_H

#include "oswClock.h"
#include "oswSched.h"
#ifndef _NO_LIMITS
#include <limits>
#else
#include <climits>
#endif


#define LOWATER	 			300
#define HIWATER    			512
#define DEFAULTBUFFERSIZE	        128

#define DAC_WRITE_OK 			0
#define DAC_WRITE_ERROR 		-1
#define DAC_WRITE_NOBUFFER 	-2

using namespace std;

namespace osw {
  
  class AudioOutputTransform;
  class AudioInputTransform;
  
  class _oswexport AudioDriver : public ClockSource {
    
  public:
    
    State<int> input_channels, output_channels;
    State<int> sampleRate, bitsPerSample;
    State<int> loWater, hiWater;
    State<int> bufferSize;
    State<int> outputBufferSize;
    State<string> friendlyName;
  
    typedef stl::list<AudioOutputTransform * > OutputTransformList;
    typedef stl::list<AudioInputTransform * > InputTransformList;
  
    AudioDriver (const string &aname, Container *acontainer);
    virtual ~AudioDriver ();
 
    virtual int WriteSamples (int achannel, float *samples, int NumSamples) = 0;
    virtual int ReadSamples (int achannel, float *samples, int NumSamples) = 0;
    virtual int FlushSamples () = 0;
    virtual int GetInputSamples () = 0;
    virtual int	 SamplesInBuffer () = 0;
    virtual void Start () = 0;
    virtual void Stop () = 0;
    virtual void StartMainLoop() = 0;
    virtual bool NeedMoreSamples ();

    virtual int GetPreferredBufferSize() { 
      return DEFAULTBUFFERSIZE;
    }

    void AddOutputTransform (AudioOutputTransform *toAdd) {
      if (find(xoutputTransforms.begin(),xoutputTransforms.end(),toAdd) 
	  == xoutputTransforms.end()) {
	xoutputTransforms.push_back(toAdd);
      }
    }

    void RemoveOutputTransform (AudioOutputTransform *toRemove) {
      xoutputTransforms.erase(remove(xoutputTransforms.begin(),
				     xoutputTransforms.end(),toRemove),
			      xoutputTransforms.end());
    }

    void AddInputTransform (AudioInputTransform *toAdd);

    void RemoveInputTransform (AudioInputTransform *toRemove) {
      xinputTransforms.erase(remove(xinputTransforms.begin(),
				     xinputTransforms.end(),toRemove),
			      xinputTransforms.end());
    }
    
    void StartTransform () {
      ++xdriverOn;
      if (xdriverOn == 1) {
	StartMainLoop();
      }
    }

    void StopTransform () {
      if (xdriverOn > 0) {
	--xdriverOn;
      }
      if (xdriverOn == 0) {
	Stop();
      }
    }

    int DriverIsOn () {
      return xdriverOn;
    }

    void StartInput () {
      ++xinputOn;
    }

    void StopInput() {
      if (xinputOn > 0) {
	--xinputOn;
      }
    }

    int InputIsOn () {
      return xinputOn;
    }

    void Lock (AudioOutputTransform *lockingTransform);
    void Unlock (AudioOutputTransform *unlockingTransform);

  protected:
    virtual void DoInputOutputThing ();
    virtual int AdjustToBufferSize () = 0;
    virtual int AdjustToSampleRate () = 0;
    virtual int AdjustToHiWater () { return 0; }

    Activation<AudioDriver> bufferSizeChanged, sampleRateChanged, hiWaterChanged, checkStatus;
    
    void sampleRateChangedExpr();
    void bufferSizeChangedExpr();
    void hiWaterChangedExpr();
    void checkStatusExpr ();

    void InitializeInternalClock();

    virtual void StartSampleRatePropagation();
    virtual void StartBufferSizePropagation();


    
    virtual void WaitUntilReady ();
    InputTransformList  xinputTransforms;
    OutputTransformList  xoutputTransforms;
    int xoldSampleRate;
    int xoldHiWater;
    int xdriverOn;
    int xinputOn;
    
    Semaphore  xsemaphore;
    bool       xready;
    bool       xhasInput;
    bool       xhasOutput;
    int        xnumWrites;

    friend class AudioOutputTransform;
    friend class AudioInputTransform;
    
    struct OutputChannel {
      AudioDriver *driver;
      int       channel;
      OutputChannel (AudioDriver *adriver = NULL, int achannel = 0) {
	driver = adriver;
	channel = achannel;
      }
      
      int WriteSamples (float *samples, int NumSamples) {
	return driver->WriteSamples(channel,samples,NumSamples);
      }
      void Lock (AudioOutputTransform *lockingTransform) {
	driver->Lock(lockingTransform);
      }
      void Unlock (AudioOutputTransform *unlockingTransform) {
	driver->Unlock(unlockingTransform);
      }
      void StartTransform () {
	driver->StartTransform();
      }
      void StopTransform () {
	driver->StopTransform();
      }      
      void AddTransform (AudioOutputTransform *toAdd) {
	driver->AddOutputTransform(toAdd);
      }
      void RemoveTransform (AudioOutputTransform *toRemove) {
	driver->RemoveOutputTransform(toRemove);
      }

      AudioDriver *GetDriver () {
	return driver;
      }

    };

    struct InputChannel {
      AudioDriver *driver;
      int       channel;
      InputChannel (AudioDriver *adriver = NULL, int achannel = 0) {
	driver = adriver;
	channel = achannel;
      }
      
      int ReadSamples (float *samples, int NumSamples) {
	return driver->ReadSamples(channel,samples,NumSamples);
      }

      void StartTransform () {
	driver->StartInput();
	driver->StartTransform();
      }
      void StopTransform () {
	driver->StopTransform();
	driver->StopInput();
      }      
      void AddTransform (AudioInputTransform *toAdd) {
	driver->AddInputTransform(toAdd);
      }
      void RemoveTransform (AudioInputTransform *toRemove) {
	driver->RemoveInputTransform(toRemove);
      }

      AudioDriver *GetDriver () {
	return driver;
      }

    };
      
    static stl::vector<OutputChannel> outputChannelMap;
    static stl::vector<InputChannel> inputChannelMap;

    void RegisterInputChannels();
    void RegisterOutputChannels();

    void ActivateInputTransforms();
  };


  /*******************/

  class _oswexport AudioOutputTransform : public TimeDomainTransform {

    enum ChannelIndexType {NO_CHANNEL = -1};

  public:
    
    Inlet<bool> onoff;
    State<bool> exclusive;
    std::vector<Inlet<Samples> *> samplesIn;
    Inlet<Samples> mix;

    AudioOutputTransform (const string &aname, Container *acontainer, 
		  int argc, char *argv[]);
    
    ~AudioOutputTransform ();

    const char *Draw(void *DrawState);

    const char *Description () const {
      return "Send output to Audio Device.";
    }
        
  protected:
    
    stl::vector<int>  channel_index;
   
    bool Connect (BaseOutlet *source, BaseState *&sink);
    bool BackDisconnect(BaseState *sink);

    friend class AudioDriver;

    void Nullify () {
      for (int i = 0; i < channel_index.size(); ++i) {
	channel_index[i] = NO_CHANNEL;
      }
    }
    
    Activation<AudioOutputTransform>  activation1, activation3;
    void activation1Expr ();
    void activation3Expr ();

    int xnumWrites;
  }; 


  class _oswexport AudioInputTransform : public TimeDomainTransform {

    enum ChannelIndexType {NO_CHANNEL = -1};

  public:

    AudioInputTransform (const string &aname, Container *acontainer, 
			  int argc, char *argv[]);
    
    ~AudioInputTransform ();

    Inlet<bool> onoff;
    std::vector<Outlet<Samples> *> samplesOut;

    const char *Draw(void *DrawState);

    const char *Description () const {
      return "Get input from Audio Device.";
    }


  protected:

    stl::vector<int>  channel_index;
    friend class AudioDriver;

    void Nullify () {
      for (int i = 0; i < channel_index.size(); ++i) {
	channel_index[i] = NO_CHANNEL;
      }
    }

    void SendInput();

    Activation<AudioInputTransform> activation1;
    void activation1Expr ();


  };

  inline void AudioDriver::AddInputTransform (AudioInputTransform *toAdd) {
    if (find(xinputTransforms.begin(),xinputTransforms.end(),toAdd)
	== xinputTransforms.end()) {
      xinputTransforms.push_back(toAdd);
      //toAdd->SampleRate = (float) sampleRate;
      //toAdd->NumberOfSamples = (unsigned int) bufferSize;
    }
  }

  /********************************************/

  typedef list<String> DriverNameList;

  struct _oswexport AudioDeviceManager : public Container {

    typedef enum {PRIORITY_NORMAL, PRIORITY_EXCLUSIVE} PriorityType;

    AudioDeviceManager(const string &aname,
		       PriorityType apriority = PRIORITY_NORMAL);

    virtual bool Initialize () = 0;
    virtual bool Cleanup() = 0;
    virtual int  GetDriverNames(DriverNameList &driverNames) = 0;
    
    virtual void PrintOptions() {
      cout << "    " << "None." << endl;
    }

  };

  void _oswexport InitAudioDrivers();
  void _oswexport CleanupAudioDrivers();
  void _oswexport PrintDriverInfo();
  
  Float _oswexport GetDefaultSampleRate ();
  Integer _oswexport GetDefaultBufferSize ();
  Integer _oswexport GetDefaultBufferSize (AudioDriver *drive);
  String _oswexport GetDefaultDriver();
  String _oswexport GetDriverOption(const String&, const String&);
  Integer _oswexport GetDriverIntegerOption(const String& key, const Integer def);
  Float _oswexport GetDriverFloatOption(const String& key, const Float def);
  
}

#endif // _OSWAUDIO_H                   


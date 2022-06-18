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
  oswLinuxJackAudio.cpp
  Jack-Audio-Server Client Audio driver for OSW/Linux
 */

#ifdef JACK

#include "oswLinuxJackAudio.h"
#include "oswTime.h"
#include "oswDelete.h"
#include "oswSched.h"
#include "oswSelect.h"

using namespace std;

namespace osw {

  /******************/

  int  osw_jack_detached_process_callback(jack_nframes_t n, void* arg)
  {
      JackDriver* jd = (JackDriver*)arg;
      /*
      int i = 0;
      float* samples;
      for(i = 0; i < jack_inports; i++) {
          samples = (float*) jack_get_buffer(jack_inputs[i], n);
          // memcpy...
      }
      
      for(i = 0; i < jack_outports; i++) {
          samples = (float*) jack_get_buffer(jack_outputs[i], n);
          // memcpy...
      }
      */
      return 0;
  }
 
  int  osw_jack_inline_process_callback(jack_nframes_t n, void* arg) 
  {
    JackDriver *jd = (JackDriver*)arg;

    // Copy data into buffer...
    
    // Tick
    jd->AdvanceClock();

    // Do work
    try {
      CheckSelectEvents(); // is non-blocking
      SelectEventsDispatch();

      // While there is something to do...
      while(! TheParallelScheduler->NothingToDo()) {
	BaseActivation *nextActivation = TheParallelScheduler->RawGetNextActivation ();
	if (nextActivation == NULL) {  // This should never happen.
	  Alert("LazyLoop Recieved NULL Activation.");
	} else {
	  (*nextActivation)(); // Run activation
	}
      }
      DoSomethingUseful();
      TheParallelScheduler->IncrementSelect();
    }
    catch(Exception *e) {
      Error(e->GetMessage());
    }
    catch(const std::exception &e) {
      Error(e.what());
    }
    catch(...) {
      // We are going to try to continue running...  but things may not be okay!
      Error("A fatal error has occured!!!");
      //exit(-1);
    }
    
    return 0;
  }
  int  osw_jack_buffer_size_callback(jack_nframes_t n, void* arg)
  {
    JackDriver *jd = (JackDriver*)arg;
    return 0;
  }
  
  /******************/
  
  JackDriver::JackDriver 
  (const string &aname, osw::Container *acontainer) : AudioDriver (aname,acontainer)
  {
    
    xjc = jack_client_new(GetDriverOption("name", "osw").c_str());
    if(xjc == NULL) {
      throw osw::Exception("Failed to connect to Jack server.");
    }
    
    Alert("Connected to Jack server.");
    
    if(jack_set_buffer_size_callback(xjc, osw_jack_buffer_size_callback, (void*)this) != 0) {
      throw osw::Exception("Failet to set buffer size changed callback.");
    }
    String method = GetDriverOption("method", "detached");
    if(method == "detached") {
      Alert("Using detached method.");
      if(jack_set_process_callback(xjc, osw_jack_detached_process_callback, (void*)this) != 0) {
        throw osw::Exception("Failed to set detached process callback.");
      }
    }
    else if(method == "inline") {
      Alert("Using inline method.");
      if(jack_set_process_callback(xjc, osw_jack_inline_process_callback, (void*)this) != 0) {
        throw osw::Exception("Failed to set inline process callback.");
      }
    } else {
      throw osw::Exception("Unknown method " + method);
    }
    
    xoldBufferSize = 0;
    this->sampleRate = jack_get_sample_rate(xjc);
    this->bufferSize = jack_get_buffer_size(xjc);
    
    input_channels = GetDriverIntegerOption("inchannels", 2);
    output_channels = GetDriverIntegerOption("outchannels", 2);
    jack_inports = input_channels;
    jack_outports = output_channels;
    
    int i = 0;
    char buffer[256];
    for(i = 0; i < input_channels; i++) {
      sprintf(buffer, "in_%d", i + 1);
      jack_inputs[i] = jack_port_register(xjc, buffer, JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput | JackPortCanMonitor, 0);
      if(jack_inputs[i] == NULL) {
	throw osw::Exception("Input port creation failed.");
      }
    }
    for(i = 0; i < output_channels; i++) {
      sprintf(buffer, "out_%d", i + 1);
      jack_outputs[i] = jack_port_register(xjc, buffer, JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput | JackPortCanMonitor, 0);
      if(jack_outputs[i] == NULL) {
	throw osw::Exception("Output port creation failed.");
      }
    }
    
    // Parse the coutput and cinput strings...
    // Connection errors are non-fatal
    std::vector<String> cinputs;
    std::vector<String> coutputs;
    Tokenize(GetDriverOption("inconnect", ""), cinputs, ",");
    Tokenize(GetDriverOption("outconnect", ""), coutputs, ",");

    std::vector<String>::iterator ci;
    i = 0;
    for(ci = cinputs.begin(); ci != cinputs.end() && i < input_channels; ++ci) {
        if(jack_connect(xjc, (*ci).c_str(), jack_port_name(jack_inputs[i])) != 0) {
            cerr << "Failed to connect input to " << *ci << endl;
        } else {
            cout << "Connected Input " << String(jack_port_name(jack_inputs[i])) << " with " << *ci << endl;
        }
        i++;
    }
    i = 0;
    for(ci = coutputs.begin(); ci != coutputs.end() && i < output_channels; ++ci) {
        if(jack_connect(xjc, jack_port_name(jack_outputs[i]), (*ci).c_str()) != 0) {
            cerr << "Failed to connect output to " << *ci << endl;
        } else {
            cout << "Connected Output " << String(jack_port_name(jack_outputs[i])) << " with " << *ci << endl;
        }
        i++;
    }
    
    RegisterInputChannels();
    RegisterOutputChannels();
 
    Alert("Activating Jack Client...");
    jack_activate(xjc);
  }

  JackDriver::~JackDriver () {
    if(xjc != NULL) {
      jack_client_close(xjc);
    }
    bufferSize = 0;
  }

  // Jack controls buffersize.
  int JackDriver::AdjustToBufferSize () {
    int i;
    if(xoldBufferSize != 0 && xoldBufferSize != bufferSize) {
        Alert("Releasing old buffers.");
        // Free old buffers
        for(i = 0; i < input_channels; i++) {
            delete jack_input_buffer[i];
        }
        for(i = 0; i < output_channels; i++) {
            delete jack_output_buffer[i];
        }
    }
    if((xoldBufferSize == 0 || xoldBufferSize != bufferSize) && bufferSize != 0) {
        Alert("Allocating new buffers.");
        // Allocate new buffers...
        for(i = 0; i < input_channels; i++) {
            jack_input_buffer[i] = new float[bufferSize];
        }
        for(i = 0; i < output_channels; i++) {
            jack_output_buffer[i] = new float[bufferSize];
        }
    }
    xoldBufferSize = bufferSize;
    return bufferSize;
  }

  // Jack controls sample rate, only
  int JackDriver::AdjustToSampleRate () {
    return jack_get_sample_rate(xjc);
  }

  // Unused...
  int JackDriver::SamplesInBuffer() {}
  extern void DoOneEvent();

  int JackDriver::FlushSamples() {
    return DAC_WRITE_OK;
  }

  int JackDriver::GetInputSamples () {
    return DAC_WRITE_OK;
  }
  
  void JackDriver::Start () {
    //Alert("Activating client.");
    //jack_activate(xjc);
  }

  void JackDriver::Stop () {
    //Alert("Deactivating client.");
    //jack_deactivate(xjc);
  }

  void JackDriver::StartMainLoop () {
    //Alert("Activating Jack callback.");
    Start();
    owns_main_clock = true; // for the moment
    //if (owns_main_clock) {
    //  MainClock = MainClock;
    //}
    //clock = clock;
  }
    
  int JackDriver::ReadSamples (int achannel, float *samples, int NumSamples) {
    return DAC_WRITE_OK;
  }
  
  int JackDriver::WriteSamples (int achannel, float *samples, int NumSamples) {
    return DAC_WRITE_OK;
  }
  
  static osw::JackDriver *JACKdriver = NULL;

  struct JackDeviceManager : public AudioDeviceManager {

    JackDeviceManager () : AudioDeviceManager("jack",PRIORITY_EXCLUSIVE) {
    }
   
    bool Initialize () {
      cout << "Trying JACK driver..." << endl;
      try {
        JACKdriver = new JackDriver ("audio/0",NULL);
      } catch (osw::Exception e) {
        cout << e.what() << endl;
        JACKdriver = NULL;
        return false;
      }
      return true;
    }

    bool Cleanup () {
      if (JACKdriver) {
	delete JACKdriver;
      }
      return true;
    }

    int GetDriverNames(DriverNameList &driverNames) {
      return 0;
    }

    void PrintOptions() {
      cout << "    " << "name : Client name (default 'osw')" << endl;
      cout << "    " << "inchannels : Number of input channels as an integer, (will be named in_1, in_2, ...)" << endl;
      cout << "    " << "inconnect : List ports to connect input channels, e.g. alsa_pcm:capture_1,alsa_pcm:capture_2,..." << endl;
      cout << "    " << "outchannels : Number of output channels as an integer, (out_1, out_2, ...)" << endl;
      cout << "    " << "outconnect : List ports to connect output channels, e.g. alsa_pcm:playback_1,alsa_pcm:playback_2,..." << endl;
      cout << "    " << "method : Choose method of interaction with jack server, 'detached' (default), or 'inline'" << endl;
    }
 
  };

  static JackDeviceManager TheJackDeviceManager;

}
#endif

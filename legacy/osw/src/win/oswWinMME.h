/*
Copyright (c) 2001-2003 Amar Chaudhary.  All rights reserved.
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
  oswWinMME.h
  Windows MME (Wave) Driver, uses PortAudio
*/

#include "oswAudio.h"
#include "portaudio.h"
#include <windows.h>

#define OKHIWATER 400
#define OKLOWATER 300
#define BADHIWATER 2048
#define BADLOWATER 1024



namespace osw {

  class WinMMEDriver : public AudioDriver {

  public:

    WinMMEDriver (const string &aname, 
                  Container *acontainer,
                  const string &afriendlyName,
                  int ainputDeviceID,
                  int aoutputDeviceID);
    ~WinMMEDriver ();


    int WriteSamples (int achannel, float *asamples, int NumSamples);
    int FlushSamples ();
    int SamplesInBuffer ();
    void Start ();
    void Stop ();
    void StartMainLoop();
    int ReadSamples (int achannel, float *samples, int NumSamples);
    int GetInputSamples ();

    int GetPreferredBufferSize();
    
  protected:

    int AdjustToBufferSize ();
    int AdjustToSampleRate ();

    void DoInputOutputThing();

    bool SetupBuffer();

    bool SetIOBuffers(float *input, float *output) {
      xinput = input;
      xoutput = output;
      // Also, zero output buffer
      memset(xoutput,0,int(bufferSize) * output_channels * sizeof(float));
      return true;
    }

    friend int PaCallback( void *inputBuffer, void *outputBuffer,
                           unsigned long framesPerBuffer,
                           PaTimestamp outTime, void *userData );

    PortAudioStream  *xstream;
    int              xinputDeviceID, xoutputDeviceID;
    float            *xinput, *xoutput;
    HANDLE           hSyncEvent;
  };



} // namespace osw











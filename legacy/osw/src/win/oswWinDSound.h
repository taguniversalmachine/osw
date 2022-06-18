/*
Copyright (c) 1998,1999  Regents of the University of California.
All rights reserved.

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
  oswdsound.h
  DirectSound drivers
*/

#include <windows.h>
#include <dsound.h>
#include "oswAudio.h"


#define OKHIWATER 400
#define OKLOWATER 300
#define BADHIWATER 2048
#define BADLOWATER 1024

// this number was taken right out of the MSDN documentation
#define NUMCAPTUREEVENTS (16)


namespace osw {

  class DirectSoundDriver : public AudioDriver {

  public:

    DirectSoundDriver (const string &aname, Container *acontainer, 
		       LPGUID lpGuidOutput = NULL,
		       LPGUID lpGuidInput = NULL);
    
    ~DirectSoundDriver ();


    int WriteSamples (int achannel, float *asamples, int NumSamples);
    int FlushSamples ();
    int SamplesInBuffer ();
    void Start ();
    void Stop ();
    void StartMainLoop();
    int ReadSamples (int achannel, float *samples, int NumSamples);
    int GetInputSamples ();

    
  protected:

    int AdjustToBufferSize ();
    int AdjustToSampleRate ();

    int xwritePos, xbytesInSampleFrame, xbigBufferSize, xplayPos;
    byte *xcurrentBuffer;
    byte *xinputBuffer;
    int prevPlayCursor;
    int xreadPos, xbytesInInputFrame, xbigInputSize;
    int xprevReadCursor;
    DWORD dwReadPos;
    void SetupBuffer();
    void SetupInputBuffer();
    bool xreadSamples;

    LONG   volume;
    LPDIRECTSOUND lpds;
    LPDIRECTSOUNDBUFFER lpDsb;
    DSBUFFERDESC dsbdesc; 
    HWND         hWnd;
    LPDIRECTSOUNDCAPTURE lpdsc;
    LPDIRECTSOUNDCAPTUREBUFFER lpDscb;
    DSCBUFFERDESC               dscbdesc;
    LPDIRECTSOUNDNOTIFY         lpdsNotify;
    HANDLE                      rghEvent[NUMCAPTUREEVENTS];
    DSBPOSITIONNOTIFY           rgdscbpn[NUMCAPTUREEVENTS];
  };


  /* class DirectSoundCaptureDriver : public ADCDriver {

  public:

    DirectSoundCaptureDriver (const string &aname, Container *acontainer, 
		       int achannels = 1, int asampleRate = 44100,
		       int abitsPerSample = 16, LPGUID lpGuid = NULL);
    
    ~DirectSoundCaptureDriver ();

    virtual Samples ReadSamples (int achannel);
    virtual int FlushSamples ();
    virtual void Start ();
    virtual void Stop ();

  protected:

    int AdjustToBufferSize ();
    int AdjustToSampleRate ();

    int xreadPos, xbytesInSampleFrame, xbigBufferSize;
    byte *xcurrentBuffer;

    void SetupBuffer();

    LPDIRECTSOUNDCAPTURE lpds;
    LPDIRECTSOUNDCAPTUREBUFFER lpDsb;
    HWND         hWnd;
    LPDIRECTSOUNDNOTIFY         lpdsNotify;
    HANDLE                      rghEvent[NUMCAPTUREEVENTS];
    DSBPOSITIONNOTIFY           rgdscbpn[NUMCAPTUREEVENTS];
  };
  */

} // namespace dsound











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
  oswWinDSound.cppexception
  DirectSound drivers
*/

#include "oswWinDSound.h"

#define BUFFERSIZE  64

/********************/

extern void HandleAudioInput ();

static void ThrowDSoundError (HRESULT errnum) {
  switch (errnum) {
  case DSERR_ALLOCATED : throw (osw::Exception("Device in use"));
  case DSERR_INVALIDPARAM : throw (osw::Exception("Invalid Parameter"));
  case DSERR_NOAGGREGATION : throw (osw::Exception("No Aggregation(?!)"));
  case DSERR_NODRIVER : throw (osw::Exception("No driver"));
  case DSERR_OUTOFMEMORY : throw (osw::Exception("Go buy some more memory!"));
  case DSERR_UNINITIALIZED : throw (osw::Exception("Device uninitialized"));
  case DSERR_UNSUPPORTED : throw (osw::Exception("Unsupported"));
  default : throw(osw::Exception("Unknown DSound Error"));
  }   
}

/*******************/

namespace osw {

  FILE *foofile;

DWORD WINAPI  DSoundMainSchedule (LPVOID driverAsVoid);

DirectSoundDriver::DirectSoundDriver (const string &aname, 
				      Container *acontainer, 
				      LPGUID lpGuidOutput,
				      LPGUID lpGuidInput) :
  AudioDriver (aname,acontainer) {
 
  //output_channels = achannels;
  //sampleRate.SetWithoutActivating(asampleRate);
  //bitsPerSample = abitsPerSample;
  xwritePos = int(hiWater) + loWater;
  xplayPos = prevPlayCursor = 0;
  xcurrentBuffer = NULL;
  lpds = NULL;
  lpdsc = NULL;
  lpDsb = NULL;
  lpDscb = NULL;
  sampleRate.SetWithoutActivating((Integer) GetDefaultSampleRate());

  HRESULT result = DirectSoundCreate (lpGuidOutput,&lpds,NULL);
  if (result != DS_OK) {
    ThrowDSoundError(result);
  }
  
  hWnd = GetForegroundWindow();
  if (hWnd == NULL) {
    hWnd = GetDesktopWindow();
  }
  
  SetupBuffer();

  xhasInput = false;
  xinputBuffer = NULL;
  if (lpGuidInput != NULL) {  
    Alert(Name() + ": has input\n");
    HRESULT result = DirectSoundCaptureCreate (lpGuidInput,&lpdsc,NULL);
    if (result != DS_OK) {
      if (lpds) {
	lpds->Release();
      }
      ThrowDSoundError(result);
    }
    SetupInputBuffer();
    xhasInput = true;
    xreadSamples = true;
  }

  RegisterInputChannels();
  RegisterOutputChannels();
}


void DirectSoundDriver::SetupBuffer() {

  HRESULT result;
  WAVEFORMATEX waveFormat;

  DSCAPS caps;
  caps.dwSize = sizeof(caps);
  lpds->GetCaps(&caps);

  // set the device to its maximum channels & bits 
  // (sample rate can be dynamically changed, so leave it alone)
  // (oops we can't change this in DirectSound - damn them!)

  //cout << Name() << ": Min rate " << caps.dwMinSecondarySampleRate
  //     << ", Max rate " << caps.dwMaxSecondarySampleRate << endl;

  if (caps.dwFlags & DSCAPS_PRIMARY16BIT || 
      caps.dwFlags & DSCAPS_SECONDARY16BIT) {
    bitsPerSample = 16;
  } else {
    bitsPerSample = 8;
  }
  char outbuf[120];
  sprintf(outbuf,"%d bits per sample.",(int) bitsPerSample);
  Alert(outbuf);

  if (caps.dwFlags & DSCAPS_PRIMARYSTEREO || 
      caps.dwFlags & DSCAPS_SECONDARYSTEREO) {
    output_channels = 2;
  } else {
    output_channels = 1;
  }
  sprintf (outbuf,"%d output channels.",(int) output_channels);
  Alert(outbuf);

  //sampleRate.SetWithoutActivating(44100);

  waveFormat.wFormatTag = WAVE_FORMAT_PCM;
  waveFormat.nChannels = int(output_channels);
  waveFormat.nSamplesPerSec = int(sampleRate);
  waveFormat.wBitsPerSample = int(bitsPerSample);
  waveFormat.nBlockAlign = (bitsPerSample / 8) * output_channels;
  waveFormat.nAvgBytesPerSec = waveFormat.nBlockAlign * sampleRate;
  
  xbytesInSampleFrame = waveFormat.nBlockAlign;
  if (xcurrentBuffer) {
    delete xcurrentBuffer;
  }
  xcurrentBuffer = new byte [int(bufferSize)*output_channels 
			      * xbytesInSampleFrame];
  memset(xcurrentBuffer,0,
         int(bufferSize)*output_channels * xbytesInSampleFrame);
  memset(&dsbdesc, 0, sizeof(DSBUFFERDESC));
  dsbdesc.dwSize = sizeof(DSBUFFERDESC); 
  
  
  if (caps.dwFlags & DSCAPS_EMULDRIVER) {
    
    Alert("Emulated DirectSound Driver\n");
    
    result = lpds->SetCooperativeLevel (hWnd,DSSCL_EXCLUSIVE);
    if (result != DS_OK) {
      ThrowDSoundError(result);
    }
    
    dsbdesc.dwFlags = DSBCAPS_CTRLPOSITIONNOTIFY 
      | DSBCAPS_GETCURRENTPOSITION2
      | DSBCAPS_CTRLVOLUME
      | DSBCAPS_STICKYFOCUS
      ;
    dsbdesc.dwBufferBytes = waveFormat.nAvgBytesPerSec;
    xbigBufferSize = dsbdesc.dwBufferBytes / xbytesInSampleFrame;
    dsbdesc.lpwfxFormat = &waveFormat;

    if (lpDsb) {
      lpDsb->Release();
    }
    lpDsb = NULL;
    result = lpds->CreateSoundBuffer(&dsbdesc,&lpDsb,NULL);
    if (result != DS_OK) {
      ThrowDSoundError(result);
    }
    
    hiWater = BADHIWATER;
    loWater = BADLOWATER;
  } else {
    
    result = lpds->SetCooperativeLevel (hWnd,DSSCL_WRITEPRIMARY);
    if (result != DS_OK) {
      ThrowDSoundError(result);
    }      
    Alert("Primary Mode Driver\n");
    
    dsbdesc.dwFlags = DSBCAPS_PRIMARYBUFFER
      | DSBCAPS_GETCURRENTPOSITION2
      | DSBCAPS_CTRLVOLUME
      | DSBCAPS_STICKYFOCUS
      ;
    
    dsbdesc.dwBufferBytes = 0;
    dsbdesc.lpwfxFormat = NULL;
    
    result = lpds->CreateSoundBuffer(&dsbdesc,&lpDsb,NULL);
    if (result != DS_OK) {
      ThrowDSoundError(result);
    }
    
    WAVEFORMATEX waveFormat2;
    lpDsb->GetFormat(&waveFormat2,sizeof(waveFormat2),NULL);

    sprintf (outbuf,"%d samples/sec, %d bits/sample, format %d",
	     waveFormat2.nSamplesPerSec,
	     waveFormat2.wBitsPerSample,
	     waveFormat2.wFormatTag);
    Alert(outbuf);

    result = lpDsb->SetFormat(&waveFormat);
    if (result != DS_OK) {
      ThrowDSoundError(result);
    }
    
    DSBCAPS dsbcaps;
    dsbcaps.dwSize = sizeof(DSBCAPS);
    lpDsb->GetCaps(&dsbcaps);
    xbigBufferSize = dsbcaps.dwBufferBytes / xbytesInSampleFrame;
    sprintf(outbuf,"Primary Buffer is %d bytes.",xbigBufferSize);
    Alert(outbuf);
    hiWater = OKHIWATER;
    loWater = OKLOWATER;
  }    

  xwritePos = int(hiWater) + loWater;
}

DirectSoundDriver::~DirectSoundDriver () {
  Stop();
  if (lpds) {
    lpds->Release();
  }
  if (lpdsc) {
    lpdsc->Release();
  }
}

/***********************/

void DirectSoundDriver::Start () {
  //foofile = fopen("foo.raw","wb");
  LONG volume;
  void *ptr1,*ptr2;
  lpDsb->GetVolume(&volume);
  lpDsb->SetVolume(DSBVOLUME_MIN);
  
  // Get the output buffer and fill it with zeros
  DWORD size1,size2;



  lpDsb->Play(0,0,DSBPLAY_LOOPING);
  lpDsb->SetCurrentPosition(0);
  lpDsb->Lock(0,xbigBufferSize,&ptr1,&size1,&ptr2,&size2,DSBLOCK_ENTIREBUFFER);
  memset(ptr1,0,size1);
  memset(ptr2,0,size2);
  lpDsb->Unlock(ptr1,size1,ptr2,size2);
  if (xhasInput) {
    lpDscb->GetCurrentPosition(NULL,&dwReadPos);
    //dwReadPos -= bufferSize * xbytesInInputFrame;
    //dwReadPos %= xbigInputSize;
    xprevReadCursor = dwReadPos;
    lpDscb->Start( DSCBSTART_LOOPING );
  }
  Sleep(300);
  lpDsb->SetVolume(volume);
}

void DirectSoundDriver::Stop () {    
  lpDsb->Stop();
  if (xhasInput) {
    lpDscb->Stop();
  }
  //fclose(foofile);
}

int DirectSoundDriver::FlushSamples () {
  void *ptr1,*ptr2;
  DWORD size1,size2;

  //cerr << "Begin Flush\n";
  lpDsb->Lock(xwritePos * xbytesInSampleFrame,int(bufferSize)*xbytesInSampleFrame,&ptr1,&size1,&ptr2,&size2,0);

  int i = 0;
  short *sample;

  for (sample = (short *) ptr1; sample < (short *)((char *)ptr1 + size1); ++sample) {

    *sample = xcurrentBuffer[i] + (xcurrentBuffer[i+1] << 8);
    i += 2;
  }
  if (ptr2 != NULL) {
    for (sample = (short *) ptr2; sample < (short *)((char *)ptr2 + size2); ++sample) {
      *sample = xcurrentBuffer[i] + (xcurrentBuffer[i+1] << 8);
      
      i += 2;
    }
  }

  //cout << xwritePos << ' ' << i << ' ' << size1 << ' ' << size2 << ' ' << (size1 + size2) << endl;

  lpDsb->Unlock(ptr1,size1,ptr2,size2);
  xwritePos += (int) bufferSize;
  if (xwritePos >= xbigBufferSize) {
    xwritePos -= xbigBufferSize;
    xplayPos -= xbigBufferSize;
  }
  //fwrite (xcurrentBuffer,bufferSize,xbytesInSampleFrame,foofile);
  memset (xcurrentBuffer,0,int(bufferSize) * int(output_channels) * sizeof(short));
  
  //cerr << "End Flush\n";
  return DAC_WRITE_OK;
}


int DirectSoundDriver::WriteSamples (int achannel, float *samples, int NumSamples) {

int i,j;
 
  switch (int(bitsPerSample)) {
  case 16 :
    for (i = 0, j = achannel * 2; i < NumSamples; ++i, j += output_channels * 2) {
      short sampleVal = short(32760.0f * samples[i]);
      xcurrentBuffer[j] += sampleVal & 0xFF;
      xcurrentBuffer[j+1] += (sampleVal >> 8) & 0xFF;
    }
    break;
  case 32 :
    for (i = 0, j = achannel * 4; i < NumSamples; ++i, j += output_channels * 4) {
      long sampleVal = xcurrentBuffer[j] 
	+ (xcurrentBuffer[j+1] << 8)
	+ (xcurrentBuffer[j+2] << 16)
	+ (xcurrentBuffer[j+3] << 24)
	+ long(2147483648.0 * samples[i]);
      xcurrentBuffer[j] = sampleVal & 0xFF;
      xcurrentBuffer[j+1] = (sampleVal >> 8) & 0xFF;
      xcurrentBuffer[j+2] = (sampleVal >> 16) & 0xFF;
      xcurrentBuffer[j+3] = (sampleVal >> 24) & 0xFF;

    }
    break;
  case 24 :
    for (i = 0, j = achannel * 3; i < NumSamples; ++i, j += output_channels * 3) {
      long sampleVal = xcurrentBuffer[j] 
	+ (xcurrentBuffer[j+1] << 8)
	+ (xcurrentBuffer[j+2] << 16)
	+ long(8388608.0 * samples[i]);
      xcurrentBuffer[j] = sampleVal & 0xFF;
      xcurrentBuffer[j+1] = (sampleVal >> 8) & 0xFF;
      xcurrentBuffer[j+2] = (sampleVal >> 16) & 0xFF;
    }
    break;
  }
  return DAC_WRITE_OK;
}

int DirectSoundDriver::ReadSamples (int achannel, float *samples, int NumSamples) {

  int i,j;
  //cout << "R " << achannel << ' ' << input_channels << ' ' << NumSamples << endl;

  switch (int(bitsPerSample)) {
  case 16 :
    for (i = 0, j = achannel * 2; i < NumSamples; ++i, j += input_channels * 2) {
#if 1
      samples[i] = float(xinputBuffer[j] 
      			 + ((signed char)xinputBuffer[j+1] << 8)) 
	/ 32768.0f;	
#else
      samples[i] = (achannel) ? -0.5 : 0.5;
#endif
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
  xreadSamples = true;
  return DAC_WRITE_OK;
}

int DirectSoundDriver::GetInputSamples () {
    
  void *ptr1,*ptr2;
  DWORD size1,size2;
  

  //cout << "Begin Input\n";

  //cout << "X " << dwCapPosition << ' ' << xprevReadCursor << ' ' << dwReadPos << endl;
  lpDscb->Lock(xprevReadCursor,bufferSize*xbytesInInputFrame,&ptr1,&size1,&ptr2,&size2,0);
  
  int i = 0;

  byte *sample;
  for (sample = (byte *) ptr1; sample < ((byte *)ptr1 + size1); ++sample) {
    
    xinputBuffer[i] = *sample;
    ++i;
  }
  if (ptr2 != NULL) {
    for (sample = (byte *) ptr2; sample < ((byte *)ptr2 + size2); ++sample) {
      xinputBuffer[i] = *sample;
      
      ++i;
    }
  }
  
  //cerr << xreadPos << endl;
  //cout << xwritePos << ' ' << i << ' ' << size1 << ' ' << size2 << ' ' << (size1 + size2) << endl;
  

  
  lpDscb->Unlock(ptr1,size1,ptr2,size2);
  dwReadPos += bufferSize * xbytesInInputFrame;
  dwReadPos %= xbigInputSize;
  xprevReadCursor += bufferSize * xbytesInInputFrame;
  xprevReadCursor %= xbigInputSize;
  //cout << "End Input\n";
  xreadSamples = false;
  return DAC_WRITE_OK;
}

int DirectSoundDriver::SamplesInBuffer () {
  DWORD playCursor;
  lpDsb->GetCurrentPosition (&playCursor,NULL);
  playCursor /= xbytesInSampleFrame;
  if (playCursor < prevPlayCursor) {
    xplayPos += playCursor - prevPlayCursor + xbigBufferSize;
  } else {
    xplayPos += playCursor - prevPlayCursor;
  }
  
  prevPlayCursor = playCursor;
  return xwritePos - xplayPos;
}

int DirectSoundDriver::AdjustToBufferSize () {
  Stop();
  if (xcurrentBuffer != NULL) {
    delete xcurrentBuffer;
	xcurrentBuffer = NULL;
  }

  SetupBuffer ();
  if (xhasInput) {
    SetupInputBuffer();
  }
  Start();
  return int(bufferSize);
}

int DirectSoundDriver::AdjustToSampleRate () {
  Stop();

  SetupBuffer ();
  
  Start();
  return 1;
}

void DirectSoundDriver::StartMainLoop () {

  Start();
  cerr << "Starting dac " << PathName () << endl;
  
  owns_main_clock = true; // for the moment
  
  if (owns_main_clock) {
    Alert("Advancing Main Clock");
    StartMainClock();
  }
  Alert("Advancing Driver Clock\n");
  clock = clock;

}

  
/*********************/


void DirectSoundDriver::SetupInputBuffer() {
    
    HRESULT result;
    DSCCAPS caps;

    memset(&dscbdesc, 0, sizeof(DSCBUFFERDESC));
    dscbdesc.dwSize = sizeof(DSCBUFFERDESC); 
    caps.dwSize = sizeof(caps);
    lpdsc->GetCaps(&caps);
    
    WAVEFORMATEX waveFormat;
    lpDsb->GetFormat(&waveFormat,sizeof(waveFormat),NULL);

    input_channels.SetWithoutActivating(waveFormat.nChannels);
    cout << Name() << ":" << input_channels << " input channels.\n";

    xbytesInInputFrame = waveFormat.nBlockAlign;
    if (xinputBuffer) {
      delete xinputBuffer;
    }
    xinputBuffer = new byte [int(bufferSize)* xbytesInInputFrame];
    
    
    if (caps.dwFlags & DSCCAPS_EMULDRIVER) {
      Alert("Emulated DirectSoundCapture Driver");
    } else {
      Alert("WDM Capture Driver");
    }    
    
    DSCAPS dcaps;
    dcaps.dwSize = sizeof(caps);
    lpds->GetCaps(&dcaps);
    dscbdesc.dwFlags = 0;
    dscbdesc.dwSize = sizeof(DSCBUFFERDESC);
    dscbdesc.dwBufferBytes = bufferSize * xbytesInInputFrame * NUMCAPTUREEVENTS;
    //dscbdesc.dwBufferBytes = waveFormat.nAvgBytesPerSec;
    dscbdesc.dwReserved = 0;
    xbigInputSize = dscbdesc.dwBufferBytes;
    dscbdesc.lpwfxFormat = &waveFormat;
    if (lpDscb) {
	lpDscb->Release();
    }
    lpDscb = NULL;

    result = lpdsc->CreateCaptureBuffer(&dscbdesc,&lpDscb,NULL);
    if (result != DS_OK) {
      if (lpds) {
	lpds->Release();
      }
      if (lpdsc) {
	lpdsc->Release();
      }
      ThrowDSoundError(result);
      //input_channels.SetWithoutActivating(0);
      //xhasInput = false;
      //return;
    }
    
    cout << Name () << ": Buffer size is now " << dscbdesc.dwBufferBytes << endl;
    xreadPos = 0;
}
  /********************************/


  struct Guids {
    
    LPGUID inputID,outputID;
    
    Guids () {
      inputID = outputID = NULL;
    }
    
  };
  
  static std::vector<AudioDriver *> drivers;
  
  static map<string,Guids,less<string> > driverMap;
  static int inputDriverCounter = 0;
  static list<String> s_driverNames;
  
  BOOL CALLBACK DSCEnumCallback (LPGUID lpGuid, LPCSTR lpcstrDescription, 
				 LPCSTR lpcstrModule, LPVOID lpContext) {
    
    cout << lpcstrDescription << ' ';
    if ( strstr(lpcstrDescription,"emulated") != NULL || 
	 strstr(lpcstrDescription,"Primary") != NULL) {
      cout << " (Skipping...)\n";
    } else {
      driverMap[lpcstrDescription].inputID = lpGuid;
      cout << endl;
    }    
    ++inputDriverCounter;
    return TRUE;
  }
  
  
  BOOL CALLBACK DSEnumCallback (LPGUID lpGuid, LPCSTR lpcstrDescription, 
				LPCSTR lpcstrModule, LPVOID lpContext) {
    cout << lpcstrDescription << ' ';
    if (strstr(lpcstrDescription,"emulated") != NULL ||
	strstr(lpcstrDescription,"Primary") != NULL) {
      cout << " (Skipping...)\n";
      return TRUE;
    }
    
    driverMap[lpcstrDescription].outputID = lpGuid;
    
    return TRUE;
  }
  
  
  BOOL CALLBACK DSEnumCallbackList (LPGUID lpGuid, LPCSTR lpcstrDescription, 
				LPCSTR lpcstrModule, LPVOID lpContext) {
    if (strstr(lpcstrDescription,"emulated") != NULL ||
	strstr(lpcstrDescription,"Primary") != NULL) {
      return TRUE;
    }
    DriverNameList *driverNames = (DriverNameList *) lpContext;
    driverNames->push_back(lpcstrDescription);
    return TRUE;
  }

  struct DSoundDeviceManager : public AudioDeviceManager {

    DSoundDeviceManager () :
      AudioDeviceManager("dsound") {
    }
    
    bool Initialize () {
      cout << "Detecting DirectSound output drivers...\n";
      DirectSoundEnumerate (DSEnumCallback,NULL);
      cout << "Detecting DirectSound input drivers...\n";
      DirectSoundCaptureEnumerate (DSCEnumCallback,NULL);
      
      char buffer[32];
      int devNum = 0;
      
      for (map<string,Guids,less<string> >::iterator index = driverMap.begin();
	   index != driverMap.end();
	   ++index) {
	cout << index->first << endl;
	try {
	  sprintf(buffer,"audio/%d",devNum);
	  drivers.push_back(new osw::DirectSoundDriver(buffer,NULL,
						       index->second.outputID,
						       index->second.inputID));
	} catch (exception e) {
	  cout << e.what() << endl;
	  continue;
	}
	(*(drivers.end()-1))->bufferSize = GetDefaultBufferSize();
	++devNum;
      }
      return true;
    }

    bool Cleanup () {
      for (int i = 0; i < drivers.size(); ++i) {
	if (drivers[i] != NULL) {
	  while (drivers[i]->DriverIsOn()) {
	    drivers[i]->StopTransform();
	  }
	  delete drivers[i];
	}
      }
      return true;
    }

    int GetDriverNames (DriverNameList &driverNames) {
       DirectSoundEnumerate (DSEnumCallbackList,&driverNames);
       return driverNames.size();
    }

  };

  static DSoundDeviceManager TheDSoundDeviceManager;
  
}



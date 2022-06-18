/*
(c) 2001-2003 Amar Chaudhary
(c) 1997-2001 Regents of the University of California

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
  oswWinMidi.cpp
  MIDI drivers for Windows
*/

/*
  The Circular buffer stuff is shamelessly ripped off from Pd
  Copyright (c) 1997-1999 Miller Puckette.
  Below is the License distributed with the Pd distribution 
  The provisions below apply only to this file and other files displaying them.

This software is copyrighted by Miller Puckette and others.  The following
terms apply to all files associated with the software unless explicitly
disclaimed in individual files.

The authors hereby grant permission to use, copy, modify, distribute,
and license this software and its documentation for any purpose, provided
that existing copyright notices are retained in all copies and that this
notice is included verbatim in any distributions. No written agreement,
license, or royalty fee is required for any of the authorized uses.
Modifications to this software may be copyrighted by their authors
and need not follow the licensing terms described here, provided that
the new terms are clearly indicated on the first page of each file where
they apply.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY
FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES
ARISING OUT OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY
DERIVATIVES THEREOF, EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES,
INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE
IS PROVIDED ON AN "AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE
NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR
MODIFICATIONS.

RESTRICTED RIGHTS: Use, duplication or disclosure by the government
is subject to the restrictions as set forth in subparagraph (c) (1) (ii)
of the Rights in Technical Data and Computer Software Clause as DFARS
252.227-7013 and FAR 52.227-19.


*/

#include "oswWinMidi.h"
#include "oswSched.h"


/*************************/

void CALLBACK midiOutProc(
  HMIDIOUT hmo,      
  UINT wMsg,         
  DWORD dwInstance,  
  DWORD dwParam1,    
  DWORD dwParam2     
  ) {
  if (wMsg == MOM_DONE) {
    MIDIHDR *pMidiHdr = (MIDIHDR *)(dwParam1);
    midiOutUnprepareHeader(hmo,pMidiHdr,sizeof(MIDIHDR));
  }
}



void osw::Win32MidiOutputDriver::OpenDriver() {

  if (midiOutOpen(&xhMidiOut,xport,(DWORD)midiOutProc,reinterpret_cast<DWORD>(this),CALLBACK_FUNCTION) != MMSYSERR_NOERROR) {
    stl::cerr << FullName << ": Could not open device\n";
  }
  stl::cerr << FullName << ": Device started\n";
  xisOpen = true;
}

void osw::Win32MidiOutputDriver::CloseDriver() {

  midiOutReset(xhMidiOut);
  midiOutClose(xhMidiOut);
  xisOpen = false;
}


void osw::Win32MidiOutputDriver::SendMidiMessage (byte channel, byte status,
						  byte first, byte second) {
  union {
    DWORD dwData;         
    BYTE bData[4];     
  } u;  
  
  // Construct the MIDI message.  
  u.bData[0] = channel | (status << 4) ;  // MIDI status byte 
  u.bData[1] = first;                     // first MIDI data byte 
  u.bData[2] = second;                    // second MIDI data byte     
  u.bData[3] = 0;  
  // Send the message.  
  midiOutShortMsg(xhMidiOut, u.dwData); 
}

void osw::Win32MidiOutputDriver::SendMidiSysex (void *buffer, int size) {
  xMidiHdr.lpData = (char *)buffer;
  xMidiHdr.dwBufferLength = size;
  xMidiHdr.dwFlags = 0;

  if (midiOutPrepareHeader(xhMidiOut, &xMidiHdr, sizeof(xMidiHdr))
      != MMSYSERR_NOERROR) {
    Error("Could not prepare MIDI header for sysex message");
    return;
  }
  midiOutLongMsg(xhMidiOut,&xMidiHdr,sizeof(xMidiHdr));
}

void osw::Win32MidiOutputDriver::SendMidiClock () {
  
  union {
    DWORD dwData;         
    BYTE bData[4];     
  } u;  
  
  // Construct the MIDI message.  
  u.bData[0] = 0xF8;  // MIDI Clock Msg 
  u.bData[1] = 0;    
  u.bData[2] = 0;    
  u.bData[3] = 0;  
  // Send the message.  
  midiOutShortMsg(xhMidiOut, u.dwData); 
}

/*************************/



osw::Win32MidiInputDriver::Win32MidiInputDriver (const string &aname, 
						 osw::Container *acontainer,
						 const string &aFullName, 
						 int aport) :
      MidiInputDriver (aname,acontainer,aFullName),
      xport (aport) {
	xisOpen = false;
    }

void osw::Win32MidiInputDriver::OpenDriver() {

  if (midiInOpen(&xhMidiIn,xport,DWORD(::midiInProc),reinterpret_cast<DWORD>(this),CALLBACK_FUNCTION) != MMSYSERR_NOERROR) {
    stl::cerr << FullName << ": Could not open device\n";
  }
  midiInStart (xhMidiIn);
  stl::cerr << FullName << ": Device started\n";
  xisOpen = true;
}

void osw::Win32MidiInputDriver::CloseDriver() {

  midiInReset(xhMidiIn);
  midiInClose(xhMidiIn);
  xisOpen = false;
}

/*************************/

#define INPUT_BUFFER_SIZE 1000     // size of input buffer in events


#define EVNT_F_ERROR    0x00000001L

// note: this structure has a minor change from the original
//   dwInstance instead of dwDevice (we resolve it later)
typedef struct event_tag
{
    DWORD fdwEvent;
    DWORD dwInstance;
    LARGE_INTEGER timestamp;
    DWORD data;
} EVENT;
typedef EVENT FAR *LPEVENT;

/* Structure to manage the circular input buffer.
 */
typedef struct circularBuffer_tag
{
    HANDLE  hSelf;          /* handle to this structure */
    HANDLE  hBuffer;        /* buffer handle */
    WORD    wError;         /* error flags */
    DWORD   dwSize;         /* buffer size (in EVENTS) */
    DWORD   dwCount;        /* byte count (in EVENTS) */
    LPEVENT lpStart;        /* ptr to start of buffer */
    LPEVENT lpEnd;          /* ptr to end of buffer (last byte + 1) */
    LPEVENT lpHead;         /* ptr to head (next location to fill) */
    LPEVENT lpTail;         /* ptr to tail (next location to empty) */
} CIRCULARBUFFER;
typedef CIRCULARBUFFER FAR *LPCIRCULARBUFFER;

static LPCIRCULARBUFFER lpInputBuffer = NULL;

/* Function prototypes
 */

static LPCIRCULARBUFFER AllocCircularBuffer(DWORD dwSize);
static void FreeCircularBuffer(LPCIRCULARBUFFER lpBuf);
static WORD FAR PASCAL GetEvent(LPCIRCULARBUFFER lpBuf, LPEVENT lpEvent);
static void FAR PASCAL PutEvent(LPCIRCULARBUFFER lpBuf, LPEVENT lpEvent);

/*****************************/


void HandleMidiEvents () {
  EVENT event;
  while (GetEvent(lpInputBuffer,&event)) {
    osw::Win32MidiInputDriver *midiInPort = 
      reinterpret_cast<osw::Win32MidiInputDriver *>(event.dwInstance);    
    DWORD dwParam1 = event.data;
    if ((dwParam1 & 0xF0) != 0xF0) {
      midiInPort->FireMidiEvents (dwParam1 & 0xF,(dwParam1 & 0xF0) >> 4,(dwParam1 & 0xFF00) >> 8, (dwParam1 & 0xFF0000) >> 16);
    }
  }
}

/*************************************/

void CALLBACK midiInProc(
			 HMIDIIN hMidiIn,	
			 UINT wMsg,	
			 DWORD dwInstance,	
			 DWORD dwParam1,	
			 DWORD dwParam2	
			 ) {
  EVENT event;
  
  switch(wMsg) {
  case MIM_OPEN:
    break;
    
    /* The only error possible is invalid MIDI data, so just pass
     * the invalid data on so we'll see it.
     */
  case MIM_ERROR:
  case MIM_DATA:
    event.fdwEvent = (wMsg == MIM_ERROR) ? EVNT_F_ERROR : 0;
    event.dwInstance = dwInstance;
    event.data = dwParam1;
    if (dwParam1 && 0xFF == 0xFE) {
      // The odious active-sensing message
      break;
    }
    /* Put the MIDI event in the circular input buffer.
     */    
    PutEvent(lpInputBuffer,&event); 
    // and schedule the event handler
    //theMidiEventHandler->ScheduleMe();
    break;
    
  default:
    break;
  }
}




/*
 * AllocCircularBuffer -    Allocates memory for a CIRCULARBUFFER structure 
 * and a buffer of the specified size.  Each memory block is allocated 
 * with GlobalAlloc() using GMEM_SHARE and GMEM_MOVEABLE flags, locked 
 * with GlobalLock(), and page-locked with GlobalPageLock().
 *
 * Params:  dwSize - The size of the buffer, in events.
 *
 * Return:  A pointer to a CIRCULARBUFFER structure identifying the 
 *      allocated display buffer.  NULL if the buffer could not be allocated.
 */
 
 
static LPCIRCULARBUFFER AllocCircularBuffer(DWORD dwSize)
{
    HANDLE hMem;
    LPCIRCULARBUFFER lpBuf;
    LPEVENT lpMem;
    
    /* Allocate and lock a CIRCULARBUFFER structure.
     */
    hMem = GlobalAlloc(GMEM_SHARE | GMEM_MOVEABLE,
                       (DWORD)sizeof(CIRCULARBUFFER));
    if(hMem == NULL)
        return NULL;

    lpBuf = (LPCIRCULARBUFFER)GlobalLock(hMem);
    if(lpBuf == NULL)
    {
        GlobalFree(hMem);
        return NULL;
    }
    
    /* Page lock the memory.  Global memory blocks accessed by
     * low-level callback functions must be page locked.
     */
    //GlobalSmartPageLock((HGLOBAL)HIWORD(lpBuf));

    /* Save the memory handle.
     */
    lpBuf->hSelf = hMem;
    
    /* Allocate and lock memory for the actual buffer.
     */
    hMem = GlobalAlloc(GMEM_SHARE | GMEM_MOVEABLE, dwSize * sizeof(EVENT));
    if(hMem == NULL)
    {
      //GlobalSmartPageUnlock((HGLOBAL)HIWORD(lpBuf));
        GlobalUnlock(lpBuf->hSelf);
        GlobalFree(lpBuf->hSelf);
        return NULL;
    }
    
    lpMem = (LPEVENT)GlobalLock(hMem);
    if(lpMem == NULL)
    {
        GlobalFree(hMem);
        //GlobalSmartPageUnlock((HGLOBAL)HIWORD(lpBuf));
        GlobalUnlock(lpBuf->hSelf);
        GlobalFree(lpBuf->hSelf);
        return NULL;
    }
    
    /* Page lock the memory.  Global memory blocks accessed by
     * low-level callback functions must be page locked.
     */
    //GlobalSmartPageLock((HGLOBAL)HIWORD(lpMem));
    
    /* Set up the CIRCULARBUFFER structure.
     */
    lpBuf->hBuffer = hMem;
    lpBuf->wError = 0;
    lpBuf->dwSize = dwSize;
    lpBuf->dwCount = 0L;
    lpBuf->lpStart = lpMem;
    lpBuf->lpEnd = lpMem + dwSize;
    lpBuf->lpTail = lpMem;
    lpBuf->lpHead = lpMem;
        
    return lpBuf;
}

/* FreeCircularBuffer - Frees the memory for the given CIRCULARBUFFER 
 * structure and the memory for the buffer it references.
 *
 * Params:  lpBuf - Points to the CIRCULARBUFFER to be freed.
 *
 * Return:  void
 */
static void FreeCircularBuffer(LPCIRCULARBUFFER lpBuf)
{
    HANDLE hMem;
    
    /* Free the buffer itself.
     */
    //GlobalSmartPageUnlock((HGLOBAL)HIWORD(lpBuf->lpStart));
    GlobalUnlock(lpBuf->hBuffer);
    GlobalFree(lpBuf->hBuffer);
    
    /* Free the CIRCULARBUFFER structure.
     */
    hMem = lpBuf->hSelf;
    //GlobalSmartPageUnlock((HGLOBAL)HIWORD(lpBuf));
    GlobalUnlock(hMem);
    GlobalFree(hMem);
}

/* GetEvent - Gets a MIDI event from the circular input buffer.  Events
 *  are removed from the buffer.  The corresponding PutEvent() function
 *  is called by the low-level callback function, so it must reside in
 *  the callback DLL.  PutEvent() is defined in the CALLBACK.C module.
 *
 * Params:  lpBuf - Points to the circular buffer.
 *          lpEvent - Points to an EVENT structure that is filled with the
 *              retrieved event.
 *
 * Return:  Returns non-zero if successful, zero if there are no 
 *   events to get.
 */
static WORD FAR PASCAL GetEvent(LPCIRCULARBUFFER lpBuf, LPEVENT lpEvent)
{
  /* If no event available, return */
    if (lpBuf->dwCount <= 0) return (0);
    
    /* Get the event.
     */
    *lpEvent = *lpBuf->lpTail;
    
    /* Decrement the byte count, bump the tail pointer.
     */
    --lpBuf->dwCount;
    ++lpBuf->lpTail;
    
    /* Wrap the tail pointer, if necessary.
     */
    if(lpBuf->lpTail >= lpBuf->lpEnd)
        lpBuf->lpTail = lpBuf->lpStart;

    return 1;
}

/* PutEvent - Puts an EVENT in a CIRCULARBUFFER.  If the buffer is full, 
 *      it sets the wError element of the CIRCULARBUFFER structure 
 *      to be non-zero.
 *
 * Params:  lpBuf - Points to the CIRCULARBUFFER.
 *          lpEvent - Points to the EVENT.
 *
 * Return:  void
*/

static void FAR PASCAL PutEvent(LPCIRCULARBUFFER lpBuf, LPEVENT lpEvent)
{
    /* If the buffer is full, set an error and return. 
     */
    if(lpBuf->dwCount >= lpBuf->dwSize){
        lpBuf->wError = 1;
        return;
    }
    
    /* Put the event in the buffer, bump the head pointer and the byte count.
     */
    *lpBuf->lpHead = *lpEvent;
    
    ++lpBuf->lpHead;
    ++lpBuf->dwCount;

    /* Wrap the head pointer, if necessary.
     */
    if(lpBuf->lpHead >= lpBuf->lpEnd)
        lpBuf->lpHead = lpBuf->lpStart;
}

/*************************/

static osw::Container *MidiInputs;
static stl::list<osw::Win32MidiInputDriver *> MidiInDrivers;
static osw::Container *MidiOutputs;
static stl::list<osw::Win32MidiOutputDriver *> MidiOutDrivers;

static void CreateMidiInDrivers () {

  char portNumStr[16];
  MidiInputs = new osw::Container("midiIn",NULL);

  int numPorts = midiInGetNumDevs();
  new osw::State<int> 
    ("count",MidiInputs,"Number of MIDI input devices.",numPorts);
  MIDIINCAPS   caps;

  cout << "Detected Midi Input Drivers:\n";
  for (int i = 0; i < numPorts; ++i) {
    midiInGetDevCaps(i,&caps,sizeof(caps));
    sprintf (portNumStr,"%d",i);
    MidiInDrivers.push_back(new osw::Win32MidiInputDriver(portNumStr,
							  MidiInputs,
							  caps.szPname,
							  i));
    cout << '\t' << portNumStr << ": " << caps.szPname << endl;
  }
}

static void DestroyMidiInDrivers () {
  for (stl::list<osw::Win32MidiInputDriver *>::iterator driver = MidiInDrivers.begin();
       driver != MidiInDrivers.end();
       ++driver) {
    delete *driver;
  }
  delete MidiInputs;
}

void CreateMidiOutDrivers () {

  char portNumStr[16];
  MidiOutputs = new osw::Container("midiOut",NULL);

  int numPorts = midiOutGetNumDevs();
  new osw::State<int> 
    ("count",MidiOutputs,"Number of MIDI output devices.",numPorts);
  MIDIOUTCAPS   caps;

  cout << "Detected Midi Output Drivers:\n";
  for (int i = 0; i < numPorts; ++i) {
    midiOutGetDevCaps(i,&caps,sizeof(caps));
    sprintf (portNumStr,"%d",i);
    MidiOutDrivers.push_back(new osw::Win32MidiOutputDriver(portNumStr,
							    MidiOutputs,
							    caps.szPname,
							    i));
    cout << '\t' << portNumStr << ": " << caps.szPname << endl;
  }
}

void DestroyMidiOutDrivers () {
  for (stl::list<osw::Win32MidiOutputDriver *>::iterator driver = MidiOutDrivers.begin();
       driver != MidiOutDrivers.end();
       ++driver) {
    delete *driver;
  }
  delete MidiOutputs;
}

namespace osw {
  void InitMidi () {
    CreateMidiInDrivers();
    CreateMidiOutDrivers();
    lpInputBuffer = AllocCircularBuffer((DWORD)(INPUT_BUFFER_SIZE));
    if (lpInputBuffer == NULL)
      {
        printf("Not enough memory available for input buffer.\n");
        return;
    }
  }
  
  void CleanupMidi () {
    DestroyMidiInDrivers();
    DestroyMidiOutDrivers();
    if (lpInputBuffer){
      FreeCircularBuffer(lpInputBuffer);
    }
  }
}

/********************/

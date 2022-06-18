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
  wininit.cpp
  Windows-specific initialization and cleanup
*/

#include <windows.h>
#include <mmsystem.h>
#include "oswSched.h"
#include "oswThread.h"
#include "oswInit.h"
#include "oswMidi.h"
#include "oswAudio.h"
#include "oswSock.h"
#include "oswLoadOSX.h"
#include "oswOSCTypes.h"
#include "oswFileUtils.h"

using namespace std;


/****************/

void osw::PreInit() {}

int osw::Init () {
 
  // Do this first
  InitInit();

  // next, load any Driver OSX's
  const char *osw_dir = osw::GetOSWDir();

  if (osw_dir) {
    cout << osw_dir << endl;
    string OSWPathName = osw_dir;

    OSWPathName += "/externals/drivers/";
    
    HANDLE findHandle;
    WIN32_FIND_DATA findFileData;
    bool cont = false;
    string findString = OSWPathName + "*.osx";
    findHandle = FindFirstFile(findString.c_str(), &findFileData);
    if (findHandle != INVALID_HANDLE_VALUE) {
      cont = true;
    }
    while (cont) {
      char *nullPoint = strchr(findFileData.cFileName,'.');
      if (nullPoint) {
        *nullPoint = '\0';
      }
      Debug("Loading " + string(findFileData.cFileName));
      LoadOSX((OSWPathName+findFileData.cFileName).c_str());      
      cont = FindNextFile(findHandle, &findFileData);
    }
    FindClose(findHandle);
    OSWPathName = osw_dir;
    OSWPathName += "/externals/types/";
    
    cont = false;
    findString = OSWPathName + "*.osx";
    findHandle = FindFirstFile(findString.c_str(), &findFileData);
    if (findHandle != INVALID_HANDLE_VALUE) {
      cont = true;
    }
    while (cont) {
      char *nullPoint = strchr(findFileData.cFileName,'.');
      if (nullPoint) {
        *nullPoint = '\0';
      }
      Debug("Loading " + string(findFileData.cFileName));
      LoadOSX((OSWPathName+findFileData.cFileName).c_str());      
      cont = FindNextFile(findHandle, &findFileData);
    }
    FindClose(findHandle);
  }

  osw::InitParallelScheduler ();  
  osw::InitUDPServers();
  osw::InitMidi();
  osw::InitAudioDrivers();
  osw::InitializeOSCServer();
  return 0;
}

int osw::Cleanup () {
  osw::StopThreads();
  osw::CleanupAudioDrivers();
  osw::CleanupMidi();
  osw::RunCleanupHandlers();
  return 0;
}


void osw::MainLoop () {
  osw::InitializeThreads(true);
}

/***********************/




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
  oswLinuxInit.cpp
  Linux-specific initialization and cleanup
*/


#include "oswSched.h"
#include "oswThread.h"
#include "oswInit.h"
#include "oswAudio.h"
#include "oswMidi.h"
#include "oswSock.h"
#include "oswLoadOSX.h"
#include "oswOSCTypes.h"
#include "oswSched.h"
#include <sched.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <X11/Xlib.h>

using namespace std;



/*****************/
namespace osw {

  void PreInit() {
    // Load Audio Drivers...
    if(getenv("OSW_DIR") == NULL) {
      Alert("ERROR: OSW_DIR is not defined.");
      exit(1);
      return;
    }
    string path(getenv("OSW_DIR"));
    path += "/externals/drivers/";
    DIR *ddir = opendir(path.c_str());
    struct dirent *file;
    string dfile;
    while(file = readdir(ddir)) {
      dfile = file->d_name;
      if(dfile > ".osx") {
	dfile = dfile.replace(dfile.find(".osx"), 4, "");
	LoadOSX(("drivers/" + dfile).c_str());
      }
    }
    closedir(ddir);
  }

  int Init () {
    // Do this first
    InitInit();

    if (mlockall(MCL_CURRENT | MCL_FUTURE) != 0) {
      perror("Hog mode: couldn't lock memory with mlockall");
    }

    osw::InitAudioDrivers();
    osw::InitUDPServers();
    osw::InitMidi();
    osw::InitParallelScheduler ();
    osw::InitializeOSCServer();


    return 0;
  }
  
  int Cleanup () {
    osw::StopThreads();
    osw::CleanupAudioDrivers();
    osw::CleanupMidi();
    osw::RunCleanupHandlers();
    exit(0);
  }

}

void osw::MainLoop () {
  osw::InitializeThreads(true);
}

/***********************/

#ifdef _STLP_DEBUG
#include "../src/aligned_buffer.h"
namespace _STLD {
  istream cin(0);
  ostream cout(0);
  ostream cerr(0);
}
#endif

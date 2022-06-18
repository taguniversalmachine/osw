
/* 
Copyright (c) 2001 Amar Chaudhary. All rights reserved.
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
   oswSelect.h
   Abstractions for asynchronous event handlers (aka "select")

   Amar Chaudhary
*/

#ifndef _OSWSELECT
#define _OSWSELECT

#include "oswCfg.h"
#ifdef OSW_NO_STDC_HEADERS
#include <string.h>
#include <stdio.h>
#else
#include <cstring>
#include <cstdio>
#endif 

#if defined(__WIN32__) || defined(_WINDOWS)
#include <winsock.h>
// "GetMessage" is a macro in Windows, which causes problems (AC)
#undef GetMessage
#else
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/times.h>
#include <sys/param.h>
#include <sys/time.h>
#include <ctype.h>
#include <pwd.h>
#include <signal.h>
#include <grp.h>
#include <sys/file.h>
#endif

#ifndef OSW_NO_STDC_HEADERS
using namespace std;
#endif

namespace osw {

  struct SelectEventManager {
    fd_set read_fds, write_fds;
    int nfds;

    void Reset () {
      FD_ZERO(&read_fds);     /* clear read_fds        */
      FD_ZERO(&write_fds);    /* clear write_fds        */
      nfds = 0;
    }
    void AddRead(int handle) {
      FD_SET(handle,&read_fds);
      nfds = (handle >= nfds) ? handle+1 : nfds;
    }
    void AddWrite(int handle) {
      FD_SET(handle,&write_fds);
      nfds = (handle >= nfds) ? handle+1 : nfds;
    }
    int IsSetRead(int handle) {
      return FD_ISSET(handle,&read_fds);
    }
    int IsSetWrite(int handle) {
      return FD_ISSET(handle,&write_fds);
    }
  };
  

  void _oswexport AddSelectGroup(void (*setupfunc)(SelectEventManager &eventManager),
				 void (*dispatchfunc)(SelectEventManager &eventManager));
  void _oswexport CheckSelectEvents ();
  void _oswexport BlockingCheckSelectEvents ();
  void _oswexport SelectEventsDispatch();

}

#endif



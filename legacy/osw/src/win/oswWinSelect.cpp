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
   oswWinSelect.cpp
   Win32 implementation of asynchronous event handlers
     (on this OS, "select" is just used for sockets)

   Amar Chaudhary
*/

#include "osw.h"
#include "oswSelect.h"
#include <vector>

using namespace std;

namespace osw {

  static SelectEventManager TheSelectEventManager;

  typedef void (*eventmgrproc)(SelectEventManager &eventManager);
  typedef std::vector<eventmgrproc> EventManagerProcList;

  static EventManagerProcList SetupProcs,DispatchProcs;

  static timeval tv = {0L,0L};
  static timeval btv = {0L,0L};

  /***********************/

  void _oswexport AddSelectGroup(eventmgrproc setupproc,
				 eventmgrproc dispatchproc) {
    SetupProcs.push_back(setupproc);
    DispatchProcs.push_back(dispatchproc);
  }

  void _oswexport CheckSelectEvents () {

    int i;
    int n = SetupProcs.size();

    TheSelectEventManager.Reset();
    for (i = 0; i < n; ++i) {
      (*SetupProcs[i])(TheSelectEventManager);
    }
    
    int r = select(TheSelectEventManager.nfds,
		   &TheSelectEventManager.read_fds,
		   &TheSelectEventManager.write_fds,
		   (fd_set *) NULL, 
		   &tv);   
    if (r < 0) {
      //cerr << "udp: Oh Shit " << nfds << ' ' << r << ' ' << h_errno << endl;
      return;
    }
    
    for (i = 0; i < n; ++i) {
      (*DispatchProcs[i])(TheSelectEventManager);
    }
    
  }
  
  
  void _oswexport BlockingCheckSelectEvents () {
    
    int i;
    int n = SetupProcs.size();

    TheSelectEventManager.Reset();
    for (i = 0; i < n; ++i) {
      (*SetupProcs[i])(TheSelectEventManager);
    }

	btv.tv_sec = 0;  // Only for a while...
	btv.tv_usec = 100;

    int r = select(TheSelectEventManager.nfds,
		   &TheSelectEventManager.read_fds,
		   &TheSelectEventManager.write_fds,
		   (fd_set *) NULL, 
		   &btv);
    if (r < 0) {
      //Error("A horrific error has occured when calling select().");
      return;
    }
    
  }
  
  void _oswexport SelectEventsDispatch() {
    
    int i;
    int n = SetupProcs.size();
    
    for (i = 0; i < n; ++i) {
      (*DispatchProcs[i])(TheSelectEventManager);
    }
    
  }
}


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
/* oswActivation.cpp
   Classes for OSW Activation expressions
*/

#include "osw.h" // includes oswActivation.h and other stuff we need
#include "oswSched.h"

namespace osw {


  void BaseActivation::proc () {
    
    //NOTE: This method may be called with a NULL value for "this"
    //Indeed, this method is implemented specifically for that case.
    //So deference this or access members here!
    
    Error("Defunct activation expression called");
  }

  /*********************/
  
  osw::BaseActivation::~BaseActivation () {
  
    for (std::vector<osw::BaseState *>::iterator p = xactivators.begin(); p != xactivators.end(); ++p) {
      if (*p != NULL) {
	(*p)->UnregisterActivation(this);
      } 
    }
  }
  
  
  void osw::BaseActivation::AddActivator (osw::BaseState *aactivator) {
    xactivators.push_back(aactivator);
    xstates.push_back(!xon);
    aactivator->RegisterActivation(this);
    ++xcount;
  }
  
  int osw::BaseActivation::RemoveActivator (osw::BaseState *aactivator, 
					    bool compress) {
    int result = -1;
    for (int i = 0; i < xactivators.size(); ++i) {
      if (aactivator == xactivators[i]) {
	xactivators[i] = NULL;
	aactivator->UnregisterActivation(this);
	result = i;
	if (compress) {
	  xactivators.erase(xactivators.begin()+i);
	  xstates.erase(xstates.begin()+i);
	  xcount = xstates.size();
	  for (int j = 0; j < xstates.size(); ++j) {
	    xstates[j] = !xon;
	  }
	}
	break;
      }
    }
    
    return result;
  }
  
  void osw::BaseActivation::ReplaceActivator (int aindex, BaseState *aactivator) {
    if (aindex <= xactivators.size()) {
      if (xactivators[aindex] != NULL) {
	xactivators[aindex]->UnregisterActivation(this);
      }
      xactivators[aindex] = aactivator;
      xactivators[aindex]->RegisterActivation(this);
    }
  }
  
  bool osw::BaseActivation::SetActivation (osw::BaseState *aactivator,
					   int aparallel) {
    //xsemaphore.Acquire();
    for (int i = 0; i < xactivators.size(); ++i) {
      if (aactivator == xactivators[i]) {
	if (xstates[i] != xon) {
	  xstates[i] = xon;
	  --xcount;
	  if (xcount == 0) {
	    Reset();
	    if (aparallel) {
	      osw::TheParallelScheduler->Schedule(this);
	    } else {
	      proc();
	    }
	  }
	}
	//xsemaphore.Release();
	return true;
      }
    }
    //xsemaphore.Release();
    return false;
  }
  
  
  void osw::BaseActivation::RemoveActivatorFast (osw::BaseState *aactivator) {
    
    // for use by BaseState object when destroying themselves
    for (int i = 0; i < xactivators.size(); ++i) {
      if (aactivator == xactivators[i]) {
	xactivators[i] = NULL;
	break;
      }
    }
  }

}


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
/* oswState.cpp
   Base class for OSW state variables, including inlets and outlets
   for transforms.
*/

#include "osw.h"
#include "oswFreeState.h"

namespace osw {

  BaseState::BaseState (const string &aname, Container *acontainer,
			const string &adesc) : 
    Nameable(aname,acontainer),
    xdesc (adesc) {
    xlock = 0;
    xorder = -1;
    xchangeable = NULL;
    xnoparallel = false;
    xparallel = 0;
    if (acontainer == NULL) {
      GetFreeStateContainer()->AddNameable(this);
    }
  }
  
  BaseState::~BaseState () {
    for (stl::list<BaseActivation *>::iterator p = xactivations.begin(); p != xactivations.end(); ++p) {
      (*p)->RemoveActivatorFast(this);
    }
    if (GetContainer() == NULL) {
      Container *freeStateContainer = GetFreeStateContainer();
      freeStateContainer->RemoveNameable(this);
      CheckFreeStateContainer();
    }
  }
  
  void BaseState::RegisterActivation (BaseActivation *toRegister) {
    
    if (xactivations.empty() || 
	toRegister->Order() >= xactivations.back()->Order()) {
    xactivations.push_back(toRegister);
    } else if (toRegister->Order() <= xactivations.front()->Order()) {
      xactivations.push_front(toRegister);
  } else {
    for (stl::list<BaseActivation *>::iterator activationIndex 
	   = xactivations.begin();
	 activationIndex != xactivations.end();
	 ++activationIndex) {
      if (toRegister->Order() <= (*activationIndex)->Order()) {
	xactivations.insert(activationIndex,toRegister);
	break;
      }
    }
  }
    if (xactivations.size() > 1) {
      xparallel |= 1;
    }
  }
  
  BaseState *(BaseState::Change) (BaseState *changeTo) {
    return xchangeable->Change (changeTo);
  }
  
  ChangeableInlet *(BaseState::SetChangeable) (ChangeableInlet *achangeable) {
    return xchangeable = achangeable;
  }
  
  void BaseState::Connected (BaseOutlet *aconnection) {
  }

  void BaseState::Disconnected (BaseOutlet *aoldConnection) {
  }
  
  _oswexport ostream & operator << (ostream &os, const BaseState &toPrint) {
    return toPrint.Print(os);
  }
  
  _oswexport istream & operator >> (istream &is, BaseState &toInput) {
  return toInput.Input(is);
  }
  
  _oswexport istream & operator >> (istream &is, BaseState *toInput) {
    return toInput->Input(is);
  }
  
 
}

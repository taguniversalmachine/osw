

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
/* oswInletOutlet.cpp
   Method implementations for Inlet and Outlet classes.
*/

#include "osw.h"
#include "oswCast.h"

namespace osw {

  void Outlet<Any>::SetActivations () {
    if (xconnect) {
      if (xconnect->TypeOf() == typeid(Any)) {
	*((State<Any> *)(xconnect)) = xval;
      } else {
	if (xconnect->TypeOf() == xval.TypeOf()) {
	  xconnect->SetFromDatum(xval.GetDatum());
	} else {
	  BaseRegisterCast *cast = FindCast(xval.TypeOf(),xconnect->TypeOf());
	  if (cast == NULL) {
	    Error ("value of " + PathName() 
		   + " is incompatible with the type of "
		   + xconnect->PathName() + ".");
            Error(PathName() + " is " + xval.TypeOf().name() + " , "
                  + xconnect->PathName() + " is " + xconnect->TypeOf().name() );
	  } else {
	    cast->CastFromDatum(*(xval.GetDatum()),*xconnect);
	  }
	}
      }
    }
    State<Any>::SetActivations();
  };

  /******************/
  
  ChangeableInlet::ChangeableInlet (const string &aname,  
				    Transform *atransform,
				    const string &adesc) :
    xinlet (new Inlet<Changeable>(aname,atransform,adesc)) {
    xinlet->SetChangeable(this);
  }
  
  BaseState *(ChangeableInlet::Change) (BaseState *ainlet) {
    Alert (xinlet->Name()+": change type to " + ainlet->TypeOf().name());
    BaseState *newinlet = ainlet->CloneInlet(xinlet->Name(),
					     dynamic_cast<Transform *>
					     (xinlet->GetContainer()),
					     xinlet->Description());
    
    while (xinlet->xactivations.size() > 0) {
      BaseState::ActivationIterator pactivation = xinlet->BeginActivations();
      BaseActivation *activation = *pactivation;
      int varIndex = activation->RemoveActivator(xinlet);
      activation->ReplaceActivator(varIndex,newinlet);
    }
    
    newinlet->SetChangeable(this);
    delete xinlet;
    xinlet = newinlet;
    
    dynamic_cast<Transform *>(xinlet->GetContainer()) -> InletTypeHasChanged(xinlet);
    return xinlet;
  }

  typedef ChangeableInlet::Changeable Changeable;
  REGISTER_TYPE(Changeable);

}

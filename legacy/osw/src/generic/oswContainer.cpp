

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
/* oswContainer.cpp
   Base class for Container components in OSW, including transforms
*/


#include "osw.h" // includes oswContainer.h


namespace osw {
  
  static int constructomatic = 666;

  Container::NameableTable 	*Container::xcontainer_table = NULL;

  Container::Container (const string &aname, Container *acontainer)  :
    Nameable(aname,acontainer) {
    Register(this);
    x_runtime = 0;
  }

  Container::~Container () {    
    for (stl::list<Nameable *>::iterator m = members.begin(); 
         m != members.end(); 
         ++m) {
      if ((*m) != NULL) {
        (*m)->xcontainer = NULL; // avoids second removal
	  delete *m;
      }
    }
    Unregister(this);
    //if(GetContainer() == NULL) {
    //  Container::xtoplevel_table->erase(PathName());
    //}
  }
  
  void Container::Register (Container *toRegister) {
    if (xcontainer_table == NULL || constructomatic != 666) {
      xcontainer_table = new NameableTable;
    }
    //if (xtoplevel_table == NULL || constructomatic != 666) {
    //  xtoplevel_table = new NameableTable;
    //}
    (*xcontainer_table)[toRegister->PathName()] = toRegister;
    //if (toRegister->GetContainer() == NULL) {
    //  (*xtoplevel_table)[toRegister->PathName()] = toRegister;
    //}
  }

  void Container::Unregister (Container *toUnregister) {
    if (xcontainer_table->find(toUnregister->PathName())
	!= xcontainer_table->end()) {
      xcontainer_table->erase(toUnregister->PathName());
      if (xcontainer_table->size() == 0) {
	// No more containers, delete table
	delete xcontainer_table;
	xcontainer_table = NULL;
      }
    }
  }
			       
  
  void Container::Rename (const string &aname) {
    
    Nameable::Rename(aname);
    for (stl::list<Nameable *>::iterator m = members.begin(); m != members.end(); ++m) {
      if ((*m) != NULL) {
	(*m)->Rename((*m)->Name());
      }
    }
  }
  
  Container::enumerator  Container::BeginEnumerate () {
    return xcontainer_table->begin();
  }
  
  bool  Container::EndEnumerate (enumerator e) {
    return e == xcontainer_table->end();
  }

  Container::enumerator  Container::BeginTopLevelEnumerate () {
    return xtoplevel_table->begin();
  }
  
  bool  Container::EndTopLevelEnumerate (enumerator e) {
    return e == xtoplevel_table->end();
  }
  
  int Container::NumTopLevelMembers() {
    return (xtoplevel_table) ? xtoplevel_table->size() : 0;
  }


}

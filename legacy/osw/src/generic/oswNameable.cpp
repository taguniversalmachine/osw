
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
/* oswNameable.cpp
   Implementation of Nameable, base class for all major components in OSW
*/

#include "osw.h"
#include "oswFreeState.h"

namespace osw {


  Nameable::NameableTable   *(Nameable::xtable) = NULL;
  Nameable::NameableTable   *(Nameable::xtoplevel_table) = NULL;

#if OSW_HASH_MAP_TRAITS
  typedef stl::hash_map<String, int > CounterMap;
#else
  typedef stl::hash_map<String, int, stl::hash<String>, stl::equal_to<String> > 
    CounterMap;
#endif

CounterMap *(Nameable::xcounter) = NULL;

  
  static int constructomatic = 666;
  
  Nameable::Nameable(const String& aname, Container *acontainer) :
    xname(aname),
    xcontainer(acontainer),
    xpath(((acontainer != NULL) ? acontainer->PathName() + "/" : "/") + aname) {    
    Register(this);
    if (xcontainer != NULL) {
      xcontainer->AddNameable(this);
    } else {
      AddTopLevelNameable(this);
    }
    if (xcounter == NULL || constructomatic != 666) {
      xcounter = new CounterMap;
    }
    ++(*xcounter)[(String)typeid(*this).name()];
  }
  
  
  Nameable::~Nameable () {
    --(*xcounter)[typeid(*this).name()];
    Unregister (xpath,this);
    if (xcontainer != NULL) {
      xcontainer->RemoveNameable(this);
    } else {
      RemoveTopLevelNameable(this);
    }
  }
  
  void Nameable::RemoveTopLevelNameable (Nameable *toRemove) {
    xtoplevel_table->erase(toRemove->PathName());
    if (xtoplevel_table->size () == 0) {
      delete xtoplevel_table;
      xtoplevel_table = NULL;
    }
  }

  void Nameable::AddTopLevelNameable (Nameable *toAdd) {
    if (xtoplevel_table == NULL) {
      xtoplevel_table = new NameableTable;
    }
    (*xtoplevel_table)[toAdd->PathName()] = toAdd;
  }


  
  int Nameable::GetNumberOfNameables () {
    return (*xtable).size();
  }
  
  int Nameable::GetNumberOfInstances () {
    return (*xcounter)[typeid(*this).name()];
  }
  
  Nameable *(Nameable::Find) (const String& toFind, Container  *acontainer) {
    
    NameableTable::iterator p;
    // replace "$" variables 
    for (size_t n = toFind.find_first_of('$');
         n != string::npos;
         n = toFind.find_first_of('$',n)) {
      size_t endOfVariable = toFind.find_first_of('/',n);
      string varname = toFind.substr(n,endOfVariable-n);
      BaseState *state = dynamic_cast<BaseState *>
        (Find(varname.c_str()+1,acontainer));
      if (state) {
        ostringstream os;
        os << *state;
        return Find(toFind.substr(0,n) + os.str() 
                    + toFind.substr(endOfVariable,string::npos));
      } else {
        Error("Cannot find variable " + varname);
        return NULL;
      }
    }
         
    switch (toFind[0]) {
    case '/' :
      p = (*xtable).find(toFind);
      break;
    case '.' :
      switch (toFind[1]) {
      case '/' :
	if (acontainer != NULL) {
	  p = (*xtable).find(acontainer->PathName()+"/"+String(toFind.c_str()+2));
	} else {
	  return NULL;
	}
	goto testp;
      case '.' :
	if (toFind[2] == '/') {
	  if (acontainer != NULL) {
	    return Find(toFind.c_str() + 3, acontainer->GetContainer());
	  } else {
	    return NULL;
	  }
	}
	// go on to default case
      } 
      // go on to default case
    default:
      if (acontainer != NULL) {
	p = (*xtable).find(acontainer->PathName() + "/" + toFind);
      } else {
	return Find(toFind,GetFreeStateContainer());
      }
    }
    
  testp:
    if (p == (*xtable).end()) {
      return NULL;
    } else {
      return (*p).second;
    }
  };
  
  ostream & Nameable::EnumerateToStream (ostream &os) {
    
    for (enumerator e = BeginEnumerate();
	 !EndEnumerate(e);
	 ++e) {
      os << GetNameFromEnumerator(e) << endl;
    }
    return os;
  }
  
  Nameable::enumerator  Nameable::BeginEnumerate () {
    return (*xtable).begin();
  }
  
  bool  Nameable::EndEnumerate (enumerator e) {
    return e == (*xtable).end();
  }
  
  
  void Nameable::Register (Nameable *toRegister) {
    if (xtable == NULL || constructomatic != 666) {
      xtable = new Nameable::NameableTable;
    }
    (*xtable)[toRegister->PathName()] = toRegister;
  }
  
  
  void Nameable::Unregister (const String& nameToUnregister, 
			     Nameable *objToUnregister) {
    
    if (Find(nameToUnregister) == objToUnregister) {
      (*xtable).erase(nameToUnregister);
      if (xtable->size() == 0) {
	// no more nameables, so delete table resources
	delete xtable;
	xtable = NULL;
	delete xcounter;
	xcounter = NULL;
      }
    }
  }
  
  int Nameable::NumberOfNameables () {
    return (*xtable).size();
  }
  
  void Nameable::Rename (const String &aname) {
    Unregister(xpath,this);
    xpath = 
      ((xcontainer != NULL) ? xcontainer->PathName() + "/" : "/") + aname;
    xname = aname;
    Register(this);
  }
  

}

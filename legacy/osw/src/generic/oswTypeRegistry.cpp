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

/* oswTypeRegistry.cpp
   Implementation of OSW data type registration
*/


#include "osw.h" // includes oswTypeRegistry.h

namespace osw {

  BaseTypeRegistration::TypeTable * BaseTypeRegistration::table = NULL;
  BaseTypeRegistration::ReverseTypeTable * BaseTypeRegistration::rtable = NULL;
  
  const string _oswexport &RegisteredTypeName(const type_info &atype) {
    return (*BaseTypeRegistration::rtable)[atype.name()];
  }
  
  void BaseTypeRegistration::RegisterMe (const string &aname,
					 const string &atypename) {
    //cout << "about to register " << aname << " as " << atypename << endl;
    if (table == NULL) {
      table = new TypeTable;
    }
    if (rtable == NULL) {
      rtable = new ReverseTypeTable;
    }
    
    (*table)[aname] = this;
    if (aname != atypename) {
      (*table)[atypename] = this;
    }
    (*rtable)[atypename] = aname;
    //cout << "Registering " << atypename << " as " << aname << endl;
  }
  
  BaseState *MakeState (const string &atypename, const string &aname,
			Container *acontainer, const string &adesc) {
    BaseTypeRegistration::TypeTable::iterator type 
      = BaseTypeRegistration::table->find(atypename);
    
    if (type == BaseTypeRegistration::table->end()) {
      type = BaseTypeRegistration::table->find("class " + atypename);
      if (type == BaseTypeRegistration::table->end()) {
	type = BaseTypeRegistration::table->find("struct " + atypename);
	if (type == BaseTypeRegistration::table->end()) {
	  return new State<Any>(aname,acontainer,adesc);
	}
      }
    }
    return (*type).second->MakeState(aname,acontainer,adesc);
  }
  
  BaseState *MakeInlet (const string &atypename, const string &aname,
			Transform *atransform, const string &adesc) {
    BaseTypeRegistration::TypeTable::iterator type 
      = BaseTypeRegistration::table->find(atypename);
    
    if (type == BaseTypeRegistration::table->end()) {
      type = BaseTypeRegistration::table->find("class " + atypename);
      if (type == BaseTypeRegistration::table->end()) {
	type = BaseTypeRegistration::table->find("struct " + atypename);
	if (type == BaseTypeRegistration::table->end()) {
	  return new Inlet<Any>(aname,atransform,adesc);
	}
      }
    }
    return (*type).second->MakeInlet(aname,atransform,adesc);
  }
  
  BaseState *MakeOutlet (const string &atypename, const string &aname,
			 Transform *atransform, const string &adesc) {
    BaseTypeRegistration::TypeTable::iterator type 
      = BaseTypeRegistration::table->find(atypename);
    
    if (type == BaseTypeRegistration::table->end()) {
      type = BaseTypeRegistration::table->find("class " + atypename);
      if (type == BaseTypeRegistration::table->end()) {
	type = BaseTypeRegistration::table->find("struct " + atypename);
	if (type == BaseTypeRegistration::table->end()) {
	  return new Outlet<Any>(aname,atransform,adesc);
	}
      }
    }
    return (*type).second->MakeOutlet(aname,atransform,adesc);
  }
  
  BaseDatum *MakeDatum (const string &atypename) {
    BaseTypeRegistration::TypeTable::iterator type 
      = BaseTypeRegistration::table->find(atypename);
    if (type == BaseTypeRegistration::table->end()) {
      return NULL;
    } else {
      return type->second->MakeDatum();
    }
  }

  /******************/
  
  static stl::list<TimeDomainTypeRegistration *> *timeDomainTypeList = NULL;
  
  TimeDomainTypeRegistration::TimeDomainTypeRegistration (const type_info &atype)
    : type(atype) {
    if (timeDomainTypeList == NULL) {
      timeDomainTypeList =
	new stl::list<TimeDomainTypeRegistration *>;
    }
    timeDomainTypeList->push_back(this);
  }
  
  
  bool IsTimeDomainType(const type_info &atype) {
    
    // as long as at least one time-domain type is registered
    //  we shouldn't bother wasting time checking if the list is NULL
    
    bool result = false;
    for (stl::list<TimeDomainTypeRegistration *>::iterator tr = timeDomainTypeList->begin();
	 tr != timeDomainTypeList->end(); ++tr) {
      if ((*tr)->type == atype) {
	result = true;
	break;
      }
    }
    return result;
  }


  /******************/
 
  // Declare basic types
 
  REGISTER_TYPE(Unit);
  REGISTER_TYPE(Any);
  REGISTER_TYPE(Byte);
  REGISTER_TYPE(Integer);
  REGISTER_TYPE(Integer16);
  REGISTER_TYPE(UInteger);
  REGISTER_TYPE(UInteger16);
  REGISTER_TYPE(Float);
  REGISTER_TYPE(Double);
  REGISTER_TYPE(Boolean);
  REGISTER_TYPE(String);
  REGISTER_TYPE(Samples);
  REGISTER_TYPE(IntegerSamples);
  REGISTER_TYPE(Integer16Samples);
  REGISTER_TYPE(ByteSamples);
  REGISTER_TYPE(DoubleSamples);
  REGISTER_TYPE(rawbytes);
  REGISTER_TYPE(Raw);
  REGISTER_TYPE(Char);

  // for pre-1.0 state variable type declarations
  //TypeRegistration<Any> _OSW_ALIAS_REGTYPE_any("any");

  REGISTER_TIMEDOMAIN_TYPE(Samples);
  REGISTER_TIMEDOMAIN_TYPE(IntegerSamples);
  REGISTER_TIMEDOMAIN_TYPE(Integer16Samples);
  REGISTER_TIMEDOMAIN_TYPE(ByteSamples);
  REGISTER_TIMEDOMAIN_TYPE(DoubleSamples);

  /************/

  //! This cleanup handler removes static tables from type registry classes
  BEGIN_CLEANUP_HANDLER(TypeRegistry)
  {
    delete timeDomainTypeList;
    delete BaseTypeRegistration::rtable;
    delete BaseTypeRegistration::table;

  }
  END_CLEANUP_HANDLER(TypeRegistry)
}


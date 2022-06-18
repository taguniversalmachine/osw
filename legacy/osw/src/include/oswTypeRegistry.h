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

/* oswTypeRegistry.h
   Classes and Macros for registering OSW data types
*/

#ifndef _OSW_TYPEREGISTRY_H
#define _OSW_TYPEREGISTRY_H

namespace osw {


  struct _oswexport BaseTypeRegistration {

    virtual ~BaseTypeRegistration() {}

#ifdef OSW_HASH_MAP_TRAITS    
    typedef stl::hash_map<string, BaseTypeRegistration *> TypeTable;
    typedef stl::hash_map<string,string> ReverseTypeTable;
#else
    typedef stl::hash_map<string, BaseTypeRegistration *, stl::hash<string>, stl::equal_to<string> > TypeTable;
    typedef stl::hash_map<string,string,stl::hash<string>,stl::equal_to<string> > ReverseTypeTable;
#endif
    static TypeTable *table;
    static ReverseTypeTable *rtable;

    void RegisterMe (const string &aname, const string &atypename);

    virtual BaseState *MakeState (const string &aname, Container *acontainer,
				  const string &adesc = "") = 0;
    virtual BaseState *MakeInlet (const string &aname, Transform *transform,
				  const string &adesc = "") = 0;
    virtual BaseState *MakeOutlet (const string &aname, Transform *atransform,
				   const string &adesc = "") = 0;
    virtual BaseDatum *MakeDatum  () = 0;
  };

  _oswexport const string &RegisteredTypeName(const type_info &atype);
  
  template <class T> 
    struct _oswexport TypeRegistration : public BaseTypeRegistration {
      
      TypeRegistration () {
	RegisterMe(typeid(T).name(),typeid(T).name());
      }
	
      TypeRegistration (const string &aname) {
	RegisterMe(aname,typeid(T).name());
      }

      ~TypeRegistration () {
      }
      
      BaseState *MakeState (const string &aname, Container *acontainer,
			    const string &adesc = "") {
	return new State<T> (aname,acontainer,adesc);
      }

      BaseState *MakeInlet (const string &aname, Transform *atransform,
			    const string &adesc = "") {
	return new Inlet<T> (aname,atransform,adesc);
      }
      
      BaseState *MakeOutlet (const string &aname, Transform *atransform,
			    const string &adesc = "") {
	return new Outlet<T> (aname,atransform,adesc);
      }

      BaseDatum *MakeDatum () {
	return new Datum<T>;
      }
    };

  _oswexport BaseState *MakeState (const string &atypename, const string &aname,
			Container *acontainer, const string &adesc = "");
  _oswexport BaseState *MakeInlet (const string &atypename, const string &aname,
			Transform *atransform, const string &adesc = "");
  _oswexport BaseState *MakeOutlet (const string &atypename, const string &aname,
			Transform *atransform, const string &adesc = "");

  _oswexport BaseDatum *MakeDatum (const string &atypename);

  #define REGISTER_TYPE(T) static TypeRegistration<T> _OSW_REGTYPE_##T(#T)

  /******************/

  // TimeDomainTypeRegistration : declares that a type is a
  // "Time-domain type."  Connections of these types are
  // used to propigate sample rate and sample-vector size info
  // among connected transforms.
  

  struct _oswexport TimeDomainTypeRegistration {
    const type_info &type;

    TimeDomainTypeRegistration (const type_info &atype = typeid(void *));
  };

  bool IsTimeDomainType (const type_info &atype);

  #define REGISTER_TIMEDOMAIN_TYPE(T) static TimeDomainTypeRegistration \
    _OSW_REGTDTYPE_##T (typeid(T))

}

#endif // _OSW_TYPEREGISTRY_H


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
/* oswContainer.h
   Base class for Container components in OSW, including transforms
*/

#ifndef _OSW_CONTAINER_H
#define _OSW_CONTAINER_H

#include "oswNameable.h"

namespace osw {

  class _oswexport BaseState;
  class _oswexport BaseOutlet;
  
  class _oswexport Container : public Nameable {
    
  public:

    Container (const string &aname, Container *acontainer = NULL);
    virtual ~Container ();

    virtual void Rename (const string &aname);

    virtual const char *Dialog () {
      return "";
    }
    virtual const char *Menu () {
      return "";
    }

    void Error (const string &msg) {
      osw::Error (PathName() + ": " + msg);
    }

    void Alert (const string &msg) {
      osw::Alert (PathName() + ": " + msg);
    }

    static NameableTable *xcontainer_table;

    static enumerator BeginEnumerate ();
    static bool EndEnumerate (enumerator e);
    static Nameable *GetNameableFromEnumerator (enumerator &e) {
      return e->second;
    }
    static const string & GetNameFromEnumerator (enumerator &e) {
      return e->first;
    }

    static int NumTopLevelMembers();
    static enumerator BeginTopLevelEnumerate ();
    static bool EndTopLevelEnumerate (enumerator e);
    static Container *GetContainerFromTopLevelEnumerator (enumerator &e) {
      return dynamic_cast<Container *>(e->second);
    }
    static const string & GetNameFromTopLevelEnumerator (enumerator e) {
      return e->first;
    }

    static void Register (Container *toRegister);
    static void Unregister (Container *toUnregister);

    typedef stl::list<Nameable *>::iterator MemberIterator;

    int NumMembers() { return members.size(); }
    MemberIterator BeginMembers () {return members.begin();}
    bool EndMembers(MemberIterator &toTest) {return toTest == members.end();}
    MemberIterator &NextMember (MemberIterator &toIncr) {return ++toIncr;}
    Nameable *MemberSelf(MemberIterator &toSelect) {return *toSelect;}

  protected:

    friend class _oswexport Nameable;
    friend class _oswexport BaseState;

    virtual void AddNameable (Nameable *toAdd) {
      members.push_back(toAdd);
    }
    
    virtual void RemoveNameable (Nameable *toRemove) {
      members.erase(remove(members.begin(),members.end(),toRemove),members.end());
    }

    stl::list<Nameable *> members;

  public:

    int x_runtime;
  };

}

#endif //  _OSW_CONTAINER_H

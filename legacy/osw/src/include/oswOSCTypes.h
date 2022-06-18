/* 
Copyright (c) 2001-2002 Amar Chaudhary. All rights reserved.
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

/* oswOSCTypes.h
   Implementation of OpenSound Control (OSC) type conversion services,
   and other OSC related bits   
*/

#ifndef _OSWOSCTYPES_H
#define _OSWOSCTYPES_H

#include <typeinfo>
#include "OSC-client.h"
#include "oswList.h"
#include "osw.h"

#define STRING_ALIGN_PAD 4

using namespace std;

namespace osw {

  struct _oswexport BaseDeclareOSCTypeTag {
    BaseDeclareOSCTypeTag(const type_info &atype, const string &atypetag);
  };
  
  template<class T>
  struct DeclareOSCTypeTag : public BaseDeclareOSCTypeTag {
    DeclareOSCTypeTag (const string &atypetag) :
      BaseDeclareOSCTypeTag(typeid(T),atypetag) {
    }
  };

#define DECLARE_OSC_TYPETAG(type,tag) static DeclareOSCTypeTag<type> dosctt##type(tag)

  _oswexport const char *GetTypeTagFromState(BaseState *state);
  _oswexport const char *GetTypeTagFromDatum(BaseDatum *datum);
  /*
  struct _oswexport BaseOSCConverter {
    BaseOSCConverter (const type_info &atype);
    virtual void StateToOSC(OSCBuf *buf, BaseState *state);
  };
  */

  // move these into methods of OSCParseTransform?
  _oswexport void StateToOSC(OSCbuf *buf, BaseState *state);
  _oswexport void DatumToOSC(OSCbuf *buf, BaseDatum *datum);
  //_oswexport BaseDatum *OSCToDatum (char *&p, char *base, int n, char *&typetag);

  class OSCParser {

  public:

    virtual void ReadOSCMessage(char *, char *);
    virtual char *DataAfterAlignedString(char *data, char *boundary);
    virtual void ParseOSCPacket(char *, char *);
    virtual BaseDatum *OSCToDatum(char *&p, char *&typetag, char *boundary);
    virtual void MessageActivate(char *, char *, char *, char *);

    virtual ~OSCParser();

  };

  // An OSCMessage (thinly disguised as a list!)
  class OSCMessage : public List {
 
  };  // TODO: Not needed?
  
  _oswexport String OSCStripLastName(const string &address);
  
  _oswexport void InitializeOSCServer();

}

#endif

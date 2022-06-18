/* 
Copyright (c) 2001-2002 Andrew Schmeder, Amar Chaudhary All rights reserved.
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

 */
 
#ifndef _OSW_INTERFACE_H
#define _OSW_INTERFACE_H

#include "osw.h"

namespace osw {

  bool IsValidNameable(const string &path);
  bool IsValidState(const string &path);
  bool IsValidPatch(const string &path);
  bool IsValidTransform(const string &path);
  bool IsValidInlet(const string &path);
  bool IsValidOutlet(const string &path);
  bool IsValidInletOrOutlet(const string &path);
  bool IsValidDataSource(const string &path);

  bool IsValidExtendedRootQuery(const string &path);
  bool IsValidExtendedMethodQuery(const string &path);
  String ExtendedMethodQueryType(const string &address);
  String RootQueryType(const string &address);
  String ExtendedMethodQueryDocumentation(const string &address);
  String RootQueryDocumentation(const string &address);

  namespace StateInterface {
    void Rename(const string &name, const string &new_name);
    List *Effects(const string &name);
    List *CoActivators(const string &name);
  }
  
  namespace TransformInterface {
    void Rename(const string &name, const string &new_name);
    List *Inlets(const string &name);
    List *Outlets(const string &name);
    String GetData(const string &name);
    void SetData(const string &name, const string &data);
    String PersistentState(const string &name);
  }
  
  namespace PatchInterface {
    String Create(const string &parent, const string &xtype, const string &name, int argc, char *argv[]);
    void Destroy(const string &name);
    void AppendArg(const string &name, const string &argname, int value);
    void CreatePatch(const string &name, const string &parent);
  }
  
  namespace OutletInterface {
    void Connect(const string &from, const string &to);
    void Disconnect(const string &from, const string &to);
  }
  
  namespace RootInterface {
    bool IsAutoNameable(const string &name);
    List *TransformTypes();
    void Restart();
    void Quit();
    void LoadOSX(const string &name);
  }
  
}

#endif

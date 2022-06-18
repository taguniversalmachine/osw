/* 
Copyright (c) 2001-2003 Amar Chaudhary. All rights reserved.
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
*/
/* oswAny.cpp
   The "Any" variant type for OSW variables
*/

#include "osw.h"
#include "oswList.h"

namespace osw {

  // Really slimy implementation if Any streaming input using Lists

  _oswexport istream &operator >> (istream &is, Any &toInput) {
    List xlist;
    is >> xlist;
    if (xlist.size() == 1) {
      toInput = xlist.nth(0);
    } else {
      toInput = new Datum<List>(xlist);
    }
    return is;
  }

#ifdef OSW_DEBUG
  //! in debug mode, # of datums (data) currently allocated
  State<Integer> *s_num_data = NULL;

  //! in debug mode, increment the # of datums
  void _oswexport IncrDatumCount() {
    if (!s_num_data) {
      s_num_data = new State<Integer>("num_data",NULL,"Number of datums (data) allocated in OSW",0);
    }
    ++(*s_num_data);
#ifdef OSW_SUPER_DEBUG
    cout << "Data Count: " << *s_num_data << endl;
#endif
  }

  //! in debug mode, decrement the # of datums
  void _oswexport DecrDatumCount() {
    --(*s_num_data);
#ifdef OSW_SUPER_DEBUG
    cout << "Data Count: " << *s_num_data << endl;
#endif
  }
#endif // OSW_DEBUG

} // namespace osw

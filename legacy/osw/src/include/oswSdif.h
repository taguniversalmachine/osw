
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


/* oswsdif.h

   Header for use by transforms that use sdif

   */

#ifndef _OSW_SDIF
#define _OSW_SDIF

#include "sdif.h"
#include "sdif-mem.h"

namespace osw {

  struct  SDIF_Frame : public SDIFmemFrameStruct {

    void * operator new (size_t unused) {
      return SDIFmem_CreateEmptyFrame();
    }

    void operator delete (void *toDelete) {
      SDIFmem_FreeFrame((SDIF_Frame *) toDelete);
    }

    bool operator < (const SDIF_Frame &toCompare) const {
      return header.time < toCompare.header.time;
    }    

    friend ostream & operator << (ostream &os, const SDIF_Frame &toPrint) {
      os << "SDIF Frame: Type " << toPrint.header.frameType[0] 
         << toPrint.header.frameType[1] 
         << toPrint.header.frameType[2] 
         << toPrint.header.frameType[3]
         << ", size " << toPrint.header.size 
         << ", time " << toPrint.header.time
         << ", ID " << toPrint.header.streamID
         << ", " << toPrint.header.matrixCount << "matrices"
         << endl;
    }

    friend istream operator >> (istream &is, SDIF_Frame &toInput) {
      // doesn't support input
    }
  };

  
  struct SDIF_Stream {

    SDIF_Frame *first;
    SDIF_Frame *last;

    SDIF_Stream () {
      first = last = NULL;
    }

    ~SDIF_Stream () {
      for (stream_map::iterator index = xmap.begin();
	   index != xmap.end();
	   ++index) {
	delete index->second;
      }
    }

    void push_back (SDIF_Frame *toPush) {
      if (last == NULL) {
	first = last = toPush;
	toPush->prev = toPush->next = NULL;
      } else {
	last->next = toPush;
	toPush->prev = last;
	toPush->next = NULL;
	last = toPush;
      }
      xmap[toPush->header.time] = toPush;
    }

    SDIF_Frame * operator [] (double time) {
      stream_map::iterator found = xmap.find(time);
      return (found != xmap.end()) ? found->second : NULL;
    }

    SDIF_Frame * upper_bound (double time) {
      stream_map::iterator found = xmap.upper_bound(time);
      return (found != xmap.end()) ? found->second : NULL;
    }

    SDIF_Frame * lower_bound (double time) {
      stream_map::iterator found = xmap.lower_bound(time);
      return (found != xmap.end()) ? found->second : NULL;
    }

    protected:

      typedef stl::map<double,SDIF_Frame *,less<double> > stream_map;
      stream_map xmap;
  };


  _oswexport SDIF_Stream *GetSDIFStreamByName (const string &name, Container *acontainer);
  
  // required for all OSW data types
  _oswexport istream & operator >> (istream &is, SDIF_Stream *&toInput);
}

#endif // _OSW_SDIF


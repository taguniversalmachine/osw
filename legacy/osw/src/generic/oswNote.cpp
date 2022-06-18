/* 
Copyright (c) 2001 Amar Chaudhary. All rights reserved.
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
   oswNote.cpp
   Implementation of note type for musical events

   Amar Chaudhary
*/

#include "oswNote.h"
#include "oswCast.h"

namespace osw {

  REGISTER_TYPE(Note);
  REGISTER_CAST(Note,Blob);
  REGISTER_CAST(Blob,Note);

  REGISTER_TYPE(Score);

  static struct {
    char pclass;
    char degree;
  } pitchClasses [] = {
    {'c',0},{'d',2},{'e',4},{'f',5},{'g',7},{'a',9},{'b',11},{0,0}
  };

  static struct {
    char type;
    float dur;
  } durationTypes [] = {
    {'l',4.0f},{'b',2.0f},{'w',1.0f},{'h',0.5f},{'q',0.25f},{'e',0.125f},
    {'s',0.0625f},{'t',0.03125f},{'x',0.015625f},{0,0.0f}
  };


  int _oswexport GetNoteNumFromString(const string &s) {

    int pitch = -1;
    for (int i = 0; pitchClasses[i].pclass > 0; ++i) {
      if (s[0] == pitchClasses[i].pclass) {
	pitch = pitchClasses[i].degree;
	break;
      }
    }
    if (pitch < 0) {
      return pitch;
    }
    int p = 1;
    if (s[p] == 'f' || s[p] == 'b') {
      --pitch;
      ++p;
    } else if (s[p] == 's' || s[p] == '#') {
      ++pitch;
      ++p;
    }

    pitch += 12 + 12 * atoi(s.c_str()+p);

    return pitch;
  }

  float _oswexport GetDurationFromString(const string &s) {
    float dur = -1.0f;
    for (int i = 0; durationTypes[i].type > 0; ++i) {
      if (s[0] == durationTypes[i].type) {
	dur = durationTypes[i].dur;
	break;
      }
    }
    if (dur < 0.0f) {
      return dur;
    }

    float dot = 0.0f;
    for (int p = 1; p < s.length(); ++p) {
      if (s[p] == '.') {
	dot += dur * pow(0.5,double(p));
      }
    }
    dur += dot;
    return dur;
  }

}


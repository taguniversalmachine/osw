
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
   oswNote.h
   Note and Score types for musical events

   Amar Chaudhary
*/

#ifndef NOTE_H
#define NOTE_H

#include "oswBlob.h"


namespace osw {


  class _oswexport Note : public Blob {

  public:

    Note (float aduration = 0.0f, float anext = 0.0f) {
      add("duration",new Datum<float>(aduration));
      add("next",new Datum<float>(anext));
      xduration = xdata.find("duration");
      xnext = xdata.find("next");
      xpitch = xdata.end();
    }

    Note (const Note &toCopy) :
      Blob(toCopy) {
      xduration = xdata.find("duration");
      xnext = xdata.find("next");
      xpitch = xdata.find("pitch");
    } 
    
    explicit Note (const Blob &toCopy) :
      Blob(toCopy) {
      xduration = xdata.find("duration");
      xnext = xdata.find("next");
      xpitch = xdata.find("pitch");
    } 
    
    Note & operator = (const Note &toAssign) {
      (Blob &)(*this) = toAssign;
      xduration = xdata.find("duration");
      xnext = xdata.find("next");
      xpitch = xdata.find("pitch");
      return *this;
    }
    
    float duration () const {
      return *((Datum<float> *)(xduration->second));
    }
    
    float next () const {
      return *((Datum<float> *)(xnext->second));
    }
    
    BaseDatum *pitch () const {
      return (xpitch != xdata.end()) ? (xpitch->second) : NULL;
    }
    
    float duration (float aduration) {
      add("duration",new Datum<float>(aduration));
      xduration = xdata.find("duration");
      return aduration;
    }
    
    float duration (BaseDatum *aduration) {
      add("duration",aduration);
      xduration = xdata.find("duration");
      return -1.0f;
    }

    float next (float anext) {
      add("next",new Datum<float>(anext));
      xnext = xdata.find("next");
      return anext;
    }

    float next (BaseDatum *anext) {
      add("next",anext);
      xnext = xdata.find("next");
      return -1.0f;
    }

    BaseDatum *pitch(BaseDatum *apitch) {
      add("pitch",apitch);
      xpitch = xdata.find("pitch");
      return apitch;
    }

    

    friend ostream & operator << (ostream &os, const Note &toOutput) {
      os << "<Note ";
      toOutput.print_stuff(os);
      os << ">";
      return os;
    }

  protected:

    const_iterator xduration,xnext,xpitch;

  };


  class Score {

    struct ScoreShare {

      typedef list<Note> list_type;
      typedef list_type::iterator iterator;
      typedef list_type::const_iterator const_iterator;

      ScoreShare() {
	xref = 1;
      }

      void AddRef() const {
	++xref;
      }

      void RemoveRef() const {
	--xref;
	if (xref <= 0) {
	  delete this;
	}
      }

      list_type note_list;
      
    private:

      mutable short  xref;
    };


  public:

    typedef ScoreShare::iterator iterator;
    typedef ScoreShare::const_iterator const_iterator;

    Score () {
      xshare = new ScoreShare;
    }

    Score (const Score &toCopy) {
      xshare = toCopy.xshare;
      xshare->AddRef();
    }

    ~Score() {
      xshare->RemoveRef();
    }

    Score & operator = (const Score &toAssign) {
      if (toAssign.xshare) {
	toAssign.xshare->AddRef();
      }
      if (xshare) {
	xshare->RemoveRef();
      }
      xshare = toAssign.xshare;
      return *this;
    }

    iterator begin() {return xshare->note_list.begin();}
    iterator end() {return xshare->note_list.end();}
    const_iterator begin() const {return xshare->note_list.begin();}
    const_iterator end() const {return xshare->note_list.end();}

    void push_back(const Note &toAdd) {
      xshare->note_list.push_back(toAdd);
    }
    void push_front(const Note &toAdd) {
      xshare->note_list.push_front(toAdd);
    }
    void clear() {
      xshare->note_list.erase(xshare->note_list.begin(),xshare->note_list.end());
    }

    bool operator == (const Score &toCompare) const {
      return xshare == toCompare.xshare;
    }

    friend ostream & operator << (ostream &os, const Score &toPrint) {
      return os << "<Score>";
    }

    friend istream & operator >> (istream &is, Score &toInput) {
      return is;
    }

  private:

    ScoreShare *xshare;
  };

  // some helpful functions for dealing with notes

  #define BADPITCH (-1)
  #define BADDURATION (-1.0f)

  int _oswexport GetNoteNumFromString(const string &s);
  float _oswexport GetDurationFromString(const string &s);


}



#endif



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

/*
  oswPatternMatchatch.cpp
  Implementation of the "PatternMatch" function, with locates state variables
  that match a particular OSC-style pattern.
  Uses pattern matching code from the OpenSound Control (OSC) Kit by
    Matthew Wright.
*/

#include "osw.h"
#include "oswPatternMatch.h"
#include "oswFreeState.h"
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
#include "OSC-common.h"
#include "OSC-pattern-match.h"

namespace osw {

  static char *NextSlashOrNull(char *p) {
    while (*p != '/' && *p != '\0') {
      p++;
    }
    return p;
  }
  
  static int CountSlashes(char *p) {
  	int i = 0;
  	while(*p != '\0') {
  	  if(*p == '/') {
  	  	i++;
  	  }
  	  p++;
  	}
  	return i;
  }
  
  static void MatchSubContainer(char *pattern, Container *c, StateList &result) {
    char *nextSlash, *restOfPattern;

    nextSlash = NextSlashOrNull(pattern);
    
    if (*nextSlash == '\0') {
      /* Base case: the pattern names methods of this container. */
      for (Container::MemberIterator members = c->BeginMembers();
	   !c->EndMembers(members);
	   c->NextMember(members)) {
	BaseState *state = dynamic_cast<BaseState *>(c->MemberSelf(members));
	if (state && PatternMatch(pattern,state->Name().c_str())) {
	  result.push_back(state);
	}
      }
    } else {
      /* Recursive case: in the middle of an address, so the job at this 
	 step is to look for containers that match.  We temporarily turn
	 the next slash into a null so pattern will be a null-terminated
	 string of the stuff between the slashes. */
      *nextSlash = '\0';
      restOfPattern = nextSlash + 1;
      
      for (Container::MemberIterator members = c->BeginMembers();
	   !c->EndMembers(members);
	   c->NextMember(members)) {
	Container *child = dynamic_cast<Container *>(c->MemberSelf(members));
	if (child && PatternMatch(pattern,child->Name().c_str())) {
	  MatchSubContainer(restOfPattern, child, result);
	}
      }
      *nextSlash = '/';
    }    
  }

  void MatchStates (char *pattern, Container *c, StateList &result) {
    if (c) {
      MatchSubContainer(pattern,c,result);
    } else {
      char *nextSlash = NextSlashOrNull(pattern);
      if (*nextSlash == '/') {
	*nextSlash = '\0';
	char *restOfPattern = nextSlash + 1;
	for (Container::enumerator toplevels = Container::BeginTopLevelEnumerate();
	     !Container::EndTopLevelEnumerate(toplevels);
	     ++toplevels) {
	  Container *toplevel = Container::GetContainerFromTopLevelEnumerator(toplevels);
	  
	  if (toplevel && PatternMatch(pattern,toplevel->Name().c_str())) {
	    MatchSubContainer(restOfPattern,toplevel,result);
	  }
	}
	*nextSlash = '/';
      } else {
	MatchSubContainer(pattern,GetFreeStateContainer(),result);
      }
    }
  }
  
  /*
   * Return a list of Nameables matched by an OSC address pattern.
   */
  void MatchNameables (const string &pattern_string, NameableList &result) {
  	
    // OSC functions require char* instead of string (ugh!)
    char *pattern = (char*)(pattern_string.c_str());
    
    // Count the slashes found in pattern
    int i = CountSlashes(pattern);

    // Examine all Nameables for pattern matching...
    for(Nameable::enumerator things = Nameable::BeginEnumerate(); !Nameable::EndEnumerate(things); ++things) {
      Nameable *thing = Nameable::GetNameableFromEnumerator(things);
      
      // Cast that babyee...
      char* checkname = (char*)(const_cast<string&>(thing->PathName()).c_str());

      // PatternMatch incorrectly globs the "/" when * is used.  i==j check fixes this.
      if(i == CountSlashes(checkname) && PatternMatch(pattern, checkname)) {
        result.push_back(thing);
      }
    }
  }

  Boolean MatchesPattern(const string &test_address, const string &test_pattern) {
    char *address = (char*)(test_address.c_str());
    char *pattern = (char*)(test_pattern.c_str());
  	return PatternMatch(pattern, address);
  }
  
}

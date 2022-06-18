
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
   oswscan.h
   Functions for interpreting string arguments to transforms
   
   Amar Chaudhary
*/

#ifndef _OSWSCAN
#define _OSWSCAN

#include "oswLex.h"

namespace osw {

  template <class T>
    T IntepretArgument (char *arg, T aval, Container *acontainer) {
    Token *token = GetToken(arg);
    if (token->type() == TOKEN_VARIABLE) {
      State<T> *state = dynamic_cast<State<T> *>
	(Nameable::Find(((VariableToken *)(token))->val,acontainer));
      if (state == NULL) {
	Error (String("could not find ") + arg);
      }
      return (*state)();
    } else if (token->type() == TOKEN_STRING) {
      T toReturn;
      istringstream(((StringToken *)(token))->val) >> toReturn;
      return toReturn;
    }
    return aval;
  }

// specialized token interpreters

  #define DECL_SPECIAL_INTERP(typeT,tokentype,tokenstruct) \
  typeT _oswexport InterpretArgument(char *arg, typeT aval, Container *acontainer);
  #define DECL_FLOAT_INTERP(typeT,tokentype,tokenstruct) \
  typeT _oswexport InterpretArgument(char *arg, typeT aval, Container *acontainer);

  DECL_SPECIAL_INTERP(int,TOKEN_INT,IntToken);
  DECL_FLOAT_INTERP(float,TOKEN_FLOAT,FloatToken);
  DECL_FLOAT_INTERP(double,TOKEN_DOUBLE,DoubleToken);
  DECL_SPECIAL_INTERP(string,TOKEN_STRING,StringToken);
  DECL_SPECIAL_INTERP(bool,TOKEN_BOOL,BoolToken);
  DECL_SPECIAL_INTERP(complex<float>,TOKEN_COMPLEX,ComplexToken);
  DECL_SPECIAL_INTERP(complex<double>,TOKEN_DOUBLECOMPLEX,DoubleComplexToken);
  DECL_SPECIAL_INTERP(Unit,TOKEN_UNIT,UnitToken);

  template <class T>
    T ScanArguments (char *tag, T aval, Container *acontainer,
		     int argc, char *argv[]) {
    for (int i = 0; i < argc; ++i) {
      if (!strcmp (argv[i],tag)) {
      	if (i + 1 < argc) {
	  ++i;
	  return InterpretArgument(argv[i],aval,acontainer);
	}
      }
    }
    return aval;
  }
}


#endif // _OSWSCAN


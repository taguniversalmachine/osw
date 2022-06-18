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

/*
   oswscan.cpp
   type-specific routines for scanning arguments
   
   Amar Chaudhary
*/

#include "osw.h"

namespace osw {


#define SPECIAL_INTERP(typeT,tokentype,tokenstruct) \
typeT InterpretArgument (char *arg, typeT aval, Container *acontainer) { \
  Token *token = GetToken(arg); \
  if (token->type() == TOKEN_VARIABLE) { \
    State<typeT > *state = dynamic_cast<State<typeT > *> \
      (Nameable::Find(((VariableToken *)(token))->val,acontainer)); \
    if (state == NULL) { \
      State<Any> *stateAsAny = dynamic_cast<State<Any> *> \
        (Nameable::Find(((VariableToken *)(token))->val,acontainer)); \
      Datum<typeT > *datum; \
      if (stateAsAny == NULL) {  \
         acontainer->Error (string("could not find ") + arg); \
         return aval; \
      } \
      datum = dynamic_cast<Datum<typeT > *>((*stateAsAny)->GetDatum()); \
      if (datum == NULL) { \
	 acontainer->Error (arg + string(" is of the wrong type.")); \
         return aval; \
      } \
      return (typeT)(*datum); \
    } \
    return (*state); \
  } else if (token->type() == tokentype) { \
    return ((tokenstruct *)(token))->val; \
  } \
  return aval; \
}

#define FLOAT_INTERP(typeT,tokentype,tokenstruct) \
typeT InterpretArgument (char *arg, typeT aval, Container *acontainer) { \
  Token *token = GetToken(arg); \
  if (token->type() == TOKEN_VARIABLE) { \
    State<typeT > *state = dynamic_cast<State<typeT > *> \
      (Nameable::Find(((VariableToken *)(token))->val,acontainer)); \
    if (state == NULL) { \
      State<Any> *stateAsAny = dynamic_cast<State<Any> *> \
        (Nameable::Find(((VariableToken *)(token))->val,acontainer)); \
      Datum<typeT > *datum; \
      if (stateAsAny == NULL) {  \
         acontainer->Error (string("could not find ") + arg); \
         return aval; \
      } \
      datum = dynamic_cast<Datum<typeT > *>((*stateAsAny)->GetDatum()); \
      if (datum == NULL) { \
	 acontainer->Error (arg + string(" is of the wrong type.")); \
         return aval; \
      } \
      return (typeT)(*datum); \
    } \
    return (*state); \
  } else if (token->type() == TOKEN_FLOAT) { \
    return (typeT) ((FloatToken *)(token))->val; \
  } else if (token->type() == TOKEN_INT) { \
    return (typeT) ((IntToken *)(token))->val; \
  } else if (token->type() == TOKEN_DOUBLE) { \
    return (typeT) ((DoubleToken *)(token))->val; \
  } \
  return aval; \
}

SPECIAL_INTERP(int,TOKEN_INT,IntToken);
FLOAT_INTERP(float,TOKEN_FLOAT,FloatToken);
FLOAT_INTERP(double,TOKEN_DOUBLE,DoubleToken);
//SPECIAL_INTERP(string,TOKEN_STRING,StringToken);
SPECIAL_INTERP(bool,TOKEN_BOOL,BoolToken);
SPECIAL_INTERP(complex<float>,TOKEN_COMPLEX,ComplexToken);
SPECIAL_INTERP(complex<double>,TOKEN_DOUBLECOMPLEX,DoubleComplexToken);
SPECIAL_INTERP(unit,TOKEN_UNIT,UnitToken);


string InterpretArgument (char *arg, string aval, Container *acontainer) {
  Token *token = GetToken(arg);
  if (token->type() == TOKEN_VARIABLE) {
    BaseState *state = dynamic_cast<BaseState *>
      (Nameable::Find(((VariableToken *)(token))->val,acontainer));
    if (state == NULL) {
      State<Any> *stateAsAny = dynamic_cast<State<Any> *>
        (Nameable::Find(((VariableToken *)(token))->val,acontainer));
      Datum<string> *datum;
      if (stateAsAny == NULL) {
         acontainer->Error (string("could not find ") + arg);
         return aval;
      }
      datum = dynamic_cast<Datum<string> *>((*stateAsAny)->GetDatum());
      if (datum == NULL) {
	 acontainer->Error (arg + string(" is of the wrong type."));
         return aval;
      }
      return (string)(*datum);
    }
    ostringstream  os;
    os << (*state) << '\0';
    return os.str();
  } else {
    return arg;
  }
  return aval;
}


}









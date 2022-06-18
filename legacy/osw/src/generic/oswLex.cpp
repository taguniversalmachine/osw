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
   oswlex.cpp
   convert a string to an OSW data type
*/

#ifndef LEXTEST
#include "osw.h"
#else
#include "oswLex.h"
#include <cstdlib>
#endif
#include <limits>

using namespace std;

namespace osw {

  static Token *GetRealOrComplex (const char *tokenStr, char *c);
  static char *EndOfString (char *c);
  static Token  *GetStringToken (const char *tokenStr, char *c);

  Token *GetToken (const char *tokenStr) {
    
    char *c = const_cast<char *>(tokenStr);
    
    while (isspace(*c)) {
      ++c;
    }
    
    
    Token *result = NULL;
    if (!c) {
      return result;
    }

  next_char:

    switch (*c) {
    case '-' :
    case '+' :
      ++c;
      goto next_char;
    case 't' :
    case 'T' :
      if (!strnicmp(tokenStr,"true",4)) {
	return new BoolToken(true,EndOfString(c));
      } else {
	return GetStringToken(tokenStr,c);
      }
    case 'f' :
    case 'F' :
      if (!strnicmp(tokenStr,"false",5)) {
	return new BoolToken(false,EndOfString(c));
      } else {
	return GetStringToken(tokenStr,c);
      }     
#ifndef LEXTEST
    case 'u' :
    case 'U' :
      if (!strnicmp(tokenStr,"unit",4)) {
	return new UnitToken(EndOfString(c));
      } else {
	return GetStringToken(tokenStr,c);
      }     
#endif
    case 'i' :
    case 'I' :
      if (!strnicmp(tokenStr,"infd",4) || !strnicmp(tokenStr,"infinityd",9)) {
	return new DoubleToken(numeric_limits<double>::infinity());
      } else if (!strnicmp(tokenStr,"inf",3) || !strnicmp(tokenStr,"infinity",8)) {
	return new FloatToken(numeric_limits<float>::infinity());
      } else {
	return GetStringToken(tokenStr,c);
      }
    case 'n' :
    case 'N' :
      if (!strnicmp(tokenStr,"nand",4)) {
	return new DoubleToken(numeric_limits<double>::quiet_NaN());
      } else if (!strnicmp(tokenStr,"nan",3)) {
	return new FloatToken(numeric_limits<float>::quiet_NaN());
      } else {
	return GetStringToken(tokenStr,c);
      }
    case '$' :
    case '#' :
      return new VariableToken(tokenStr+1,EndOfString(c));
    case '{' :
      return new BeginListToken(c+1);
    case '}' :
      return new EndListToken(c+1);
    case '0' :
      if (c[1] == 'x' || c[1] == 'X') {
	c += 2;
	int hexnum = 0;
	while (isxdigit(*c)) {
	  hexnum *= 16;
	  switch (*c) {
	  case 'A' : hexnum += 10 + *c - 'A' ; break;
	  case 'a' : hexnum += 10 + *c - 'a' ; break;
	  case 'B' : hexnum += 10 + *c - 'A' ; break;
	  case 'b' : hexnum += 10 + *c - 'a' ; break;
	  case 'C' : hexnum += 10 + *c - 'A' ; break;
	  case 'c' : hexnum += 10 + *c - 'a' ; break;
	  case 'D' : hexnum += 10 + *c - 'A' ; break;
	  case 'd' : hexnum += 10 + *c - 'a' ; break;
	  case 'E' : hexnum += 10 + *c - 'A' ; break;
	  case 'e' : hexnum += 10 + *c - 'a' ; break;
	  case 'F' : hexnum += 10 + *c - 'A' ; break;
	  case 'f' : hexnum += 10 + *c - 'a' ; break;
	  default: hexnum += *c - '0';
	  }
	  ++c;
	}
	return new IntToken(hexnum,c);
      }
    default :
      if (isdigit(*c) || *c == '.') {
	while (isdigit(*c)) {
	  ++c;
	}
	switch (*c) {
	case '\0':
	case '}':
	case '{':
	  {
	    char temp = *c;
	    *c = '\0';
            int i = atoi(tokenStr);
            *c = temp;
	    return new IntToken(i,c);
	  }
	case '.' :
	  return GetRealOrComplex(tokenStr,++c);
	    case 'E' :
	case 'e' :
	  ++c;
	  if (*c == '+' || *c == '-') {
	    ++c;
	  }
	  return GetRealOrComplex(tokenStr,c);
	case 'd' :
	case 'D' :
	case 'I' :
	case 'i' :
	case '-' : 
	case '+' :
	  return GetRealOrComplex(tokenStr,c);
	}
      }
      return GetStringToken(tokenStr,c);
    }
  }
  
  Token *GetRealOrComplex (const char *tokenStr, char *c) {
    
    bool doublePrec = false;
    double realPart = 0.0;
    char *imagStart = NULL;
    char imagSign = 0;
    
    while (isdigit(*c)) {
      ++c;
    }
    
  next_char:
    
    switch (*c) {
    case '\0' :
    case '}' :
    case '{' :
      if (imagSign != 0) {
	// this should be the imaginary part of a complex number,
	// it can't be an imaginary number w/o a trailing "i",
	// so the whole token isn't a number
	return GetStringToken(tokenStr,c);
      }
      if (doublePrec) {
	return new DoubleToken (atof(tokenStr),c);
      } else {
	return new FloatToken (atof(tokenStr),c);
      }
    case 'd' :
    case 'D' :
      doublePrec = true;
      ++c;
      goto next_char;
    case 'i' :
    case 'I' :
      if (doublePrec) {
	return new DoubleComplexToken(complex<double>(realPart,atof(tokenStr)),c+1);
      } else {
	return new ComplexToken(complex<float>(realPart,atof(tokenStr)),c+1);
      }
    case '+' :
    case '-' : 
      imagSign = *c;
      *c = '\0';
      realPart = atof(tokenStr);
      *c = imagSign;
      tokenStr = c;
      ++c;
      while (isdigit(*c)) {
	++c;
      }
      goto next_char;
    case '.' :
      if (imagSign == 0) {
	// can't have two points in the same floating number
	return GetStringToken(tokenStr,c);
	++c;
	while (isdigit(*c)) {
	  ++c;
	}
	goto next_char;
      }
    case 'E' :
    case 'e' :
      ++c;
      while (isdigit(*c)) {
	++c;
      }
      goto next_char;
      
    default :
      return GetStringToken(tokenStr,c);
    }
  }

  char *EndOfString(char *c) {
    for (;;++c) {
      switch (*c) {
      case '\0' :
      case '{' :
      case '}' :
	return c;
      }
    }
    return NULL; // should never happen
  }

  Token  *GetStringToken (const char *tokenStr, char *c) {
    char *eos = EndOfString(c);
    return new StringToken(string(tokenStr,eos-tokenStr),eos);
  }
}

// test routine

#if LEXTEST
int main () {

  char tokenStr[256];

  while (1) {
    cin >> tokenStr;
    
    osw::Token * token = osw::GetToken (tokenStr);
    switch (token->type()) {
    case osw::TOKEN_INT :
      cout << "Int : " << ((osw::IntToken *)(token))->val;
      break;
    case osw::TOKEN_BOOL :
      cout << "bool : " << ((osw::BoolToken *)(token))->val;

      break;
    case osw::TOKEN_FLOAT :
      cout << "Float : " << ((osw::FloatToken *)(token))->val;
      break;
    case osw::TOKEN_DOUBLE :
      cout << "Double : " << ((osw::DoubleToken *)(token))->val;
      break;
    case osw::TOKEN_STRING :
      cout << "String : " << ((osw::StringToken *)(token))->val;
      break;
    case osw::TOKEN_COMPLEX :
      cout << "Complex : " << ((osw::ComplexToken *)(token))->val;
      break;
    case osw::TOKEN_DOUBLECOMPLEX :
      cout << "DComplex : " << ((osw::DoubleComplexToken *)(token))->val;
      break;
    case osw::TOKEN_VARIABLE :
      cout << "Variable : " << ((osw::VariableToken *)(token))->val;
    } 
    cout << endl;
  }

  return 0;
}
#endif









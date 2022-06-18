
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
  oswlex.h
  converts a string to a standard OSW data type

  Amar Chaudhary
*/


#ifndef _OSW_LEX
#define _OSW_LEX

#include <complex>
#include <string>

using namespace std;

namespace osw {

  typedef enum {
    TOKEN_INT, TOKEN_FLOAT, TOKEN_DOUBLE,
    TOKEN_STRING, TOKEN_BOOL, TOKEN_UNIT,
    TOKEN_COMPLEX, TOKEN_DOUBLECOMPLEX, TOKEN_LIST, 
    TOKEN_CHAR,
    TOKEN_VARIABLE,
    TOKEN_BEGIN_LIST,TOKEN_END_LIST,
    TOKEN_QUOTED_STRING,
    TOKEN_OPEN_PAREN,TOKEN_CLOSE_PAREN,
    TOKEN_USER = 1000
  } TokenType;
  

  struct Token {

    virtual ~Token () {}
    virtual TokenType type() const = 0;
    char *remaining;

    Token (char *aremaining = NULL)
      : remaining(aremaining) {
    }
  };


  struct IntToken : public Token {
    int  val;
    TokenType type() const {return TOKEN_INT;}

    IntToken (int aval, char *aremaining = NULL) 
      : Token(aremaining), val(aval) {
    }
  };

  struct FloatToken : public Token {
    float val;
    TokenType type() const {return TOKEN_FLOAT;}
    FloatToken (float aval, char *aremaining = NULL) 
      : Token(aremaining), val(aval) {
    }
  };

  struct DoubleToken : public Token {
    double val;
    TokenType type() const {return TOKEN_DOUBLE;}
    DoubleToken (double aval, char *aremaining = NULL) 
      : Token(aremaining), val(aval) {
    }
  };

  struct StringToken : public Token {
    string val;
    TokenType type() const {return TOKEN_STRING;}
    StringToken (const string &aval, char *aremaining = NULL) 
      :  Token(aremaining), val(aval) {
    }
  };

  struct VariableToken : public Token {
    string val;
    TokenType type() const {return TOKEN_VARIABLE;}
    VariableToken (const string &aval, char *aremaining = NULL) 
     :  Token(aremaining), val(aval) {
    }
  };

  struct BoolToken : public Token {
    bool val;
    TokenType type() const {return TOKEN_BOOL;}
    BoolToken (bool aval, char *aremaining = NULL) 
      : Token(aremaining), val(aval) {
    }
  };

  struct CharToken : public Token {
    char val;
    TokenType type() const {return TOKEN_CHAR;}
    CharToken (char aval, char *aremaining = NULL) 
      : Token(aremaining), val(aval) {
    }
  };

#ifndef LEXTEST
  struct UnitToken : public Token {
    Unit val;
    TokenType type() const {return TOKEN_UNIT;}
    UnitToken(char *aremaining = NULL) 
      : Token(aremaining) {
    }
  };
#endif
  
  struct ComplexToken : public Token {
    complex<float> val;
    TokenType type() const {return TOKEN_COMPLEX;}
    ComplexToken (const complex<float> &aval, char *aremaining = NULL) 
      : Token(aremaining), val(aval) {
    }
  };

  struct DoubleComplexToken : public Token {
    complex<double> val;
    TokenType type() const {return TOKEN_DOUBLECOMPLEX;}
    DoubleComplexToken (const complex<double> &aval, char *aremaining = NULL) 
      : Token(aremaining), val(aval) {
    }
  };

  struct BeginListToken : public Token {
    TokenType type () const {return TOKEN_BEGIN_LIST;}
    BeginListToken(char *aremaining = NULL) 
      : Token(aremaining) {
    }
  };

  struct EndListToken : public Token {
    TokenType type () const {return TOKEN_END_LIST;}
    EndListToken(char *aremaining = NULL) 
      : Token(aremaining) {
    }
  };

  Token *GetToken (const char *tokenStr);

}

#endif // _OSW_LEX




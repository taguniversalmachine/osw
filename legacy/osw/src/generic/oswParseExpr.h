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

#define FUNCTION           0
#define EXPONENT_OP        1
#define NEGATION           2
#define FACTORIAL          3
#define LOGICAL_NEG        4
#define MULTIPLY_OP        5
#define DIVIDE_OP          6
#define MODULUS_OP         7
#define ADDITION           8
#define SUBTRACTION        9
#define LSHIFT_OP          10
#define RSHIFT_OP          11
#define LT_OP              12
#define GT_OP              13
#define LTEQ_OP            14
#define GTEQ_OP            15
#define EQ_OP              16
#define NEQ_OP             17
#define BIT_AND            18
#define BIT_OR             19
#define BIT_XOR            20
#define LOG_AND            21
#define LOG_OR             22
#define QUESTIONMARK       23
#define VARIABLE           24
#define INTVAL             25
#define FLOATVAL           26
#define DOUBLEVAL          27
#define COMPLEXVAL         28
#define DOUBLECOMPLEXVAL   29
#define LISTINDEX          30
#define LISTCONS           31
#define LISTNIL            32
#define LISTSINGLE         33
#define UNIT_EXPR          34

typedef struct _subexpr {

  int expr_type;
  union {
    int    ival;
    float  fval;
    double dval;
  } val;
  char     *strval;
  struct _subexpr  *part1,*part2,*part3;

} SubExpr;

typedef struct {

  char *input;
  char *ptr;
  SubExpr *expr;

} ParseStruct;


#ifdef __cplusplus
extern "C" {
#endif
  SubExpr *NewSubExpr();
  void FreeSubExpr (SubExpr *toFree);
  void PrintSubExpr (SubExpr *toFree);
  ParseStruct *NewParseStruct(char *input);
  void FreeParseStruct(ParseStruct *toFree);
  char GetNextChar(ParseStruct *source);
  char PeekNextChar(ParseStruct *source);
  void PutBackChar(ParseStruct *source);
  int  EndOfInput(ParseStruct *source);
  ParseStruct *Parse(char *toParse);

  float GetFloatInfinity();
  double GetDoubleInfinity();
  float GetFloatNaN();
  double GetDoubleNaN();

#ifdef __cplusplus
}
#endif



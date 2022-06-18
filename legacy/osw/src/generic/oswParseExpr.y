

%{

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


#include <stdlib.h>
#if __GNUC__ < 3 || !defined(__APPLE__)
#include <malloc.h>
#endif
#include <string.h>
#include <stdio.h>
#include "oswParseExpr.h"

#ifndef _MSC_VER
#define stricmp strcasecmp
#endif


#ifndef M_PI
#define M_PI        3.14159265358979323846
#endif

#ifndef M_E
#define M_E         2.71828182845905
#endif

extern float GetFloatInfinity ();
extern double GetDoubleInfinity ();

SubExpr *NewSubExpr() {
  SubExpr *result = malloc(sizeof(SubExpr));
  memset(result,0,sizeof(SubExpr));
  return result;
}

void FreeSubExpr (SubExpr *toFree) {
  if (toFree->strval) {
    free(toFree->strval);
  }
  if (toFree->part1) {
    FreeSubExpr(toFree->part1);
  }
  if (toFree->part2) {
    FreeSubExpr(toFree->part2);
  }
  if (toFree->part3) {
    FreeSubExpr(toFree->part3);
  }
  free(toFree);
}

void PrintSubExpr (SubExpr *toPrint) {
  if (toPrint->strval) {
    printf ("(%d",toPrint->strval);
  }
  if (toPrint->part1) {
    printf (",");
    PrintSubExpr(toPrint->part1);
  }
  if (toPrint->part2) {
    printf (",");
    PrintSubExpr(toPrint->part2);
  }
  if (toPrint->part3) {
    printf (",");
    PrintSubExpr(toPrint->part3);
  }
  printf (")");
}

ParseStruct *NewParseStruct(char *input) {
  ParseStruct *result = malloc(sizeof(ParseStruct));
  result->input = input;
  result->ptr = result->input;
  return result;
}

 void FreeParseStruct(ParseStruct *toFree) {
   free(toFree);
 }

 char GetNextChar(ParseStruct *source) {
   char result = *(source->ptr);
   //if (*(source->ptr)) {
   ++source->ptr;
   //}
   return result;
 }

 char PeekNextChar(ParseStruct *source) {
   return (*(source->ptr));
 }

 void PutBackChar(ParseStruct *source) {
   --source->ptr;
 }

 int EndOfInput (ParseStruct *source) {
   return *(source->ptr) == 0;
 }

#define YYPARSE_PARAM  param
#define YYLEX_PARAM param

%}

%union {
  int    ival;
  float  fval;
  double dval;
  char   *sval;
  SubExpr *exp;
}

%type <exp> exp
%type <exp> list

%token <ival> INT
%token <fval> FLOAT
%token <dval> DOUBLE
%token <fval> COMPLEX
%token <dval> DOUBLECOMPLEX
%token <sval>  FUNC
%token <sval>   VAR


%left AND OR
%left EQ NEQ
%left '<' '>' GTEQ LTEQ
%left PBITAND PBITOR PBITXOR
%left LSHIFT RSHIFT
%left '-' '+'
%left '*' '/' '%'
%left '~'
%left '!'
%left NEG     /* Negation--unary minus */
%right '^'    /* Exponentiation        */
%nonassoc TRUE FALSE UNIT

%start hyperexp

%pure_parser

%%

hyperexp: exp 
     {((ParseStruct *) param)->expr = $1;}
;

exp :  '(' exp ')' { $$ = $2; }
    |  FUNC '(' exp ')'   { $$ = NewSubExpr(); $$->expr_type = FUNCTION; 
                            $$->strval = $1; $$->part1 = $3;}
    |  exp '[' exp ']'    { $$ = NewSubExpr(); $$->expr_type = LISTINDEX; 
                            $$->part1 = $1; $$->part2 = $3;}
    |  '[' list ']'       { $$ = $2;}
    |  '-' exp %prec NEG  { $$ = NewSubExpr(); $$->expr_type = NEGATION; 
			    $$->part1=$2;}
    |  exp '^' exp        { $$ = NewSubExpr(); $$->expr_type = EXPONENT_OP; 
			    $$->part1=$1; $$->part2=$3;}
    |  exp '!'		  { $$ = NewSubExpr(); $$->expr_type = FACTORIAL; 
			    $$->part1=$1;}
    |  '~' exp		  { $$ = NewSubExpr(); $$->expr_type = LOGICAL_NEG; 
			    $$->part1=$2;}
    |  exp '*' exp        { $$ = NewSubExpr(); $$->expr_type = MULTIPLY_OP; 
			    $$->part1=$1; $$->part2=$3;}
    |  exp '/' exp	  { $$ = NewSubExpr(); $$->expr_type = DIVIDE_OP; 
			    $$->part1=$1; $$->part2=$3;}
    |  exp '%' exp	  { $$ = NewSubExpr(); $$->expr_type = MODULUS_OP; 
			    $$->part1=$1; $$->part2=$3;}
    |  exp '+' exp	  { $$ = NewSubExpr(); $$->expr_type = ADDITION; 
			    $$->part1=$1; $$->part2=$3;}
    |  exp '-' exp	  { $$ = NewSubExpr(); $$->expr_type = SUBTRACTION; 
			    $$->part1=$1; $$->part2=$3;}
    |  exp LSHIFT exp	  { $$ = NewSubExpr(); $$->expr_type = LSHIFT_OP; 
			    $$->part1=$1; $$->part2=$3;}
    |  exp RSHIFT exp	  { $$ = NewSubExpr(); $$->expr_type = RSHIFT_OP; 
			    $$->part1=$1; $$->part2=$3;}
    |  exp '>' exp	  { $$ = NewSubExpr(); $$->expr_type = GT_OP; 
			    $$->part1=$1; $$->part2=$3;}
    |  exp '<' exp	  { $$ = NewSubExpr(); $$->expr_type = LT_OP; 
			    $$->part1=$1; $$->part2=$3;}
    |  exp LTEQ exp	  { $$ = NewSubExpr(); $$->expr_type = LTEQ_OP; 
			    $$->part1=$1; $$->part2=$3;}
    |  exp GTEQ exp	  { $$ = NewSubExpr(); $$->expr_type = GTEQ_OP; 
			    $$->part1=$1; $$->part2=$3;}
    |  exp EQ exp	  { $$ = NewSubExpr(); $$->expr_type = EQ_OP; 
			    $$->part1=$1; $$->part2=$3;}
    |  exp NEQ exp	  { $$ = NewSubExpr(); $$->expr_type = NEQ_OP; 
			    $$->part1=$1; $$->part2=$3;}
    |  exp PBITAND exp	  { $$ = NewSubExpr(); $$->expr_type = BIT_AND; 
			    $$->part1=$1; $$->part2=$3;}
    |  exp PBITOR  exp    { $$ = NewSubExpr(); $$->expr_type = BIT_OR; 
			    $$->part1=$1; $$->part2=$3;}
    |  exp PBITXOR  exp   { $$ = NewSubExpr(); $$->expr_type = BIT_XOR; 
			    $$->part1=$1; $$->part2=$3;}
    |  exp AND exp	  { $$ = NewSubExpr(); $$->expr_type = LOG_AND; 
			    $$->part1=$1; $$->part2=$3;}
    |  exp OR  exp        { $$ = NewSubExpr(); $$->expr_type = LOG_OR; 
			    $$->part1=$1; $$->part2=$3;}
    |  INT		  { $$ = NewSubExpr(); $$->expr_type = INTVAL;
			    $$->val.ival = $1;}
    |  FLOAT		  { $$ = NewSubExpr(); $$->expr_type = FLOATVAL;
			    $$->val.fval = $1;}
    |  DOUBLE		  { $$ = NewSubExpr(); $$->expr_type = DOUBLEVAL;
			    $$->val.dval = $1;}
    |  COMPLEX		 { $$ = NewSubExpr(); $$->expr_type = COMPLEXVAL;
			    $$->val.fval = $1;}
    |  DOUBLECOMPLEX	  { $$ = NewSubExpr(); $$->expr_type = DOUBLECOMPLEXVAL;
			    $$->val.dval = $1;}
    |  VAR                { $$ = NewSubExpr(); $$->expr_type = VARIABLE;
                            $$->strval = $1;}
    |  UNIT               { $$ = NewSubExpr(); $$->expr_type = UNIT_EXPR;}
                            
;

list :                    { $$ = NewSubExpr(); $$->expr_type = LISTNIL;
                            $$->part1 = $$->part2 = NULL;} 
     | exp               {  $$ = NewSubExpr(); $$->expr_type = LISTSINGLE;
                            $$->part1 = $1; $$->part2 = NULL;}
     | list ',' exp       { $$ = NewSubExpr(); $$->expr_type = LISTCONS;
                            $$->part1 = $1; $$->part2 = $3;}
%%

int yyerror (char *s) {
   fprintf (stderr, "%s\n", s);
   return 0;
}

int yylex (YYSTYPE *lvalp, void *param) {
  
  char  c;
  char  saved;
  ParseStruct *parseStruct = (ParseStruct *)param;
  while (isspace(c = GetNextChar(parseStruct)));

  if (c == '$') {
    int wordsize;
    char *wordstart = parseStruct->ptr;
    while(isalnum(c = GetNextChar(parseStruct)));
    PutBackChar(parseStruct);
    wordsize = parseStruct->ptr - wordstart;
    lvalp->sval = malloc(wordsize+1);
    strncpy(lvalp->sval,wordstart,wordsize);
    lvalp->sval[wordsize] = '\0';
    /*puts ("VAR");*/
    return VAR;
  }
  if (isalpha(c)) {
    int wordsize;
    char *wordstart = parseStruct->ptr-1;
    PutBackChar(parseStruct);
    while(isalnum(c = GetNextChar(parseStruct)));
    PutBackChar(parseStruct);
    wordsize = parseStruct->ptr - wordstart;
    lvalp->sval = malloc(wordsize+1);
    strncpy(lvalp->sval,wordstart,wordsize);
    lvalp->sval[wordsize] = '\0';
    /*puts(lvalp->sval);*/
    if (!stricmp(lvalp->sval,"true")) {
      free(lvalp->sval);
      lvalp->ival = 1;
      /*puts ("INT1");*/
      return INT;
    } else if (!stricmp(lvalp->sval,"false")) {
      /*puts ("INT2");*/
      return INT;
    } else if (!stricmp(lvalp->sval,"pi")) {
      free(lvalp->sval);
      lvalp->fval = M_PI;
      return FLOAT;
    } else if (!stricmp(lvalp->sval,"e")) {
      free(lvalp->sval);
      lvalp->fval = M_E;
      return FLOAT;
    } else if (!stricmp(lvalp->sval,"i")) {
      free(lvalp->sval);
      lvalp->fval = 1;
      return COMPLEX;
    } else if (!stricmp(lvalp->sval,"and")) {
      free(lvalp->sval);
      return AND;
    } else if (!stricmp(lvalp->sval,"or")) {
      free(lvalp->sval);
      return OR;
    } else if (!stricmp(lvalp->sval,"id")) {
      free(lvalp->sval);
      lvalp->fval = 1;
      return DOUBLECOMPLEX;
    } else if (!stricmp(lvalp->sval,"infd") || !stricmp(lvalp->sval,"infinityd")) {
      free(lvalp->sval);
      lvalp->dval = GetDoubleInfinity();
      return DOUBLE;
    } else if (!stricmp(lvalp->sval,"inf") || !stricmp(lvalp->sval,"infinity")) {
      free(lvalp->sval);
      lvalp->dval = GetFloatInfinity();
      return FLOAT;
    } else if (!stricmp(lvalp->sval,"nand")) {
      free(lvalp->sval);
      lvalp->dval = GetDoubleNaN();
      return DOUBLE;
    } else if (!stricmp(lvalp->sval,"nan")) {
      free(lvalp->sval);
      lvalp->fval = GetFloatNaN();
      return FLOAT;
    } else if (!stricmp(lvalp->sval,"unit")) {
      free(lvalp->sval);
      return UNIT;
    } else {
      /*puts ("FUNC");*/
      return FUNC;
    }
  }
  if (isdigit(c) || (c == '-' && isdigit(PeekNextChar(parseStruct)))) {
    char *numbegin;
    if (c == '0') {
      if ((c = GetNextChar(parseStruct)) == 'x' || c == 'X') {
	int hexnum = 0;
	while (isxdigit(c = GetNextChar(parseStruct))) {
	  hexnum *= 16;
	  if (isdigit(c)) {
	    hexnum += c - '0';
	  } else {
	    hexnum += 10 + tolower(c)-'a';
	  }
	}
	lvalp->ival = hexnum;
	/*puts ("INT3");*/
	return INT;
      } else {
	PutBackChar(parseStruct);
      }
    }
    numbegin = parseStruct->ptr-1;
    while (isdigit(c = GetNextChar(parseStruct)));
    switch (c) {
    case '.' :
      while (isdigit(c = GetNextChar(parseStruct)));
      switch (c) {
      case 'D':
      case 'd':
	lvalp->dval = atof(numbegin);
	/*puts("DOUBLE");*/
	return DOUBLE;
      case 'I':
      case 'i':
	if (tolower(PeekNextChar(parseStruct)) == 'd') {
	  GetNextChar(parseStruct);
	  lvalp->dval = atof(numbegin);
	  /*puts("DOUBLECOMPLEX");*/
	  return DOUBLECOMPLEX;
	} else {
	  lvalp->fval = atof(numbegin);
	  /*puts("COMPLEX");*/
	  return COMPLEX;
	}
      case 'E':
      case 'e':
	if (PeekNextChar(parseStruct) == '+' ||
	    PeekNextChar(parseStruct) == '-') {
	  GetNextChar(parseStruct);
	}
	while (isdigit(c = GetNextChar(parseStruct)));
	switch(c) {
	case 'D':
	case 'd':
	  lvalp->dval = atof(numbegin);
	  /*puts("DOUBLEC");*/
	  return DOUBLE;
	case 'I':
	case 'i':
	  if (tolower(PeekNextChar(parseStruct)) == 'd') {
	    GetNextChar(parseStruct);
	    lvalp->dval = atof(numbegin);
	    /*puts("DOUBLECOMPLEX");*/
	    return DOUBLECOMPLEX;
	  } else {
	    lvalp->fval = atof(numbegin);
	    /*puts("COMPLEX");*/
	    return COMPLEX;
	  }
	default:
	  PutBackChar(parseStruct);
	  /*puts("FLOAT");*/
	  lvalp->fval = atof(numbegin);
	  return FLOAT;
	}
      default:
	PutBackChar(parseStruct);
	/*puts("FLOAT");*/
	lvalp->fval = atof(numbegin);
	return FLOAT;
      }
    case 'D':
    case 'd':
      lvalp->dval = atof(numbegin);
      /*puts("DOUBLE");*/
      return DOUBLE;
    case 'I':
    case 'i':
      if (tolower(PeekNextChar(parseStruct)) == 'd') {
	GetNextChar(parseStruct);
	lvalp->dval = atof(numbegin);
	/*puts("DOUBLECOMPLEX");*/
	return DOUBLECOMPLEX;
      } else {
	lvalp->fval = atof(numbegin);
	/*puts("COMPLEX");*/
	return COMPLEX;
      }
    case 'E':
    case 'e':
      if (PeekNextChar(parseStruct) == '+' ||
	  PeekNextChar(parseStruct) == '-') {
	GetNextChar(parseStruct);
      }
      while (isdigit(c = GetNextChar(parseStruct)));
      switch(c) {
      case 'D':
      case 'd':
	lvalp->dval = atof(numbegin);
	/*puts ("DOUBLE");*/
	return DOUBLE;
      case 'I':
      case 'i':
	if (tolower(PeekNextChar(parseStruct)) == 'd') {
	  GetNextChar(parseStruct);
	  lvalp->dval = atof(numbegin);
	  /*puts ("DOUBLECOMPLEX");*/
	  return DOUBLECOMPLEX;
	} else {
	  lvalp->fval = atof(numbegin);
	  /*puts ("COMPLEX");*/
	  return COMPLEX;
	}
      default:
	PutBackChar(parseStruct);
	lvalp->fval = atof(numbegin);
	/*puts ("FLOAT");*/
	return FLOAT;
      }
    default:
      PutBackChar(parseStruct);
      lvalp->ival = atoi(numbegin);
      /*puts("INT4");*/
      return INT;
    }
  }
  switch (c) {
  case '=' :
    if (PeekNextChar(parseStruct) == '=') {
      GetNextChar(parseStruct);
      return EQ;
    }
    break;
  case '!' :
    if (PeekNextChar(parseStruct) == '=') {
      GetNextChar(parseStruct);
      return NEQ;
    }
    break;
  case '>' :
    switch(PeekNextChar(parseStruct)) {
    case '>' :
      GetNextChar(parseStruct);
      return RSHIFT;
    case '=' :
      GetNextChar(parseStruct);
      return GTEQ;
    }
    break;
  case '<' :
    switch(PeekNextChar(parseStruct)) {
    case '<' :
      GetNextChar(parseStruct);
      return LSHIFT;
    case '=' :
      GetNextChar(parseStruct);
      return LTEQ;
    }
    break;
  case '&' :
    if (PeekNextChar(parseStruct) == '&') {
      GetNextChar(parseStruct);
      return AND;
    }
    break;
  case '|' :
    if (PeekNextChar(parseStruct) == '|') {
      GetNextChar(parseStruct);
      return OR;
    }
    break;
  case '?' :
    switch (PeekNextChar(parseStruct)) {
    case '&' : GetNextChar(parseStruct); return BIT_AND;
    case '|' : GetNextChar(parseStruct); return BIT_AND;
    case '^' : GetNextChar(parseStruct); return BIT_XOR;
    }
    break;
  }
  /*printf ("token %d\n",c);*/
  return c;
}

ParseStruct *Parse(char *toParse) {
  ParseStruct *p = NewParseStruct(toParse);
  yyparse((void *)p);
  return p;
}

#if 0
int main () {
  char buffer[256];
  gets(buffer);
  while(1) {
    ParseStruct *p;
    gets(buffer);
    p = NewParseStruct(buffer);
    yyparse((void *)p);
    FreeParseStruct(p);
  }
}
#endif

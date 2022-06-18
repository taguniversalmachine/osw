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
   oswbinop3b.cpp
   Still more declarations for the basic arithmetic operators (+,-,*,/,%,etc.)
*/

#include "oswBinop.h"
#include "oswLex.h"



using namespace std;

namespace osw {
 
  DeclareScalarRelops<int,float> DCR6;
  DeclareScalarRelops<int,double> DCR7;
  DeclareScalarRelops<float,double> DCR10;


  // Declare bitwise operations on 16 bit and 32 bit integers
  
  template<class T>
  struct DeclareBitwiseOps {
    DeclareBinop<lshift<T,T,T>,SHIFTL> lshift1;
    DeclareBinop<rshift<T,T,T>,SHIFTR> rshift1;
    DeclareBinop<binop_bitand<T,T,T>,BITAND> bitand1;
    DeclareBinop<binop_bitor<T,T,T>,BITOR> bitor1;
    DeclareBinop<bitxor<T,T,T>,BITXOR> bitxor1;
  };

  template<class V, class S>
  struct DeclareVectorBitwiseOps {
    CastLeftBinop<lshift<S,osw::vector<V>,osw::vector<V> >, S, SHIFTL> lshift1;
    CastRightBinop<lshift<osw::vector<V>,S,osw::vector<V> >, S, SHIFTL> lshift2;
    CastLeftBinop<rshift<S,osw::vector<V>,osw::vector<V> >, S, SHIFTR> rshift1;
    CastRightBinop<rshift<osw::vector<V>,S,osw::vector<V> >, S, SHIFTR> rshift2;
    CastLeftBinop<binop_bitand<S,osw::vector<V>,osw::vector<V> >, S, BITAND> bitand1;
    CastRightBinop<binop_bitand<osw::vector<V>,S,osw::vector<V> >, S, BITAND> bitand2;
    CastLeftBinop<binop_bitor<S,osw::vector<V>,osw::vector<V> >, S, BITOR> bitor1;
    CastRightBinop<binop_bitor<osw::vector<V>,S,osw::vector<V> >, S, BITOR> bitor2;
    CastLeftBinop<bitxor<S,osw::vector<V>,osw::vector<V> >, S, BITXOR> bitxor1;
    CastRightBinop<bitxor<osw::vector<V>,S,osw::vector<V> >, S, BITXOR> bitxor2;
  };

  DeclareBitwiseOps<int>  DCBIT1;
  DeclareBitwiseOps<short>  DCBIT2;
  DeclareBitwiseOps<osw::vector<int> > DCBIT3;
  DeclareBitwiseOps<osw::vector<short> > DCBIT4;
  DeclareVectorBitwiseOps<int,int> DCBIT5;
  DeclareVectorBitwiseOps<short,short> DCBIT6;


  // oh, and also the addition operator for strings

  DeclareBinop <add<string,string,string>, ADD> DCS1;
  DeclareBinop <binop_eq<string,string,bool>, EQUAL> DCS2;
  DeclareBinop <binop_neq<string,string,bool>, NOTEQUAL> DCS3;
  DeclareBinop <binop_less<string,string,bool>,LESS > DCS4;
  DeclareBinop <binop_greater<string,string,bool>,GREATER > DCS5;
  DeclareBinop <binop_less_eq<string,string,bool>,LTEQ > DCS6;
  DeclareBinop <binop_greater_eq<string,string,bool>,GTEQ > DCS7;


  // More of this declare-use bull*$@$ from Apple
  OSW_DECLARE_USE(DCS1);
  OSW_DECLARE_USE(DCS2);
  OSW_DECLARE_USE(DCS3);
  OSW_DECLARE_USE(DCS4);
  OSW_DECLARE_USE(DCS5);
  OSW_DECLARE_USE(DCS6);
  OSW_DECLARE_USE(DCS7);

  OSW_DECLARE_USE(DCR6);
  OSW_DECLARE_USE(DCR7);
  OSW_DECLARE_USE(DCR10);

  OSW_DECLARE_USE(DCBIT1);
  OSW_DECLARE_USE(DCBIT2);
  OSW_DECLARE_USE(DCBIT3);
  OSW_DECLARE_USE(DCBIT4);
  OSW_DECLARE_USE(DCBIT5);
  OSW_DECLARE_USE(DCBIT6);

}








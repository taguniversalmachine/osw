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
   oswbinop5.cpp
   Basic arithmetic operators (+,-,*,/,%,etc.) on lists
   Not very efficient, but at least it works!
*/

#include "oswBinop.h"
#include "oswList.h"

namespace osw {

  static Datum<string> undef("undef");

  
  _STLP_TEMPLATE_NULL
  struct add<List,List,List> {
    typedef List Arg1Type;
    typedef List Arg2Type;
    typedef List ResultType;

    List operator () (const List &arg1, const List &arg2) {
      List result;
      List::const_iterator index1,index2;

      for (index1 = arg1.begin(), index2 = arg2.begin();
	   index1 != arg1.end() || index2 != arg2.end();
	   ++index1,++index2) {
	Untyped v1 = *index1;
	Untyped v2 = *index2;
	BaseBinop *op = LookupBinop(ADD,v1->TypeOf(),v2->TypeOf());
	if (op != NULL) {
	  result.append(op->eval_from_data(*v1,*v2));
	} else {
	  result.append(&undef);
	}
      }
      return result;
    }
  };
  
  _STLP_TEMPLATE_NULL
  struct subtract<List,List,List> {
    typedef List Arg1Type;
    typedef List Arg2Type;
    typedef List ResultType;

    List operator () (const List &arg1, const List &arg2) {
      List result;
      List::const_iterator index1,index2;

      for (index1 = arg1.begin(), index2 = arg2.begin();
	   index1 != arg1.end() || index2 != arg2.end();
	   ++index1,++index2) {
	Untyped v1 = *index1;
	Untyped v2 = *index2;
	BaseBinop *op = LookupBinop(SUBTRACT,v1->TypeOf(),v2->TypeOf());
	if (op != NULL) {
	  result.append(op->eval_from_data(*v1,*v2));
	} else {
	  result.append(&undef);
	}
      }
      return result;
    }
  };
  
  _STLP_TEMPLATE_NULL
  struct multiply<List,List,List> {
    typedef List Arg1Type;
    typedef List Arg2Type;
    typedef List ResultType;

    List operator () (const List &arg1, const List &arg2) {
      List result;
      List::const_iterator index1,index2;

      for (index1 = arg1.begin(), index2 = arg2.begin();
	   index1 != arg1.end() || index2 != arg2.end();
	   ++index1,++index2) {
	Untyped v1 = *index1;
	Untyped v2 = *index2;
	BaseBinop *op = LookupBinop(MULTIPLY,v1->TypeOf(),v2->TypeOf());
	if (op != NULL) {
	  result.append(op->eval_from_data(*v1,*v2));
	} else {
	  result.append(&undef);
	}
      }
      return result;
    }
  };

  _STLP_TEMPLATE_NULL
  struct divide<List,List,List> {
    typedef List Arg1Type;
    typedef List Arg2Type;
    typedef List ResultType;

    List operator () (const List &arg1, const List &arg2) {
      List result;
      List::const_iterator index1,index2;

      for (index1 = arg1.begin(), index2 = arg2.begin();
	   index1 != arg1.end() || index2 != arg2.end();
	   ++index1,++index2) {
	Untyped v1 = *index1;
	Untyped v2 = *index2;
	BaseBinop *op = LookupBinop(DIVIDE,v1->TypeOf(),v2->TypeOf());
	if (op != NULL) {
	  result.append(op->eval_from_data(*v1,*v2));
	} else {
	  result.append(&undef);
	}
      }
      return result;
    }
  };

  _STLP_TEMPLATE_NULL
  struct exponent<List,List,List> {
    typedef List Arg1Type;
    typedef List Arg2Type;
    typedef List ResultType;

    List operator () (const List &arg1, const List &arg2) {
      List result;
      List::const_iterator index1,index2;

      for (index1 = arg1.begin(), index2 = arg2.begin();
	   index1 != arg1.end() ||index2 != arg2.end();
	   ++index1,++index2) {
	Untyped v1 = *index1;
	Untyped v2 = *index2;
	BaseBinop *op = LookupBinop(EXPONENT,v1->TypeOf(),v2->TypeOf());
	if (op != NULL) {
	  result.append(op->eval_from_data(*v1,*v2));
	} else {
	  result.append(&undef);
	}
      }
      return result;
    }
  };

  template<class T, int opnum>
  struct scalar_left_list_op {
    typedef T Arg1Type;
    typedef List Arg2Type;
    typedef List ResultType;
    
    List operator () (const T &arg1, const List &arg2) {
      List result;
      List::const_iterator index2;

      Datum<T> *v1 = new Datum<T>(arg1);
      for (index2 = arg2.begin(); index2 != arg2.end(); ++index2) {
	Untyped v2 = *index2;
	BaseBinop *op = LookupBinop(opnum,v1->TypeOf(),v2->TypeOf());
	if (op != NULL) {
	  result.append(op->eval_from_data(*v1,*v2));
	} else {
	  result.append(&undef);
	}
      }
      return result;
    }
  };

  template<class T, int opnum>
  struct scalar_right_list_op {
    typedef List Arg1Type;
    typedef T Arg2Type;
    typedef List ResultType;
    
    List operator () (const List &arg1, const T &arg2) {
      List result;
      List::const_iterator index1;

      Datum<T> *v2 = new Datum<T>(arg2);
      for (index1 = arg1.begin(); index1 != arg1.end(); ++index1) {
	Untyped v1 = *index1;
	BaseBinop *op = LookupBinop(opnum,v1->TypeOf(),v2->TypeOf());
	if (op != NULL) {
	  result.append(op->eval_from_data(*v1,*v2));
	} else {
	  result.append(&undef);
	}
      }
      return result;
    }
  };  

  template<class T>
  struct DeclareScalarListOps {
    DeclareBinop <scalar_left_list_op<T,ADD>, ADD> add1;
    DeclareBinop <scalar_right_list_op<T,ADD>, ADD> add2;
    DeclareBinop <scalar_left_list_op<T,SUBTRACT>, SUBTRACT> subtract1;
    DeclareBinop <scalar_right_list_op<T,SUBTRACT>, SUBTRACT> subtract2;
    DeclareBinop <scalar_left_list_op<T,MULTIPLY>, MULTIPLY> multiply1;
    DeclareBinop <scalar_right_list_op<T,MULTIPLY>, MULTIPLY> multiply2;
    DeclareBinop <scalar_left_list_op<T,DIVIDE>, DIVIDE> divide1;
    DeclareBinop <scalar_right_list_op<T,DIVIDE>, DIVIDE> divide2;
    DeclareBinop <scalar_left_list_op<T,EXPONENT>, EXPONENT> exponent1;
    DeclareBinop <scalar_right_list_op<T,EXPONENT>, EXPONENT> exponent2;
  };
    
  // these don't work for some reason I forget (AC 10/9/2001)
  
  /*
  _STLP_TEMPLATE_NULL
  struct binop_less<List,List,List> {
    typedef List Arg1Type;
    typedef List Arg2Type;
    typedef List ResultType;

    List operator () (Listarg1, const List &arg2) {
      bool result;
      List::const_iterator index1,index2;

      for (index1 = arg1.begin(), index2 = arg2.begin();
	   index1 != arg1.end() || index2 != arg2.end();
	   ++index1,++index2) {
	Untyped v1 = *index1;
	Untyped v2 = *index2;
	BaseBinop *op = LookupBinop(LESS,v1->TypeOf(),v2->TypeOf());
	if (op != NULL) {
	  result.append(op->eval_from_data(*v1,*v2));
	} else {
	  result.append(&undef);
	}
      }
    }
  };

  _STLP_TEMPLATE_NULL
  struct binop_greater<List,List,List> {
    typedef List Arg1Type;
    typedef List Arg2Type;
    typedef List ResultType;

    List operator () (const List &arg1, const List &arg2) {
      List result;
      List::const_iterator index1,index2;

      for (index1 = arg1.begin(), index2 = arg2.begin();
	   index1 != arg1.end() || index2 != arg2.end();
	   ++index1,++index2) {
	Untyped v1 = *index1;
	Untyped v2 = *index2;
	BaseBinop *op = LookupBinop(GREATER,v1->TypeOf(),v2->TypeOf());
	if (op != NULL) {
	  result.append(op->eval_from_data(*v1,*v2));
	} else {
	  result.append(&undef);
	}
      }
    }
  };

  _STLP_TEMPLATE_NULL
  struct binop_less_eq<List,List,List> {
    typedef List Arg1Type;
    typedef List Arg2Type;
    typedef List ResultType;

    List operator () (const List &arg1, const List &arg2) {
      List result;
      List::const_iterator index1,index2;

      for (index1 = arg1.begin(), index2 = arg2.begin();
	   index1 != arg1.end() || index2 != arg2.end();
	   ++index1,++index2) {
	Untyped v1 = *index1;
	Untyped v2 = *index2;
	BaseBinop *op = LookupBinop(LTEQ,v1->TypeOf(),v2->TypeOf());
	if (op != NULL) {
	  result.append(op->eval_from_data(*v1,*v2));
	} else {
	  result.append(&undef);
	}
      }
    }
  };

  _STLP_TEMPLATE_NULL
  struct binop_greater_eq<List,List,List> {
    typedef List Arg1Type;
    typedef List Arg2Type;
    typedef List ResultType;

    List operator () (const List &arg1, const List &arg2) {
      List result;
      List::const_iterator index1,index2;

      for (index1 = arg1.begin(), index2 = arg2.begin();
	   index1 != arg1.end() || index2 != arg2.end();
	   ++index1,++index2) {
	Untyped v1 = *index1;
	Untyped v2 = *index2;
	BaseBinop *op = LookupBinop(GTEQ,v1->TypeOf(),v2->TypeOf());
	if (op != NULL) {
	  result.append(op->eval_from_data(*v1,*v2));
	} else {
	  result.append(&undef);
	}
      }
    }
  };
  
  _STLP_TEMPLATE_NULL
  struct binop_eq<List,List,List> {
    typedef List Arg1Type;
    typedef List Arg2Type;
    typedef List ResultType;

    List operator () (const List &arg1, const List &arg2) {
      List result;
      List::const_iterator index1,index2;

      for (index1 = arg1.begin(), index2 = arg2.begin();
	   index1 != arg1.end() || index2 != arg2.end();
	   ++index1,++index2) {
	Untyped v1 = *index1;
	Untyped v2 = *index2;
	BaseBinop *op = LookupBinop(ADD,v1->TypeOf(),v2->TypeOf());
	if (op != NULL) {
	  result.append(op->eval_from_data(*v1,*v2));
	} else {
	  result.append(&undef);
	}
      }
    }
  };

  */

  static DeclareBinop <add<List,List,List>, ADD> add1;
  static DeclareBinop <subtract<List,List,List>, SUBTRACT> subtract1;
  static DeclareBinop <multiply<List,List,List>, MULTIPLY> multiply1;
  static DeclareBinop <divide<List,List,List>, DIVIDE> divide1;
  static DeclareBinop <exponent<List,List,List>,EXPONENT > exponent1;

  static DeclareScalarListOps <int> listint;
  static DeclareScalarListOps <short> listshort;
  static DeclareScalarListOps <float> listfloat;
  static DeclareScalarListOps <double> listdouble;
  static DeclareScalarListOps <complex<float> > listcomplex;
  static DeclareScalarListOps <complex<double> > listdoublecomplex;

  OSW_DECLARE_USE(add1);
  OSW_DECLARE_USE(subtract1);
  OSW_DECLARE_USE(multiply1);
  OSW_DECLARE_USE(divide1);
  OSW_DECLARE_USE(exponent1);

  OSW_DECLARE_USE(listint);
  OSW_DECLARE_USE(listshort);
  OSW_DECLARE_USE(listfloat);
  OSW_DECLARE_USE(listdouble);
  OSW_DECLARE_USE(listcomplex);
  OSW_DECLARE_USE(listdoublecomplex);

}

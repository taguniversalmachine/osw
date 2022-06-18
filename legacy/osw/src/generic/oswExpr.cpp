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
   oswExpr.cpp
   Expression evaluator and conditional evaluator

   Amar Chaudhary
*/

#include "oswBinop.h"
#include "oswMath.h"
#include "oswParseExpr.h"
#include "oswList.h"
#include <limits>

namespace osw {

  struct Node {
    virtual ~Node() {}
    virtual const type_info &Type() const = 0;
    virtual void GetResult (BaseState &result) = 0;
    virtual bool isConstant () {return false;}
  };
  

  static char *RandomName () {
    static char name[5];
    name[0] = 'a' + (rand() % 26);
    name[1] = 'a' + (rand() % 26);
    name[2] = 'a' + (rand() % 26);
    name[3] = 'a' + (rand() % 26);
    name[4] = '\0';
    return name;
  }

  class Expr;

  static Node *MakeExprTree(Transform *transform, SubExpr *subexpr);

  /**************/

  class Expr : public TimeDomainTransform {

  public:
    
    vector<BaseState *> inlets;
    BaseState           *result;

    Expr (const string &aname, Container *acontainer,
	  int argc, char *argv[]) :
      TimeDomainTransform (aname,acontainer,argc,argv) {

	  node = NULL;
	  outlet = NULL;
      if (argc > 0) {
        string exprStr;
        for (int i = 0; i < argc; ++i) {
          exprStr += ' ';
          exprStr += argv[i];
        }
        
        ParseStruct *p = Parse(const_cast<char *>(exprStr.c_str()));
        if (p->expr) {
          node = MakeExprTree(this,p->expr);
          if (node) {
            outlet = MakeOutlet(node->Type().name(),"out",this,"Output from expression");
            if (NumInlets() == 0) {
              new Inlet<Unit>("doExpr",this,"Evaluate constant expression");
            }
	    
            activations.reserve(NumInlets());
            
            for (InletIterator inletIndex = BeginInlets();
                 !EndInlets(inletIndex);
                 inletIndex = NextInlet(inletIndex)) {
              activations.push_back(new Activation<Expr>(InletSelf(inletIndex),
                                                         this,
                                                         &Expr::activation1Expr));
              (*(activations.end()-1))->AddEffect(outlet);
            }
            
          }
        }
        FreeParseStruct(p);
      }
    }

    ~Expr() {
      if (node) {
	delete node;
      }
      for (int i = 0; i < activations.size(); ++i) {
	if (activations[i]) {
	  delete activations[i];
	}
      }
      if (outlet) {
	delete outlet;
      }
    }
     
  private:
    Node              *node;
    stl::vector<Activation<Expr> *>  activations;
    BaseState         *outlet;

    void activation1Expr() {
      if (node) {
	node->GetResult(*outlet);
      }
    }
  };
  
  static TransformFactory<Expr>  ExprFactory("Expr",true);
  OSW_DECLARE_USE(ExprFactory);

  /**********************************/

  template<class T>
  struct ConstantNode : public Node {
    
    ConstantNode(const T &aval) :
      val(aval) {
    }
    
    const type_info &Type() const {return typeid(T);}
    void GetResult (BaseState &result) {
      dynamic_cast<State<T> &>(result) = val;
    }
    bool isConstant () {return true;}
    
    T val;
  };


  struct ListNilNode : public Node {
    
    const type_info &Type() const {return typeid(List);}
    
    void GetResult (BaseState &result) {
      dynamic_cast<State<List> &>(result) = List();
    }
    bool isConstant () {return true;
    }
  };

  struct VariableNode : public Node {

    VariableNode(BaseState &avar) :
      var(avar) {
    }

    const type_info &Type() const {return var.TypeOf();}
    void GetResult (BaseState &result) {
      result = var;
    }   

    BaseState &var;
  };

  struct BinopNode : public Node {

    BinopNode (Transform *parent, 
               BaseBinop *abinop, 
               Node *aleft, 
               Node *aright,
               int aopnum) :
      binop(abinop),left(aleft),right(aright) {
      leftState = MakeState(left->Type().name(),RandomName(),parent);
      rightState = MakeState(right->Type().name(),RandomName(),parent);
      xopnum = aopnum;
    }
    ~BinopNode() {
      delete left;
      delete right;
    }
  
    const type_info &Type() const { return (binop) ? binop->ResultType() : typeid(Any);}
    void GetResult (BaseState &result) {
      left->GetResult(*leftState);
      right->GetResult(*rightState);
      if (!binop) {
        if (left->Type() == typeid(Any)) {
          State<Any> *leftAny = (State<Any> *)leftState;
          if (right->Type() == typeid(Any)) {
            State<Any> *rightAny = (State<Any> *)rightState;
            binop = LookupBinop(xopnum,
                                (*leftAny)->TypeOf(),
                                (*rightAny)->TypeOf());
            if (binop) {
              BaseDatum *datum = binop->eval_from_data(*(*leftAny)->GetDatum(),
                                                       *(*rightAny)->GetDatum());
              (State<Any> &)result = datum;
            }
          } else {
            binop = LookupBinop(xopnum,
                                (*leftAny)->TypeOf(),
                                rightState->TypeOf());
            if (binop) {
              BaseDatum *datum = binop->eval_from_data(*(*leftAny)->GetDatum(),
                                                       *(rightState->MakeDatum()));
              (State<Any> &)result = datum;
            }
          }
        } else {
          // should be able to assume that right is an Any          
          State<Any> *rightAny = (State<Any> *)rightState;          
          binop = LookupBinop(xopnum,
                              leftState->TypeOf(),
                              (*rightAny)->TypeOf());
          if (binop) {
            BaseDatum *datum = binop->eval_from_data(*(leftState->MakeDatum()),
                                                     *(*rightAny)->GetDatum());
            (State<Any> &)result = datum;
          }
        }
        if (!binop) {
          Error("Invalid dynamic types to binary operator");
        }
        binop = NULL;
      } else {
        binop->eval(*leftState,*rightState,result);
      }
    }

    Node *left,*right;
    BaseState *leftState,*rightState;
    BaseBinop *binop;
    int xopnum;
  };

  struct NegationNode : public Node {

    NegationNode (Transform *parent, Node *ainput) :
      input(ainput) {
      state = MakeState(input->Type().name(),RandomName(),parent);
    }

    ~NegationNode() {
      delete input;
    }

    const type_info &Type() const {return state->TypeOf();}
    void GetResult (BaseState &result) {
      input->GetResult(*state);
      if (state->TypeOf() == typeid(float)) {
	dynamic_cast<State<float> &>(result) = -((State<float> &)(*state));
      } else if (state->TypeOf() == typeid(Samples)) {
	dynamic_cast<State<Samples> &>(result) = -((State<Samples> &)(*state));
      } else if (state->TypeOf() == typeid(int)) {
	dynamic_cast<State<int> &>(result) = -((State<int> &)(*state));
      } else if (state->TypeOf() == typeid(double)) {
	dynamic_cast<State<double> &>(result) = -((State<double> &)(*state));
      } else if (state->TypeOf() == typeid(Integer16Samples)) {
	dynamic_cast<State<Integer16Samples> &>(result) = -((State<Integer16Samples> &)(*state));
      } else if (state->TypeOf() == typeid(complex<float>)) {
	dynamic_cast<State<complex<float> > &>(result) = -((State<complex<float> > &)(*state));
      } else if (state->TypeOf() == typeid(complex<double>)) {
	dynamic_cast<State<complex<double> > &>(result) = -((State<complex<double> > &)(*state));
      } else if (state->TypeOf() == typeid(IntegerSamples)) {
	dynamic_cast<State<IntegerSamples> &>(result) = -((State<IntegerSamples> &)(*state));
      } else if (state->TypeOf() == typeid(DoubleSamples)) {
	dynamic_cast<State<DoubleSamples> &>(result) = -((State<DoubleSamples> &)(*state));
      } else if (state->TypeOf() == typeid(vector<complex<float> >)) {
	dynamic_cast<State<vector<complex<float> > >&>(result) = -((State<vector<complex<float> > >&)(*state));
      } else if (state->TypeOf() == typeid(vector<complex<float> >)) {
	dynamic_cast<State<vector<complex<double> > >&>(result) = -((State<vector<complex<double> > >&)(*state));
      } else {
	Error ("Input type not supported");
      }
    }

    Node      *input;
    BaseState *state;
  };

  struct LogicalNegationNode : public Node {
    
    LogicalNegationNode (Transform *parent, Node *ainput) :
      input(ainput) {
      state = MakeState(input->Type().name(),RandomName(),parent);
    }
    ~LogicalNegationNode() {
      delete input;
    }

    const type_info &Type() const {return state->TypeOf();}
    void GetResult (BaseState &result) {
      input->GetResult(*state);
      if (state->TypeOf() == typeid(int)) {
	dynamic_cast<State<int> &>(result) = !((State<int> &)(*state));
      } else if (state->TypeOf() == typeid(Integer16Samples)) {
	dynamic_cast<State<Integer16Samples> &>(result) = !((State<Integer16Samples> &)(*state));
      } else if (state->TypeOf() == typeid(IntegerSamples)) {
	dynamic_cast<State<IntegerSamples> &>(result) = !((State<IntegerSamples> &)(*state));
      } else {
	Error ("Input type not supported");
      }
    }
    
    Node      *input;
    BaseState *state;
  };

  struct FunctionNode : public Node {
    
    FunctionNode (Transform *parent, BaseDeclareMathFunc *afunc, Node *ainput) :
      func(afunc),
      input(ainput) {
      state = MakeState(input->Type().name(),RandomName(),parent);
    }

    const type_info &Type() const {return func->ReturnType();}

    void GetResult (BaseState &result) {
      input->GetResult(*state);
      func->eval(*state,result);
    }

    Node *input;
    BaseDeclareMathFunc *func;
    BaseState *state;
  };

  struct ListSingleNode : public Node {
    ListSingleNode (Transform *parent, Node *asingle) :
      single(asingle) {
      singleState = MakeState(single->Type().name(),RandomName(),parent);
    }

    ~ListSingleNode() {
      delete single;
    }

    const type_info &Type() const {return typeid(List);}
    void GetResult(BaseState &result) {
      single->GetResult(*singleState);
      List alist;
      if (single->Type() == typeid(Any)) {
        alist.append((*((State<Any> *)singleState))->GetDatum());
      } else {
        alist.append(singleState->MakeDatum());
      }
      (State<List> &) result = alist;
    }
    Node *single;
    BaseState *singleState;
  };

  struct ConsNode : public Node {

    ConsNode (Transform *parent, Node *aleft, Node *aright) :
      left(aleft),right(aright)  {
      leftState = MakeState(left->Type().name(),RandomName(),parent);
      rightState = MakeState(right->Type().name(),RandomName(),parent);
    }
    ~ConsNode() {
      delete left;
      delete right;
    }

    const type_info &Type() const {return typeid(List);}
    void GetResult(BaseState &result) {
      try {
        left->GetResult(*leftState);
        right->GetResult(*rightState);  
        State<List> &leftListState = dynamic_cast<State<List> &>(*leftState);
        if (right->Type() == typeid(Any)) {
          leftListState->append((*((State<Any> *)rightState))->GetDatum());
        } else {
          leftListState->append(rightState->MakeDatum());
        }
        (State<List> &) result = leftListState;
      } catch (...) {
        Error("Left side of list constructor not a list!");
      }
    }

    Node *left,*right;
    BaseState *leftState,*rightState;
  };


  struct IndexNode : public Node {

    IndexNode (Transform *parent, Node *aleft, Node *aright) :
      left(aleft),right(aright)  {
      leftState = MakeState(left->Type().name(),RandomName(),parent);
      rightState = MakeState(right->Type().name(),RandomName(),parent);
    }
    ~IndexNode() {
      delete left;
      delete right;
    }
  
    const type_info &Type() const { return typeid(Any);}
    void GetResult (BaseState &result) {
      left->GetResult(*leftState);
      right->GetResult(*rightState);     
      if (left->Type() == typeid(List)) {        
        if (right->Type() == typeid(Integer)) {
          (State<Any> &)result = (*((State<List> *)leftState))->nth(*((State<Integer> *)rightState));
        } else if (right->Type() == typeid(Float)) {
          (State<Any> &)result = (*((State<List> *)leftState))->nth(Integer(*((State<Float> *)rightState)));
        } else if (right->Type() == typeid(Double)) {
          (State<Any> &)result = (*((State<List> *)leftState))->nth(Integer(*((State<Double> *)rightState)));
        } else if (right->Type() == typeid(Any)) {
          Untyped untyped;
          untyped = (*((State<Any> *) rightState))->GetDatum();
          if (untyped->TypeOf() == typeid(Integer)) {
            (State<Any> &)result = (*((State<List> *)leftState))->nth(*((Datum<Integer> *)untyped));
          } else if (untyped->TypeOf() == typeid(Float)) {
            (State<Any> &)result = (*((State<List> *)leftState))->nth(Integer(*((Datum<Float> *)untyped)));
          } else if (untyped->TypeOf() == typeid(Double)) {
            (State<Any> &)result = (*((State<List> *)leftState))->nth(Integer(*((Datum<Double> *)untyped)));
          } else {
            Error("Invalid index type");
          }
        }
      } else if (left->Type() == typeid(Any)) {
        Untyped untyped0;
        untyped0 = (*((State<Any> *)leftState))->GetDatum();
        if (untyped0->TypeOf() == typeid(List)) {
          if (right->Type() == typeid(Integer)) {
            (State<Any> &)result = List(*(Datum<List> *)untyped0).nth(*((State<Integer> *)rightState));
          } else if (right->Type() == typeid(Float)) {
            (State<Any> &)result = List(*(Datum<List> *)untyped0).nth(Integer(*((State<Float> *)rightState)));
          } else if (right->Type() == typeid(Double)) {
            (State<Any> &)result = List(*(Datum<List> *)untyped0).nth(Integer(*((State<Double> *)rightState)));
          } else if (right->Type() == typeid(Any)) {
            Untyped untyped;
            untyped = (*((State<Any> *) rightState))->GetDatum();
            if (untyped->TypeOf() == typeid(Integer)) {
              (State<Any> &)result = List(*(Datum<List> *)untyped0).nth(*((Datum<Integer> *)untyped));
            } else if (untyped->TypeOf() == typeid(Float)) {
              (State<Any> &)result = List(*(Datum<List> *)untyped0).nth(Integer(*((Datum<Float> *)untyped)));
            } else if (untyped->TypeOf() == typeid(Double)) {
              (State<Any> &)result = List(*(Datum<List> *)untyped0).nth(Integer(*((Datum<Double> *)untyped)));
            } else {
              Error("Invalid index type");
            }
          }
        } else {
          Error("List type expected");
        }
      } else {
        Error("List type expected");
      }
    }

    Node *left,*right;
    BaseState *leftState,*rightState;
    BaseBinop *binop;

  };

  struct UnitNode : public Node {
    const type_info &Type() const {return typeid(Unit);}
    
    void GetResult (BaseState &result) {
      dynamic_cast<State<Unit> &>(result) = Unit();
    }
    bool isConstant () {return true;
    }
  };
    
  static struct {
    char *prefix;
    char *type;
  } typePrefixes[] = {
    {"cds","ComplexDoubleSamples"},
    {"cd","ComplexDouble"},
    {"ds","DoubleSamples"},
    {"ss","Integer16Samples"},
    {"is","IntegerSamples"},
    {"cs","ComplexSamples"},
    {"f","Float"},
    {"d","Double"},
    {"i","Integer"},
    {"s","Samples"},
    {"c","Complex"},
    {"l","List"},
    {NULL,NULL}
  };

  static BaseBinop *LookupBinop (const type_info &type1, const type_info &type2,int opnum) {
    BinopMap::iterator found = TheBinopMap[opnum].find(type1.name());
    if (found != TheBinopMap[opnum].end()) {
      stl::map<string, BaseBinop *, less<string> >::iterator reallyFound
	= (*found).second.find(type2.name());
      if (reallyFound != (*found).second.end()) {
	return (*reallyFound).second;
      }
    }
    return NULL;
  }

  static Node *MakeBinopExprTree(Transform *transform, SubExpr *subexpr, int opnum) {
    Node *left = MakeExprTree(transform,subexpr->part1);
    Node *right = MakeExprTree(transform,subexpr->part2);
    if (!left || !right) {
      return NULL;
    }
    BaseBinop *binop = LookupBinop(left->Type(),right->Type(),opnum);
    if (binop == NULL) {
      if (left->Type() != typeid(Any) && right->Type() != typeid(Any)) {
        ostringstream os;
        os << transform->PathName() << ": incorrect types for operator "
           << opnum << ": " << left->Type().name() << " & " << right->Type().name() << endl;
        Error(os.str());
        return NULL;
      }
    }
    return new BinopNode(transform,binop,left,right,opnum);
  }


  static Node *MakeExprTree(Transform *transform, SubExpr *subexpr) {
    switch (subexpr->expr_type) {
    case INTVAL :
      return new ConstantNode<int>(subexpr->val.ival);
    case DOUBLEVAL :
      return new ConstantNode<double>(subexpr->val.dval);
    case FLOATVAL :
      return new ConstantNode<float>(subexpr->val.fval);
    case COMPLEXVAL :
      return new ConstantNode<complex<float> >(complex<float>(0.0f,subexpr->val.fval));
    case DOUBLECOMPLEXVAL :
      return new ConstantNode<complex<double> >(complex<double>(0.0f,subexpr->val.fval));
    case VARIABLE :
      {
	BaseState *state = dynamic_cast<BaseState *>(Nameable::Find(subexpr->strval,transform));
	if (state == NULL) {
          state = dynamic_cast<BaseState *>(Nameable::Find(subexpr->strval,
                                                           transform->GetPatch()));
          if (state == NULL) {
            for (int i = 0; typePrefixes[i].prefix != NULL; ++i) {
              if (!strnicmp(typePrefixes[i].prefix,subexpr->strval,strlen(typePrefixes[i].prefix))) {
                state = MakeInlet(typePrefixes[i].type,subexpr->strval,transform);
                break;
              }
            }
          }
        }
	if (state == NULL) {
	  Error(transform->PathName()+": unknown type prefix: "+subexpr->strval);
          state = new Inlet<Any>(subexpr->strval,transform,"Expression variable");
        }
        return new VariableNode(*state);
      }
    case ADDITION :
      return MakeBinopExprTree(transform,subexpr,ADD);
    case SUBTRACTION :
      return MakeBinopExprTree(transform,subexpr,SUBTRACT);
    case MULTIPLY_OP :
      return MakeBinopExprTree(transform,subexpr,MULTIPLY);
    case DIVIDE_OP :
      return MakeBinopExprTree(transform,subexpr,DIVIDE);
    case MODULUS_OP :
      return MakeBinopExprTree(transform,subexpr,MODULUS);
    case EXPONENT_OP :
      return MakeBinopExprTree(transform,subexpr,EXPONENT);
    case LT_OP :
      return MakeBinopExprTree(transform,subexpr,LESS);
    case GT_OP :
      return MakeBinopExprTree(transform,subexpr,GREATER);
    case LTEQ_OP :
      return MakeBinopExprTree(transform,subexpr,LTEQ);
    case GTEQ_OP :
      return MakeBinopExprTree(transform,subexpr,GTEQ);
    case EQ_OP :
      return MakeBinopExprTree(transform,subexpr,EQUAL);
    case LOG_AND :
      return MakeBinopExprTree(transform,subexpr,LOGAND);
    case LOG_OR :
      return MakeBinopExprTree(transform,subexpr,LOGOR);
    case BIT_AND :
      return MakeBinopExprTree(transform,subexpr,BITAND);
    case BIT_OR :
      return MakeBinopExprTree(transform,subexpr,BITOR);
    case BIT_XOR :
      return MakeBinopExprTree(transform,subexpr,BITXOR);
    case LSHIFT_OP :
      return MakeBinopExprTree(transform,subexpr,SHIFTL);
    case RSHIFT_OP :
      return MakeBinopExprTree(transform,subexpr,SHIFTR);
    case NEGATION :
      {
	Node *input = MakeExprTree(transform,subexpr->part1);
	if (input) {
	  return new NegationNode(transform,input);
	}
      }
      break;
    case LOGICAL_NEG :
      {
	Node *input = MakeExprTree(transform,subexpr->part1);
	if (input) {
	  return new LogicalNegationNode(transform,input);
	}
      }
      break;
    case FUNCTION :
      {
	Node *input = MakeExprTree(transform,subexpr->part1);
	if (input) {
	  BaseDeclareMathFunc *func = LookupMathFunc(subexpr->strval,
						     input->Type().name());
	  if (func) {
	    return new FunctionNode(transform,func,input);
	  } else {
	    Error(transform->PathName()+": cannot find a function "+subexpr->strval+" for type "+input->Type().name());
	  }
	}
      }
    case LISTINDEX :
      {
        Node *left = MakeExprTree(transform,subexpr->part1);
        Node *right = MakeExprTree(transform,subexpr->part2);
        if (!left || !right) {
          return NULL;
        }
        return new IndexNode(transform,left,right);
      }
    case LISTCONS :
      {
        Node *left = MakeExprTree(transform,subexpr->part1);
        Node *right = MakeExprTree(transform,subexpr->part2);
        if (!left || !right) {
          return NULL;
        }
        return new ConsNode(transform,left,right);
      }
    case LISTSINGLE :
      {
        Node *left = MakeExprTree(transform,subexpr->part1);
        if (!left) {
          return NULL;
        }
        return new ListSingleNode(transform,left);
      }
    case LISTNIL :
      return new ListNilNode();
    case UNIT_EXPR :
      return new UnitNode();
    default:
      Error(transform->PathName()+": unknown or unsupported expression type");
    }
    
    return NULL;
  }

  /******************/

  class If : public TimeDomainTransform {

  public:
    
    vector<BaseState *> inlets;
    BaseState           *result;

    If (const string &aname, Container *acontainer,
	  int argc, char *argv[]) :
      TimeDomainTransform (aname,acontainer,argc,argv),
      predicate("predicate",this,"most recent value of predicate expression") {

      node1 = node2 = node3 = NULL;
	  consequent = alternative = NULL;
      if (argc > 0) {
        int i;
        for (i = 0; i < argc; ++i) {
          exprStr += ' ';
          if (!strcmp(argv[i],"then")) {
            ++i;
            break;
          }
          exprStr += argv[i];
        }
        for (; i < argc; ++i) {
          thenStr += ' ';
          if (!strcmp(argv[i],"else")) {
            ++i;
            break;
          }
          thenStr += argv[i];
        }
        for (; i < argc; ++i) {
          elseStr += ' ';
          elseStr += argv[i];
        }
        
        ParseStruct *p = Parse(const_cast<char *>(exprStr.c_str()));
        if (p->expr) {
          node1 = MakeExprTree(this,p->expr);
          if (node1) {
            if (node1->Type() != typeid(bool)) {
              Error("Predicate must return type bool.");
            } else {
              activations.reserve(NumInlets());
              
              for (InletIterator inletIndex = BeginInlets();
                   !EndInlets(inletIndex);
                   inletIndex = NextInlet(inletIndex)) {
                activations.push_back(new Activation<If>(InletSelf(inletIndex),
                                                         this,
                                                         &If::activation1Expr));
              }
              
              ParseStruct *q = Parse(const_cast<char *>(thenStr.c_str()));
              if (q->expr) {
                node2 = MakeExprTree(this,q->expr);
                if (node2) {
                  consequent = MakeOutlet(node2->Type().name(),"consequent",this,"Output if expression is true");
                  for (int i = 0; i < activations.size(); ++i) {
                    activations[i]->AddEffect(consequent);
                  }
                }
              } else {
                consequent = new Outlet<string>("consequent",this,"Output if expression is true");
                for (int i = 0; i < activations.size(); ++i) {
                  activations[i]->AddEffect(consequent);
                }
              }
              FreeParseStruct(q);
              if (elseStr != "" && elseStr != " ") {
                q = Parse(const_cast<char *>(elseStr.c_str()));
                if (q->expr) {
                  node3 = MakeExprTree(this,q->expr);
                  if (node3) {
                    alternative = MakeOutlet(node3->Type().name(),"alternative",this,"Output if expression is false");
                    for (int i = 0; i < activations.size(); ++i) {
                      activations[i]->AddEffect(alternative);
                    }
                  }
                } else {
                  alternative = new Outlet<string>("alternative",this,"Output if expression is false");
                  for (int i = 0; i < activations.size(); ++i) {
                    activations[i]->AddEffect(alternative);
                  }
                }
                FreeParseStruct(q);
              }
            }
          }
        }
        FreeParseStruct(p);
      }
    }

    ~If() {
      if (node1) {
	delete node1;
      }
      if (node2) {
	delete node2;
      }
      if (node3) {
	delete node3;
      }
      for (int i = 0; i < activations.size(); ++i) {
	if (activations[i]) {
	  delete activations[i];
	}
      }
      if (consequent) {
	delete consequent;
      }
      if (alternative) {
	delete alternative;
      }
    }
     
  private:
    Node              *node1,*node2,*node3;
    stl::vector<Activation<If> *>   activations;
    BaseState         *consequent,*alternative;
    State<bool>       predicate;
    string exprStr,thenStr,elseStr;

    void activation1Expr() {
      if (node1) {
	node1->GetResult(predicate);
	if ((bool)predicate) {
	  if (node2) {
	    node2->GetResult(*consequent);
	  } else {
	    *(dynamic_cast<Outlet<string> *>(consequent)) = thenStr;
	  } 
	} else if (node3) {
	  node3->GetResult(*alternative);
	} else if (alternative) {
	  *(dynamic_cast<Outlet<string> *>(alternative)) = elseStr;
	}
      }
    }
  };
  
  static TransformFactory<If>  IfFactory("If",true);
  OSW_DECLARE_USE(IfFactory);

  /*********************************/
  // C++ numeric routines to support the parser
  
  extern "C" float GetFloatInfinity () {
    return numeric_limits<float>::infinity();
  }

  extern "C" double GetDoubleInfinity () {
    return numeric_limits<double>::infinity();
  }

  extern "C" float GetFloatNaN () {
    return numeric_limits<float>::quiet_NaN();
  }

  extern "C" double GetDoubleNaN () {
    return numeric_limits<double>::quiet_NaN();
  }
}



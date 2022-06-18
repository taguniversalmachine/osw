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
   oswlist.cpp
   Implementation of dynamic list type
   and some basic transforms for creating/manipulating them
*/

#include "oswTcl.h"
#include "oswList.h"
#include "oswLex.h"
#include "oswDataSource.h"
#include <fstream>
#include <sstream>

using namespace std;

namespace osw {

  List::List(const List &toCopy) {
    for (const_iterator p = toCopy.ximp.begin(); p != toCopy.ximp.end(); ++p) {
      ximp.push_back(*p);
    }
  }

  List::~List() {
    clear();
  }


  List & List::operator = (const List &toCopy) {
    if (this == &toCopy) {
      return *this;
    }
    iterator p;
    clear();    
    ximp = toCopy.ximp;
    return *this;
  }
  
  bool List::operator == (const List &toCompare) const {
    return false; // for now
  }

  ostream & operator << (ostream &os, const List &toPrint) {
    for (List::const_iterator p = toPrint.ximp.begin(); p != toPrint.ximp.end(); ++p) {
      if ((*p).datum->TypeOf() == typeid(List)) {
	    os << "{ " << *((*p).datum) << " }";
      } else {
	  os << *((*p).datum);
      }
      List::const_iterator q = p;
      ++q;
      if (q != toPrint.ximp.end()) {
	os << ' ';
      }
      
    }

    return os;
  }

  static istream & GetList (istream &is, List &toInput, int nest = 0, char *oldbuf = NULL) {

    char strbuf[256];
    strbuf[0] = 0;
    char *bufptr = (oldbuf != NULL) ? oldbuf : strbuf;
    toInput.clear();
    while (!is.eof()) {
      if (bufptr[0] == 0) {
	while (isspace(is.peek())) {
	  is.get();
	}
	if (!is.good()) {
	  break;
	}
	if (is.eof()) {
	  break;
	}
	is >> strbuf;
	bufptr = strbuf;
      }
      Token *token = GetToken(bufptr);
      bufptr = token->remaining;
      switch (token->type()) {
      case TOKEN_INT :
	toInput.append(new Datum<int>(((IntToken *)token)->val));
	break;
      case TOKEN_FLOAT :
	{
	  Datum<float> *d = new Datum<float>(((FloatToken *)token)->val) ;
	  toInput.append(d);
	}
	break;
      case TOKEN_DOUBLE :
	toInput.append(new Datum<double>(((DoubleToken *)token)->val));
	break;
      case TOKEN_COMPLEX :
	toInput.append(new Datum<complex<float> >
		     (((ComplexToken *)token)->val));
	break;
      case TOKEN_DOUBLECOMPLEX :
	toInput.append(new Datum<complex<double> >
		     (((DoubleComplexToken *)token)->val));
	break;
      case TOKEN_BOOL :
	toInput.append(new Datum<bool>(((BoolToken *)token)->val));
	break;
      case TOKEN_UNIT :
	toInput.append(new Datum<unit>(((UnitToken *)token)->val));
	break;
      case TOKEN_STRING :
	toInput.append(new Datum<string>(((StringToken *)token)->val));
	break;
      case TOKEN_END_LIST :
	if (nest == 0) {
	  Error ("unmatched \"}\" in list definition");
	} else {
	  for (int i = strlen(bufptr)-1; i >= 0; --i) {
	    is.putback(bufptr[i]);
	  }
	  return is;
	}
	/*case TOKEN_SEMICOLIN :
	if (nest == 0) {
	  	  error ("unmatched \"}\" in list definition");
	}
	return is; */
      case TOKEN_BEGIN_LIST :
	{
	  Datum<List> *newlist = new Datum<List>;
	  GetList(is,newlist->Reference(),nest+1,bufptr);
	  is >> bufptr;
	  toInput.append(newlist);
	  break;
	}
      }      

    }
    if (nest != 0) {
      Error ("missing end-of-list (\"}\")");
    }
    return is;
  }

  istream & operator >> (istream &is, List &toInput) {
    toInput.clear();
    return GetList(is,toInput);
  }

  REGISTER_TYPE(List);

  /**************************/

  class _oswexport ListTransform : public Transform, public DataSource {

  public:
    
    Inlet<unit> unitIn;
    State<List> xlist;
    Outlet<List> listOut;
    Inlet<string> file;
    
    ListTransform (const string &aname, Container *acontainer,
		   int argc, char *argv[]) :
      Transform (aname,acontainer,argc,argv),
      DataSource(this,acontainer,argc,argv),
      unitIn("unitIn",this,"Send the list now!"),
      listOut("listOut",this),
      xlist("xlist",this),
      activation1(&unitIn,this,&ListTransform::activation1Expr) {
      activation1.AddEffect(&listOut);
      
      if (file != string("")) {
	ifstream is(file->c_str(),ios::in);
	if (!is.fail()) {
	  is >> xlist;
	} else {
	  Error ("cannot open file " + file);
	}
      } else {
	string liststring;
	for (int i = 0; i < argc-1; ++i) {
	  liststring += argv[i];
	  liststring += " ";
	}
	if (argc > 0) {
	  liststring += argv[argc-1];
	}
	istringstream is(liststring.c_str());
	is >> xlist;
      }
    }
    
    const char *Description () const {
      return "Create a new list object.";
    }
    
    const char *Dialog () {
      return "oswLoadScript oswBigTextBox.tcl\n"
	"bigTextBox [file tail $transformPath] "
	" [list oswSetData $transformPath] "
	" [xjoin [oswGetData $transformPath]] \n";
      //return "oswLoadScript oswLoadFile.tcl;\n"
      //	"oswLoadFile $transformPath/file {.txt} {Text File} ;\n";
    }
    
  protected:
    
    Activation<ListTransform> activation1;
    void activation1Expr () {
      listOut = xlist;
    }
    
    void SetDataFromSource () {
      if (src == string("")) {
	return;
      }
      unitIn.Lock();
      ifstream is(src->c_str());
      is >> xlist;
      unitIn.Unlock();
    }
    
    void SetData (const string &adata) {
      DataSource::SetData(adata);
      unitIn.Lock();
      string data = adata;
      int spacer = data.size()-1;
      while (isspace(data[spacer])) {
	--spacer;
      }
      data.erase(spacer+1,string::npos);
      istringstream is(data);
      is >> xlist;
      unitIn.Unlock();
    }
    
    string GetData () const {
      ostringstream os;
      os << xlist;
      return os.str();
    }
    
  };

  static TransformConstructor<ListTransform> 
    ListTransformConstructor("List",true);


  /****************/

  class Pack : public Transform {

  public:
    
    Outlet<List>              listOut;
    std::vector<BaseState *>  inputs;
        
    Pack (const string &aname, Container *acontainer,
	   int argc, char *argv[]) :
      Transform (aname,acontainer,argc,argv),
      listOut("listOut",this,""),
      activation1(ScanArguments<int>("-inputs",2,acontainer,argc,argv),
		  this,&Pack::activation1Expr) {
    	
	int NumInputs = ScanArguments<int>("-inputs",2,acontainer,argc,argv);
	inputs.reserve(NumInputs);

	char buffer[32];
	for (int i = 0; i < NumInputs; ++i) {
	  sprintf (buffer,"in%d",i+1);
	  inputs.push_back(new Inlet<ChangeableInlet::Changeable>(buffer,this,""));
	  inputs[i]->SetOrder(i);
	  //activation1.AddActivator(inputs[i]);
	}
	activation1.AddEffect(&listOut);
	listOut.SetOrder(0);
	numconnected = 0;
    }
    
    ~Pack() {
      for (int i = 0; i < inputs.size(); ++i) {
        delete inputs[i];
      }
    }
    
    virtual bool Connect (BaseOutlet *source, BaseState *&sink) {
      BaseState *sourceAsState = dynamic_cast<BaseState *>(source);

      if (sourceAsState->TypeOf() != sink->TypeOf()) {
      
	int index;
	for (index = 0; index < inputs.size(); ++index) {
	  if (inputs[index] == sink) {
	    break;
	  }
	}
	string inputName = sink->Name();
	delete sink;
	sink = inputs[index] = 
	  sourceAsState->CloneInlet(inputName,this,"");
	sink->SetOrder(index);
      }
      activation1.AddActivator(sink);

      ++numconnected;
      if (numconnected >= inputs.size()) {
        char buffer[32];
        sprintf(buffer,"in%d",numconnected+1);
        inputs.push_back(sink->CloneInlet(buffer,this));
        inputs[numconnected]->SetOrder(numconnected);	
      }
      ostringstream os;
      os << "oswFixTransformTerminals " << PathName() << endl;
      oswTclEvalString(os.str().c_str());
      
      if (!Transform::Connect(source,sink)) {
	--numconnected;
	return false;
      }
      return true;	      
    }

    bool BackDisconnect(BaseState *sink) {
      activation1.RemoveActivator(sink,true);
      --numconnected;
      return Transform::BackDisconnect(sink);
    }

    const char *Description () const {
      return "Packs data from inlets into a list";
    }

  protected:

    Activation<Pack> activation1;
    int numconnected;

    void activation1Expr () {

      List newList;
      for (int i = 0; i < inputs.size(); ++i) {
	if (inputs[i]->IsActivator()) {
          if (inputs[i]->TypeOf() == typeid(Any)) {
	    newList.append((*((State<Any> *)(inputs[i])))->GetDatum());
            //(*((State<Any> *)(inputs[i])))->GetDatum()->AddRef();
	  } else {
	    newList.append(inputs[i]->MakeDatum());
	  }
	}
      }
      listOut = newList;
    }
  };

  static TransformConstructor<Pack> 
    PackConstructor("Pack",true);

  /**************************/


  class AsyncPack : public Transform {

  public:
    
    Outlet<List>              listOut;
    std::vector<BaseState *>  inputs;
        
    AsyncPack (const string &aname, Container *acontainer,
	   int argc, char *argv[]) :
      Transform (aname,acontainer,argc,argv),
      listOut("listOut",this,"") {
      
      int NumInputs = ScanArguments<int>("-inputs",2,acontainer,argc,argv);
      inputs.reserve(NumInputs);
      activation1.reserve(NumInputs);
      char buffer[32];
      for (int i = 0; i < NumInputs; ++i) {
	sprintf (buffer,"in%d",i+1);
	inputs.push_back(new Inlet<ChangeableInlet::Changeable>(buffer,this,""));
	inputs[i]->SetOrder(i);
        activation1.push_back(new Activation<AsyncPack>
			      (0,this,&AsyncPack::activation1Expr));
	activation1[i]->AddEffect(&listOut);
      }
      
      listOut.SetOrder(0);
      numconnected = 0;
    }
    
    ~AsyncPack() {
      for (int i = 0; i < inputs.size(); ++i) {
	delete inputs[i];
	delete activation1[i];
      }
    }
    
    virtual bool Connect (BaseOutlet *source, BaseState *&sink) {
      BaseState *sourceAsState = dynamic_cast<BaseState *>(source);

      if (sourceAsState->TypeOf() != sink->TypeOf()) {
      
	int index;
	for (index = 0; index < inputs.size(); ++index) {
	  if (inputs[index] == sink) {
	    break;
	  }
	}
	string inputName = sink->Name();
	activation1[index]->RemoveActivator(sink,true);
	delete sink;
	sink = inputs[index] = 
	  sourceAsState->CloneInlet(inputName,this,"");
	sink->SetOrder(index);
      }
      int index;
      for (index = 0; index < inputs.size(); ++index) {
	if (inputs[index] == sink) {
	  break;
	}
      }
      activation1[index]->AddActivator(sink);
      ++numconnected;
      if (numconnected >= inputs.size()) {
        char buffer[32];
        sprintf(buffer,"in%d",numconnected+1);
        inputs.push_back(sink->CloneInlet(buffer,this));
        inputs[numconnected]->SetOrder(numconnected);	
        activation1.push_back(new Activation<AsyncPack>
			      (0,this,&AsyncPack::activation1Expr));
	activation1[numconnected]->AddEffect(&listOut);
      }
      ostringstream os;
      os << "oswFixTransformTerminals " << PathName() << endl;
      oswTclEvalString(os.str().c_str());
      
      if (!Transform::Connect(source,sink)) {
	--numconnected;
	return false;
      }
      return true;	      
    }

    bool BackDisconnect(BaseState *sink) {
      //activation1.RemoveActivator(sink,true);
      int index;
      for (index = 0; index < inputs.size(); ++index) {
	if (inputs[index] == sink) {
	  break;
	}
      }
      activation1[index]->RemoveActivator(sink,true);
      --numconnected;
      return Transform::BackDisconnect(sink);
    }

    const char *Description () const {
      return "Aynschronously packs data from inlets into a list";
    }

  protected:

    std::vector<Activation<AsyncPack> *> activation1;
    int numconnected;

    void activation1Expr () {

      List newList;
      for (int i = 0; i < inputs.size(); ++i) {
	if (inputs[i]->IsActivator()) {
          if (inputs[i]->TypeOf() == typeid(Any)) {
	    newList.append((*((State<Any> *)(inputs[i])))->GetDatum());
            //(*((State<Any> *)(inputs[i])))->GetDatum()->AddRef();
	  } else {
	    newList.append(inputs[i]->MakeDatum());
	  }
	}
      }
      listOut = newList;
    }
  };

  static TransformConstructor<AsyncPack> 
    AsyncPackConstructor("AsyncPack",true);



  /**************/

  class Unpack : public Transform {

  public:

    Inlet<List>  toUnpack;
    std::vector<Outlet<Any> *> outputs;

    Unpack(const string &aname, Container *acontainer,
	   int argc, char *argv[]) :
      Transform (aname,acontainer,argc,argv),
      toUnpack("toUnpack",this,""),
      activation1(&toUnpack,this,&Unpack::activation1Expr) {
    	
	int NumOutputs = ScanArguments<int>("-outputs",2,acontainer,argc,argv);
	outputs.reserve(NumOutputs);

	char buffer[32];
	for (int i = 0; i < NumOutputs; ++i) {
	  sprintf (buffer,"out%d",i+1);
	  outputs.push_back(new Outlet<Any>(buffer,this,""));
	  outputs[i]->SetOrder(i);
	  activation1.AddEffect(outputs[i]);
	}
	toUnpack.SetOrder(0);
	numconnected = 0;
    }

    ~Unpack() {
      for (int i = 0; i < outputs.size(); ++i) {
	delete outputs[i];
      }
    }
 
    bool BackConnect (BaseOutlet *source, BaseState *sink) {
      ++numconnected;
      if (numconnected >= outputs.size()) {
	char buffer[32];
	sprintf(buffer,"out%d",numconnected+1);
	outputs.push_back(new Outlet<Any>(buffer,this,""));
	outputs[numconnected]->SetOrder(numconnected);
	activation1.AddEffect(outputs[numconnected]);
	ostringstream os;
	os << "oswFixTransformTerminals " << PathName() << endl;
	oswTclEvalString(os.str().c_str());
      }
      if (!Transform::BackConnect(source,sink)) {
	--numconnected;
	return false;
      }
      return true;
    }
   
    virtual bool Disconnect (BaseOutlet *source) {
      --numconnected;
      return Transform::Disconnect(source);
    }

  protected:

    Activation<Unpack> activation1;
    int numconnected;

    void activation1Expr () {
      
      int n = (outputs.size() < toUnpack->size()) ? outputs.size()
	: toUnpack->size();

      for (int i = 0; i < n; ++i) {
	*outputs[i]=toUnpack->nth(i);
      }
    }
  };

  static TransformConstructor<Unpack> 
    UnpackConstructor("Unpack",true);


  /**************/
  
  class _oswexport Route : public Transform {

  public:

    Inlet<List>  toRoute;
    std::vector<Outlet<List> *> outputs;

    Route(const string &aname, Container *acontainer,
	   int argc, char *argv[]) :
      Transform (aname,acontainer,argc,argv),
      toRoute("toRoute",this,""),
      activation1(&toRoute,this,&Route::activation1Expr) {
    	
	int NumOutputs = argc;
	outputs.reserve(NumOutputs);

	for (int i = 0; i < NumOutputs; ++i) {
	  outputs.push_back(new Outlet<List>(argv[i],this,""));
	  outputs[i]->SetOrder(i);
	  activation1.AddEffect(outputs[i]);
	  Token *token = GetToken(argv[i]);
	  if (token->type() == TOKEN_INT) {
	    xintMap[((IntToken *) token)->val] = i;
	  } else {
	    xstringMap[argv[i]] = i;
	  }
	}
	toRoute.SetOrder(0);
    }

    ~Route() {
      for (int i = 0; i < outputs.size(); ++i) {
	delete outputs[i];
      }
    }
    
  protected:

    typedef stl::map<string,int, less<string> > StringMap;
    typedef stl::map<int,int,less<int> >        IntMap;
    StringMap  xstringMap;
    IntMap     xintMap;

    Activation<Route> activation1;
    
    void activation1Expr () {

      BaseDatum *datum = toRoute->nth(0);
      if (datum->TypeOf() == typeid(int)) {
	IntMap::iterator found = xintMap.find(*((Datum<int> *)(datum)));
	if (found != xintMap.end()) {
	  *outputs[found->second] = toRoute;
	}
      } else {
	ostringstream s;
	s << *datum << '\0';
	StringMap::iterator found = xstringMap.find(s.str());
	if (found != xstringMap.end()) {
	  *outputs[found->second] = toRoute;
	}
      }
    }
  };

  static TransformConstructor<Route> 
    RouteConstructor("Route",true);

  /***********************/
}









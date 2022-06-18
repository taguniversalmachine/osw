
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
/* oswInletOutlet.h
   Inlet and Outlet classes for OSW transforms.
*/

#ifndef _OSW_INLETOUTLET_H
#define _OSW_INLETOUTLET_H


#include "oswState.h"
#include "oswTransform.h"

namespace osw {


  /*********************/
  
  template <class T>
  class _oswexport Inlet : public State<T> {
    
  public:
    
    Inlet (const string &aname = "", Transform *atransform = NULL, 
	   const string &adesc = "", const T &aval = T()) :
      State<T> (aname,atransform,adesc,aval) {
      if (atransform != NULL) {
	atransform->AddInlet(this);
      }
      xcast = NULL;
    }
    
    ~Inlet() {
      Transform *atransform = dynamic_cast<Transform *>(this->GetContainer());
      if (atransform != NULL) {
	atransform->RemoveInlet(this);
      }
    }
    
    // the assignment operator cannot be inherited, so it has
    // to be redefined here
    
    Inlet<T>& operator = (const T& aval) {
      SafeAssign(aval);
      return *this;
    }
    
    Inlet<T>& operator = (const State<T> &toAssign) {
      Assign(toAssign);
      return *this;
    }
    
    bool IsInlet() const {
      return true;
    }
    
    // essentially to support dynamic casting
    virtual void BackDisconnect () {
      if (xcast != NULL) {
	delete xcast;
	delete this;
      }
    }
    
    virtual void Connected(BaseOutlet *aconnection);
    virtual void Disconnected(BaseOutlet *aoldConnection);
    
  protected:
    
    BaseActivation *xcast;
    
    void AddImplicitCast (BaseActivation *acast) {
      xcast = acast;
    }
    
    friend class BaseOutlet;
  };
  
  template<class T>
  void Inlet<T>::Connected(BaseOutlet *aconnection) {
  }
  template<class T>
  void Inlet<T>::Disconnected(BaseOutlet *aoldConnection) {
  }
  
  /*****************/
  
  //! Changeable (dynamicly-typed) Inlets

  class _oswexport ChangeableInlet {
    
    //! Pointer to the actual inlet state variable
    BaseState *xinlet;
    
  public:    

    //! Dummy type for Changeable inlets
    struct Changeable {
     
      //! equality operator, always equal
      bool operator == (const Changeable &toCompare) const {
        return true;
      }
      
      //! unordered, so always false
      bool operator < (const Changeable &toCompare) const {
        return false;
      }
      
      //! stream-out operator (required for all OSW data types)
      friend _oswexport ostream & operator << (ostream &os, const Changeable &toPrint) {
        return os << "<Changeable>";
      }
      
      //! stream-in operator (leaves stream unchanged);
      friend _oswexport istream &operator >> (istream &is, Changeable &toInput) {
        return is;
      }

      //! Interpret as integer zero
      operator int () const {return 0;}

      //! Interpret as float zero
      operator float() const {return 0;}

      //! Interpet as double zero 
      operator double() const {return 0;}

    };

    //! Constructor for ChangeableInlet
    /**
     * @param aname string name of changeable inlet
     * @param atransform pointer to Transform that will contain this changeable inlet
     * @param adesc string description of changeable inlet
     */
    ChangeableInlet (const string &aname = "", Transform *atransform = NULL,
		     const string &adesc = "");
    
    
    //! Destructor for ChangeableInlet
    ~ChangeableInlet () {
      if (xinlet != NULL) {
	delete xinlet;
      }
    }
    
    //! Returns a pointer to the actual state variable used by this changeable inlet
    operator BaseState * () {
      return xinlet;
    }
    
    //! Returns a reference to the actual state variable used by this changeable inlet
    BaseState & operator * () {
      return *xinlet;
    }
    
    //! Access state-variable methods
    BaseState * operator -> () {
      return xinlet;
    }
    
    //! Return the data type of this changeable inlet
    const type_info & TypeOf () const {
      if (xinlet != NULL) {
	return xinlet->TypeOf();
      } else {
	// it really doesn't matter in this case
	return typeid(this);
      }
    }
        
    //! Relative order of this inlet when displaying associated transform
    int Order () const {
      return xinlet->Order();
    }
    
    //! Set the relative order of this inlet (for display purposes)
    int SetOrder (int aorder) {
      return xinlet->SetOrder(aorder);
    }
    
    //! Change the state variable used by this changeable inlet
    BaseState *Change (BaseState *changeTo);
  };
  
  /*****************/
  
  template <class T>
  class _oswexport Outlet : public State<T>, public BaseOutlet {
    
  public:
    
    
    Outlet (const string &aname = "", Transform *atransform = NULL, 
	    const string &adesc = "", const T& aval = T())
      : State<T>(aname, atransform, adesc,aval) {
      xconnect = NULL;
      if (atransform != NULL) {
	atransform->AddOutlet(this);
      }
    }
    
    ~Outlet() {
      Transform *atransform = dynamic_cast<Transform *>(this->GetContainer());
      if (atransform != NULL) {
	atransform->RemoveOutlet(this);
      }
    }
    
    Outlet<T>& operator = (const T& aval) {
      SafeAssign(aval);
      return *this;
    }
    
    
    Outlet<T>& operator = (const State<T> &toAssign) {
      Assign(toAssign);
      return *this;
    }
    
    bool Connect (BaseState *ainlet);
    
    void Disconnect () {
      xconnect = NULL;
    }
    
    BaseState * GetConnection() {
      return const_cast<BaseState *>((const BaseState *)xconnect);
    }
    
  protected:
    
    virtual void SetActivations ();
    
    Inlet<T> *xconnect;
  };
  
  template <class T>
  void Outlet<T>::SetActivations () {
    if (xconnect) {
      //(Inlet<T>&)(*xconnect) = xval;
      //*(dynamic_cast<Inlet<T> *>(xconnect)) = xval;
      *xconnect = this->xval;
    }
    State<T>::SetActivations();
  };
  
  template<class T>
  inline bool Outlet<T>::Connect (BaseState *ainlet) {
    Inlet<T> *p = dynamic_cast<Inlet<T> *>(ainlet);
    bool result = false;
    if (xconnect) {
      const_cast<Inlet<T> *>(xconnect)->BackDisconnect();
    }
    if (p != NULL) {
      xconnect = p;
      result = true;
      if ((dynamic_cast<Transform *>(this->GetContainer()))->NumOutlets() > 1 
          && !this->xnoparallel) {
	ainlet->SetParallel();
      } 
    } else {
      p = dynamic_cast<Inlet<T> *>(CheckCast(ainlet));
      if (p != NULL) {
	xconnect = p;
	result = true;
	if ((dynamic_cast<Transform *>(this->GetContainer()))->NumOutlets() > 1
            && !this->xnoparallel) {
	  ainlet->SetParallel();
	}
      } else {
	xconnect = NULL;
      }
    }
    
    return result;
  }
  
  
  
  // now that we've defined Inlet and Outlet, we can clone them
  
  template<class T>
  BaseState  *(State<T>::CloneInlet) (const string &aname, 
				      Transform *atransform,
				      const string &adesc) {
    return new Inlet<T> (aname,atransform,adesc,xval);
  }
  
  template<class T>
  BaseState  *(State<T>::CloneOutlet) (const string &aname, 
				       Transform *atransform,
				       const string &adesc) {
    return new Outlet<T> (aname,atransform,adesc,xval);
  }
  
  /***********************/


  // now the special Outlet<any> class
  // (the asymmetric equivalent of ChangeableInlet)
  
  _STLP_TEMPLATE_NULL class _oswexport Outlet<Any> : public State<Any>, public BaseOutlet {
    
  public:
     
    Outlet (const string &aname = "", Transform *atransform = NULL, 
	    const string &adesc = "")
      : State<Any>(aname, atransform, adesc) {
      xconnect = NULL;
      if (atransform != NULL) {
	atransform->AddOutlet(this);
      }
    }
    
    ~Outlet() {
      Transform *atransform = dynamic_cast<Transform *>(GetContainer());
      if (atransform != NULL) {
	atransform->RemoveOutlet(this);
      }
    }
    
    Outlet<Any>& operator = (const Any& aval) {
      SafeAssign(aval);
      return *this;
    }
    
    
    Outlet<Any>& operator = (const State<Any> &toAssign) {
      Assign(toAssign);
      return *this;
    }
    
    bool Connect (BaseState *ainlet);
    
    void Disconnect () {
      xconnect = NULL;
    }
    
    BaseState * GetConnection() {
      return const_cast<BaseState *>((const BaseState *)xconnect);
    }
    
  protected:
    
    virtual void SetActivations ();
    
    BaseState *xconnect;
  };
  
  inline bool Outlet<Any>::Connect (BaseState *aconnect) {
    xconnect = aconnect;
    return true;
  }

  _STLP_TEMPLATE_NULL
  inline BaseState  *(State<Any>::CloneOutlet) (const string &aname, 
                                         Transform *atransform,
                                         const string &adesc) {
    return new Outlet<Any> (aname,atransform,adesc);
  }
}

#endif //  _OSW_INLETOUTLET_H

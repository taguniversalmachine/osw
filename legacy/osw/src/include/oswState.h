
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
/* oswState.h
   Base class for OSW state variables, including inlets and outlets
   for transforms.
*/

#ifndef _OSW_STATE_H
#define _OSW_STATE_H

namespace osw {

  class _oswexport BaseActivation;
  class _oswexport Transform;
  
  //! The base class for all OSW state variables (including inlets and outlets)
  
  class _oswexport BaseState : public Nameable {
    
  public:

    //! The constructor for BaseState
    /**
     * @param aname a string containing the name of the state variable
     * @param acontainer a pointer to the Container into which this variable will be placed
     * @param adesc a string containing a brief description of the variable
     * @see Container
     */
    BaseState (const string &aname, Container *acontainer, const string &adesc);

    //! The destructor for BaseState
    virtual ~BaseState ();

    //! Registers an Activation to be triggered when the variable changes
    /**
     * @param toRegister a pointer to the Activation to register
     * @see BaseActivation
     * @see Activation
     */

    void RegisterActivation (BaseActivation *toRegister);    

    //! Removes an Activation from this variable's activation list
    /**
     * @param toUnregister a pointer to the Activation to unregister
     * @see RegisterActivation
     * @see BaseActivation
     * @see Activation
     */

    void UnregisterActivation (BaseActivation *toUnregister) {
      xactivations.erase(find(xactivations.begin(),xactivations.end(),toUnregister));
      if (xactivations.size() < 2) {
	xparallel &= ~1;
      }
    }
    
    //! The stream-out operator for state variables
    /**
     * @param os a valid output stream (ostream)
     * @param toPrint a reference to the variable (BaseState) to print
     * @see Print
     */
    friend _oswexport ostream & operator << (ostream &os, const BaseState &toPrint);

    //! The stream-in operator for references to state variables
    /**
     * @param is a valid is a input stream (istream)
     * @param toInput a reference to the variable receiving stream input
     * @see Input
     */
    friend _oswexport istream & operator >> (istream &is, BaseState &toInput);

    //! The stream-in operator for pointers to state variables
    /**
     * @param is a valid is a input stream (istream)
     * @param toInput a pointer to the variable receiving stream input
     * @see Input
     */
    friend _oswexport istream & operator >> (istream &is, BaseState *toInput);
 
    virtual ostream & Print (ostream &os) const = 0;
    virtual istream & Input (istream &is) = 0;


    //! returns true if this variable is an inlet, false otherwise
    /**
     * @see Inlet
     */
    virtual bool IsInlet () const {return false;}
    
    //! returns the C++ type of the variable
    /**
     * @return type_info (C++ RTTI type)
     */
    virtual const type_info & TypeOf () const = 0;

    // The "Virtual Constructors" for dynamic-type transforms.

    //! Create a new state variable with the same type as this variable
    /**
     * @param aname a string containing the name of the state variable
     * @param acontainer a pointer to the Container into which this variable will be placed
     * @param adesc a string containing a brief description of the variable
     * @see Container
     * @see State
     * @return BaseState *
     */
    virtual BaseState *CloneState (const string &aname, 
				   Container *acontainer,
				   const string &adesc = "" ) = 0;
    //! Create a new inlet variable with the same type as this variable
    /**
     * @param aname a string containing the name of the state variable
     * @param acontainer a pointer to the Container into which this variable will be placed
     * @param adesc a string containing a brief description of the variable
     * @see Container
     * @see Inlet
     * @return BaseState *
     */
    virtual BaseState *CloneInlet (const string &aname, 
				   Transform *atransform,
				   const string &adesc = "" ) = 0;
    //! Create a new outlet with the same type as this variable
    /**
     * @param aname a string containing the name of the state variable
     * @param acontainer a pointer to the Container into which this variable will be placed
     * @param adesc a string containing a brief description of the variable
     * @see Container
     * @see Outlet
     * @return BaseState *
     */
    virtual BaseState *CloneOutlet (const string &aname, 
				    Transform *atransform,
				    const string &adesc = "" ) = 0;

    //! Assign a new value to the state variable from a BaseState reference
    /**
     * @param toAssign a reference to a BaseState
     * the type must be compatible with the type of the state variable being 
     * assigned
     */
    BaseState & operator = (const BaseState &toAssign) {
      return AssignVirtual(toAssign);
    }

    //! Test if two state variables are equal
    /**
     * @param toCompare a reference to a BaseState
     * @return true if the types of the variables are compatible and their values are equal
     */
    bool operator == (const BaseState &toCompare) {
      return EqualVirtual(toCompare);
    }

    //! Get the description of the state variable
    /**
     * @return a string containing the description of the variable
     */
    string & Description () { return xdesc; }
    //! Set the description of the state variable
    /**
     * @param adesc a string containing the new description
     */
    void SetDescription (const string &adesc) {
      xdesc = adesc;
    }
    
    //! Return the documentation associated with a state variable (same as Description)
    /** 
     * @see Description
     * Refer to Description for backwards compatibility
     */
    string & Documentation () { return Description(); }
    //! Set the documentation of a state variable
    /**
     * @see SetDescription
     */
    void SetDocumentation(const string &astring) {
    	SetDescription(astring);
    } 

    //! Test if variable is an activator
    /**
     * @return true if the variable is an activator
     * @sa Activation, BaseActivation
     */
    bool IsActivator () {
      return !(xactivations.empty());
    }

    //! Iterator for Activations of a state variable
    /**
     * @sa Activation, BaseActivation
     */
    typedef stl::list<BaseActivation *>::iterator ActivationIterator;

    ActivationIterator BeginActivations () {
      return xactivations.begin();
    }

    bool EndActivations (const ActivationIterator &toTest) {
      return toTest == xactivations.end();
    }

    int Order () const {
      return xorder;
    }

    int SetOrder (int aorder) {
      return xorder = aorder;
    }

    virtual BaseDatum *MakeDatum () = 0;
    virtual BaseState &SetFromDatum (BaseDatum *toAssign) = 0;
    
    bool IsParallel () {
      //return false;
      return xparallel;
    }

    void SetParallel () {
      if (!xnoparallel) {xparallel |= 2;}
    }

    void SetParallel (bool aparallel) {
      xparallel = aparallel;
    }

    void NoParallel () {
      xnoparallel = 1;
      xparallel = 0;
    }

    // needed for implicit casting
    virtual void BackDisconnect() {
    }

    // needed for "fancy" inlets, like implicit Time sources

    virtual void Connected(BaseOutlet *aconnection);
    virtual void Disconnected(BaseOutlet *aoldConnection);

    // conversions to raw bytes (for sending via network sockets, etc.)

    //! return the data size of the variable
    /**
     * the data size is the size of the data associated with the variable
     * and does not include the state-variable data structures
     */

    virtual int Size () const  = 0;

    //! return a pointer to the value of the variabe
    virtual const char *Bytes () = 0;

    //! set the value of the variable from a block of memory
    virtual void SetFromBytes (const char *ptr, int size) = 0;

  protected:

    //! Assign a new value to the state variable from a BaseState reference
    /**
     * @param toAssign a reference to a BaseState
     * This method is used by the assignment operator 
     * The type must be compatible with the type of the state variable being 
     * assigned
     */
    virtual BaseState &AssignVirtual(const BaseState &toAssign) = 0;

    //! Test if two state variables are equal
    /**
     * @param toCompare a reference to a BaseState
     * @return true if the types of the variables are compatible and their values are equal
     * This method is used by the equality (==) operator
     */
    virtual bool EqualVirtual(const BaseState &toCompare) = 0;
    
    stl::list<BaseActivation * > xactivations;
    int xlock;
    string xdesc;
    int  xorder;
    byte xparallel,xnoparallel;

    friend class _oswexport ChangeableInlet;
    friend class _oswexport BaseOutlet;

    ChangeableInlet *xchangeable;

    bool IsChangeable () {
      return xchangeable != NULL;
    }

    BaseState *Change (BaseState *changeTo);
    ChangeableInlet *SetChangeable (ChangeableInlet *achangeable);

    virtual void AddImplicitCast(BaseActivation *acast) {
    }
   
    friend class _oswexport Transform;
  };

  /************/

  template<class T>
    class _oswexport Inlet;

  template<class T>
    class _oswexport Outlet;

  template<class T>
    class _oswexport State : public BaseState {
      
    public:
      
      State (const string &aname = "", Container *acontainer = NULL, 
	     const string &adesc = "")
	: BaseState(aname, acontainer,adesc),
	  xval(),
	  xdirty(false),
          xdeferred() {
      }

      State (const string &aname, Container *acontainer,
             const string &adesc, const T &aval)
        : BaseState(aname, acontainer, adesc),
          xval(aval),
          xdirty(false),
          xdeferred() {
      }

      virtual ~State() {}
      
      //operator T () const {return xval;}
      operator T& () {return xval;}
      operator const T & () const {return xval;}

      State<T>& operator = (const T& aval) {
	SafeAssign(aval);
	return *this;
      }
      
      State<T>& operator = (const State<T> &toAssign) {
	Assign(toAssign);
	return *this;
      }
     
      T* operator -> () {return &xval;}

      T& operator *  () {return xval;}

      T& operator -- () {return --xval;}
      T& operator ++ () {return ++xval;}
      
      T operator ! () {return !xval;}
      T operator ~ () {return ~xval;}
      
      T operator - () {return -xval;}
      T operator + () {return +xval;}
      
      virtual ostream& Print (ostream &os) const {
	return os << xval;
      }
      
      virtual istream& Input (istream &is) {
	if (xlock) {
	  xsemaphore.Acquire();
	  ((istream &)is) >> ((T &)xdeferred);
	  xdirty = true;
	  xsemaphore.Release();
	} else {
	  ((istream &)is) >> ((T &)xval);
	  SetActivations();
	}
	return is;
      }

      BaseState *CloneState (const string &aname, 
			     Container *acontainer,
			     const string &adesc = "") {
	return new State<T> (aname, acontainer, adesc, xval);
      }

      BaseState *CloneInlet (const string& aname, 
			     Transform *atransform,
			     const string &adesc = "");
      BaseState *CloneOutlet (const string &aname, 
			      Transform *atransform,
			      const string &adesc = "");

      virtual const type_info & TypeOf () const {
	return typeid(T);
      }

      bool Valid () const {return true;}

      State<T> &SetWithoutActivating (const T &toAssign) {
	// no locking on this one
	xval = toAssign;
	return *this;
      }

      int Lock () {      
	xsemaphore.Acquire();
	++xlock;
	xsemaphore.Release();
	return xlock;
      }

      int Unlock () {
	if (xlock) {
	  xsemaphore.Acquire();
	  --xlock;
	  xsemaphore.Release();
	  if (!xlock && xdirty) {
	    xval = xdeferred;
	    xdirty = false;
	    SetActivations();
	  }
	}
	return xlock;
      }

      // friend functions use Externalizer-friendly syntax
      friend int Lock (State<T> *toLock) {
	return toLock->Lock();
      }
      friend int Unlock (State<T> *toUnlock) {
	return toUnlock->Unlock();
      }

      BaseDatum *MakeDatum () {
	BaseDatum *datum = new Datum<T> (xval);
        //datum->AddRef();
        return datum;
      }

      BaseState &SetFromDatum (BaseDatum *toAssign) {
	SafeAssign(T(*(Datum<T> *)toAssign));
	return *this;
      }

      virtual int Size () const {return sizefunc(xval);}
      virtual const char *Bytes() {return ptrfunc(xval);}
      virtual void SetFromBytes(const char *ptr, int size) {
	if (xlock) {
	  setfromptr(ptr,size,xdeferred);
	  xdirty = true;
	} else {
	  setfromptr(ptr,size,xval);
	  SetActivations();
	}
      }

    protected:
      
      virtual void SetActivations();
      T	xval;
      bool xdirty;
      T xdeferred;
      Semaphore  xsemaphore;

      void SafeAssign (const T &aval) {
        try {
          if (xlock) {
            xdeferred = aval;
            xdirty = true;
          } else {
            xval = aval;
            SetActivations();
          }
        } catch (const Exception &e) {
          Error(PathName()+": "+e.what());
        }
      }

      void Assign (const State<T>& toAssign) {
	SafeAssign(toAssign.xval);
      }

      BaseState &AssignVirtual (const BaseState &toAssign) {
	SafeAssign(dynamic_cast<const State<T> &>(toAssign).xval);
	return *this;
      }

      bool EqualVirtual (const BaseState &toCompare) {
	try {
	  return xval == dynamic_cast<const State<T> &>(toCompare).xval;
	} catch (...) {
	  return false;
	}
      }
    };
  

  template<class T>
    inline bool operator == (const State<T> &state, const T &toCompare) {
    return (const T&)(state) == toCompare;
  }

  template<class T>
    inline bool operator == (const T &toCompare, const State<T> &state) {
    return (const T&)(state) == toCompare;
  }

  template<class T>
    inline bool operator < (const State<T> &state, const T &toCompare) {
    return (const T&)(state) < toCompare;
  }

  template<class T>
    inline bool operator < (const T &toCompare, const State<T> &state) {
    return  toCompare < (const T&)(state);
  }

  template<class T>
    inline bool operator > (const State<T> &state, const T &toCompare) {
    return (const T&)(state) > toCompare;
  }

  template<class T>
    inline bool operator > (const T &toCompare, const State<T> &state) {
    return  toCompare > (const T&)(state);
  }

  template<class T>
    inline bool operator != (const State<T> &state, const T &toCompare) {
    return (const T&)(state) != toCompare;
  }

  template<class T>
    inline bool operator != (const T &toCompare, const State<T> &state) {
    return (const T&)(state) != toCompare;
  }

  template<class T>
    inline bool operator <= (const State<T> &state, const T &toCompare) {
    return (const T&)(state) <= toCompare;
  }
  
  template<class T>
    inline bool operator <= (const T &toCompare, const State<T> &state) {
    return  toCompare <= (const T&)(state);
  }

  template<class T>
    inline bool operator >= (const State<T> &state, const T &toCompare) {
    return (const T&)(state) >= toCompare;
  }

  template<class T>
    inline bool operator >= (const T &toCompare, const State<T> &state) {
    return  toCompare >= (const T&)(state);
  }

  template<class S, class T>
    inline T operator + (const State<T> &state1, const S &val2) {
    return (const T&)(state1) + val2;
  }
  template<class S, class T>
    inline T operator + (const S &val1, const State<T> &state2) {
    return val1 + (const T&)(state2);
  }
  template<class S, class T>
    inline T operator - (const State<T> &state1, const S &val2) {
    return (const T&)(state1) - val2;
  }
  template<class S, class T>
    inline T operator - (const S &val1, const State<T> &state2) {
    return val1 - (const T&)(state2);
  }
  template<class S, class T>
    inline T operator * (const State<T> &state1, const S &val2) {
    return (const T&)(state1) * val2;
  }
  template<class S, class T>
    inline T operator * (const S &val1, const State<T> &state2) {

    return val1 * (const T&)(state2);
  }
  template<class S, class T>
    inline T operator / (State<T> &state1, const S &val2) {
    return (const T&)(state1) / val2;
  }
  template<class S, class T>
    inline T operator / (const S &val1, const State<T> &state2) {
    return val1 / (const T&)(state2);
  }
  template<class S, class T>
    inline T operator % (const State<T> &state1, const S &val2) {
    return (const T&)(state1) % val2;
  }
  template<class S, class T>
    inline T operator % (const S &val1, const State<T> &state2) {
    return val1 % (const T&)(state2);
  }
  template<class S, class T>
    inline T operator && (const State<T> &state1, const S &val2) {
    return (const T&)(state1) && val2;
  }
  template<class S, class T>
    inline T operator && (const S &val1, const State<T> &state2) {
    return val1 && (const T&)(state2);
  }
  template<class S, class T>
    inline T operator || (const State<T> &state1, const S &val2) {
    return (const T&)(state1) || val2;
  }
  template<class S, class T>
    inline T operator || (const S &val1, const State<T> &state2) {
    return val1 || (const T&)(state2);
  }
  template<class S, class T>
    inline T operator & (const State<T> &state1, const S &val2) {
    return (const T&)(state1) & val2;
  }  
  template<class S, class T>
    inline T operator & (const S &val1, const State<T> &state2) {
    return val1 & (const T&)(state2);
  }
  template<class S, class T>
    inline T operator | (const State<T> &state1, const S &val2) {
    return (const T&)(state1) | val2;
  }
  template<class S, class T>
    inline T operator | (const S &val1, const State<T> &state2) {
    return val1 | (const T&)(state2);
  }
  template<class S, class T>
    inline T operator ^ (const State<T> &state1, const S &val2) {
    return (const T&)(state1) ^ val2;
  }
  template<class S, class T>
    inline T operator ^ (const S &val1, const State<T> &state2) {
    return val1 ^ (const T&)(state2);
  }


  /*************/
  //  Specialization of constructor for State<Any>
  _STLP_TEMPLATE_NULL
  inline State<Any>::State<Any>(const string &aname, Container *acontainer, 
                                const string &adesc) 
    : BaseState(aname, acontainer,adesc),
      xval((BaseDatum *)NULL),
      xdirty(false),
      xdeferred() {
  }


  //  Specialization of MakeDatum for State<Any>

  _STLP_TEMPLATE_NULL
  inline BaseDatum *(State<Any>::MakeDatum) () {
	return xval;
  }  
  
  /*************/
  // Probably not the best place for this defintition, but it
  // needs to come early

  struct _oswexport BaseOutlet {
    
    virtual bool Connect (BaseState *ainlet) = 0;
    virtual BaseState *GetConnection () = 0;
    
  protected:
    BaseState *CheckCast (BaseState *toCheck);
  };
  

}

#endif // _OSW_STATE_H

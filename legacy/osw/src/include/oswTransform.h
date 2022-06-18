
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
/* oswTransform.h
   OSW Transform class
   and Transform Factory classes
*/

#ifndef _OSW_TRANSFORM_H
#define _OSW_TRANSFORM_H

namespace osw {

  
  class _oswexport Patch;
  
  class _oswexport Transform : public Container {
    
  protected:
    
    friend class BaseState;
    
    
    struct Connection {
      BaseOutlet *source;
      BaseState *sink;
      
      Connection (BaseOutlet *asource, BaseState *asink) {
	source = asource;
	sink = asink;
      }
    };
    
    struct MatchesInlet {
      BaseState *inlet;
      
      MatchesInlet (BaseState *ainlet) {
	inlet = ainlet;
      }
      
      bool operator () (const Connection &toTest) {
	return toTest.sink == inlet;
      }
    };
    
    struct MatchesOutlet {
      BaseOutlet *outlet;
      
      MatchesOutlet (BaseOutlet *aoutlet) {
	outlet = aoutlet;
      }
      
      bool operator () (const Connection &toTest) {
	return toTest.source == outlet;
      }
    };

    struct InletMap : public stl::list<Connection> {
      BaseOutlet *& operator [] (BaseState *asink) {
	iterator found = find_if(begin(),end(),MatchesInlet(asink));
	if (found == end()) {
	  throw (Exception(
		  (string("Can't find inlet ") + asink->PathName()).c_str()));
	}
	return found->source;
      }
    };

    struct OutletMap : public stl::list<Connection> {
      BaseState *& operator [] (BaseOutlet *asource) {
	iterator found = find_if(begin(),end(),MatchesOutlet(asource));
	if (found == end()) {
	  throw (Exception(
		 (string("Can't find outlet ") + 
		  dynamic_cast<Nameable *>(asource)->PathName()).c_str()));
	}
	return found->sink;
      }
    };

    InletMap xinlets;
    OutletMap xoutlets;

  public:

    typedef InletMap::iterator InletIterator;
    typedef OutletMap::iterator OutletIterator;
    
    Transform (const string &aname, Container *acontainer) : 
      Container (aname,acontainer) {
      xargc = 0;
      xargv = NULL;
    }
    
    Transform (const string &aname, Container *acontainer, int argc, char *argv[]) : 
      Container (aname,acontainer) {
      xargc = argc;
      xargv = new char *[argc];
      for (int i = 0; i < argc; ++i) {
        xargv[i] = strdup(argv[i]);
      }
    }

    virtual ~Transform ();

    virtual bool Connect (BaseOutlet *source, BaseState *&sink);
    virtual bool BackConnect (BaseOutlet *source, BaseState *sink);
    
    virtual bool Disconnect(BaseOutlet *source);
    virtual bool BackDisconnect(BaseState *sink);

    int NumInlets () {
      return xinlets.size();
    }
    int NumOutlets () {
      return xoutlets.size();
    }

    InletIterator BeginInlets () {
      return xinlets.begin();
    }

    bool EndInlets (const InletIterator &toTest) {
      return toTest == xinlets.end();
    }

    InletIterator &NextInlet(InletIterator &toIncr) {
      return ++toIncr;
    }

    BaseState *InletSelf (InletIterator &toSelect) {
      return toSelect->sink;
    }

    BaseOutlet *InletConnection (InletIterator &toSelect) {
      return toSelect->source;
    }

    BaseOutlet *InletConnection (BaseState *ainlet) {
      return xinlets[ainlet];
    }

    OutletIterator BeginOutlets () {
      return xoutlets.begin();
    }

    bool EndOutlets (const OutletIterator &toTest) {
      return toTest == xoutlets.end();
    }

    OutletIterator &NextOutlet(OutletIterator &toIncr) {
      return ++toIncr;
    }

    BaseOutlet *OutletSelf (OutletIterator &toSelect) {
      return toSelect->source;
    }

    BaseState *OutletConnection (OutletIterator &toSelect) {
      return toSelect->sink;
    }

    BaseState *OutletConnection (BaseOutlet *aoutlet) {
      return xoutlets[aoutlet];
    }

    virtual void AddInlet (BaseState *toAdd) {
      xinlets.push_back(Connection(NULL,toAdd));
    }
    virtual void AddOutlet (BaseOutlet *toAdd) {
      xoutlets.push_back(Connection(toAdd,NULL));
    }
    virtual void RemoveInlet (BaseState *toRemove);
    virtual void RemoveOutlet (BaseOutlet *toRemove);

    virtual const char *Draw(void *DrawState) {return "";}

    virtual string PersistentState () const {return "";}

    virtual const char *ClassName () const {
      return typeid(*this).name();
    }

    virtual const char *Description () const {return "";}

    /*
     * Refer to Description for backwards compatibility
     * We have to make an explicit copy of the return string to avoid problems
     */
    string & Documentation () { SetDocumentation(Description()); return xdoc; }
    
    virtual const char *Package () const { return "";}

    Patch * GetPatch ();

    int GetArgc() const { return xargc;}
    char **GetArgv() const {return xargv;}


  protected:

    virtual void InletTypeHasChanged (BaseState *changedInlet);

    friend class _oswexport ChangeableInlet;

  private:

    int xargc;
    char **xargv;
  };

  /**********************/
  
  // TransformFactory classes are used to instantiate
  // a type of transform from an interpreter (e.g., Tcl)
  
  struct _oswexport BaseTransformFactory {
    
    BaseTransformFactory (const string &aname, const string &ahello,
			      bool aautoNameable = false);
    
    virtual Transform *Construct (const string &aname, 
				  const string &acontainerName,
				  int argc, char *argv[]) = 0;
    

    friend Transform *MakeTransform (const string &axformType, 
                                     const string &aname, 
				     const string &containerName, 
				     int argc, char *argv[]);
    friend Transform *MakeTransformRTTI (const string &axformType, 
                                         const string &aname, 
                                         const string &containerName, 
                                         int argc, char *argv[]);

    friend bool AutoNameable (const string &axformType);

    typedef stl::map<string,BaseTransformFactory *, stl::less<string> > FactoryMap;

    friend FactoryMap *GetFactories ();


  protected:
    
    static FactoryMap	*xfactories;
    bool xautonameable;
    
    void MapRTTIToOSWName(const string &rttiName,const string &oswName);

  };
  
  template <class T>
    struct _oswexport TransformFactory : public BaseTransformFactory {


      TransformFactory ()
	: BaseTransformFactory(typeid(T).name(),"",false) {
        MapRTTIToOSWName(typeid(T).name(),typeid(T).name());
      }

      TransformFactory (const string &aname,
			    const string &ahello = "")
	: BaseTransformFactory (aname,ahello,false) {
        MapRTTIToOSWName(typeid(T).name(),aname);
      }

      TransformFactory (const string &aname,
			    bool aautonameable,
			    const string &ahello = "")
	: BaseTransformFactory (aname,ahello,aautonameable) {
        MapRTTIToOSWName(typeid(T).name(),aname);
      }
      
      virtual Transform *Construct (const string &aname, const string &acontainerName, 
			    int argc, char *argv[]) {
	return new T (aname,dynamic_cast<Container *>(Nameable::Find(acontainerName)),
		      argc,argv);
      }

      TransformFactory<T> & operator = (const TransformFactory<T> &toAssign) {
        // do nothing
        return *this;
      }
    };

  
  // legacy (pre-1.0) name of transform factory classes
  // should eventually be able to eliminate this
  #define TransformConstructor TransformFactory

}


#endif // _OSW_TRANSFORM_H


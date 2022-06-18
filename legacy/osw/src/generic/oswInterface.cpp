/* 
Copyright (c) 2002 Andrew Schmeder, Amar Chaudhary.  All rights reserved.
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

       Andrew W. Schmeder (andy@e-molecule.com)

  
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
  oswinterface.cpp

  An Interface class is capable of manipulating patches in the OSW namespace.
  It is used to connect OSC to the patch control engine 
*/

#include "osw.h"
#include "oswDelete.h"
#include "oswLoadOSX.h"
#include "oswList.h"
#include "oswInit.h"
#include "oswDataSource.h"
#include "oswSched.h"
#include "oswThread.h"
#include "oswOSCTypes.h"
#include "oswInletOutlet.h"
#include "oswPatternMatch.h"

using namespace std;

namespace osw {

  // Functions to test OSC paths for validity.
  
  bool IsValidNameable(const string &path) {
    return Nameable::Find(path) != NULL 
      || Nameable::Find(String(path.c_str()+1)) != NULL; 
  }
  bool IsValidState(const string &path) {
    return dynamic_cast<BaseState *>(Nameable::Find(path)) != NULL;
  }
  bool IsValidContainer(const string &path) {
    return dynamic_cast<Container *>(Nameable::Find(path)) != NULL;
  }
  bool IsValidPatch(const string &path) {
    return dynamic_cast<Patch *>(Nameable::Find(path)) != NULL;
  }
  bool IsValidTransform(const string &path) {
    return dynamic_cast<Transform *>(Nameable::Find(path)) != NULL;
  }
  bool IsValidInlet(const string &path) {
  //  return dynamic_cast<Inlet *>(Nameable::Find(path)) != NULL;
    return IsValidState(path);
  }
  bool IsValidOutlet(const string &path) {
    return dynamic_cast<BaseOutlet *>(Nameable::Find(path)) != NULL;
  }
  bool IsValidDataSource(const string &path) {
    return dynamic_cast<DataSource *>(Nameable::Find(path)) != NULL;
  }

  bool IsValidExtendedRootQuery(const string &address) {
    return (
     address == "/osw-restart" ||
     address == "/load-osx" ||
     address == "/osw-quit" ||
     address == "/transform-types" ||
     address == "/is-auto-nameable" ||
     address == "/advance-clock"
    );
  }

  bool IsValidExtendedMethodQuery(const string &address) {
    if(MatchesPattern(address, "*/set-name")) {
      return IsValidNameable(OSCStripLastName(address));
    }
    else if(
     MatchesPattern(address, "*/get-coactivators") ||
     MatchesPattern(address, "*/get-effects")
    ) {
      return IsValidState(OSCStripLastName(address));
    } else if(
     MatchesPattern(address, "*/add-transform") ||
     MatchesPattern(address, "*/delete-transform")
    ) {
      return IsValidPatch(OSCStripLastName(address));
    } else if(
     MatchesPattern(address, "*/get-inlets") ||
     MatchesPattern(address, "*/get-outlets") ||
     MatchesPattern(address, "*/persistent-states")
    ) {
      return IsValidTransform(OSCStripLastName(address));
    } else if(
     MatchesPattern(address, "*/get-data") ||
     MatchesPattern(address, "*/set-data")
    ) {
      return IsValidDataSource(OSCStripLastName(address));
    } else if(
     MatchesPattern(address, "*/connect") ||
     MatchesPattern(address, "*/disconnect")
    ) {
      return IsValidOutlet(OSCStripLastName(address));
    } else {
       // Extended method is not recognized.
       return false;
    }
  }

  String RootQueryType(const string &address) {
    if(address == "/load-osx" || address == "/is-auto-nameable") {
      return "s";
    } else {
      return "N";
    }
  }
  
  String ExtendedMethodQueryType(const string &address) {
    if(MatchesPattern(address, "*/set-name") ||
     MatchesPattern(address, "*/delete-transform") ||
     MatchesPattern(address, "*/set-data") ||
     MatchesPattern(address, "*/connect") ||
     MatchesPattern(address, "*/disconnect")
    ) {
      return "s";
    }
    else if(
     MatchesPattern(address, "*/add-transform")
    ) {
      return "sss*";
    }
    else if(
     MatchesPattern(address, "*/get-coactivators") ||
     MatchesPattern(address, "*/get-effects") ||
     MatchesPattern(address, "*/get-inlets") ||
     MatchesPattern(address, "*/get-outlets") ||
     MatchesPattern(address, "*/get-data")
    ) {
      return "N";
    } else {
      return "";
    }
  }
  
  String RootQueryDocumentation(const string &address) {
    if(address == "/transform-types") {
      return "List all available transforms by registered classname";
    }
    if(address == "/osw-quit") {
      return "Force OSW server to exit";
    }
    if(address == "/osw-restart") {
      return "Cleanup and restart threads";
    }
    if(address == "/load-osx") {
      return "Load a dynamic-linkable .osx";
    }
    if(address == "/is-auto-nameable") {
      return "Determine if the transform class is Auto-nameable";
    }
    if(address == "/advance-clock") {
      return "Advance the main clock by one frame (triggers all generative time-domain transforms)";
    }
    return "Unknown root method";
  }
  
  String ExtendedMethodQueryDocumentation(const string &address) {
    if(MatchesPattern(address, "*/set-name")) {
      return "Rename this object";
    }
    if(MatchesPattern(address, "*/add-transform")) {
      return "Add a transform to this patch: transform-type initial-name args...";
    }
    if(MatchesPattern(address, "*/delete-transform")) {
      return "Remove a transform from this patch";
    }
    if(MatchesPattern(address, "*/set-data")) {
      return "Set data for this DataSource";
    }
    if(MatchesPattern(address, "*/get-data")) {
      return "Get data for this DataSource";
    }
    if(MatchesPattern(address, "*/connect")) {
      return "Connect this outlet to an inlet";
    }
    if(MatchesPattern(address, "*/disconnect")) {
      return "Disconnect this outlet from an inlet";
    }
    if(MatchesPattern(address, "*/get-coactivators")) {
      return "Give a list of peer activators associated with this activator state";
    }
    if(MatchesPattern(address, "*/get-effects")) {
      return "Give a list of states which are affected by the action of this activator state";
    }
    if(MatchesPattern(address, "*/get-inlets")) {
      return "List inlets for this transform";
    }
    if(MatchesPattern(address, "*/get-outlets")) {
      return "List outlets for this transform";
    }
    return "";
  }

  // Functions to act on OSC paths.

  namespace StateInterface {

    void Rename(const string &name, const string &new_name) {
      Nameable *t = Nameable::Find(name);
      if(t == NULL) {
        throw Exception("Bad path, Nameable not found.");
      }
      t->Rename(new_name);
    }

    List *Effects(const string &name) {
      BaseState *state = dynamic_cast<BaseState *>(Nameable::Find(name));
      if(state == NULL) {
        throw Exception("Bad path, State not found.");
      }
      List *alist = new List();
      for (BaseState::ActivationIterator a = state->BeginActivations();
       !state->EndActivations(a); ++a)
      {
        for (BaseActivation::EffectsIterator effect = (*a)->BeginEffects();
         !(*a)->EndEffects(effect); ++effect)
        {
          alist->append(new Datum<string>((*effect)->PathName()));
        }
      }
      return alist;
    }

    List *CoActivators(const string &name) {
      BaseState *s = dynamic_cast<BaseState *>(Nameable::Find(name));
      if(s == NULL) {
        throw Exception("Bad path, State not found.");
      }
      List *alist = new List();
      for (BaseState::ActivationIterator a = s->BeginActivations();
       !s->EndActivations(a); ++a) 
      {
        for (BaseActivation::ActivatorIterator r = (*a)->BeginActivators();
         !(*a)->EndActivators(r); ++r) 
        {
       	  alist->append(new Datum<string>((*r)->PathName()));
        }
      }
      return alist;
    }
    
  }

  namespace TransformInterface {

    void Rename(const string &name, const string &new_name) {
      Container *t = dynamic_cast<Container *>(Nameable::Find(name));
      if(t == NULL) {
        throw Exception("Bad path, Container does not exist.");
      }
      t->Rename(new_name);
    }

    // Inlets
    List *Inlets(const string &name) {
      Transform *t = dynamic_cast<Transform *>(Nameable::Find(name));
      if(t == NULL) { throw Exception("Transform not found."); }
      List *alist = new List();
      for (Transform::InletIterator i = t->BeginInlets(); !(t->EndInlets(i)); t->NextInlet(i)) {
        if (t->InletSelf(i) != NULL) {
          alist->append(new Datum<int>(t->InletSelf(i)->Order()));
          alist->append(new Datum<string>(t->InletSelf(i)->PathName()));
        } else {
          // TODO: What is the deal with NULL Inlets?? -aws
          alist->append(new Datum<int>(-1));
          alist->append(new Datum<string>(""));
        }
      }
      return alist;
    }
      
    List *Outlets(const string &name) {
      Transform *t = dynamic_cast<Transform *>(Nameable::Find(name));
      if(t == NULL) {
      	throw Exception("Transform not found.");
      }
      List *alist = new List();
      for (Transform::OutletIterator i = t->BeginOutlets(); !(t->EndOutlets(i)); t->NextOutlet(i)) {
        BaseOutlet *o = t->OutletSelf(i);
        if (dynamic_cast<BaseState *>(o) != NULL) {
          alist->append(new Datum<int>(dynamic_cast<BaseState *>(o)->Order()));
          alist->append(new Datum<string>(dynamic_cast<BaseState *>(o)->PathName()));
        } else {
          // TODO: What is the deal with NULL Outlets?? -aws
          alist->append(new Datum<int>(-1));
          alist->append(new Datum<string>(""));
        }
      }
      return alist;
    }

    // Get data as a string
    String GetData(const string &name) {
      Transform *t = dynamic_cast<Transform *>(Nameable::Find(name));
      if (t == NULL) {
      	throw Exception("Bad path, could not find transform.");
      }
      DataSource *d = dynamic_cast<DataSource *>(t);
      if (d == NULL) {
      	throw Exception("Transform is not a data source.");
      }
      return d->GetData();
    }

    // Set data from a string
    void SetData(const string &name, const string &data) {
      Transform *t = dynamic_cast<Transform *>(Nameable::Find(name));
      if (t == NULL) {
      	throw Exception("Bad path, could not find transform.");
      }
      DataSource *d = dynamic_cast<DataSource *>(t);
      if (d == NULL) {
      	throw Exception("Transform is not a data source.");
      }
      d->SetData(data);
    }

    // PersistentState (returns a space-seperated list of states with persistent attribute?)
    String PersistentState(const string &name) {
      Transform *t = dynamic_cast<Transform *>(Nameable::Find(name));
      if(t == NULL) {
      	throw Exception("Transform not found.");
      }
      return t->PersistentState();
    }

  }

  namespace OutletInterface {

    // Connect
    void Connect(const string &from, const string &to) {
      BaseOutlet *source = dynamic_cast<osw::BaseOutlet *>(osw::Nameable::Find(from));
      BaseState *sink   = dynamic_cast<osw::BaseState *>(osw::Nameable::Find(to));
      if(sink == NULL) {
      	throw Exception("Could not find sink state.");
      }
      if(source == NULL) {
      	throw Exception("Could not find source outlet.");
      }
      if(sink->GetContainer() == NULL) {
      	throw Exception("Bad sink, does not have a container.");
      }
      if(dynamic_cast<osw::Transform *>(sink->GetContainer()) == NULL) {
      	throw Exception("Bad sink, container is not a transform.");
      }
      if(! dynamic_cast<osw::Transform *>(sink->GetContainer())->Connect(source,sink)) {
      	throw Exception("Connect failed.  Perhaps the types are incompatible?");
      }
    }
 
    // Disconnect
    void Disconnect(const string &from, const string &to) {
      BaseOutlet *source = dynamic_cast<osw::BaseOutlet *>(osw::Nameable::Find(from));
      if(source == NULL) {
      	throw Exception("Disconnect failed, source not found.");
      }
      dynamic_cast<osw::Transform *>(dynamic_cast<osw::BaseState *>(source)->GetContainer())->Disconnect(source);
    }

  }
 
  namespace PatchInterface {

    // Create (TODO: Support variable args)
    String Create(const string &parent, const string &xtype, const string &name, int argc, char *argv[]) {
      Container *p = dynamic_cast<Container *>(Nameable::Find(parent));
      Transform *x = MakeTransform(xtype,name,p->PathName(),argc,argv);
      if(x == NULL) {
        throw Exception("Could not create Transform.");
      }
      return x->PathName();
    }

    // Destroy
    void Destroy(const string &name) {
      Nameable *thing = Nameable::Find(name);
      if(thing == NULL) {
        throw Exception("Bad path, Nameable not found");
      }
      // TODO: Make sure this is working as we expect
      DeferredDelete(thing);
    }

    // Append args (TODO: Support non-integer arg types.  Support arg descriptions.)
    void AppendArg(const string &name, const string &argname, int value) {
      Patch *p = dynamic_cast<Patch *>(Nameable::Find(name));
      if(p == NULL) {
      	throw Exception("Patch not found.");
      } 
      p->AddArgument(argname, "int", "");
    }

    void CreatePatch(const string &name, const string &parent) {
      Container *c = dynamic_cast<Container *>(Nameable::Find(parent));
      Patch *p     = new Patch(name,c,0,NULL);
      if(p == NULL) {
      	throw Exception("Could not create patch.");
      }
    }

  }

  namespace RootInterface {
    
    // IsAutoNameable  (name is a transform class)
    bool IsAutoNameable(const string &name) {
      return AutoNameable(name);
    }

    // Return a list of all available transform types (from the collection of transform factories)
    List *TransformTypes() {
      BaseTransformFactory::FactoryMap *factories = osw::GetFactories();
      List *alist = new List();
      for (BaseTransformFactory::FactoryMap::iterator index = factories->begin();
       index != factories->end(); ++index) 
      {
      	alist->append(new Datum<string>((*index).first));
      }
      return alist;
    }

    void Restart() {
      Alert("OSW reset.");
      StopThreads();
      MainLoop();
    }

    void Quit() {
      Alert("OSW recieved quit command.");
      osw::Quit();
#ifdef OSW_NEEDS_EXPLICIT_CLEANUP
      osw::Cleanup();
#endif      
    }

    void LoadOSX(const string &name) {
      // TODO: Error handling during LoadOSX?? -aws
      ::LoadOSX(name.c_str());
    }

  }


}


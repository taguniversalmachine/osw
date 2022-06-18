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
  oswtcl.cpp
  Interface between OSW and TCL
  
*/

#include "oswTcl.h"
#include "oswTkDraw.h"
#include "oswDelete.h"
#include "oswLoadOSX.h"
#include "oswList.h"
#include "oswThread.h"
#include "oswSock.h"
#include "oswInit.h"
#include "oswDataSource.h"
#include "oswArgs.h"
#include "oswPrefs.h"

#define TCL_PORTNUM  7776

#ifndef _WINDOWS
#include "X11/Xlib.h"
#endif

using namespace std;

/*********************/

static int OSWTcl_Patch (ClientData cd, Tcl_Interp *interp, 
                         int objc, Tcl_Obj * const objv[]);
static int OSWTcl_PatchCmd (ClientData cd, Tcl_Interp *interp, 
                            int objc, Tcl_Obj * const objv[]);
static int OSWTcl_Set (ClientData cd, Tcl_Interp *interp, 
                       int objc, Tcl_Obj * const objv[]);
static int OSWTcl_Connect (ClientData cd, Tcl_Interp *interp, 
                           int objc, Tcl_Obj * const objv[]);
static int OSWTcl_Get (ClientData cd, Tcl_Interp *interp, 
                       int objc, Tcl_Obj * const objv[]);
static int OSWTcl_Type (ClientData cd, Tcl_Interp *interp, 
                        int objc, Tcl_Obj * const objv[]);
static int OSWTcl_Description (ClientData cd, Tcl_Interp *interp, 
                               int objc, Tcl_Obj * const objv[]);
static int OSWTcl_Inlets (ClientData cd, Tcl_Interp *interp, 
                          int objc, Tcl_Obj * const objv[]);
static int OSWTcl_Outlets (ClientData cd, Tcl_Interp *interp, 
                           int objc, Tcl_Obj * const objv[]);
static int OSWTcl_IsDataSource (ClientData cd, Tcl_Interp *interp, 
                                int objc, Tcl_Obj * const objv[]);
static int OSWTcl_GetData (ClientData cd, Tcl_Interp *interp, 
                           int objc, Tcl_Obj * const objv[]);
static int OSWTcl_SetData (ClientData cd, Tcl_Interp *interp, 
                           int objc, Tcl_Obj * const objv[]);
static int OSWTcl_DrawTransform (ClientData cd, Tcl_Interp *interp, 
                                 int objc, Tcl_Obj * const objv[]);
static int OSWTcl_PersistentState (ClientData cd, Tcl_Interp *interp, 
                                   int objc, Tcl_Obj * const objv[]);
static int OSWTcl_Delete (ClientData cd, Tcl_Interp *interp, 
                          int objc, Tcl_Obj * const objv[]);
static int OSWTcl_IsActivator (ClientData cd, Tcl_Interp *interp, 
                               int objc, Tcl_Obj * const objv[]);
static int OSWTcl_Effects (ClientData cd, Tcl_Interp *interp, 
                           int objc, Tcl_Obj * const objv[]);
static int OSWTcl_Coactivators (ClientData cd, Tcl_Interp *interp, 
                                int objc, Tcl_Obj * const objv[]);
static int OSWTcl_Dialog (ClientData cd, Tcl_Interp *interp, 
                          int objc, Tcl_Obj * const objv[]);
static int OSWTcl_TransformMenu (ClientData cd, Tcl_Interp *interp, 
                                 int objc, Tcl_Obj * const objv[]);
static int OSWTcl_AutoNameable (ClientData cd, Tcl_Interp *interp, 
                                int objc, Tcl_Obj * const objv[]);
static int OSWTcl_TransformTypes (ClientData cd, Tcl_Interp *interp, 
                                  int objc, Tcl_Obj * const objv[]);
static int OSWTcl_LoadOSX (ClientData cd, Tcl_Interp *interp, 
                           int objc, Tcl_Obj * const objv[]);
static int OSWTcl_Profile (ClientData cd, Tcl_Interp *interp, 
                           int objc, Tcl_Obj * const objv[]);
static int OSWTcl_DumpProfile (ClientData cd, Tcl_Interp *interp, 
                               int objc, Tcl_Obj * const objv[]);
static int OSWTcl_Version (ClientData cd, Tcl_Interp *interp, 
                           int objc, Tcl_Obj * const objv[]);
static int OSWTcl_Quit (ClientData cd, Tcl_Interp *interp, 
                        int objc, Tcl_Obj * const objv[]);
static int OSWTcl_MainLoop (ClientData cd, Tcl_Interp *interp, 
                            int objc, Tcl_Obj * const objv[]);
static int OSWTcl_Cleanup (ClientData cd, Tcl_Interp *interp, 
                           int objc, Tcl_Obj * const objv[]);
static int OSWTcl_GetCommandLineFiles (ClientData cd, Tcl_Interp *interp, 
                                       int objc, Tcl_Obj * const objv[]);
static int OSWTcl_GetPreference (ClientData cd, Tcl_Interp *interp, 
                                 int objc, Tcl_Obj * const objv[]);
static int OSWTcl_SetPreference (ClientData cd, Tcl_Interp *interp, 
                                 int objc, Tcl_Obj * const objv[]);

/***********/

static void SetupEvalServer ();
static void CleanupEvalServer ();

/***********/

static Tcl_Interp *lastInterp = NULL;

static osw::ThreadID s_threadid = NULL;

_oswexport int Osw_Init (Tcl_Interp *interp) {

  osw::PreInit();

#if !defined(_WINDOWS) && !defined(__APPLE__) && !defined(__sgi)
  if(XInitThreads() != 0) {
    osw::Alert("Xlib threading initialized.");
  }
  else {
    osw::Alert("Could not initialize Xlib threading.");
  }
#endif

  Tcl_Obj *argvObj = Tcl_ObjGetVar2(interp,Tcl_NewStringObj("argv",-1),NULL,TCL_GLOBAL_ONLY);
  int objc = 0;
  Tcl_Obj **objv = NULL;
  if (Tcl_ListObjGetElements(interp,argvObj,&objc,&objv) == TCL_OK) {
    char **argv = new char * [objc];
    for (int i = 0; i < objc; ++i) {
      argv[i] = Tcl_GetString(objv[i]);
    }
    osw::ProcessArgs(objc,argv);
    delete [] argv;
  }


  Tcl_CreateObjCommand (interp,"oswPatch",OSWTcl_Patch,NULL,NULL);
  Tcl_CreateObjCommand (interp,"oswSet",OSWTcl_Set,NULL,NULL);
  Tcl_CreateObjCommand (interp,"oswConnect",OSWTcl_Connect,NULL,NULL);
  Tcl_CreateObjCommand (interp,"oswDrawTransform",OSWTcl_DrawTransform,NULL,NULL);
  Tcl_CreateObjCommand (interp,"oswGet",OSWTcl_Get,NULL,NULL);
  Tcl_CreateObjCommand (interp,"oswType",OSWTcl_Type,NULL,NULL);
  Tcl_CreateObjCommand (interp,"oswDescription",OSWTcl_Description,NULL,NULL);
  Tcl_CreateObjCommand (interp,"oswInlets",OSWTcl_Inlets,NULL,NULL);
  Tcl_CreateObjCommand (interp,"oswOutlets",OSWTcl_Outlets,NULL,NULL);
  Tcl_CreateObjCommand (interp,"oswIsDataSource",OSWTcl_IsDataSource,NULL,NULL);
  Tcl_CreateObjCommand (interp,"oswGetData",OSWTcl_GetData,NULL,NULL);
  Tcl_CreateObjCommand (interp,"oswSetData",OSWTcl_SetData,NULL,NULL);
  Tcl_CreateObjCommand (interp,"oswPersistentState",OSWTcl_PersistentState,NULL,NULL);
  Tcl_CreateObjCommand (interp,"oswDelete",OSWTcl_Delete,NULL,NULL);
  Tcl_CreateObjCommand (interp,"oswIsActivator",OSWTcl_IsActivator,NULL,NULL);
  Tcl_CreateObjCommand (interp,"oswEffects",OSWTcl_Effects,NULL,NULL);
  Tcl_CreateObjCommand (interp,"oswCoactivators",OSWTcl_Coactivators,NULL,NULL);
  Tcl_CreateObjCommand (interp,"oswDialog",OSWTcl_Dialog,NULL,NULL);
  Tcl_CreateObjCommand (interp,"oswTransformMenu",OSWTcl_TransformMenu,NULL,NULL);
  Tcl_CreateObjCommand (interp,"oswAutoNameable",OSWTcl_AutoNameable,NULL,NULL);
  Tcl_CreateObjCommand (interp,"oswTransformTypes",OSWTcl_TransformTypes,NULL,NULL);
  Tcl_CreateObjCommand (interp,"oswLoadOSX",OSWTcl_LoadOSX,NULL,NULL);
  Tcl_CreateObjCommand (interp,"oswProfile",OSWTcl_Profile,NULL,NULL);
  Tcl_CreateObjCommand (interp,"oswDumpProfile",OSWTcl_DumpProfile,NULL,NULL);
  Tcl_CreateObjCommand (interp,"oswVersion",OSWTcl_Version,NULL,NULL);
  Tcl_CreateObjCommand (interp,"oswQuit",OSWTcl_Quit,NULL,NULL);
  Tcl_CreateObjCommand (interp,"oswMainLoop",OSWTcl_MainLoop,NULL,NULL);
  Tcl_CreateObjCommand (interp,"oswGetCommandLineFiles",OSWTcl_GetCommandLineFiles,NULL,NULL);
  Tcl_CreateObjCommand (interp,"oswGetPreference",OSWTcl_GetPreference,NULL,NULL);
  Tcl_CreateObjCommand (interp,"oswSetPreference",OSWTcl_SetPreference,NULL,NULL);

  SetupEvalServer();
  osw::Init();

  s_threadid = osw::GetThreadID();
  return TCL_OK;
}

#ifdef __APPLE__
_oswexport Osw_SafeInit (Tcl_Interp *interp) {
  return Osw_Init(interp);
}
#endif

/*****************************/

static int OSWTcl_Patch (ClientData cd, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[]) {

  if (objc < 3) {
    Tcl_AppendResult (interp,"usage: oswPatch parent name ",NULL);
    return TCL_ERROR;
  }
  
  char *parentName = Tcl_GetStringFromObj (objv[1],NULL);

  osw::Container *parent = dynamic_cast<osw::Container *>(osw::Nameable::Find(parentName));
  
  char *name = Tcl_GetStringFromObj (objv[2],NULL);
  
  char **argv = new char * [objc - 3];
  
  for (int i = 3; i < objc; ++i) {
    argv[i - 3] = Tcl_GetStringFromObj(objv[i],NULL);
  }

  osw::Patch *patch = new osw::Patch (name,parent,objc-3,argv);
  
  Tcl_CreateObjCommand (interp,const_cast<char *>(patch->PathName().c_str()),OSWTcl_PatchCmd,ClientData(patch),NULL);
  delete [] argv;
  return TCL_OK;
};


static int OSWTcl_PatchCmd (ClientData cd, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[]) {
  
  if (objc < 2) {
    Tcl_AppendResult (interp,"usage: ",Tcl_GetStringFromObj(objv[0],NULL)," options... ",NULL);
    return TCL_ERROR;
  }
  
  char *cmd = Tcl_GetStringFromObj (objv[1],NULL);
  
  osw::Patch *patch = reinterpret_cast<osw::Patch *>(cd);
  
  if (!strcmp(cmd,"add")) {
    
    if (objc < 4) {
      Tcl_AppendResult (interp,"usage: add transformType transformName ?options?",NULL);
      return TCL_ERROR;
    }
    
    char *xformType = Tcl_GetStringFromObj(objv[2],NULL);
    char *xformName = Tcl_GetStringFromObj(objv[3],NULL);
    
    char **argv = new char * [objc - 4];
    
    for (int i = 4; i < objc; ++i) {
      argv[i - 4] = Tcl_GetStringFromObj(objv[i],NULL);
    }
    
    
    osw::Transform *xform = osw::MakeTransform(xformType,xformName,patch->PathName(),objc-4,argv);
    delete [] argv;
    if (xform == NULL) {
      Tcl_AppendResult (interp," could not make transform of type ",xformType,NULL);
      return TCL_ERROR;
    }
    Tcl_SetObjResult(interp,Tcl_NewStringObj(const_cast<char *>
					     (xform->Name().c_str()),-1));
    return TCL_OK;
  } else if (!strcmp(cmd,"remove")) {
    char *nameToRemove =  Tcl_GetStringFromObj(objv[2],NULL);
    osw::Nameable *toRemove = osw::Nameable::Find(nameToRemove);
    if (toRemove == NULL) {
      Tcl_AppendResult(interp,"could not find object ",nameToRemove,NULL);
      return TCL_ERROR;
    } else {
      if (dynamic_cast<osw::Patch *>(toRemove) != NULL) {
	osw::Patch *patchToRemove = dynamic_cast<osw::Patch *>(toRemove);
	return Tcl_VarEval(interp,"delete object ",patchToRemove->window->c_str(),NULL);
      }
      
      osw::DeferredDelete(toRemove);
      return TCL_OK;
    }
  } else if (!strcmp(cmd,"addargument")) {
    char *argname = Tcl_GetStringFromObj(objv[2],NULL);
    char *argtype = "int";
    char *argdesc = "";
    if (objc > 3) {
      argtype = Tcl_GetStringFromObj(objv[3],NULL);
    }      
    if (objc > 4) {
      argdesc = Tcl_GetStringFromObj(objv[4],NULL);
    }     
    
    patch->AddArgument(argname,argtype,argdesc);
    Tcl_SetObjResult(interp,objv[2]);
    return TCL_OK;
  } else {
    Tcl_AppendResult (interp," bad option: ",cmd,NULL);
    return TCL_ERROR;
  }
}


/********************/

static int OSWTcl_Set (ClientData cd, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[]) {

   if (objc < 3) {
   	Tcl_AppendResult (interp,"usage: oswSet object value ",NULL);
      return TCL_ERROR;
   }

   lastInterp = interp;
   char *name = Tcl_GetStringFromObj (objv[1],NULL);
   char *value = Tcl_GetStringFromObj (objv[2],NULL);

   osw::BaseState *state = 
     dynamic_cast<osw::BaseState *>(osw::Nameable::Find(name));
   
   if (state != NULL) {
     if (state->TypeOf() == typeid(string)) {
       *(dynamic_cast<osw::State<string> *>(state)) = value;
     } else {
       istringstream  is(value);
       if (state->TypeOf() == typeid(osw::Any)) {
	 osw::List alist;
	 is >> alist;
	 if (alist.size() == 1) {
	   *(dynamic_cast<osw::State<osw::Any> *>(state)) = alist.nth(0);
	 } else {
	   *(dynamic_cast<osw::State<osw::Any> *>(state)) =
	     new osw::Datum<osw::List>(alist);
	 }
       } else {
	 is >> (*state);
       }
     }
     // don't waste time returning a string here
     return TCL_OK;

   }

   // but do if there is a problem
   Tcl_AppendResult (interp," could not set object ",name,NULL);
   return TCL_ERROR;
}

static int OSWTcl_Get (ClientData cd, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[]) {


   if (objc < 2) {
   	Tcl_AppendResult (interp,"usage: oswGet object ",NULL);
      return TCL_ERROR;
   }

   char *name = Tcl_GetStringFromObj (objv[1],NULL);
   osw::BaseState *state = dynamic_cast<osw::BaseState *>(osw::Nameable::Find(name));
   
   if (state != NULL) {

     ostringstream  os;
     os << (*state);
     Tcl_SetResult(interp,
                   const_cast<char *>(os.str().c_str()),
                   TCL_VOLATILE);
     
     return TCL_OK;

   }

   Tcl_AppendResult (interp," could not find object ",name,NULL);
   return TCL_ERROR;
}

/****************/

static int OSWTcl_Connect (ClientData cd, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[]) {
  
  if (objc < 3) {
    Tcl_AppendResult (interp,"usage: ossConnect source sink ",NULL);
    return TCL_ERROR;
  }
  
  lastInterp = interp;
  
  char *asource = Tcl_GetStringFromObj (objv[1],NULL);
  char *asink = Tcl_GetStringFromObj (objv[2],NULL);
  
  
  osw::BaseOutlet *source = dynamic_cast<osw::BaseOutlet *>(osw::Nameable::Find(asource));
  
  if (source != NULL) {
    
    if (!strcmp(asink,"none")) {
      dynamic_cast<osw::Transform *>
        (dynamic_cast<osw::BaseState *>(source)->GetContainer())
        ->Disconnect(source);
      return TCL_OK;
    }
    try {
      osw::BaseState * sink = dynamic_cast<osw::BaseState *>(osw::Nameable::Find(asink));
      if (sink != NULL) {
        if (sink->GetContainer() != NULL &&
            dynamic_cast<osw::Transform *>(sink->GetContainer()) != NULL &&
            dynamic_cast<osw::Transform *>(sink->GetContainer()) 
            -> Connect(source,sink)) {
          return TCL_OK;
        } else {
          Tcl_AppendResult (interp,"could not connect ",asource," to ",asink,
                            ".  Perhaps the types are not compatible?",NULL);
          return TCL_ERROR;
        }
      } else {
        Tcl_AppendResult (interp,"could not find ",asink,NULL);
        return TCL_ERROR;
      }
    } catch (const exception &x) {
      cerr << x.what() << endl;
      return TCL_ERROR;
    }
  }
  if (!strcmp(asink,"none")) {
    return TCL_OK;
  }
  Tcl_AppendResult (interp,"could not find ",asource,NULL);
  return TCL_ERROR;
}

/*************/

static int OSWTcl_Type (ClientData cd, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[]) {

   if (objc < 2) {
   	Tcl_AppendResult (interp,"usage: oswType object ",NULL);
      return TCL_ERROR;
   }

   char *name = Tcl_GetStringFromObj (objv[1],NULL);

   osw::BaseState *state = dynamic_cast<osw::BaseState *>(osw::Nameable::Find(name));
   
   if (state != NULL) {
     
     Tcl_AppendResult(interp,osw::RegisteredTypeName(state->TypeOf()).c_str(),NULL);
     return TCL_OK;

   }

   // but do if there is a problem
   Tcl_AppendResult (interp," could not find object ",name,NULL);
   return TCL_ERROR;
}


/*************/

static int OSWTcl_Description (ClientData cd, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[]) {

   if (objc < 2) {
   	Tcl_AppendResult (interp,"usage: oswDescription object ",NULL);
      return TCL_ERROR;
   }

   char *name = Tcl_GetStringFromObj (objv[1],NULL);

   osw::BaseState *state = dynamic_cast<osw::BaseState *>(osw::Nameable::Find(name));
   
   if (state != NULL) {
     
     Tcl_AppendResult(interp,state->Description().c_str(),NULL);
     return TCL_OK;

   } else {
     osw::Transform *transform = dynamic_cast<osw::Transform *>
       (osw::Nameable::Find(name));
     if (transform != NULL) {
       Tcl_AppendResult(interp,transform->Description(),NULL);
       return TCL_OK;
     }
   }

   Tcl_AppendResult (interp," could not find object ",name,NULL);
   return TCL_ERROR;
}

/*************/

static int OSWTcl_IsActivator (ClientData cd, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[]) {

   if (objc < 2) {
   	Tcl_AppendResult (interp,"usage: oswIsActivator object ",NULL);
      return TCL_ERROR;
   }

   char *name = Tcl_GetStringFromObj (objv[1],NULL);

   osw::BaseState *state = dynamic_cast<osw::BaseState *>(osw::Nameable::Find(name));
   
   if (state != NULL) {
     
     Tcl_SetObjResult(interp,Tcl_NewBooleanObj(state->IsActivator()));
     return TCL_OK;

   }

   Tcl_AppendResult (interp," could not find object ",name,NULL);
   return TCL_ERROR;
}

static int OSWTcl_Effects (ClientData cd, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[]) {

   if (objc < 2) {
   	Tcl_AppendResult (interp,"usage: oswEffects object ",NULL);
      return TCL_ERROR;
   }

   char *name = Tcl_GetStringFromObj (objv[1],NULL);

   osw::BaseState *state = dynamic_cast<osw::BaseState *>(osw::Nameable::Find(name));
   
   if (state != NULL) {
     
     Tcl_Obj *result = Tcl_NewListObj(0,NULL);
     
     for (osw::BaseState::ActivationIterator activation = state->BeginActivations();
	  !state->EndActivations(activation); ++activation) {
       for (osw::BaseActivation::EffectsIterator effect = (*activation)->BeginEffects();
	    !(*activation)->EndEffects(effect); ++effect) {
	 Tcl_ListObjAppendElement(interp,result,
				  Tcl_NewStringObj(const_cast<char *>
						   ((*effect)->PathName().c_str()),
						   -1)
				  );
       }
     }

     Tcl_SetObjResult(interp,result);
     return TCL_OK;
   }

   Tcl_AppendResult (interp," could not find object ",name,NULL);
   return TCL_ERROR;
}

static int OSWTcl_Coactivators (ClientData cd, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[]) {

   if (objc < 2) {
   	Tcl_AppendResult (interp,"usage: oswCoactivators object ",NULL);
      return TCL_ERROR;
   }

   char *name = Tcl_GetStringFromObj (objv[1],NULL);

   osw::BaseState *state = dynamic_cast<osw::BaseState *>(osw::Nameable::Find(name));
   
   if (state != NULL) {
     
     Tcl_Obj *result = Tcl_NewListObj(0,NULL);
     
     for (osw::BaseState::ActivationIterator activation = state->BeginActivations();
	  !state->EndActivations(activation); ++activation) {
       for (osw::BaseActivation::ActivatorIterator activator = (*activation)->BeginActivators();
	    !(*activation)->EndActivators(activator); ++activator) {
	 Tcl_ListObjAppendElement(interp,result,
				  Tcl_NewStringObj(const_cast<char *>
						   ((*activator)->PathName().c_str()),
						   -1)
				  );
       }
     }

     Tcl_SetObjResult(interp,result);
     return TCL_OK;
   }

   Tcl_AppendResult (interp," could not find object ",name,NULL);
   return TCL_ERROR;
}



/*************/

static int OSWTcl_Inlets (ClientData cd, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[]) {

   if (objc < 2) {
   	Tcl_AppendResult (interp,"usage: oswInlets transform ",NULL);
      return TCL_ERROR;
   }

   char *name = Tcl_GetStringFromObj (objv[1],NULL);
   
   osw::Transform *transform = dynamic_cast<osw::Transform *>(osw::Nameable::Find(name));
   
   if (transform == NULL) {
     Tcl_AppendResult (interp,"couldn't find transform ",name,NULL);
     return TCL_ERROR;
   }

   Tcl_Obj *inletList = Tcl_NewListObj(0,NULL);

   for (osw::Transform::InletIterator inletIndex = transform->BeginInlets(); 
	!(transform->EndInlets(inletIndex)); 
	transform->NextInlet(inletIndex)) {
     Tcl_Obj *inletObj,*orderObj;

     if (transform->InletSelf(inletIndex) != NULL) {
       inletObj = Tcl_NewStringObj
	 (const_cast<char *>(transform->InletSelf(inletIndex)->PathName().c_str()),-1);
       orderObj = Tcl_NewIntObj(transform->InletSelf(inletIndex)->Order());
     } else {
       inletObj = Tcl_NewStringObj("",0);
       orderObj = Tcl_NewIntObj(-1);
     }
     
     Tcl_Obj *pair = Tcl_NewListObj(1,&orderObj);
     Tcl_ListObjAppendElement(interp,pair,inletObj);
     Tcl_ListObjAppendElement(interp,inletList,pair);
   }

   Tcl_SetObjResult(interp,inletList);
   return TCL_OK;
}

/*************/

static int OSWTcl_Outlets (ClientData cd, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[]) {

   if (objc < 2) {
   	Tcl_AppendResult (interp,"usage: oswOutlets transform ",NULL);
      return TCL_ERROR;
   }

   char *name = Tcl_GetStringFromObj (objv[1],NULL);
   
   osw::Transform *transform = dynamic_cast<osw::Transform *>(osw::Nameable::Find(name));
   
   if (transform == NULL) {
     Tcl_AppendResult (interp,"couldn't find transform ",name,NULL);
     return TCL_ERROR;
   }

   Tcl_Obj *outletList = Tcl_NewListObj(0,NULL);

   for (osw::Transform::OutletIterator outletIndex = transform->BeginOutlets(); 
	!(transform->EndOutlets(outletIndex)); ++outletIndex) {
     Tcl_Obj *outletObj,*orderObj;
     
     osw::BaseOutlet *outlet = transform->OutletSelf(outletIndex);
     if (dynamic_cast<osw::BaseState *>(outlet) != NULL) {
       outletObj = Tcl_NewStringObj
	 (const_cast<char *>(dynamic_cast<osw::BaseState *>
			     (outlet)->PathName().c_str()),-1);
       orderObj = Tcl_NewIntObj((dynamic_cast<osw::BaseState *>(outlet))->Order());
     } else {
       outletObj = Tcl_NewStringObj("",0);
       orderObj = Tcl_NewIntObj(-1);
     }

     Tcl_Obj *pair = Tcl_NewListObj(1,&orderObj);
     Tcl_ListObjAppendElement(interp,pair,outletObj);     
     Tcl_ListObjAppendElement(interp,outletList,pair);
   }

   Tcl_SetObjResult(interp,outletList);
   return TCL_OK;
}

/*************/

static int OSWTcl_IsDataSource (ClientData cd, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[]) {
  if (objc < 2) {
   	Tcl_AppendResult (interp,"usage: oswIsDataSource transform ",NULL);
      return TCL_ERROR;
   }

   char *name = Tcl_GetStringFromObj (objv[1],NULL);
   
   osw::Transform *transform = dynamic_cast<osw::Transform *>(osw::Nameable::Find(name));
   
   if (transform == NULL) {
     Tcl_AppendResult (interp,"couldn't find transform ",name,NULL);
     return TCL_ERROR;
   }

   bool result = dynamic_cast<osw::DataSource *>(transform) != NULL;

   Tcl_SetObjResult(interp,Tcl_NewBooleanObj(result));
   return TCL_OK;
}

static int OSWTcl_GetData (ClientData cd, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[]) {

   if (objc < 2) {
   	Tcl_AppendResult (interp,"usage: oswGetData transform ",NULL);
      return TCL_ERROR;
   }

   char *name = Tcl_GetStringFromObj (objv[1],NULL);
   
   osw::Transform *transform = dynamic_cast<osw::Transform *>(osw::Nameable::Find(name));
   
   if (transform == NULL) {
     Tcl_AppendResult (interp,"couldn't find transform ",name,NULL);
     return TCL_ERROR;
   }
   
   osw::DataSource *dataSource = dynamic_cast<osw::DataSource *>(transform);
   if (dataSource == NULL) {
     Tcl_AppendResult (interp,"transform ",name," is not a data source",NULL);
     return TCL_ERROR;
   }

   Tcl_SetResult(interp,const_cast<char *>(dataSource->GetData().c_str()),
		 TCL_VOLATILE);
   return TCL_OK;
}

/*************/

static int OSWTcl_SetData (ClientData cd, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[]) {

   if (objc < 3) {
   	Tcl_AppendResult (interp,"usage: oswSetData transform data_string",NULL);
      return TCL_ERROR;
   }
   lastInterp = interp;
   char *name = Tcl_GetStringFromObj (objv[1],NULL);
   
   osw::Transform *transform = dynamic_cast<osw::Transform *>(osw::Nameable::Find(name));

   if (transform == NULL) {
     Tcl_AppendResult (interp,"couldn't find transform ",name,NULL);
     return TCL_ERROR;
   }

   osw::DataSource *dataSource = dynamic_cast<osw::DataSource *>(transform);
   if (dataSource == NULL) {
     Tcl_AppendResult (interp,"transform ",name," is not a data source",NULL);
     return TCL_ERROR;
   }

   dataSource->SetData(Tcl_GetStringFromObj (objv[2],NULL));

   return TCL_OK;
}


/*************/

static int OSWTcl_DrawTransform (ClientData cd, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[]) {

   if (objc < 3) {
     Tcl_AppendResult (interp,"usage: oswDrawTransform transform window",NULL);
     return TCL_ERROR;
   }
   
   char *name = Tcl_GetStringFromObj (objv[1],NULL);
   char *drawingSpaceName = Tcl_GetStringFromObj (objv[2],NULL);

   osw::Transform *transform = dynamic_cast<osw::Transform *>(osw::Nameable::Find(name));
   
   if (transform == NULL) {
     Tcl_AppendResult (interp,"couldn't find transform ",name,NULL);
     return TCL_ERROR;
   }

   if (dynamic_cast<osw::TkInteractiveDrawing *>(transform) != NULL) {
     osw::TkInteractiveDrawing *tkdraw = 
       dynamic_cast<osw::TkInteractiveDrawing *>(transform);
     osw::TkDrawingInfo drawInfo;
     drawInfo.interp = interp;
     drawInfo.drawingSpace = Tk_NameToWindow(interp,drawingSpaceName,
					     Tk_MainWindow(interp));
     tkdraw->drawInfo = drawInfo;
     tkdraw->widget = drawingSpaceName;
   }

   Tcl_AppendResult(interp,transform->Draw(NULL),NULL);
   return TCL_OK;
}

/*************/

static int OSWTcl_PersistentState (ClientData cd, Tcl_Interp *interp, 
                                   int objc, Tcl_Obj * const objv[]) 
{
   
   if (objc < 2) {
      Tcl_AppendResult (interp,"usage: oswPersistentState transform ",NULL);
      return TCL_ERROR;
   }
   
   char *name = Tcl_GetStringFromObj (objv[1],NULL);
   
   osw::Transform *transform 
      = dynamic_cast<osw::Transform *>(osw::Nameable::Find(name));
   
   if (transform == NULL) {
      Tcl_AppendResult (interp,"couldn't find transform ",name,NULL);
      return TCL_ERROR;
   }
   
   Tcl_AppendResult(interp,transform->PersistentState().c_str(),NULL);
   return TCL_OK;
}

/*************/

static int OSWTcl_Delete (ClientData cd, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[]) {

   if (objc < 2) {
     Tcl_AppendResult (interp,"usage: oswDelete transform ",NULL);
     return TCL_ERROR;
   }
   
   char *name = Tcl_GetStringFromObj (objv[1],NULL);

   osw::Transform *transform = dynamic_cast<osw::Transform *>(osw::Nameable::Find(name));
   
   if (transform == NULL) {
     Tcl_AppendResult (interp,"couldn't find transform ",name,NULL);
     return TCL_ERROR;
   }

   osw::DeferredDelete(transform);
   return TCL_OK;
}   

/***************/

static int OSWTcl_Dialog (ClientData cd, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[]) {

   if (objc < 2) {
     Tcl_AppendResult (interp,"usage: oswDialog container ",NULL);
     return TCL_ERROR;
   }
   
   char *name = Tcl_GetStringFromObj (objv[1],NULL);

   osw::Container *container = dynamic_cast<osw::Container *>(osw::Nameable::Find(name));
   
   if (container == NULL) {
     Tcl_AppendResult (interp,"couldn't find container ",name,NULL);
     return TCL_ERROR;
   }

   Tcl_AppendResult(interp,container->Dialog(),NULL);
   return TCL_OK;
}

/***************/

static int OSWTcl_TransformMenu (ClientData cd, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[]) {

   if (objc < 2) {
     Tcl_AppendResult (interp,"usage: oswTransformMenu container ",NULL);
     return TCL_ERROR;
   }
   
   char *name = Tcl_GetStringFromObj (objv[1],NULL);

   osw::Container *container = dynamic_cast<osw::Container *>(osw::Nameable::Find(name));
   
   if (container == NULL) {
     Tcl_AppendResult (interp,"couldn't find container ",name,NULL);
     return TCL_ERROR;
   }

   Tcl_AppendResult(interp,container->Menu(),NULL);
   return TCL_OK;
}

/***************/

static int OSWTcl_AutoNameable (ClientData cd, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[]) {

   if (objc < 2) {
     Tcl_AppendResult (interp,"usage: oswAutoNameable transformClass",NULL);
     return TCL_ERROR;
   }
   
   char *name = Tcl_GetStringFromObj (objv[1],NULL);

   Tcl_SetObjResult(interp,Tcl_NewIntObj(osw::AutoNameable(name)));
   return TCL_OK;
}

/****************/

static int OSWTcl_TransformTypes (ClientData cd, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[]) {

  char *prefix = "";
  int prefixLength = 0;

  if (objc > 1) {
    prefix = Tcl_GetStringFromObj(objv[1],&prefixLength);
  }

  Tcl_Obj *result = Tcl_NewListObj(0,NULL);
  
  osw::BaseTransformFactory::FactoryMap *factories 
    = osw::GetFactories();

  for (osw::BaseTransformFactory::FactoryMap::iterator index 
	 = factories->begin();
       index != factories->end();
       ++index) {
    if (!strncmp(prefix,(*index).first.c_str(),prefixLength)) {
      Tcl_ListObjAppendElement(interp,result,
			       Tcl_NewStringObj(const_cast<char *>
						((*index).first.c_str()),-1));
    }
  }

  Tcl_SetObjResult(interp,result);
  return TCL_OK;
}

static int OSWTcl_LoadOSX (ClientData cd, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[]) {

  if (objc < 2) {
     Tcl_AppendResult (interp,"usage: oswLoadOSX osxname ",NULL);
     return TCL_ERROR;
   }
   
  char *osxname = Tcl_GetStringFromObj (objv[1],NULL);

  Tcl_SetObjResult(interp,Tcl_NewIntObj(LoadOSX(osxname,interp)));

  return TCL_OK;
}


static int OSWTcl_Profile (ClientData cd, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[]) {
  
  if (objc < 2) {
    Tcl_AppendResult (interp,"usage: oswProfile (true|false)",NULL);
    return TCL_ERROR;
  }

  int onoff;

  Tcl_GetBooleanFromObj(interp,objv[1],&onoff);

  if (onoff) {
    osw::EnableProfiling();
  } else {
    osw::DisableProfiling();
  }

  return TCL_OK;
}

static int OSWTcl_DumpProfile (ClientData cd, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[]) {
  
  if (objc < 2) {
    Tcl_AppendResult (interp,"usage: oswDumpProfile filename",NULL);
    return TCL_ERROR;
  }
  osw::DumpProfileToFile(Tcl_GetStringFromObj(objv[1],NULL));
  return TCL_OK;
}

static int OSWTcl_Version (ClientData cd, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[]) {

  Tcl_AppendResult(interp,const_cast<char *>(osw::GetVersionString()),NULL);

  return TCL_OK;
}

static int OSWTcl_Quit (ClientData cd, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[]) {

  osw::Quit();
  CleanupEvalServer();
  osw::Cleanup();

  return TCL_OK;
}

static int OSWTcl_MainLoop (ClientData cd, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[]) {

  osw::MainLoop();
  
  return TCL_OK;
}

static int OSWTcl_GetCommandLineFiles (ClientData cd, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[]) {

  Tcl_Obj *listResult = Tcl_NewListObj (0,NULL);
  std::list<osw::String> *commandLineFiles = osw::GetCommandLineFiles();
  
  for (std::list<osw::String>::iterator p = commandLineFiles->begin();
       p != commandLineFiles->end();
       ++p) {
    Tcl_ListObjAppendElement(interp,listResult,Tcl_NewStringObj(const_cast<char *>(p->c_str()),
								-1));
  }

  Tcl_SetObjResult(interp,listResult);
  return TCL_OK;

}

static int OSWTcl_GetPreference (ClientData cd, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[]) {
  if (objc < 2) {
    Tcl_AppendResult (interp,"usage: oswGetPreference name [default]",NULL);
    return TCL_ERROR;
  }
  string name = Tcl_GetString(objv[1]);
  string defaultValue = (objc > 2) ? Tcl_GetString(objv[2]) : "";
  string value;
  osw::GetPreference(name,value,defaultValue);
  Tcl_SetObjResult(interp,Tcl_NewStringObj(const_cast<char *>(value.c_str()),-1));
  return TCL_OK;
}


static int OSWTcl_SetPreference (ClientData cd, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[]) {
  if (objc < 3) {
    Tcl_AppendResult (interp,"usage: oswGetPreference name value",NULL);
    return TCL_ERROR;
  }  
  osw::SetPreference(Tcl_GetString(objv[1]),Tcl_GetString(objv[2]));
  return TCL_OK;
}



/********************/

static void *evalServer = NULL;

static void SetupEvalServer () {
  
  evalServer = osw::OpenTCPSocket("localhost",TCL_PORTNUM);
  
}

static void CleanupEvalServer () {
  osw::CloseSocket(evalServer);
}


static void EvalWhenIdle(ClientData cd) {
  char *toEval = (char *) cd;
  Tcl_EvalEx(lastInterp,toEval,-1,TCL_EVAL_GLOBAL);
  free(toEval);
}

extern "C" void _oswexport oswTclEvalString (const char *toEval) {
  if (lastInterp && osw::IsThreadIDRunning(s_threadid)) {
    Tcl_DoWhenIdle(EvalWhenIdle,(ClientData)strdup(toEval));
  } else if (evalServer) {
    osw::TCPSocketSend(evalServer,toEval,strlen(toEval));
  }
}

extern "C" void _oswexport oswTclEvalStringOnTheEdge (const char *toEval) {
  if (lastInterp && osw::IsThreadIDRunning(s_threadid)) {
    Tcl_DoWhenIdle(EvalWhenIdle,(ClientData)strdup(toEval));	
  } else if (evalServer) {
    osw::TCPSocketSend(evalServer,toEval,strlen(toEval));
  }
}

/*******************/

namespace osw {
  
  class _oswexport TclEvalTransform : public Transform {

  public:

    ChangeableInlet  toEval;
    
    TclEvalTransform (const string &aname, Container *acontainer,
		      int argc, char *argv[]) :
      Transform (aname,acontainer,argc,argv),
      toEval ("toEval",this,"Tcl script to evaluate."),
      activation1(toEval,this,&TclEvalTransform::activation1Expr) {
	
    }
	
  protected:

    Activation<TclEvalTransform> activation1;

    void activation1Expr () {

      ostringstream os;
      os << *toEval << endl;
      oswTclEvalString(os.str().c_str());
    }
  };

  static TransformFactory<TclEvalTransform> TclEvalFactory ("TclEval",true);
}





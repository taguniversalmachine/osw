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

#include <tcl.h>
#include "osw.h"
#include "oswLoadOSX.h"
#include "oswFileUtils.h"
#include <dlfcn.h>
#include <unistd.h>

using namespace std;


bool LoadOSX (const char *osxname) {

  string fileNameWithExt (osxname);
  fileNameWithExt += ".osx";
  
  void * handle = dlopen(fileNameWithExt.c_str(),RTLD_NOW | RTLD_GLOBAL);

  if (handle == NULL) {
    string OSWPathName = osw::GetOSWDir();
    OSWPathName += "/externals/";
    OSWPathName += fileNameWithExt;
    handle = dlopen(OSWPathName.c_str(),RTLD_NOW | RTLD_GLOBAL);

    if (handle == NULL) {
      osw::Error(dlerror());
      return false;
    }
  }

  return true;
}
  
#ifdef _USING_TCL

bool LoadOSX (const char *osxname, Tcl_Interp *interp) {

  string fileNameWithExt (osxname);
  fileNameWithExt += ".osx";
  
  void * handle = dlopen(fileNameWithExt.c_str(),RTLD_NOW | RTLD_GLOBAL);

  if (handle == NULL) {
    string OSWPathName = osw::GetOSWDir();
    OSWPathName += "/externals/";
    OSWPathName += fileNameWithExt;
    handle = dlopen(OSWPathName.c_str(),RTLD_NOW | RTLD_GLOBAL);

    if (handle == NULL) {
      osw::Error(dlerror());
      return false;
    }
  }

  
  string initprocName(osxname);
  if (initprocName.find_last_of('/') != string::npos) {
    initprocName.erase(0,initprocName.find_last_of('/')+1);
  }
  initprocName += "_Init";
  Tcl_PackageInitProc *initproc = 
    (Tcl_PackageInitProc *) dlsym(handle,initprocName.c_str());
  if (initproc != NULL && interp != NULL) {
    (*initproc)(interp);
  }
  
  return true;
}

#endif

bool InstallOSX (const char *osxname) {
  char *ext = strstr(osxname,".osx");
  string rootName;
  if (ext) {
    rootName = string(osxname,ext-osxname);
  } else {
    rootName = osxname;
  }

  string SourcePath = "";
  if (rootName.find_last_of("/") != string::npos) {
    SourcePath = string(rootName,rootName.find_last_of("/"));
    rootName = string(rootName,rootName.find_last_of("/")+1,
		      string::npos);
  }

  osw::Alert("root: "+rootName);
  osw::Alert("path: "+SourcePath);
  if (SourcePath != "") {
    SourcePath += "/";
  }
  if (SourcePath == "" || SourcePath[0] != '/') {
    char buf[256];
    getcwd(buf,256);
    SourcePath = buf + ("/" + SourcePath);
  }
  string OSWPathName = osw::GetOSWDir();
  OSWPathName += "/externals/";
  
  string OSXFile = rootName + ".osx";
  string OSDFile = rootName + ".osd";
  string OSLFile = rootName + ".osl";
  string HTMLPage = rootName + ".html";
  string HelpPatch = "help_"+rootName+".osw";
  string IncludeFile = rootName + ".h"; // for type externals

  FILE *test = fopen((SourcePath+OSXFile).c_str(),"rb");
  if (!test) {
    osw::Error("Cannot open file " + OSXFile);
    return false;
  }
  fclose(test);

  void * handle = dlopen((SourcePath+OSXFile).c_str(),RTLD_NOW | RTLD_GLOBAL);
  if (handle == NULL) {
    osw::Error("Cannot open " + OSXFile + " as library.");
    return false;
  }

  char **packageName = (char **)dlsym(handle,"g_Package");
  if (packageName == NULL) {
    osw::Error(OSXFile + " does not appear to a valid OSW external");
    goto end1;
  }

  OSWPathName += *packageName;
  OSWPathName += "/";
  if (OSWPathName == SourcePath) {
    osw::Error("Cannot copy external onto itself");
  }
  osw::MakeDir(OSWPathName);
  if (system(("cp "+SourcePath+OSXFile+" "+OSWPathName).c_str())) {
    perror("InstallOSX");
    goto end1;
  }
  osw::Alert(OSXFile + " copied");
  if ((test = fopen((SourcePath+OSDFile).c_str(),"r")) != NULL) {
    if (system(("cp "+SourcePath+OSDFile+" "+OSWPathName).c_str())) {
      perror("InstallOSX");
      goto end1;
    } 
    osw::Alert(OSDFile + " copied");
  }
  if ((test = fopen((SourcePath+OSLFile).c_str(),"r")) != NULL) {
    if (system(("cp "+SourcePath+OSLFile+" "+OSWPathName).c_str())) {
      perror("InstallOSX");
      goto end1;
    } 
    osw::Alert(OSLFile + " copied");
  }
  if ((test = fopen((SourcePath+HTMLPage).c_str(),"r")) != NULL) {
    OSWPathName = osw::GetOSWDir();
    OSWPathName += "/html/";
    OSWPathName += *packageName;
    OSWPathName += "/";
    if (system(("cp "+SourcePath+HTMLPage+" "+OSWPathName).c_str())) {
      perror("InstallOSX");
      goto end1;
    } 
    osw::Alert(HTMLPage + " copied");
  }
  if ((test = fopen((SourcePath+HelpPatch).c_str(),"r")) != NULL) {
    OSWPathName = osw::GetOSWDir();
    OSWPathName += "/help_patches/";
    OSWPathName += *packageName;
    OSWPathName += "/";
    if (system(("cp "+SourcePath+HTMLPage+" "+OSWPathName).c_str())) {
      perror("InstallOSX");
      goto end1;
    } 
    osw::Alert(HelpPatch + " copied");
  }

  if ((test = fopen((SourcePath+IncludeFile).c_str(),"r")) != NULL) {
    OSWPathName = osw::GetOSWDir();
    OSWPathName += "/help_patches/";
    OSWPathName += *packageName;
    OSWPathName += "/";
    if (system(("cp "+SourcePath+HTMLPage+" "+OSWPathName).c_str())) {
      perror("InstallOSX");
      goto end1;
    } 
    osw::Alert(IncludeFile + " copied");
  }

 end1:
  dlclose(handle);
  return true;
}

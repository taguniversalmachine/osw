
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
   oswArgs.cpp
   Functions for managing command-line arguments to OSW
*/


#ifdef _WINDOWS
#include <windows.h>
#endif

#include "osw.h"
#include "oswArgs.h"
#include "oswLoadOSX.h"
#include "oswFileUtils.h"

namespace osw {

  typedef list<CommandLineArg> CommandLineArgs;

  int s_test = 666;
  static CommandLineArgs *s_commandLineArgs = NULL;
  static std::list<String>    s_commandLineFiles;

  CommandLineArg::CommandLineArg (const String &ashortArg, 
				  const String &alongArg, 
				  const String &adesc,
				  CommandLineArgProc aproc) :
    shortArg (ashortArg),
    longArg (alongArg),
    desc (adesc),
    proc (aproc) {
    if (s_commandLineArgs == NULL || s_test != 666) {
      s_commandLineArgs = new CommandLineArgs;
    }
    s_commandLineArgs->push_back(*this);
  }

  ostream & operator << (ostream &os, const CommandLineArg &toPrint) {
    os << '\t';
    if (toPrint.shortArg != "") {
      os << toPrint.shortArg;
      if (toPrint.longArg != "") {
	os << ',';
      }
    }
    if (toPrint.longArg != "") {
      os << toPrint.longArg;
    }
    os << '\t';
    os << toPrint.desc;
    return os;
  }

  void _oswexport ProcessArgs (int argc, char *argv[]) {
    for (int i = 0; i < argc; ++i) {
      if (argv[i][0] != '-') {
	//not an option, assume it is a file
	s_commandLineFiles.push_back(argv[i]);
	continue;
      }
      CommandLineArgs::iterator found = 
	find(s_commandLineArgs->begin(),s_commandLineArgs->end(),argv[i]);
      if (found == s_commandLineArgs->end()) {
	Error(String("Invalid option: ")+argv[i]);
	Error("Use \"--help\" to see a list of valid options.");
	exit(-1);
      } else {
	if ((*found)(argc,argv,i)) {
	  exit(0);
	}
      }
    }
  }

  std::list<String> *GetCommandLineFiles () {
    return &s_commandLineFiles;
  }

  static bool HelpArgProc (int argc, char *argv[], int &index) {
    cout << "Command-line options:" << endl;
    for (CommandLineArgs::iterator p = s_commandLineArgs->begin();
	 p != s_commandLineArgs->end();
	 ++p) {
      cout << *p << endl;
    }
    // help exits
    return true;
  }

  static CommandLineArg 
  s_help("-h","--help","List available command-line options",HelpArgProc);

  static bool VersionArgProc (int argc, char *argv[], int &index) {
    cout << "OSW Version " << GetVersionString() << endl;
    // version exits
    return true;
  }

  static CommandLineArg
  s_version("-V","--version","Display OSW version",VersionArgProc);

  /******************************/

  static bool InstallArgProc(int argc, char *argv[], int &index) {
    ++index;
    if (index == argc) {
      osw::Error("usage: --install [name][.osx]");
      return true;
    }
    bool result = InstallOSX(argv[index]);
    return result;
  }

  static CommandLineArg
  s_install("--install","","Install external (.osx and help files)",InstallArgProc);

  static bool InstallWaveArgProc (int argc, char *argv[], int &index) {
    ++index;
    if (index == argc) {
      osw::Error(string("usage: ")+argv[index-1]+string(" [name]"));
      return true;
    }
    string OSWDir = GetOSWDir();
    string AudioDir = OSWDir + GetDirSeparator() + string("audio");
    MakeDir(OSWDir + GetDirSeparator() + string("audio"));
    CopyFile(argv[index],
	     (AudioDir + GetDirSeparator() + string(argv[index])));
    return true;
  }

  static CommandLineArg
  s_installwave("--install-audio","","Intall audio file",InstallWaveArgProc);

  /******************/

  //! This cleanup handler removes s_commandLineArgs

  BEGIN_CLEANUP_HANDLER(Args)
  {
    delete s_commandLineArgs;
  }
  END_CLEANUP_HANDLER(Args)

} // namespace osw



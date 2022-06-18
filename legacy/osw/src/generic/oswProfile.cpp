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

/*
   oswProfile.cp
   Implementation of transform profiling 
   Amar Chaudhary
*/


#include "oswTime.h"
#include <fstream>
#ifndef _NO_LIMITS
#include <limits>
#else
#include <climits>
#endif

using namespace std;

namespace osw {

  bool profile_on(false);

  void _oswexport EnableProfiling () {
    for (Nameable::enumerator e = Nameable::BeginEnumerate();
	 !Nameable::EndEnumerate(e);
	 ++e) {
      Container *container = dynamic_cast<Container *>(e->second);
      if (container != NULL) {
	container->x_runtime = 0;
      }
    }
    profile_on = true;
  }

  void _oswexport DisableProfiling () {
    profile_on = false;
  }

  void DumpProfileToFile (const char *filename) {

    ofstream os(filename);

    float period = 1.0f / HiResTimerFrequency();
    for (Nameable::enumerator e = Nameable::BeginEnumerate();
	 !Nameable::EndEnumerate(e);
	 ++e) {
      Container *container = dynamic_cast<Container *>(e->second);
      if (container != NULL) {
	if (container->x_runtime > 0) {
	  os << container->PathName() << '\t' << (container->x_runtime * period) << endl;
	  //cout << container->PathName() << '\t' << (container->x_runtime * period) << endl;
	}
      }
    }
  }


  /*************/

  class Profile : public TimeDomainTransform {

  public:

    Inlet<Time>  timeIn;
    State<string> file;
    State<float> interval;
    Profile (const string &aname, Container *acontainer,
	     int argc, char *argv[]) :
      TimeDomainTransform(aname,acontainer,argc,argv),
      timeIn("timeIn",this,"Explicit time control"),
      file("file",this,"Output file","profile.txt"),
      interval("interval",this,"Interval between measurements",0.1),
      activation1(&timeIn,stl::numeric_limits<int>::max(),this,&Profile::activation1Expr),
      fileChanged(&file,this,&Profile::fileChangedExpr)
    {
      os.open(file->c_str());
      oldtime = 0.0;
      sincelasttick = 0.0;
    }

    ~Profile () {
      DisableProfiling();
      os.close();
    }

    string PersistentState () const {return "file";}

  private:

    ofstream os;
    double oldtime;
    double sincelasttick;

    Activation<Profile> activation1,fileChanged;

    void activation1Expr () {
      double newtime = double(((Time &) timeIn));
      double timediff = fabs(newtime - oldtime);
      sincelasttick += timediff;
      oldtime = newtime;
      if (sincelasttick >= ((float &) interval)) {
	sincelasttick -= ((float &) interval); 
	Dump();
	EnableProfiling();
	os << oldtime << endl;
      }
    }

    void fileChangedExpr () {
      os.close();
      os.open(file->c_str());
    }
    
    void Dump() {
      float period = 1.0f / HiResTimerFrequency();
      for (Container::enumerator e = Container::BeginEnumerate();
	   !Container::EndEnumerate(e);
	   ++e) {
	Container *container = (Container *) e->second;
	if (container->x_runtime > 0) {
	  os << container->PathName() << '\t' << (container->x_runtime * period) << endl;
	  //cout << container->PathName() << '\t' << (container->x_runtime * period) << endl;
	}
      }
      os << ".\n";
    }
    
  };

  static TransformConstructor<Profile> ProfileConstructor
	("Profile",false,"");

}





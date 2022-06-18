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

  
*/

/*
 oswLaunchURL.h
 Invoke platform-specific web-browser on URL
*/

#ifndef _OSW_LAUNCH_URL_H
#define _OSW_LAUNCH_URL_H

#include <string>

using namespace std;

namespace osw {

  //! Open platform-specific browser on a URL
  /**
   * @param url string containing url to open in browser
   *
   */

  void LaunchURL(const string &url);

}

#endif // _OSW_LAUNCH_URL_H

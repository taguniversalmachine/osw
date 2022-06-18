
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
   oswLinuxPrefs.cpp
   Linux implementation of functions for managing persistent preferences
*/

#include "osw.h"
#include "oswPrefs.h"
#include <sys/types.h>
#include <limits.h>
#include <sys/stat.h>
#include <fcntl.h>

#define DB_FILE ".osw_preferences"

namespace osw {
   
   static char s_buffer[512];

   struct Preferences  {
      
   private:
       
      typedef map<String,String> implementation;
      implementation             *xdata;
      
   public:
      
      Preferences () {
         xdata = NULL;
         
      }
      
      void Init () {
         if (xdata == NULL) {
            xdata = new implementation;
         }

         char *prefix = getenv("HOME");

         if (!prefix) {
            prefix = getenv("OSW_DIR");
         }
         
         sprintf (s_buffer,"%s/%s",prefix,DB_FILE);
         FILE *dbfile = fopen(s_buffer,"r");
         if (!dbfile) {
            return;
         }
         
         while (!feof(dbfile)) {
           if (!fgets(s_buffer,511,dbfile)) {
             break;
           }
           int r = strlen(s_buffer);
           if(r > 0 && s_buffer[r-1] == '\n') {
             s_buffer[r-1] = '\0';
           }
           char *p;
           for (p = s_buffer; *p && !isspace(*p); ++p);
           *p = 0;
           for (++p; *p && isspace(*p); ++p);
           (*xdata)[s_buffer] = p;
         }
         fclose(dbfile);
      }
      
      ~Preferences () {
      }
    
    String & operator [] (const String &index) {
      if (!xdata) Init();
      return (*xdata)[index];
    }
    
    void Remove (const String &index) {
      if (!xdata) Init();
      xdata->erase(index);
    }
    
    bool Exists (const String &toTest) {
      if (!xdata) Init();
      return xdata->find(toTest) != xdata->end();
    }

   
     void Save () {
       if (xdata) {
         char *prefix = getenv("HOME");
         if (!prefix) {
           prefix = getenv("OSW_DIR");
         }
         sprintf (s_buffer,"%s/%s",prefix,DB_FILE);
         FILE *dbfile = fopen(s_buffer,"w");
         if (dbfile == NULL) {
           perror("osw preferences");
           return;
         }
         for (implementation::iterator p = xdata->begin();
              p != xdata->end();
              ++p) {
           fprintf (dbfile,"%s\t%s\n",p->first.c_str(),p->second.c_str());
         }
         fclose(dbfile);
       }
     }
  };
  
  static Preferences s_Preferences;
  
  _oswexport bool GetPreference(const string &name,
				string &value, 
				const string &defaultValue) {
    if (s_Preferences.Exists(name)) {
      value = s_Preferences[name];
    } else {
      value = defaultValue;
    }
    return true;
  }

  _oswexport bool SetPreference(const string &name, const string &value) {
    s_Preferences[name] = value;
    s_Preferences.Save();
    return true;
  }

  _oswexport bool RemovePreference(const string &name) {
    s_Preferences.Remove(name);
    s_Preferences.Save();
    return true;
  }


} // namespace osw

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
  oswtable.cpp
  Lookup tables
  Amar Chaudhary
*/


#include "oswTable.h"
#include "oswDataSource.h"
#include "oswTkDraw.h"
#include "oswTcl.h"
#include <fstream>

#define TABLESIZE 1024

#define M_PIf          (3.14159265358979323846f)

using namespace std;

namespace osw {

   static MemInit _memInit; // there was a reason this had to be placed here

#ifdef OSW_NEEDS_MATHF_FUNCTIONS
   static float sinf2pi (float x) {
      return sin(2.0f * M_PIf * x);
   }
   static float cosf2pi (float x) {
      return cos(2.0f * M_PIf * x);
   }
   static float expfnpi (float x) {
      return exp(-M_PIf * x);
   }
   static float sincf (float x) {
      x -= 0.5f;
      x *= 4.0f * M_PIf;
      return (x == 0.0f) ? 1 : sin(x)/x;
   }
#else
   static float sinf2pi (float x) {
      return sinf(2.0f * M_PIf * x);
   }
   static float cosf2pi (float x) {
      return cosf(2.0f * M_PIf * x);
   }
   static float expfnpi (float x) {
      return expf(-M_PIf * x);
   }
   static float sincf (float x) {
      x -= 0.5f;
      x *= 4.0f * M_PIf;
      return (x == 0.0f) ? 1 : sinf(x)/x;
   }
#endif

   _oswexport Table<float> SineTable(sinf2pi,TABLESIZE,0.0f,1.0f);
   _oswexport Table<float> CosineTable(cosf2pi,TABLESIZE,0.0f,1.0f);
   _oswexport Table<float>  ExpTable(expfnpi,TABLESIZE,0.0f,1.0f);

   _oswexport Table<float> SincTable(sincf,TABLESIZE,0.0f,1.0f);

  
   REGISTER_TYPE(FloatTable);
   REGISTER_TYPE(IntegerTable);

   static State<Table<float> > SineTableState("sine",NULL,
                                              "Sine function table",
                                              SineTable);
   static State<Table<float> > CosineTableState("cosine",NULL,
                                                "CosineFunction table",
                                                CosineTable);
   static State<Table<float> > ExpTableState("exp",NULL,
                                             "Exponentional function between -PI/2 and 0",
                                             ExpTable);
   static State<Table<float> > SincTableState("sinc",NULL,
                                              "Sinc function table",
                                              SincTable);

   typedef Table<int> IntTable;
   REGISTER_TYPE(IntTable);

   /*****************/

   class _oswexport TableTransform : public Transform, 
                                     public DataSource,
                                     public TkInteractiveDrawing {

   public:

     Table<float>  table;
     State<int> size;
     State<float> min,max;
     State<Boolean> drawInline;
     State<Unit> tableChanged;
     TableTransform (const string &aname, Container *acontainer,
                     int argc, char *argv[]) :
       Transform (aname,acontainer,argc,argv),
       DataSource(this,acontainer,argc,argv),
       TkInteractiveDrawing(this),
       size("size",this,"Number of elements in table.",
            ScanArguments<int>("-size",-1,acontainer,argc,argv)),
       xcopy(ScanArguments<string>("-copy","",
                                   acontainer,argc,argv)),
       min("min",this,"",ScanArguments<float>("-min",0.0f,
                                              acontainer,argc,argv)),
       max("max",this,"",ScanArguments<float>("-max",1.0f,
                                              acontainer,argc,argv)),
       drawInline("inline",this,
                  "Show the table data inline in the patch window",
                  ScanArguments<Boolean>("-inline",false,
                                         acontainer,argc,argv)),
       tableChanged("changed",this,"set when table is changed externally"),
       update_inline(&tableChanged,this,&TableTransform::update_inline_expr)
     {
       
       if (xcopy != "") {
         Alert("Copying from table " + xcopy);
         Table<float> *temp = GetTable(xcopy,GetContainer());
         if (size == -1) {
           if (temp != NULL) {
             size = temp->size();
           } else {
             size = 256;
           }
         }
         table = Table<float>(size,min,max);
         if (temp != NULL) {
           int n = std::min((int)size,temp->size());
           for (int i = 0; i < n; ++i) {
             table[i] = (*temp)[i];
           }
         } else {
           Error ("Can't find table " + xcopy);
         }
       }
       else
         {
           if (size == -1) {
             size = 256;
           }
           table = Table<float>(size,min,max);
           for ( int i = 0; i < (int)size; i++ )
             {
               table[i] = 0.0f;
             }
         }
       tableChanged.SetParallel();
     }
     
     string PersistentState () const {
       return Transform::PersistentState() + " size min max ";
     }
     
     const char *Description () const {
       return "Defines a lookup table.";
     }
     
     const char *Dialog () {
       return "oswLoadScript table.tcl ; "
         "ShowTable $transformPath \n";
     }
     
     const char *Draw (void *DrawState) {
       if (drawInline) {
         return "oswLoadScript table.tcl ; "
           "HideName\n"
           "label $drawingSpace.name -text [file tail $transform]  -bg [$drawingSpace cget -bg]\n"
           "grid $drawingSpace.name -row 0 -column 1\n"
           "ShowTableInline $transformPath $drawingSpace\n";
       } else {
         return "";
       }
     }
     
     string GetData () const {
       ostringstream os;
       os << table;
       return os.str();
     }
     
     void SetData (const string &data) {
       istringstream is(data);
       is >> table;
     }
     
     void SetDataFromSource () {
       ifstream is(src->c_str());
       is >> table;
     }
     
   private:
     
     string xcopy;

     Activation<TableTransform> update_inline;

     void update_inline_expr () {
       if (drawInline) {
         string cmd = "DrawTableInline " + PathName() + " " + widget + ".c";
         WidgetMethod("DrawTableInline",
                      PathName(),
                      widget+".c");
       }
     }
     
   };
  
  static 
  TransformFactory<TableTransform> TableTransformFactory ("Table");
  
  OSW_DECLARE_USE(TableTransformFactory);
  
  /******************/
  
  Table<float> *GetTable (const string &aname, Container *acontainer) {
    Nameable *found = Nameable::Find(aname,acontainer);
    
    if (found == NULL) {
      return NULL;
    }
    if (dynamic_cast<State<Table<float> > *>(found) != NULL) {
      return &((Table<float> &)(*dynamic_cast<State<Table<float> > *>(found)));
    }
    if (dynamic_cast<TableTransform *>(found) != NULL) {
      return &(dynamic_cast<TableTransform *>(found)->table);
    }
    return NULL;
  }
  
   State<Unit> *GetTableChangedVariable (const string &aname, 
                                         Container *acontainer) {
     Nameable *found = Nameable::Find(aname+"/changed",acontainer);
     return dynamic_cast<State<Unit> *>(found);
   }

   /**************************/

   // More pre-defined tables (for FFTs, etc.)

  

} // namespace osw
  








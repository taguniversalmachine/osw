#include <cstdlib>
#include <tcl.h>
#include <tk.h>
#include "oswCfg.h"
#include "oswInit.h"

using namespace std;

/*****************/


typedef struct ThreadSpecificData {
    Tcl_Interp *interp;         /* Interpreter for this thread. */
    Tcl_DString command;        /* Used to assemble lines of terminal input
				 * into Tcl commands. */
    Tcl_DString line;           /* Used to read the next line from the
				 * terminal input. */
    int tty;                    /* Non-zero means standard input is a 
				 * terminal-like device.  Zero means it's
				 * a file. */
} ThreadSpecificData;
Tcl_ThreadDataKey dataKey;


//slimy hack alert!
static int gargc;
static char **gargv;

int Tcl_AppInit (Tcl_Interp *interp) {

   Tcl_SetVar(interp,"argv",Tcl_Merge(gargc-1,gargv+1),TCL_GLOBAL_ONLY);
   Tcl_SetVar(interp, "tcl_interactive","0",TCL_GLOBAL_ONLY);
   Tk_Window main;
   if (Tcl_InitStubs(interp, TCL_VERSION, 1) == NULL) {
     abort();
   }
   if (Tcl_Init(interp) == TCL_ERROR) {
     return TCL_ERROR;
   }
   if (Tk_Init(interp) == TCL_ERROR) {
     return TCL_ERROR;
   }

   Tcl_StaticPackage(interp, "Tk", Tk_Init, Tk_SafeInit);

   //get the proper full path for oswinit.tcl
   char path[256];
   sprintf (path,"%s",Tcl_GetNameOfExecutable());
   int path_end = 0;
   for (int i = 0; path[i] != '\0'; ++i) {
     if (path[i] == '/' || path[i] == '\\') {
       path_end = i;
     }
   }
   sprintf(path+path_end,"/oswinit.tcl");
   Tcl_EvalFile(interp,path);

   return TCL_OK;
}



int main (int argc, char *argv[]) {

  Tcl_FindExecutable(argv[0]);
  Tcl_Interp *interp = Tcl_CreateInterp();
  Tcl_SetVar(interp,"argv",Tcl_Merge(argc-1,argv+1),TCL_GLOBAL_ONLY);

  gargc = argc;
  gargv = argv;
  Tk_Main(1,argv,Tcl_AppInit);
    
  Tcl_DeleteInterp(interp);
  //osw::Init();
  //osw::MainLoop();
  return 0;
}


/***********************/




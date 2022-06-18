
#Copyright (c) 2001-2003 Amar Chaudhary. All rights reserved.
#Copyright (c) 1998-2001 Regents of the University of California.
#All rights reserved.
#
#     ``The contents of this file are subject to the Open Sound World Public
#     License Version 1.0 (the "License"); you may not use this file except in
#     compliance with the License. A copy of the License should be included
#     in a file named "License" or "License.txt" in the distribution from 
#     which you obtained this file. 
#
#     Software distributed under the License is distributed on an "AS IS"
#     basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
#     License for the specific language governing rights and limitations
#     under the License.
#
#     The Original Code is Open Sound World (OSW) software.
#
#     The Initial Developer of the Original Code is Amar Chaudhary.
#     Portions created by Amar Chaudhary are Copyright (C) 1998-2001 
#     Regents of the University of California and Amar Chaudhary. 
#     All Rights Reserved.
#
#     Contributor(s):

CC = cl
OLDC = cl

INCLUDES = -D_OSWOSX  -I$(OSWDIR)/include -I. -I$(TCL_DIR)/include  -I$(OSWDIR)/types -I$(OSWDIR)/generic -I$(OSWDIR)/sdif
OPTFLAGS   = -O2 $(INCLUDES) -GR -GX -G6 -Oa -Ob2 -MD -D "_WINDOWS" -Zm1000
DEBUGCFLAGS = $(INCLUDES) -GR -GX -GZ -Zi -MDd -D "_WINDOWS" -D "OSW_DEBUG" -Zm100

!IFDEF USEDEBUG
CFLAGS = $(DEBUGCFLAGS)
!ELSE
CFLAGS = $(OPTFLAGS)
!ENDIF

OBJECTS  = $(OSXNAME).obj $(USEROBJ)

!IFDEF USEDEBUG
LIBFLAGS = /DLL /LIBPATH:$(TCL_DIR)/lib /LIBPATH:$(OSWLIBDIR) /MACHINE:X86 /SUBSYSTEM:console /FORCE:multiple /EXETYPE:DYNAMIC /DEBUG
!ELSE
LIBFLAGS = /DLL /LIBPATH:$(TCL_DIR)/lib /LIBPATH:$(OSWLIBDIR) /MACHINE:X86 /SUBSYSTEM:console /FORCE:multiple /EXETYPE:DYNAMIC
!ENDIF

LIBRARIES = tcl82.lib tk82.lib winmm.lib user32.lib libosw.lib $(USERLIB) 


all: clear $(OSXNAME).osx done

$(OSXNAME).osx : $(OBJECTS)
	link @<<
  /OUT:$(OSXNAME).osx $(LIBFLAGS) $(OBJECTS) $(LIBRARIES)
<<

$(OSXNAME).obj : $(OSXNAME).cpp $(DOT_H)
	$(CC) $(CFLAGS) -c $(OSXNAME).cpp

install: $(OSXNAME).osx
	copy *.osx $(INSTALL_DIR)\externals\$(PACKAGE)
	copy *.osd $(INSTALL_DIR)\externals\$(PACKAGE)
	copy *.osl $(INSTALL_DIR)\externals\$(PACKAGE)	

clean:
	del *.obj 
	del *.osx

clear:
	del *.qqq

done:
	echo 1 > $(OSXNAME).qqq



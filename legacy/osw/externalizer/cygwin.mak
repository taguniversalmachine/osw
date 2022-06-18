
#Copyright (c) 2001 Amar Chaudhary. All rights reserved.
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

CC = g++
OLDC = gcc
STLOPTS = -D_STLP_USE_OWN_NAMESPACE -D_STLP_OWN_IOSTREAMS  -D_STLP_NO_NATIVE_WIDE_FUNCTIONS -D_REENTRANT -D_GNU_SOURCE
INCLUDES = -D_OSWOSX  -I$(OSWDIR)/include -I. -I$(OSWDIR)/types -I$(OSWDIR)/generic -I$(TCL_DIR)/include -I$(OSWDIR)/sdif -I$(STLDIR)/stlport
CFLAGS   = -O2 $(INCLUDES) $(STLOPTS)  -ftemplate-depth-64  -felide-constructors -w -D "_WINDOWS" -D "_WIN32"
DEBUGCFLAGS = $(INCLUDES) -g


OBJECTS  = $(OSXNAME).o $(USEROBJ)

LIBFLAGS = -shared -L$(TCL_DIR)/lib -L$(INSTALL_DIR) -Wl,--dll,-d,-E,--exclude-symbols,tcl82_NULL_THUNK_DATA:tk82_NULL_THUNK_DATA
LIBRARIES =  -ltcl82 -ltk82 -losw $(USERLIB) -lstlport_cygwin

all: $(OSXNAME).osx

$(OSXNAME).osx : $(OBJECTS)
	g++ -shared -o $(OSXNAME).osx $(LIBFLAGS) $(OBJECTS) $(LIBRARIES)


$(OSXNAME).o : $(OSXNAME).cpp $(DOT_H)
	$(CC) $(CFLAGS) -c $(OSXNAME).cpp

install: $(OSXNAME).osx
	cp *.osx $(INSTALL_DIR)/externals/$(PACKAGE)
	cp *.osd $(INSTALL_DIR)/externals/$(PACKAGE)

clean:
	rm *.o *.osx





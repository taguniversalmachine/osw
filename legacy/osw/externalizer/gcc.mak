
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

CC = g++
OLDC = gcc


INCLUDES = -D_OSWOSX  -I$(OSWDIR)/include -I. -I$(OSWDIR)/types -I$(OSWDIR)/generic $(TCL_INCLUDE_DIR) $(TK_INCLUDE_DIR) -I$(OSWDIR)/sdif 
MTFLAGS = -D_REENTRANT -pthread
CFLAGS   = -O2 $(INCLUDES) -DOSW_EXT_HASH_MAP -ftemplate-depth-64  -felide-constructors -w $(MTFLAGS) -g
DEBUGCFLAGS = $(INCLUDES) -g



OBJECTS  = $(OSXNAME).o $(USEROBJ)

LIBFLAGS = -shared -L$(INSTALL_DIR)
LIBRARIES =  $(OSWLIBS) -losw $(USERLIB)

all: $(OSXNAME).osx

$(OSXNAME).osx : $(OBJECTS)
	g++ -o $(OSXNAME).osx $(LIBFLAGS) $(OBJECTS) $(LIBRARIES)


$(OSXNAME).o : $(OSXNAME).cpp $(DOT_H)
	$(CC) $(CFLAGS) -c $(OSXNAME).cpp

install: $(OSXNAME).osx
	cp *.osx $(INSTALL_DIR)/externals/$(PACKAGE)
	cp *.osd $(INSTALL_DIR)/externals/$(PACKAGE)

clean:
	rm *.o *.osx





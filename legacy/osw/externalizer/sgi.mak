
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

CC = CC
OLDC = cc
STLOPTS = 
#-D_STLP_USE_OWN_NAMESPACE -D_STLP_OWN_IOSTREAMS  -D_STLP_NO_NATIVE_WIDE_FUNCTIONS -D_REENTRANT
INCLUDES = -D_OSWOSX  -I$(OSWDIR)/include -I. -I$(OSWDIR)/types  $(TCL_INCLUDE_DIR) -I$(OSWDIR)/sdif -I/usr/local/include -I/usr/include -I/usr/include/CC
CFLAGS   = -O2 $(INCLUDES) $(STLOPTS) -w -LANG:std  -n32 -OPT:Olimit=0  -ptused
DEBUGCFLAGS = $(INCLUDES) -g


OBJECTS  = $(OSXNAME).o $(USEROBJ)

LIBFLAGS = -n32 -shared  -L/usr/local/lib 
LIBRARIES =  -ltcl8.3 -ltk8.3 -losw $(USERLIB)

all: $(OSXNAME).osx

$(OSXNAME).osx : $(OBJECTS)
	CC -o $(OSXNAME).osx $(LIBFLAGS) $(OBJECTS) $(LIBRARIES)


$(OSXNAME).o : $(OSXNAME).cpp $(DOT_H)
	$(CC) $(CFLAGS) -c $(OSXNAME).cpp

install: $(OSXNAME).osx
	cp *.osx $(INSTALL_DIR)/externals/$(PACKAGE)
	cp *.osd $(INSTALL_DIR)/externals/$(PACKAGE)

clean:
	rm *.o *.osx





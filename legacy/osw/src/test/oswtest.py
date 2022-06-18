# ======================================================================
# file:		oswtest.py
# author:	Amar Chaudhary
# contents:	main module for OSW automated testing
# license:	OSW Public License (see License file)
# ======================================================================
# 
# ======================================================================
#Copyright (c) 2001-2004 Amar Chaudhary. All rights reserved.
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
#
#
#
# ======================================================================
# main module for OSW automated testing
# loads and execudes test cases
# ======================================================================

import Globals
import TestSupport
import sys, time, glob, unittest, getopt, os, re, string

opts,args = getopt.getopt(sys.argv[1:],[],['server=','gui=',
                                           'srcdir=','group=',
                                           'args='])

Globals.oswcmd = 'oswbin'
Globals.oswgui = 'osw'
Globals.srcdir = '..'
Globals.group = None
Globals.args = []

for (opt,value) in opts:
   if (opt == '--srcdir'):
      Globals.srcdir = value
   elif (opt == '--server'):
      Globals.oswcmd = value
   elif (opt == '--gui'):
      Globals.oswgui = value
   elif (opt == '--group'):
      Globals.group = value
   elif (opt == '--args'):
      Globals.args = string.split(value,',')
      
saveddir = os.getcwd()
if (len(sys.argv) > 0):
   if (os.path.dirname(sys.argv[0]) != ""):
      os.chdir(os.path.dirname(sys.argv[0]))


for t in (glob.glob('test-*.py')):
   if (Globals.group != None):
      if (re.search('\-'+Globals.group+'\-',t)):
         execfile(t)
      else:
         pass
   else:
      execfile(t)

unittest.TextTestRunner(verbosity=2).run(TestSupport.suite)

if (os.getcwd() != saveddir):
   os.chdir(saveddir)

   



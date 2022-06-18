# ======================================================================
# file:		test-2-externals.py
# author:	Amar Chaudhary
# contents:	tests for external instantiation
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
# this module tests instantiation of external transforms
# ======================================================================

import unittest
import TestSupport
import Globals
import time
import glob
import os


class TestExternals(unittest.TestCase):
        """Enumerate Built-in Transform Types"""

        def test_AAAStartServer(self):
           """Starting OSW Server"""
           TestSupport.startOSWServer('none')
           time.sleep(5)

        def test_zzzStopServer(self):
           """Stopping OSW Server"""
           TestSupport.stopOSW()
           
        def xtest(self,package,extern,i):
           """helper function for transform instantiation"""
           #print package+"::"+extern+"  "+str(i)
           xresult = TestSupport.runOSCCommand("/load-osx",[package+"/"+extern])
           xresult = TestSupport.runOSCCommand("/patch/add-transform",
                                               [package+"::"+extern,
                                                "tester"+str(i)])
           self.assertEqual(xresult[2],0)   


           

i = 0
for osd in (glob.glob(Globals.srcdir+'/externals/*/*/*.osd')):
        extern = None
        package = None
        (head,tail) = os.path.split(osd)
        (head,extern) = os.path.split(head)
        (head,package) = os.path.split(head)
        if (extern == "UDPSend" or extern == "UDPListen"):
                package = 'net'                
        if (package == 'win32'):
                continue
        elif (package == 'linux'):
                continue
        elif (package == 'osw'):
                doc = '"""Instantiating transform '+extern+'"""'        
        else:
                doc = '"""Instantiating transform '+package+"::"+extern+'"""'
        exec("def test_xtrn"+str(i)+
             "(self):\n\t"+doc+"\n\tself.xtest(\""
             +package+'","'+extern+'",'+str(i)+')')
        setattr(TestExternals,"test_x"+package+"_"+extern,
                eval("test_xtrn"+str(i)))
        i += 1



TestSupport.register(TestExternals)

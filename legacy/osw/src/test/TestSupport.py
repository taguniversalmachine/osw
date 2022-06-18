# ======================================================================
# file:		TestSupport.py
# author:	Amar Chaudhary
# contents:	support classes/functions for test cases
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
# this module provides several common functions/classes for use in
# test cases, including sending/receiving OSC messages and starting/stopping
# OSW in gui or server mode
# ======================================================================

import OSC
import socket
import DecodeOSC
import Globals
import os
import unittest
import urllib

outport = 7707
inport = 7708

buffersize = 1000

receiver = socket.socket(socket.SOCK_DGRAM, socket.AF_INET)
receiver.bind(('localhost',inport))
receiver.settimeout(5)

pid = 0

suite = unittest.TestSuite()

def register(testcase):
    suite.addTest(unittest.makeSuite(testcase,'test'))

def sendOSC(address,arguments):
    OSC.Message(address,arguments).sendlocal(outport)

def runOSCCommand(address,arguments):
    sendOSC(address,arguments)
    r = receiver.recvfrom(buffersize)
    return DecodeOSC.decodeOSC(r[0])

def startOSWServer(audio = None):   
    if audio == None:
      pid = os.spawnv(os.P_NOWAIT,Globals.oswcmd,
                      [Globals.oswcmd]+Globals.args)    
    else:
      pid = os.spawnv(os.P_NOWAIT,Globals.oswcmd,
                      [Globals.oswcmd]+Globals.args+["-dr",audio])

def startOSWGUI(audio = None):
    if audio == None:
      pid = os.spawnv(os.P_NOWAIT,Globals.oswgui,
                      [Globals.oswgui]+Globals.args)    
    else:
      pid = os.spawnv(os.P_NOWAIT,Globals.oswgui,
                      [Globals.oswgui]+Globals.args+["-dr",audio])
      
def stopOSW():
    sendOSC("/osw-quit",[])
    os.waitpid(pid,0)

def loadOSWPatch(patch):
    urllib.urlopen('http://localhost:7775/OSW_DIR/'+patch)
    


# ======================================================================
# file:		test-3-basic-audio.py
# author:	Amar Chaudhary
# contents:	tests for basic audio functionality
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
# this module provides test cases for basic audio functionality
# ======================================================================

import unittest
import TestSupport
import Globals
import time
import glob
import os


class TestBasicAudio(unittest.TestCase):
        """Test some basic audio functionality"""

        def errorTest(self,result,value=0):
           self.assertEqual(result[2],value,len(result) > 3 and result[3])

        def setUp(self):
           """Starting OSW Server"""
           TestSupport.startOSWServer()
           time.sleep(5)

        def tearDown(self):
           """Stopping OSW Server"""
           TestSupport.stopOSW()

        def test_1simple_output(self):
           """Simple White-Noise Output"""
           TestSupport.runOSCCommand('/load-osx',['osw/WhiteNoise'])
           result = TestSupport.runOSCCommand('/patch/add-transform',
                                              ['AudioOutput','out1','1','2'])
           self.errorTest(result)
           result = TestSupport.runOSCCommand('/patch/add-transform',
                                              ['osw::WhiteNoise','noise1'])
           self.errorTest(result)
           result = TestSupport.runOSCCommand('/patch/noise1/samplesOut/connect',
                                              ['/patch/out1/mix'])
           self.errorTest(result)
           result = TestSupport.runOSCCommand('/patch/out1/onoff',[True])
           self.errorTest(result,1)
           time.sleep(2)
           result = TestSupport.runOSCCommand('/patch/out1/onoff',[False])
           self.errorTest(result,1)

        def test_2scaled_output(self):
           """Scale volume of output"""
           TestSupport.runOSCCommand('/load-osx',['osw/WhiteNoise'])
           TestSupport.runOSCCommand('/load-osx',['osw/Gain'])
           result = TestSupport.runOSCCommand('/patch/add-transform',
                                              ['AudioOutput','out1','1','2'])
           self.errorTest(result)
           result = TestSupport.runOSCCommand('/patch/add-transform',
                                              ['osw::WhiteNoise','noise1'])
           self.errorTest(result)
           result = TestSupport.runOSCCommand('/patch/add-transform',
                                              ['osw::Gain','gain1'])
           self.errorTest(result)
           result = TestSupport.runOSCCommand('/patch/noise1/samplesOut/connect',
                                              ['/patch/gain1/samplesIn'])
           self.errorTest(result)
           result = TestSupport.runOSCCommand('/patch/gain1/samplesOut/connect',
                                              ['/patch/out1/mix'])
           self.errorTest(result)
           result = TestSupport.runOSCCommand('/patch/out1/onoff',[True])
           self.errorTest(result,1)
           for i in range(0,50):
                   result = TestSupport.runOSCCommand('/patch/gain1/amp_scale',
                                                      [i/50.0])
                   self.errorTest(result,1)
                   time.sleep(0.02)
           for i in range(50,0,-1):
                   result = TestSupport.runOSCCommand('/patch/gain1/amp_scale',
                                                      [i/50.0])
                   self.errorTest(result,1)                   
                   time.sleep(0.02)
                   
           result = TestSupport.runOSCCommand('/patch/out1/onoff',[False])
           self.errorTest(result,1)

        def test_3simple_output(self):
           """Various Oscillator Transforms"""
           TestSupport.runOSCCommand('/load-osx',['osw/WhiteNoise'])
           TestSupport.runOSCCommand('/load-osx',['osw/PinkNoise'])
           TestSupport.runOSCCommand('/load-osx',['osw/BrownNoise'])
           TestSupport.runOSCCommand('/load-osx',['osw/Sinewave'])
           TestSupport.runOSCCommand('/load-osx',['osw/Phasor'])
           TestSupport.runOSCCommand('/load-osx',['osw/Square'])
           TestSupport.runOSCCommand('/load-osx',['osw/Triangle'])
           result = TestSupport.runOSCCommand('/patch/add-transform',
                                              ['AudioOutput','out1','1','2'])
           self.errorTest(result)
           result = TestSupport.runOSCCommand('/patch/add-transform',
                                              ['osw::WhiteNoise','noise1'])
           self.errorTest(result)
           result = TestSupport.runOSCCommand('/patch/add-transform',
                                              ['osw::PinkNoise','pink1'])
           self.errorTest(result)
           result = TestSupport.runOSCCommand('/patch/add-transform',
                                              ['osw::BrownNoise','brown1'])
           self.errorTest(result)
           result = TestSupport.runOSCCommand('/patch/add-transform',
                                              ['osw::Sinewave','sine1'])
           self.errorTest(result)
           result = TestSupport.runOSCCommand('/patch/add-transform',
                                              ['osw::Square','square1'])
           self.errorTest(result)
           result = TestSupport.runOSCCommand('/patch/add-transform',
                                              ['osw::Triangle','triangle1'])
           self.errorTest(result)
           result = TestSupport.runOSCCommand('/patch/add-transform',
                                              ['osw::Phasor','phasor1'])
           self.errorTest(result)
           result = TestSupport.runOSCCommand('/patch/out1/onoff',[True])
           self.errorTest(result,1)
                   
           for source in ['sine1','phasor1','square1','triangle1',
                          'noise1','pink1','brown1']:
                   result = TestSupport.runOSCCommand('/patch/'+source+'/samplesOut/connect',
                                                      ['/patch/out1/mix'])
                   self.errorTest(result)
                   time.sleep(2)
                   result = TestSupport.runOSCCommand('/patch/'+source+'/samplesOut/disconnect',
                                                      ['/patch/out1/mix'])
                   self.errorTest(result)

           result = TestSupport.runOSCCommand('/patch/out1/onoff',[False])
           self.errorTest(result,1)                   


 



TestSupport.register(TestBasicAudio)



# ======================================================================
# file:		test-99-superbad.py
# author:	Amar Chaudhary
# contents:	large-scale tests using OSW demo and tutorial patches
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
# this module provides large-scale tests using OSW demo and tutorial patches
# ======================================================================

import unittest
import TestSupport
import time, random, math


class TestSuperbad(unittest.TestCase):
        """Test some basic audio functionality"""

        def errorTest(self,result,value=0):
           self.assertEqual(result[2],value,len(result) > 3 and result[3])

        def setUp(self):
           """Starting OSW Server"""
           TestSupport.startOSWGUI()
           time.sleep(5)

        def tearDown(self):
           """Stopping OSW Server"""
           TestSupport.stopOSW()

        def testBiquadExplore(self):
           """Biquads"""
           TestSupport.loadOSWPatch('tutorials/audio/biquad_explore.osw')
           time.sleep(5)
           result = TestSupport.runOSCCommand('/biquad_explore/out0/onoff',
                                               [True])
           for i in range(0,8):
                   a0 = random.random()
                   a1 = random.random() * 2 - 1
                   a2 = random.random() * 2 - 1
                   b1 = random.random() * 2 - 1
                   b2 = random.random() * 2 - 1
                   result = TestSupport.runOSCCommand('/biquad_explore/pack0/listOut',
                                                      [a0,a2,a2,b1,b2])
                   time.sleep(3)
           result = TestSupport.runOSCCommand('/biquad_explore/out0/onoff',
                                              [False])
           
        def testStepModulatedPulses(self):
           """Step-modulated Pulses"""
           TestSupport.loadOSWPatch('tutorials/audio/step_modulated_pulses.osw')
           time.sleep(8)
           result = TestSupport.runOSCCommand('/step_modulated_pulses/out0/onoff',
                                               [True])
           self.errorTest(result,1)
           time.sleep(6)
           result = TestSupport.runOSCCommand('/step_modulated_pulses/out0/onoff',
                                               [False])
           self.errorTest(result,1)
        

        def testPWM(self):
            """Pulse-width modulation test"""
            TestSupport.loadOSWPatch('help_patches/osw/help_Square.osw')
            time.sleep(8)
            result = TestSupport.runOSCCommand('/help_Square/out0/onoff',
                                               [True])
            result = TestSupport.runOSCCommand('/help_Square/floatbox0/floatIn',
                                               [256.0])
            self.errorTest(result,1)
            for i in range(1,100):
                result = TestSupport.runOSCCommand('/help_Square/square0/width',
                                                   [i * 0.01])
                self.errorTest(result,1)
                time.sleep(0.02)
                
            for i in range(100,1,-1):
                result = TestSupport.runOSCCommand('/help_Square/square0/width',
                                                   [i * 0.01])
                self.errorTest(result,1)
                time.sleep(0.02)                

            result = TestSupport.runOSCCommand('/help_Square/out0/onoff',
                                               [False])
            self.errorTest(result,1)

        def testLoop(self):
            """Sample Loop"""
            TestSupport.loadOSWPatch('tutorials/audio/looping.osw')
            time.sleep(5)            
            result = TestSupport.runOSCCommand('/looping/out0/onoff',
                                               [True])
            self.errorTest(result,1)
            time.sleep(2)
            for i in range(100,200):
                result = TestSupport.runOSCCommand('/looping/hslider0/floatIn',
                                                   [i * 0.01])
                self.errorTest(result,1)
                time.sleep(0.01)
            for i in range(200,1,-1):
                result = TestSupport.runOSCCommand('/looping/hslider0/floatIn',
                                                   [i * 0.01])
                self.errorTest(result,1)
                time.sleep(0.01)
            result = TestSupport.runOSCCommand('/looping/out0/onoff',
                                               [False])
            self.errorTest(result,1)

        def testNotes(self):
            """Notes/Score"""
            TestSupport.loadOSWPatch('tutorials/music/synth_score.osw')
            time.sleep(5)
            #add a Put transform to detect end-of-score condition
            result = TestSupport.runOSCCommand('/synth_score/add-transform',
                                               ['Put','put0','/done'])
            self.errorTest(result)
            result = TestSupport.runOSCCommand('/synth_score/playscore0/endScore/connect',
                                               ['/synth_score/put0/input'])
            self.errorTest(result)
            result = TestSupport.runOSCCommand('/synth_score/out0/onoff',
                                               [True])
            self.errorTest(result,1)
            result = TestSupport.runOSCCommand('/synth_score/notes0/unitIn',
                                               [None])
            self.errorTest(result,1)
            i = 0
            while True:
                result = TestSupport.runOSCCommand('/done/current-value',[])
                self.errorTest(result)
                if (result[3] == True):
                    break
                time.sleep(1)
                ++i
                if (i == 5):
                    self.failIf(True,"Timed out waiting for toggle")
            result = TestSupport.runOSCCommand('/synth_score/out0/onoff',
                                               [False])
            self.errorTest(result,1)


        def testAdditive(self):
            """Simple Additive Synthesis"""
            TestSupport.loadOSWPatch('tutorials/audio/additive.osw')
            time.sleep(8)
            result = TestSupport.runOSCCommand('/additive/out0/onoff',
                                               [True])
            self.errorTest(result,1)
            result = TestSupport.runOSCCommand('/additive/knob0/value',
                                               [0.14])
            self.errorTest(result,1)
            result = TestSupport.runOSCCommand('/additive/keyboard0/noteIn',
                                               [60])
            self.errorTest(result,1)
            for i in range(0,10):
                result = TestSupport.runOSCCommand('/additive/array1/'
                                                   +str(i)+'/floatIn',
                                                   [1.0])
                self.errorTest(result,1)
                time.sleep(1)
            result = TestSupport.runOSCCommand('/additive/out0/onoff',
                                               [False])
            self.errorTest(result,1)
            

        def testFM(self):
            """FM Synthesis"""
            TestSupport.loadOSWPatch('tutorials/audio/fm.osw')
            time.sleep(5)
            result = TestSupport.runOSCCommand('/fm/out0/onoff',
                                               [True])
            self.errorTest(result,1)
            for i in range(1,100):
                result = TestSupport.runOSCCommand('/fm/floatbox0/floatIn',
                                                   [i * 0.01])
                self.errorTest(result,1)
                time.sleep(0.01)
                
            for i in range(100,1,-1):
                result = TestSupport.runOSCCommand('/fm/floatbox0/floatIn',
                                                   [i * 0.01])
                time.sleep(0.01)
            result = TestSupport.runOSCCommand('/fm/knob0/value',[0.66])
            self.errorTest(result,1)
            time.sleep(0.01)
            result = TestSupport.runOSCCommand('/fm/keyboard0/noteIn',[60])
            self.errorTest(result,1)
            for i in range(1,100):
                result = TestSupport.runOSCCommand('/fm/floatbox0/floatIn',
                                                   [i * 0.01])
                self.errorTest(result,1)
                time.sleep(0.01)
                
            for i in range(100,1,-1):
                result = TestSupport.runOSCCommand('/fm/floatbox0/floatIn',
                                                   [i * 0.01])
                time.sleep(0.01)
            result = TestSupport.runOSCCommand('/fm/out0/onoff',
                                               [False])           
            self.errorTest(result,1)

        def testPhaseShifter(self):
            """Phase Shifter transform"""
            TestSupport.loadOSWPatch('help_patches/fx/help_PhaseShifter.osw')
            time.sleep(5)
            result = TestSupport.runOSCCommand('/help_PhaseShifter/out0/onoff',
                                               [True])
            self.errorTest(result,1)
            time.sleep(2)
            result = TestSupport.runOSCCommand('/help_PhaseShifter/floatbox1/floatIn',
                                               [0.5])
            self.errorTest(result,1)
            time.sleep(2)
            result = TestSupport.runOSCCommand('/help_PhaseShifter/floatbox2/floatIn',
                                               [1.0])
            self.errorTest(result,1)
            time.sleep(2)
            result = TestSupport.runOSCCommand('/help_PhaseShifter/floatbox0/floatIn',
                                               [0.99])
            self.errorTest(result,1)
            time.sleep(2)
            result = TestSupport.runOSCCommand('/help_PhaseShifter/out0/onoff',
                                               [False])
            self.errorTest(result,1)
            
        def testResPing(self):
            """Resonance-model Arpeggiator"""
            TestSupport.loadOSWPatch('demos/Resonances/Arpeggiator.osw')
            time.sleep(5)
            result = TestSupport.runOSCCommand('/Arpeggiator/out0/onoff',
                                               [True])
            self.errorTest(result,1)
            result = TestSupport.runOSCCommand('/Arpeggiator/toggle0/toggle',
                                               [None])
            self.errorTest(result,1)
            i = 0
            while True:
                result = TestSupport.runOSCCommand('/Arpeggiator/toggle0/toggleOut/current-value',[])
                self.errorTest(result)
                if (result[3] == False):
                    break
                time.sleep(1)
                ++i
                if (i == 10):
                    self.failIf(True,"Timed out waiting for toggle")

            result = TestSupport.runOSCCommand('/Arpeggiator/out0/onoff',
                                               [False])
            self.errorTest(result,1)
                
            
        def testSwissCheese(self):
            """bitwise-mask signal (aka \"Swiss Cheese\")"""
            TestSupport.loadOSWPatch('tutorials/audio/swiss_cheese.osw')
            time.sleep(8)
            result = TestSupport.runOSCCommand('/swiss_cheese/out0/onoff',
                                               [True])
            self.errorTest(result,1)
            for i in range(255,0,-1):
                result = TestSupport.runOSCCommand('/swiss_cheese/intbox0/intIn',
                                                   [i])
                self.errorTest(result,1)
                time.sleep(0.01)
            result = TestSupport.runOSCCommand('/swiss_cheese/out0/onoff',
                                               [False])
            self.errorTest(result,1)


        def testShepard(self):
            """Shepard Tones demo"""
            TestSupport.loadOSWPatch('demos/ShepardTones/ShepardTones.osw')
            time.sleep(10)
            result = TestSupport.runOSCCommand('/ShepardTones/out0/onoff',
                                               [True])
            self.errorTest(result,1)
            time.sleep(3.5)
            result = TestSupport.runOSCCommand('/ShepardTones/floatbox1/floatIn',
                                               [4.0])
            self.errorTest(result,1)
            time.sleep(5)
            result = TestSupport.runOSCCommand('/ShepardTones/floatbox0/floatIn',
                                               [0.6])
            self.errorTest(result,1)
            time.sleep(5)
            result = TestSupport.runOSCCommand('/ShepardTones/floatbox2/floatIn',
                                               [100.0])
            self.errorTest(result,1)
            time.sleep(5)
            
            result = TestSupport.runOSCCommand('/ShepardTones/out0/onoff',
                                               [False])
            self.errorTest(result,1)

        def testFilterSweep (self):
            """LFO Filter Sweep"""
            TestSupport.loadOSWPatch('tutorials/audio/lfo_filter_sweep.osw')
            time.sleep(5)
            result = TestSupport.runOSCCommand('/lfo_filter_sweep/out0/onoff',
                                               [True])
            self.errorTest(result,1)
            time.sleep(2)
            result = TestSupport.runOSCCommand('/lfo_filter_sweep/lfo0/shape',
                                               ['saw'])
            self.errorTest(result,1)
            result = TestSupport.runOSCCommand('/lfo_filter_sweep/floatbox2/floatIn',
                                               [2.0])
            self.errorTest(result,1)
            time.sleep(2)
            result = TestSupport.runOSCCommand('/lfo_filter_sweep/lfo0/shape',
                                               ['sine1_2'])
            self.errorTest(result,1)
            result = TestSupport.runOSCCommand('/lfo_filter_sweep/floatbox2/floatIn',
                                               [0.5])
            self.errorTest(result,1)
            time.sleep(2)
            result = TestSupport.runOSCCommand('/lfo_filter_sweep/floatbox1/floatIn',
                                               [8.0])
            self.errorTest(result,1)
            time.sleep(2)
            result = TestSupport.runOSCCommand('/lfo_filter_sweep/out0/onoff',
                                               [False])
            self.errorTest(result,1)
            

        def calcNotch (self,freq,bw,sr):
            """Calculate Notch Filter Coefficients"""
            omega = 2 * math.pi * freq / sr
            alpha = math.sin(omega) * math.sinh(math.log(2)/2*bw*omega/math.sin(omega))
            a1 = -2 * math.cos(omega)
            a0 = 1
            a2 = 1
            b0 = 1 + alpha
            b1 = -2 * math.cos(omega)
            b2 = 1 - alpha
            return [a0/b0,a1/b0,a2/b0,-b1/b0,-b2/b0]


        def testNotch (self):
            """Notch Filter"""
            TestSupport.loadOSWPatch('tutorials/audio/calc_notch.osw')
            time.sleep(5)
            result = TestSupport.runOSCCommand('/calc_notch/floatbox0/floatIn',
                                               [4000.0])
            self.errorTest(result,1)
            result = TestSupport.runOSCCommand('/calc_notch/floatbox1/floatIn',
                                               [0.50])
            self.errorTest(result,1)
            result = TestSupport.runOSCCommand('/calc_notch/expr3/out/current-value',[])
            self.errorTest(result)
            self.assertEqual(len(result),8)
            for (a,b) in zip(result[3:],self.calcNotch(4000.0,0.50,44100.0)):
                self.assertAlmostEqual(a,b,1)
            result = TestSupport.runOSCCommand('/calc_notch/out0/onoff',
                                               [True])
            time.sleep(1)
            result = TestSupport.runOSCCommand('/calc_notch/floatbox0/floatIn',
                                               [1000.0])
            self.errorTest(result,1)
            result = TestSupport.runOSCCommand('/calc_notch/floatbox1/floatIn',
                                               [0.08])
            self.errorTest(result,1)
            result = TestSupport.runOSCCommand('/calc_notch/expr3/out/current-value',[])
            self.errorTest(result)
            self.assertEqual(len(result),8)
            for (a,b) in zip(result[3:],self.calcNotch(1000.0,0.08,44100.0)):
                self.assertAlmostEqual(a,b,1)
            time.sleep(1)
            result = TestSupport.runOSCCommand('/calc_notch/floatbox0/floatIn',
                                               [10000.0])
            self.errorTest(result,1)
            result = TestSupport.runOSCCommand('/calc_notch/floatbox1/floatIn',
                                               [0.80])
            self.errorTest(result,1)
            result = TestSupport.runOSCCommand('/calc_notch/expr3/out/current-value',[])
            self.errorTest(result)
            self.assertEqual(len(result),8)
            for (a,b) in zip(result[3:],self.calcNotch(10000.0,0.80,44100.0)):
                self.assertAlmostEqual(a,b,1)
            time.sleep(1)
            result = TestSupport.runOSCCommand('/calc_notch/out0/onoff',
                                               [False])
            self.errorTest(result,1)

        def testFibThis(self):
            """Fibinocci Numbers using This Transform"""
            TestSupport.loadOSWPatch('tutorials/math/fib.osw')
            time.sleep(5)
            result = TestSupport.runOSCCommand('/fib/intbox1/intIn',
                                               [10])
            self.errorTest(result,1)
            #this one takes a while
            time.sleep(10)
            result = TestSupport.runOSCCommand('/fib/intbox0/intIn/current-value',[])
            self.errorTest(result)
            self.assertEqual(result[3],89)

        def testSpectralLimit(self):
            """Spectral Limit"""
            TestSupport.loadOSWPatch('help_patches/spectral/help_SpectralLimit.osw')
            time.sleep(5)
            result = TestSupport.runOSCCommand('/help_SpectralLimit/out0/onoff',
                                               [True])
            self.errorTest(result,1)
            time.sleep(1.5)
            result = TestSupport.runOSCCommand('/help_SpectralLimit/floatbox0/floatIn',[1000.0]);
            self.errorTest(result,1)
            result = TestSupport.runOSCCommand('/help_SpectralLimit/floatbox1/floatIn',[4000.0]);
            self.errorTest(result,1)
            time.sleep(1.5)
            result = TestSupport.runOSCCommand('/help_SpectralLimit/floatbox0/floatIn',[1500.0]);
            self.errorTest(result,1)
            result = TestSupport.runOSCCommand('/help_SpectralLimit/floatbox1/floatIn',[2000.0]);
            self.errorTest(result,1)
            time.sleep(1.5)
            result = TestSupport.runOSCCommand('/help_SpectralLimit/floatbox0/floatIn',[1550.0]);
            self.errorTest(result,1)
            result = TestSupport.runOSCCommand('/help_SpectralLimit/floatbox1/floatIn',[1570.0]);
            self.errorTest(result,1)
            time.sleep(1.5)
            result = TestSupport.runOSCCommand('/help_SpectralLimit/out0/onoff',
                                               [False])
            self.errorTest(result,1)

        def testInharmonicity(self):
            """Sinusoidal-model Inharmonicity"""
            TestSupport.loadOSWPatch('help_patches/cnmat/help_Inharmonicity.osw')
            time.sleep(6)
            result = TestSupport.runOSCCommand('/help_Inharmonicity/out0/onoff',
                                               [True])
            self.errorTest(result,1)
            result = TestSupport.runOSCCommand('/help_Inharmonicity/tm0/rate',
                                               [0.1])
            self.errorTest(result,1)
            result = TestSupport.runOSCCommand('/help_Inharmonicity/tm0/zero',
                                               [None])
            self.errorTest(result,1)
            time.sleep(0.4)
            for i in range(0,100):
                result = TestSupport.runOSCCommand('/help_Inharmonicity/hslider0/floatIn',
                                                   [float(i)])
                self.errorTest(result,1)
                time.sleep(0.1)
            time.sleep(0.4)
            result = TestSupport.runOSCCommand('/help_Inharmonicity/out0/onoff',
                                               [False])
            self.errorTest(result,1)            

        def testScaleResonances(self):
            """ScaleResonances"""
            TestSupport.loadOSWPatch('help_patches/cnmat/help_ScaleResonances.osw')
            time.sleep(6)
            result = TestSupport.runOSCCommand('/help_ScaleResonances/out0/onoff',
                                               [True])
            self.errorTest(result,1)
            result = TestSupport.runOSCCommand('/help_ScaleResonances/knob0/value',
                                               [1.0])
            self.errorTest(result,1)
            time.sleep(2)            
            result = TestSupport.runOSCCommand('/help_ScaleResonances/vslider0/floatIn',
                                               [0.5])
            self.errorTest(result,1)
            time.sleep(1)
            result = TestSupport.runOSCCommand('/help_ScaleResonances/vslider0/floatIn',
                                               [3.0])
            self.errorTest(result,1)
            time.sleep(1)
            result = TestSupport.runOSCCommand('/help_ScaleResonances/vslider0/floatIn',
                                               [1.0])
            self.errorTest(result,1)
            result = TestSupport.runOSCCommand('/help_ScaleResonances/vslider2/floatIn',
                                               [0.5])
            self.errorTest(result,1)
            time.sleep(1)
            result = TestSupport.runOSCCommand('/help_ScaleResonances/vslider2/floatIn',
                                               [4.0])
            self.errorTest(result,1)
            time.sleep(1)
            result = TestSupport.runOSCCommand('/help_ScaleResonances/vslider2/floatIn',
                                               [1.0])
            self.errorTest(result,1)
            time.sleep(1)
            result = TestSupport.runOSCCommand('/help_ScaleResonances/vslider1/floatIn',
                                               [0.5])
            self.errorTest(result,1)                        
            time.sleep(1)
            result = TestSupport.runOSCCommand('/help_ScaleResonances/vslider1/floatIn',
                                               [0.0])
            self.errorTest(result,1)                        
            time.sleep(0.5)
            result = TestSupport.runOSCCommand('/help_ScaleResonances/out0/onoff',
                                               [False])
            self.errorTest(result,1)
            
TestSupport.register(TestSuperbad)






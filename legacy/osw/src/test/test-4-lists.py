# ======================================================================
# file:		test-4-lists.py
# author:	Amar Chaudhary
# contents:	tests for OSW lists
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
# this module provides test cases for OSW lists
# ======================================================================

import unittest
import TestSupport



class TestListPatches(unittest.TestCase):

    def errorTest(self,result,value=0):
        self.assertEqual(result[2],value,len(result) > 3 and result[3])
            
    def test_AAAStartServer(self):
        """Starting OSW Server"""
        TestSupport.startOSWGUI('none')
        time.sleep(5)

    def test_zzzStopServer(self):
        """Stopping OSW Server"""
        TestSupport.stopOSW()
           
    def test_duplicate(self):
        """Tutorial: remove duplicate elements from list"""
        TestSupport.loadOSWPatch('tutorials/list/duplicate_elements.osw')
        time.sleep(5)
        result = TestSupport.runOSCCommand('/duplicate_elements/msgbox0/message/current-value',[])
        self.errorTest(result)
        self.assertEqual(result[3],"1 2 3 3 4 5 5 5 6")
        result = TestSupport.runOSCCommand('/duplicate_elements/msgbox0/sendInternal',[None])
        self.errorTest(result,1)
        result = TestSupport.runOSCCommand('/duplicate_elements/msgbox1/message/current-value',[])
        self.errorTest(result)
        self.assertEqual(result[3],"1 2 3 4 5 6")

    def test_cons_expr(self):
        """Constructing Lists from Expressions"""
        TestSupport.loadOSWPatch('tutorials/list/list_cons_expr.osw')
        time.sleep(5)
        for i in range(1,5):
            result = TestSupport.runOSCCommand('/list_cons_expr/intbox0/intIn',
                                               [i])
            self.errorTest(result,1)
            result = TestSupport.runOSCCommand('/list_cons_expr/msgbox0/messageOut/current-value',[])
            self.errorTest(result)
            self.assertEqual(result[3],i)
            self.assertEqual(result[4],2*i)
            self.assertEqual(result[5],2+i)

    def test_series_approx_general(self):
        """Generalized Series Approximation"""
        TestSupport.loadOSWPatch('tutorials/list/series_approx_general.osw')
        time.sleep(5)
        for i in [2,3,4,5,7,87]:
            result = TestSupport.runOSCCommand('/series_approx_general/floatbox1/floatIn',
                                               [float(i)])
            self.errorTest(result,1)
            result = TestSupport.runOSCCommand('/series_approx_general/floatbox2/floatIn/current-value',[])
            self.errorTest(result)
            closedForm = 1.0/(1-1.0/i)
            self.assertAlmostEqual(result[3],closedForm,6)
            result = TestSupport.runOSCCommand('/series_approx_general/intbox0/intIn',
                                               [0])
            self.errorTest(result,1)
            result = TestSupport.runOSCCommand('/series_approx_general/range0/send',
                                               [None])
            self.errorTest(result,1)
            result = TestSupport.runOSCCommand('/series_approx_general/floatbox3/floatIn/current-value',[])
            self.errorTest(result)
            self.assertAlmostEqual(result[3],closedForm-1,6)
            prev = closedForm - 1
            for j in [5,10,15,20]:
                result = TestSupport.runOSCCommand('/series_approx_general/intbox0/intIn',
                                                   [j])
                self.errorTest(result,1)
                result = TestSupport.runOSCCommand('/series_approx_general/range0/send',
                                                   [None])
                self.errorTest(result,1)
                result = TestSupport.runOSCCommand('/series_approx_general/floatbox3/floatIn/current-value',[])
                self.errorTest(result)
                self.failUnless(result[3] <= prev)
                prev = result[3]
        
                                             
class TestListServer(unittest.TestCase):
    def errorTest(self,result,value=0):
        self.assertEqual(result[2],value,len(result) > 3 and result[3])

    def test_AAAStartServer(self):
        """Starting OSW Server"""
        TestSupport.startOSWServer('none')
        time.sleep(5)

    def test_zzzStopServer(self):
        """Stopping OSW Server"""
        TestSupport.stopOSW()

    def test_Shuffle(self):
        """Shuffle"""
        TestSupport.runOSCCommand('/load-osx',['list/Shuffle'])
        result = TestSupport.runOSCCommand('/patch/add-transform',
                                           ['list::Shuffle','shuffle1'])
        self.errorTest(result)
        l = [1,2,2.718281728459,3,3.14159265358979323846,4,5]
        result = TestSupport.runOSCCommand('/patch/shuffle1/listIn',l)
        self.errorTest(result,len(l))
        result = TestSupport.runOSCCommand('/patch/shuffle1/listOut/current-value',[])
        self.errorTest(result)
        self.assertEqual(len(result),len(l)+3)
        r1 = result[3:]
        r1.sort()        
        for (a,b) in zip(r1,l):
            self.assertAlmostEqual(a,b,6)

        l = ['apple','pear',[1,2,3],'banana']
        result = TestSupport.runOSCCommand('/patch/shuffle1/listIn',l)
        self.errorTest(result,len(l))
        result = TestSupport.runOSCCommand('/patch/shuffle1/listOut/current-value',[])
        self.errorTest(result)
        self.assertEqual(len(result),len(l)+3)


TestSupport.register(TestListServer)
TestSupport.register(TestListPatches)




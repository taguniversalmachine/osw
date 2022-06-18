# ======================================================================
# file:		test-5-math.py
# author:	Amar Chaudhary
# contents:	tests for mathematical-function transforms
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
# this module provides test cases for mathematical-function transforms
# ======================================================================


import unittest
import TestSupport
import time
import math, cmath
import types

def asinh(x):
   if (type(x) == types.ComplexType):
           return -cmath.log(cmath.sqrt(1 + x * x)-x)
   else:
           return -math.log(math.sqrt(1 + x * x)-x)

def acosh(x):
   return asinh(cmath.sqrt(x * x - 1))


#add more builtins to this list as they are added to the core system
#(or remove if obsolete or made external)
mathfuncs = [
        ("Abs",[(-1,1),(1.5,1.5),(0,0),(1j,1),(3+4j,5)]),
        ("Acos",[(0.0,math.pi/2),(1.0,0),(3j,cmath.acos(3j))]),
        ("Acosh",[(1.0,0),(3.0,1.7627471740390872),(3j,-cmath.acos(3j)*1j)]),
        ("Acot",[(0.0,math.pi/2),(1.0,math.pi/4)]),
        ("Acoth",[(1/math.tanh(3),3)]),
        ("Acsc",[(1.0,math.pi/2),(3.0,math.asin(1.0/3)),(3j,cmath.asin(1/3j))]),
#        ("Acsch",[(1/math.sinh(3),3)]),
        ("Angle",[(4j,math.pi/2),(1+1j,math.pi/4)]),
        ("Asec",[(1.0,0),(3.0,math.acos(1.0/3)),(3j,cmath.acos(1/3j))]),
        ("Asech",[(1.0,0),(1/3.0,acosh(3.0).real),(3j,acosh(1/3j))]),
        ("Asin",[(1.0,math.pi/2),(0.0,0),(3j,cmath.asin(3j))]),
        ("Asinh",[(0.0,0.0),(3.0,asinh(3.0)),(3j,asinh(3j))]),
        ("Atan",[(0.0,0.0),(1.0,math.pi/4)]),
        ("Atanh",[(0.0,0),(math.tanh(3),3),(cmath.tanh(1/3j),1/3j)]),
        ("Ceil",[(1.33333,2.0),(-1.33333,-1.0)]),
        ("Conj",[(1+2j,1-2j),(-3.1j,3.1)]),
        ("Cos",[(0.0,1),(math.pi/2,0), (3j,cmath.cos(3j))]),
        ("Cosh",[(0.0,1),(3.0,math.cosh(3.0)),(3j,cmath.cosh(3j))]),
        ("Cot",[(math.pi/2,0),(math.pi/4,1),(3j,1/cmath.tan(3j))]),
        ("Coth",[(3.0,1/math.tanh(3)),(3j,1/cmath.tanh(3j))]),
        ("Csc",[(math.pi/2,1),(3.0,1/math.sin(3)),(3j,1/cmath.sin(3j))]),
        ("Csch",[(3.0,1/math.sinh(3)),(3j,1/cmath.sinh(3j))]),
        ("Exp",[(0.0,1.0),(1.0,2.718281828459),(-1j*math.pi,-1)]),
        ("Fact",[(6,720),(7,5040),(0,1)]),
        ("Floor",[(1.33333,1.0),(-1.33333,-2.0)]),
        ("Frac",[(1.33333,0.33333),(-1.3,0.7),(1.0,0)]),
        ("Gamma",[(3.0,2.0),(0.5,math.sqrt(math.pi))]),
        ("Imag",[(-3.5j,-3.5),(4+4j,4)]),
        ("Log",[(math.e,1),(1.0,0),(-1j,-0.5j*math.pi)]),
        ("Log10",[(100.0,2),(1.0,0),(-1+0j,math.pi/math.log(10)*1j)]),
        ("Norm",[(1+1j,2),(3+4j,25)]),
        ("Real",[(3.5j,0),(4+4j,4)]),
        ("Sec",[(0.0,1),(3.0,1/math.cos(3)),(3j,1/cmath.cos(3j))]),
        ("Sech",[(0.0,1),(3.0,1/math.cosh(3)),(3j,1/cmath.cosh(3j))]),
        ("Sgn",[(-666,-1),(3.14159,1),(0,0)]),
        ("Sin",[(0.0,0),(5*math.pi/2,1),(3j,cmath.sin(3j))]),
        ("Sinc",[(0.0,1),(3.0,math.sin(3)/3),(3j,cmath.sin(3j)/3j)]),
        ("Sinh",[(0.0,0),(3.0,math.sinh(3)),
                 (-3.0,-math.sinh(3)),(3j,cmath.sinh(3j))]),
        ("Sinhc",[(0.0,1),(3.0,math.sinh(3)/3),(3j,cmath.sinh(3j)/3j)]),
        ("Sqrt",[(1.0,1.0),(2.0,math.sqrt(2)),(-100+0j,10j)]),
        ("Tan",[(0.0,0),(math.pi/4,1),(3j,cmath.tan(3j))]),
        ("Tanh",[(0.0,0),(3.0,math.tanh(3)),(3j,cmath.tanh(3j))])
        ]

class TestMath(unittest.TestCase):
        """Test Mathematical-Function Transforms"""


        def errorTest(self,result,value=0):
                self.assertEqual(result[2],value,
                                 len(result) > 3 and result[3])

        def test_AAAStartServer(self):
           """Starting OSW Server"""
           TestSupport.startOSWServer('none')
           time.sleep(5)

        def test_zzzStopServer(self):
           """Stopping OSW Server"""
           TestSupport.stopOSW()
           

        def btest(self,b,l,i):
           """helper function for transform instantiation"""
           #print b+"  "+str(i)
           name = "tester"+str(i)
           var =  "var"+str(i)
           get = "get+"+str(i)
           result = TestSupport.runOSCCommand("/patch/add-transform",[b,name])
           self.errorTest(result)
           result = TestSupport.runOSCCommand("/patch/add-transform",
                                              ['Get',get,var])
           self.errorTest(result)
           result = TestSupport.runOSCCommand("/patch/"+get+'/value/connect',
                                              ['/patch/'+name+'/in']);
           self.errorTest(result)           
           for (arg,expected) in l:
                   result = TestSupport.runOSCCommand("/patch/"+var,
                                                      [arg])
                   self.errorTest(result,1)
                   result = TestSupport.runOSCCommand("/patch/"+name+'/out/current-value',[])
                   #print result
                   self.errorTest(result)
                   if (type(result[3]) == types.ComplexType):
                           self.assertAlmostEqual(abs(result[3]),
                                                  abs(expected),5,
                                                  "Expected "+str(expected)+
                                                  " for "+
                                                  b+'('+str(arg)+') but got '+
                                                  str(result[3]))
                   else:
                           self.assertAlmostEqual(result[3],expected,5,
                                                  "Expected "+str(expected)+
                                                  " for "+
                                                  b+'('+str(arg)+') but got '+
                                                  str(result[3]))

           
i = 0
for b in mathfuncs:
        t = b[0];
        l = b[1];
        doc = '"""Testing '+t+'"""'
        if (len(l) == 0):
             continue
        exec("def test_builtin"+str(i)+"(self):\n\t"+
             doc+'\n\tself.btest("'+t+'",'+str(l)+','+str(i)+")")
        setattr(TestMath,"test_b"+t,eval("test_builtin"+str(i)))
        i += 1



TestSupport.register(TestMath)

/* 
Copyright (c) 2001-2004 Amar Chaudhary. All rights reserved.
Copyright (c) 1998-2001 Regents of the University of California.
All rights reserved.

     ``The contents of this file are subject to the Open Sound World Public
     License Version 1.0 (the "License"); you may not use this file except in
     compliance with the License. A copy of the License should be included
     in a file named "License" or "License.txt" in the distribution from 
     which you obtained this file. 

     Software distributed under the License is distributed on an "AS IS"
     basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
     License for the specific language governing rights and limitations
     under the License.

     The Original Code is Open Sound World (OSW) software.

     The Initial Developer of the Original Code is Amar Chaudhary.
     Portions created by Amar Chaudhary are Copyright (C) 1998-2001 
     Regents of the University of California and Amar Chaudhary. 
     All Rights Reserved.

     Contributor(s):

  
From the UC Regents:

Permission to use, copy, modify, and distribute this software and its
documentation, without fee and without a signed licensing agreement, is hereby
granted, provided that the above copyright notice, this paragraph and the
following two paragraphs appear in all copies, modifications, and
distributions.  Contact The Office of Technology Licensing, UC Berkeley, 2150
Shattuck Avenue, Suite 510, Berkeley, CA 94720-1620, (510) 643-7201, for
commercial licensing opportunities.

Written by Amar Chaudhary, The Center for New Music and Audio Technologies, 
University of California, Berkeley.

     IN NO EVENT SHALL REGENTS BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT,
     SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST
     PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS
     DOCUMENTATION, EVEN IF REGENTS HAS BEEN ADVISED OF THE POSSIBILITY OF
     SUCH DAMAGE.

     REGENTS SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT
     LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
     FOR A PARTICULAR PURPOSE. THE SOFTWARE AND ACCOMPANYING
     DOCUMENTATION, IF ANY, PROVIDED HEREUNDER IS PROVIDED "AS IS".
     REGENTS HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES,
     ENHANCEMENTS, OR MODIFICATIONS.
     
*/

/*
  oswMacOSXAnnoyingInit.cpp
  Forced Initialization to compensate for deficiencies in Darwin gcc

  Amar Chaudhary
*/

#include "osw.h"

namespace osw {


  bool AnnoyingInit () {
    OSW_USE_DECLARE_USE(FanInFactory);
    OSW_USE_DECLARE_USE(GateFactory);
    OSW_USE_DECLARE_USE(ArrayFactory);
    OSW_USE_DECLARE_USE(BundleManyFactory);
    OSW_USE_DECLARE_USE(UnbundleFactory);
    OSW_USE_DECLARE_USE(Add2AFactory);
    OSW_USE_DECLARE_USE(Subtract2AFactory);
    OSW_USE_DECLARE_USE(Multiply2AFactory);
    OSW_USE_DECLARE_USE(Divide2AFactory);
    OSW_USE_DECLARE_USE(Modulus2AFactory);
    OSW_USE_DECLARE_USE(Less2AFactory);
    OSW_USE_DECLARE_USE(Greater2AFactory);
    OSW_USE_DECLARE_USE(Lteq2AFactory);
    OSW_USE_DECLARE_USE(Gteq2AFactory);
    OSW_USE_DECLARE_USE(Equal2AFactory);
    OSW_USE_DECLARE_USE(NotEqual2AFactory);
    OSW_USE_DECLARE_USE(LogOr2AFactory);
    OSW_USE_DECLARE_USE(LogAnd2AFactory);
    OSW_USE_DECLARE_USE(ExponentFactory);
    OSW_USE_DECLARE_USE(RShiftFactory);
    OSW_USE_DECLARE_USE(LShiftFactory);
    OSW_USE_DECLARE_USE(BitAndFactory);
    OSW_USE_DECLARE_USE(BitOrFactory);
    OSW_USE_DECLARE_USE(BitXorFactory);
    OSW_USE_DECLARE_USE(TableTransformFactory);
    OSW_USE_DECLARE_USE(InternalPutTransformFactory);
    OSW_USE_DECLARE_USE(InternalGetTransformFactory);
    OSW_USE_DECLARE_USE(GlobTransformFactory);
    OSW_USE_DECLARE_USE(ExprFactory);
    OSW_USE_DECLARE_USE(IfFactory);
    OSW_USE_DECLARE_USE(ThisTransformFactory);
    OSW_USE_DECLARE_USE(OSCBundleFactory);
    OSW_USE_DECLARE_USE(OSCSendFactory);

    // Various static initializers associated w/ polymorphic binary operators

    OSW_USE_DECLARE_USE(DCB6);
    OSW_USE_DECLARE_USE(DCB7);
    OSW_USE_DECLARE_USE(DCB10);
    OSW_USE_DECLARE_USE(DCB11);
    OSW_USE_DECLARE_USE(DCB14);
    OSW_USE_DECLARE_USE(DCM1);
    OSW_USE_DECLARE_USE(DCM2);
    OSW_USE_DECLARE_USE(DCM3);
    OSW_USE_DECLARE_USE(DCM4);
    OSW_USE_DECLARE_USE(DCM5);
    OSW_USE_DECLARE_USE(DCM6);
    OSW_USE_DECLARE_USE(DCL1);
    OSW_USE_DECLARE_USE(DCL2);
    OSW_USE_DECLARE_USE(DCL3);
    OSW_USE_DECLARE_USE(DCL4);
    OSW_USE_DECLARE_USE(DCS1);
    OSW_USE_DECLARE_USE(DCS2);
    OSW_USE_DECLARE_USE(DCS3);
    OSW_USE_DECLARE_USE(DCS4);
    OSW_USE_DECLARE_USE(DCS5);
    OSW_USE_DECLARE_USE(DCS6);
    OSW_USE_DECLARE_USE(DCS7);
    OSW_USE_DECLARE_USE(DCR6);
    OSW_USE_DECLARE_USE(DCR7);
    OSW_USE_DECLARE_USE(DCR10);
    OSW_USE_DECLARE_USE(DCBIT1);
    OSW_USE_DECLARE_USE(DCBIT2);
    OSW_USE_DECLARE_USE(DCBIT3);
    OSW_USE_DECLARE_USE(DCBIT4);
    OSW_USE_DECLARE_USE(DCBIT5);
    OSW_USE_DECLARE_USE(DCBIT6);
    OSW_USE_DECLARE_USE(DSB7);
    OSW_USE_DECLARE_USE(DSB8);
    OSW_USE_DECLARE_USE(DSB9);
    OSW_USE_DECLARE_USE(DSB10);
    OSW_USE_DECLARE_USE(DSB11);
    OSW_USE_DECLARE_USE(DSB12);
    OSW_USE_DECLARE_USE(DVB1);
    OSW_USE_DECLARE_USE(DVB2);
    OSW_USE_DECLARE_USE(DVB3);
    OSW_USE_DECLARE_USE(DVB4);
    OSW_USE_DECLARE_USE(DVB5);
    OSW_USE_DECLARE_USE(DVB6);


    return true;
  }
}

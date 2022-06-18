
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

# generic make dependencies

REQUIRED_H = ../include/osw.h ../include/oswCfg.h ../include/oswTypes.h \
	../include/oswCrap.h ../include/oswVect.h ../include/oswMutex.h \
	../include/oswExcept.h ../include/oswDatum.h ../include/oswNameable.h \
	../include/oswContainer.h ../include/oswState.h ../include/oswScan.h \
	../include/oswTransform.h ../include/oswInletOutlet.h ../include/oswAny.h \
	../include/oswActivation.h ../include/oswPatch.h ../include/oswRaw.h \
	../include/oswTypeRegistry.h ../include/oswEnumeratedString.h

BINOPS = oswBinop.$(OBJ) oswBinop2.$(OBJ) oswBinop3.$(OBJ) oswBinop3a.$(OBJ) oswBinop3b.$(OBJ) oswBinop4.$(OBJ) oswBinop4a.$(OBJ) oswBinop5.$(OBJ)

OBJECTS = base64.$(OBJ) \
	osw.$(OBJ) \
        oswMisc.$(OBJ) \
	oswMem.$(OBJ) \
	oswNameable.$(OBJ) \
	oswContainer.$(OBJ) \
	oswState.$(OBJ) \
	oswFreeState.$(OBJ) \
	oswTransform.$(OBJ) \
	oswInletOutlet.$(OBJ) \
	oswActivation.$(OBJ) \
	oswPatch.$(OBJ) \
	oswTypeRegistry.$(OBJ) \
	oswScan.$(OBJ) \
	oswCast.$(OBJ) \
        oswDataSource.$(OBJ) \
	oswInit.$(OBJ) \
	oswCleanup.$(OBJ) \
	oswArgs.$(OBJ) \
        oswAny.$(OBJ) \
	oswFanout.$(OBJ) \
	oswFanin.$(OBJ) \
	oswClock.$(OBJ) \
	oswAudio.$(OBJ) \
	oswSched.$(OBJ) \
	oswThread.$(OBJ) \
	oswTime.$(OBJ) \
	oswTable.$(OBJ) \
	oswMidi.$(OBJ) \
	oswDelete.$(OBJ) \
	oswInletOutletTransform.$(OBJ) \
	oswMath.$(OBJ) \
	oswLex.$(OBJ) \
	$(BINOPS) \
	oswGetPut.$(OBJ) \
	oswList.$(OBJ) \
        oswEnumeratedString.$(OBJ) \
	oswBundle.$(OBJ) \
	oswArray.$(OBJ) \
	oswSdif.$(OBJ) \
	oswTcl.$(OBJ) \
	oswTkDraw.$(OBJ) \
	oswExpr.$(OBJ) \
	oswParseExpr.$(OBJ) \
	oswBlob.$(OBJ) \
	oswNote.$(OBJ) \
	oswSpectrum.$(OBJ) \
	oswPatternMatch.$(OBJ) \
	oswProfile.$(OBJ) \
	oswOSCClient.$(OBJ) \
	oswOSCTypes.$(OBJ) \
	oswOSCServer.$(OBJ) \
	oswInterface.$(OBJ) \
        oswThis.$(OBJ) 

SDIF_OBJECTS = sdif.$(OBJ) sdif-mem.$(OBJ) sdif-types.$(OBJ) 

OSC_OBJECTS = OSC-pattern-match.$(OBJ) \
	OSC-system-dependent.$(OBJ) \
	OSC-client.$(OBJ) \
	OSC-timetag.$(OBJ) 

##########

base64.$(OBJ) : ../generic/base64.cpp ../include/base64.h
	$(CXX) $(CXXFLAGS) -c ../generic/base64.cpp

oswMem.$(OBJ) : ../generic/oswMem.cpp ../include/oswMem.h
	$(CXX) $(CXXFLAGS) -c ../generic/oswMem.cpp

oswMisc.$(OBJ) : ../generic/oswMisc.cpp ../include/oswMisc.h
	$(CXX) $(CXXFLAGS) -c ../generic/oswMisc.cpp

osw.$(OBJ) : ../generic/osw.cpp $(REQUIRED_H)
	$(CXX) $(CXXFLAGS) -c ../generic/osw.cpp

oswNameable.$(OBJ) : ../generic/oswNameable.cpp $(REQUIRED_H)
	$(CXX) $(CXXFLAGS) -c ../generic/oswNameable.cpp

oswContainer.$(OBJ) : ../generic/oswContainer.cpp $(REQUIRED_H)
	$(CXX) $(CXXFLAGS) -c ../generic/oswContainer.cpp

oswState.$(OBJ) : ../generic/oswState.cpp $(REQUIRED_H)
	$(CXX) $(CXXFLAGS) -c ../generic/oswState.cpp

oswFreeState.$(OBJ) : ../generic/oswFreeState.cpp $(REQUIRED_H) \
	../include/oswFreeState.h
	$(CXX) $(CXXFLAGS) -c ../generic/oswFreeState.cpp

oswTransform.$(OBJ) : ../generic/oswTransform.cpp $(REQUIRED_H)
	$(CXX) $(CXXFLAGS) -c ../generic/oswTransform.cpp

oswInletOutlet.$(OBJ) : ../generic/oswInletOutlet.cpp $(REQUIRED_H)
	$(CXX) $(CXXFLAGS) -c ../generic/oswInletOutlet.cpp

oswActivation.$(OBJ) : ../generic/oswActivation.cpp $(REQUIRED_H)
	$(CXX) $(CXXFLAGS) -c ../generic/oswActivation.cpp

oswPatch.$(OBJ) : ../generic/oswPatch.cpp $(REQUIRED_H)
	$(CXX) $(CXXFLAGS) -c ../generic/oswPatch.cpp

oswTypeRegistry.$(OBJ) : ../generic/oswTypeRegistry.cpp $(REQUIRED_H)
	$(CXX) $(CXXFLAGS) -c ../generic/oswTypeRegistry.cpp

oswScan.$(OBJ) : ../generic/oswScan.cpp $(REQUIRED_H)
	$(CXX) $(CXXFLAGS) -c ../generic/oswScan.cpp

oswCast.$(OBJ) : ../generic/oswCast.cpp $(REQUIRED_H) ../include/oswCast.h
	$(CXX) $(CXXFLAGS) -c ../generic/oswCast.cpp

oswDataSource.$(OBJ) : ../generic/oswDataSource.cpp $(REQUIRED_H) \
	 ../include/oswDataSource.h
	$(CXX) $(CXXFLAGS) -c ../generic/oswDataSource.cpp

oswInit.$(OBJ) : ../generic/oswInit.cpp ../include/oswInit.h $(REQUIRED_H) 
	$(CXX) $(CXXFLAGS) -c ../generic/oswInit.cpp

oswArgs.$(OBJ) : ../generic/oswArgs.cpp ../include/oswArgs.h $(REQUIRED_H) 
	$(CXX) $(CXXFLAGS) -c ../generic/oswArgs.cpp

oswAny.$(OBJ) : ../generic/oswAny.cpp ../include/oswList.h $(REQUIRED_H) 
	$(CXX) $(CXXFLAGS) -c ../generic/oswAny.cpp

oswFanout.$(OBJ) : ../generic/oswFanout.cpp $(REQUIRED_H) 
	$(CXX) $(CXXFLAGS) -c ../generic/oswFanout.cpp

oswFanin.$(OBJ) : ../generic/oswFanin.cpp $(REQUIRED_H)
	$(CXX) $(CXXFLAGS) -c ../generic/oswFanin.cpp

oswClock.$(OBJ) : ../generic/oswClock.cpp $(REQUIRED_H) \
	 ../include/oswClock.h ../include/oswTime.h
	$(CXX) $(CXXFLAGS) -c ../generic/oswClock.cpp

oswAudio.$(OBJ) : ../generic/oswAudio.cpp $(REQUIRED_H) ../include/oswAudio.h \
	../include/oswTime.h ../include/oswClock.h
	$(CXX) $(CXXFLAGS) -c ../generic/oswAudio.cpp


oswSched.$(OBJ) : ../generic/oswSched.cpp $(REQUIRED_H) ../include/oswSched.h \
	 ../include/oswTime.h
	$(CXX) $(CXXFLAGS) -c ../generic/oswSched.cpp

oswThread.$(OBJ) : ../generic/oswThread.cpp $(REQUIRED_H) \
	 ../include/oswSched.h ../include/oswThread.h
	$(CXX) $(CXXFLAGS) -c ../generic/oswThread.cpp

oswTcl.$(OBJ) : ../generic/oswTcl.cpp $(REQUIRED_H) ../include/oswTcl.h
	$(CXX) $(CXXFLAGS) -c ../generic/oswTcl.cpp

oswTime.$(OBJ) : ../generic/oswTime.cpp $(REQUIRED_H)  ../include/oswTime.h
	$(CXX) $(CXXFLAGS) -c ../generic/oswTime.cpp

oswTable.$(OBJ) : ../generic/oswTable.cpp $(REQUIRED_H) ../include/oswTable.h
	$(CXX) $(CXXFLAGS) -c ../generic/oswTable.cpp

oswMidi.$(OBJ) : ../generic/oswMidi.cpp $(REQUIRED_H) ../include/oswMidi.h
	$(CXX) $(CXXFLAGS) -c ../generic/oswMidi.cpp

oswDelete.$(OBJ) : ../generic/oswDelete.cpp $(REQUIRED_H) \
	../include/oswDelete.h ../include/oswSched.h
	$(CXX) $(CXXFLAGS) -c ../generic/oswDelete.cpp

oswTkDraw.$(OBJ) : ../generic/oswTkDraw.cpp $(REQUIRED_H) \
	../include/oswTkDraw.h ../include/oswTcl.h
	$(CXX) $(CXXFLAGS) -c ../generic/oswTkDraw.cpp

oswInletOutletTransform.$(OBJ) : ../generic/oswInletOutletTransform.cpp \
	$(REQUIRED_H)
	$(CXX) $(CXXFLAGS) -c ../generic/oswInletOutletTransform.cpp

oswGetPut.$(OBJ) : ../generic/oswGetPut.cpp $(REQUIRED_H)
	$(CXX) $(CXXFLAGS) -c ../generic/oswGetPut.cpp

oswLex.$(OBJ) : ../generic/oswLex.cpp $(REQUIRED_H)
	$(CXX) $(CXXFLAGS) -c ../generic/oswLex.cpp

oswBinop.$(OBJ) : ../generic/oswBinop.cpp $(REQUIRED_H) ../include/oswLex.h \
	 ../include/oswBinop.h
	$(CXX) $(CXXFLAGS) -c ../generic/oswBinop.cpp 

oswBinop2.$(OBJ) : ../generic/oswBinop2.cpp $(REQUIRED_H) ../include/oswLex.h \
	../include/oswBinop.h
	$(CXX) $(CXXFLAGS) -c ../generic/oswBinop2.cpp 

oswBinop3.$(OBJ) : ../generic/oswBinop3.cpp $(REQUIRED_H) ../include/oswLex.h \
	../include/oswBinop.h
	$(CXX) $(CXXFLAGS) -c ../generic/oswBinop3.cpp 

oswBinop3a.$(OBJ) : ../generic/oswBinop3a.cpp $(REQUIRED_H) \
	../include/oswLex.h ../include/oswBinop.h
	$(CXX) $(CXXFLAGS) -c ../generic/oswBinop3a.cpp 

oswBinop3b.$(OBJ) : ../generic/oswBinop3b.cpp $(REQUIRED_H) \
	../include/oswLex.h ../include/oswBinop.h
	$(CXX) $(CXXFLAGS) -c ../generic/oswBinop3b.cpp 

oswBinop4.$(OBJ) : ../generic/oswBinop4.cpp $(REQUIRED_H) \
	../include/oswLex.h ../include/oswBinop.h
	$(CXX) $(CXXFLAGS) -c ../generic/oswBinop4.cpp 

oswBinop4a.$(OBJ) : ../generic/oswBinop4a.cpp $(REQUIRED_H) \
	../include/oswLex.h ../include/oswBinop.h
	$(CXX) $(CXXFLAGS) -c ../generic/oswBinop4a.cpp 

oswBinop5.$(OBJ) : ../generic/oswBinop5.cpp $(REQUIRED_H) \
	../include/oswList.h ../include/oswBinop.h
	$(CXX) $(CXXFLAGS) -c ../generic/oswBinop5.cpp 

oswList.$(OBJ) : ../generic/oswList.cpp $(REQUIRED_H) ../include/oswLex.h \
	../include/oswList.h
	$(CXX) $(CXXFLAGS) -c ../generic/oswList.cpp 

oswEnumeratedString.$(OBJ) : ../generic/oswEnumeratedString.cpp $(REQUIRED_H) 
	$(CXX) $(CXXFLAGS) -c ../generic/oswEnumeratedString.cpp 

oswBundle.$(OBJ) : ../generic/oswBundle.cpp $(REQUIRED_H) \
	../include/oswBundle.h
	$(CXX) $(CXXFLAGS) -c ../generic/oswBundle.cpp

oswArray.$(OBJ) : ../generic/oswArray.cpp $(REQUIRED_H) ../include/oswBundle.h
	$(CXX) $(CXXFLAGS) -c ../generic/oswArray.cpp

oswMath.$(OBJ) : ../generic/oswMath.cpp $(REQUIRED_H) ../include/oswMath.h
	$(CXX) $(CXXFLAGS) -c ../generic/oswMath.cpp

oswExpr.$(OBJ) : ../generic/oswExpr.cpp $(REQUIRED_H) ../include/oswMath.h ../include/oswBinop.h
	$(CXX) $(CXXFLAGS) -c ../generic/oswExpr.cpp

oswParseExpr.$(OBJ) : ../generic/oswParseExpr.c ../generic/oswParseExpr.h
	$(CC) $(CFLAGS) -c ../generic/oswParseExpr.c

../generic/oswParseExpr.c : ../generic/oswParseExpr.y
	bison ../generic/oswParseExpr.y -o ../generic/oswParseExpr.c

oswSdif.$(OBJ) : ../generic/oswSdif.cpp ../include/sdif.h ../include/sdif-mem.h \
	../include/oswSdif.h $(REQUIRED_H)
	$(CXX) $(CXXFLAGS)  -c ../generic/oswSdif.cpp

oswBlob.$(OBJ) : ../generic/oswBlob.cpp ../include/oswBlob.h $(REQUIRED_H)
	$(CXX) $(CXXFLAGS) -c ../generic/oswBlob.cpp

oswNote.$(OBJ) : ../generic/oswNote.cpp ../include/oswNote.h \
	../include/oswBlob.h $(REQUIRED_H)
	$(CXX) $(CXXFLAGS) -c ../generic/oswNote.cpp

oswSpectrum.$(OBJ) : ../generic/oswSpectrum.cpp ../types/Spectrum.h \
	../include/oswBinop.h $(REQUIRED_H)
	$(CXX) $(CXXFLAGS) -c ../generic/oswSpectrum.cpp

oswPatternMatch.$(OBJ) : ../generic/oswPatternMatch.cpp $(REQUIRED_H) \
	../include/oswPatternMatch.h
	$(CXX) $(CXXFLAGS) -c ../generic/oswPatternMatch.cpp

oswProfile.$(OBJ) : ../generic/oswProfile.cpp ../include/oswProfile.h \
	../include/oswHiResTimer.h $(REQUIRED_H)
	$(CXX) $(CXXFLAGS) -c ../generic/oswProfile.cpp

oswOSCClient.$(OBJ) : ../generic/oswOSCClient.cpp ../include/oswOSCPacket.h \
	../include/oswOSCTypes.h $(REQUIRED_H)
	$(CXX) $(CXXFLAGS) -c ../generic/oswOSCClient.cpp

oswOSCTypes.$(OBJ) : ../generic/oswOSCTypes.cpp \
	../include/oswOSCTypes.h $(REQUIRED_H)
	$(CXX) $(CXXFLAGS) -c ../generic/oswOSCTypes.cpp

oswOSCServer.$(OBJ) : ../generic/oswOSCServer.cpp ../include/oswOSCPacket.h \
	../include/oswOSCTypes.h $(REQUIRED_H) ../include/oswClock.h
	$(CXX) $(CXXFLAGS) -c ../generic/oswOSCServer.cpp

oswInterface.$(OBJ) : ../generic/oswInterface.cpp $(REQUIRED_H)
	$(CXX) $(CXXFLAGS) -c ../generic/oswInterface.cpp

oswThis.$(OBJ) : ../generic/oswThis.cpp $(REQUIRED_H)
	$(CXX) $(CXXFLAGS) -c ../generic/oswThis.cpp

oswCleanup.$(OBJ) : ../generic/oswCleanup.cpp ../include/oswCleanup.h
	$(CXX) $(CXXFLAGS) -c ../generic/oswCleanup.cpp

###############

sdif.$(OBJ) : ../sdif/sdif.c ../include/sdif.h
	$(CC) $(CFLAGS)  -c ../sdif/sdif.c

sdif-mem.$(OBJ) : ../sdif/sdif-mem.c ../include/sdif.h ../include/sdif-mem.h
	$(CC) $(CFLAGS) -c ../sdif/sdif-mem.c

sdif-types.$(OBJ) : ../sdif/sdif-types.c ../include/sdif.h ../include/sdif-types.h
	$(CC) $(CFLAGS)  -c ../sdif/sdif-types.c

###############

OSC-pattern-match.$(OBJ) : ../osc/OSC-pattern-match.c ../osc/OSC-common.h \
			   ../osc/OSC-pattern-match.h 
	$(CC) $(CFLAGS) -c ../osc/OSC-pattern-match.c

OSC-system-dependent.$(OBJ) : ../osc/OSC-system-dependent.c ../osc/OSC-common.h
	$(CC) $(CFLAGS) -c ../osc/OSC-system-dependent.c

OSC-client.$(OBJ) : ../osc/OSC-client.c ../osc/OSC-client.h
	$(CC) $(CFLAGS) -c ../osc/OSC-client.c

OSC-timetag.$(OBJ) :  ../osc/OSC-timetag.c ../osc/OSC-timetag.h
	$(CC) $(CFLAGS) -c ../osc/OSC-timetag.c

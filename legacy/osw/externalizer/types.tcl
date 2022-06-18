
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

namespace eval externalizer {

}
proc settypes {srcdir} {
    set externalizer::types {Integer Float Unit Boolean Double String Samples \
            List Time IntegerSamples \
	    FloatTable IntegerTable Any Char "UInteger" Byte \
	    Blob Note Score \
	    Integer16Samples ByteSamples DoubleSamples \
	    Complex DoubleComplex Changeable Raw}
    
    foreach type $externalizer::types {
	set externalizer::typeLocation($type) internal
	set externalizer::typeLocation([string tolower $type]) internal
    }
    
    set typedir .
    catch {set typedir $srcdir/types}
    
    regsub -all -- \\\\ $typedir "\/" typedir
    
    lappend types SDIF_Frame
    set externalizer::typeLocation(SDIF_Frame) oswsdif.h
    foreach dot_h [glob -nocomplain -- $typedir/*.h] {
	set chn [open $dot_h r] 
	set firstline [gets $chn]
	if {[lindex $firstline 0] == "//T"} {
	    lappend externalizer::types [lindex $firstline 1]
	    set externalizer::typeLocation([lindex $firstline 1]) [file tail $dot_h]
	}
	close $chn
    }
    
    set externalizer::scanproc(Integer) atoi
    set externalizer::scanproc(Float) atof
    set externalizer::scanproc(Double) atof
    set externalizer::scanproc(Byte) atoi
    set externalizer::scanproc(byte) atoi
    set externalizer::scanproc(String) "identity<char *>()"
    
}




    



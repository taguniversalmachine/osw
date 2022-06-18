
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
#From the UC Regents:
#
#Permission to use, copy, modify, and distribute this software and its
#documentation, without fee and without a signed licensing agreement, is hereby
#granted, provided that the above copyright notice, this paragraph and the
#following two paragraphs appear in all copies, modifications, and
#distributions.  Contact The Office of Technology Licensing, UC Berkeley, 2150
#Shattuck Avenue, Suite 510, Berkeley, CA 94720-1620, (510) 643-7201, for
#commercial licensing opportunities.
#
#Written by Amar Chaudhary, The Center for New Music and Audio Technologies, 
#University of California, Berkeley.
#
#     IN NO EVENT SHALL REGENTS BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT,
#     SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST
#     PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS
#     DOCUMENTATION, EVEN IF REGENTS HAS BEEN ADVISED OF THE POSSIBILITY OF
#     SUCH DAMAGE.
#
#     REGENTS SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT
#     LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
#     FOR A PARTICULAR PURPOSE. THE SOFTWARE AND ACCOMPANYING
#     DOCUMENTATION, IF ANY, PROVIDED HEREUNDER IS PROVIDED "AS IS".
#     REGENTS HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES,
#     ENHANCEMENTS, OR MODIFICATIONS.
#     
#


foreach transform [oswTransformTypes ""] {
    set AvailableTransforms($transform) osx
    set TransformPackage($transform) $transform
}
set AvailableTransforms(Patch) special
set TransformPackage(Patch) special

proc _ScanExternalPath {path} {
    regsub -all "\\\\" $path "/" path
    foreach f [glob -nocomplain "$path/*"] {
	if {[file extension $f] == ".osx"} {
	    set localPackage [oswGetPackageFromPath $f]
	    if {[catch {set ::AvailableTransforms([file root [file tail $f]])}]} {
		set ::AvailableTransforms([file root [file tail $f]]) unloaded
		set ::TransformPackage([file root [file tail $f]]) $localPackage
	    }
	    set ::AvailableTransforms($localPackage) unloaded
	    set ::TransformPackage($localPackage) $localPackage
	} 
	if {[file extension $f] == ".osw"} {
	    set ::AvailableTransforms([file root [file tail $f]]) unloaded
	    set localPackage [oswGetPackageFromPath $f]
	    set ::TransformPackage([file root [file tail $f]]) $localPackage
	    set ::AvailableTransforms($localPackage) unloaded
	    set ::TransformPackage($localPackage) $localPackage
	} 
	if {[file isdirectory $f] && [file tail $f] != "types"} {
	    _ScanExternalPath $f
	}
    }
}

proc oswGetPackageFromName {transformClass} {

    regsub "::" $transformClass "/" transformClass
    return [file dir $transformClass]
}

proc oswGetNameWithoutPackage {transformClass} {
    if {$transformClass == "/"} {
	return "div"
    } elseif {$transformClass == "%"} {
	return "mod"
    }
    regsub "::" $transformClass "/" transformClass
    return [file tail $transformClass]
}

proc oswGetPackageFromPath {transformPath} {
    if {$::tcl_platform(platform) == "windows"} {
	regsub -all "\\\\" $::env(OSW_DIR) "/" patternPrefix
    } else {
	set patternPrefix $::env(OSW_DIR)
    }
    regsub "$patternPrefix/externals/" $transformPath "" packagePath
    regsub "/" $packagePath "::" result
    set result [file root $result]
    return $result
}

proc oswGetPackage {transformClass} {
    #puts "Package for $transformClass"
    return $::TransformPackage($transformClass)
}

proc oswLoadDependencies {oslpath} {
    if {[file exists $oslpath]} {
	set chn [open $oslpath r]
	while {![eof $chn]} {
	    set dependency "$::env(OSW_DIR)/types/[gets $chn]"
	    if {[file exists $dependency.osx]} {
		#puts "Loading $dependency"
		oswLoadOSX $dependency
	    }
	}
    }
}

proc oswFindTransform {transformClass} {

    if {$transformClass == "*"} {
	return osx
    }

    regsub "::" $transformClass "/" transformClass
    
    set entry [array get ::AvailableTransforms $transformClass]
    if {[llength $entry] == 0 || [lindex $entry 1] == "unloaded"} {

	set osxpath $transformClass.osx
	if {![file exists $transformClass.osx]} {
	    set osxpath [oswSearchExternalPath "$transformClass.osx"] 
	} 
	if {$osxpath != ""} {
	    set osxpath [file root $osxpath]
            oswLoadDependencies $osxpath.osl
            oswLoadOSX $osxpath
	    set ::AvailableTransforms($transformClass) osx
	    set packageLocal [oswGetPackageFromPath $osxpath]
	    set ::TransformPackage($transformClass) $packageLocal
	    set ::AvailableTransforms($packageLocal) osx
	    set ::TransformPackage($packageLocal) $packageLocal

	    return osx
	} else {
	    set oswpath $transformClass.osw
	    if {![file exists $oswpath]} {
		set oswpath [oswSearchExternalPath "$transformClass.osw"]
	    }
	    if {$oswpath != ""} {
		set ::AvailableTransforms($transformClass) $transformClass.osw
		set packageLocal [oswGetPackageFromPath $osxpath]
		set ::TransformPackage($transformClass) $packageLocal
		set ::AvailableTransforms($packageLocal) patch
		set ::TransformPackage($packageLocal) $packageLocal

		return $oswpath
	    }
	}
	return none
    }

    return [lindex $entry 1]
}

proc oswFindTransformSpecification {transformClass} {

    if {$transformClass == "*"} {
	return ""
    }

    set osxpath $transformClass.osx
    if {![file exists $transformClass.osd]} {
	return [oswSearchExternalPath "$transformClass.osd"] 
    } else {
	return $transformClass.osd
    }



}

proc oswGetCompletions {prefix} {
    set result {}
    set l [string length $prefix]
    foreach name [array names ::AvailableTransforms] {
	if {[string equal -nocase -length $l $prefix $name]} {
	    lappend result $name
	}
    }
    return $result
}

foreach type [glob -nocomplain $env(OSW_DIR)/externals/types/*.osx] {
    puts $type
    oswLoadOSX [file root $type]
}

_ScanExternalPath "$env(OSW_DIR)/externals"










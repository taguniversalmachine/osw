
#Copyright (c) 2001-2003 Amar Chaudhary. All rights reserved.
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


#    iwidgets::hyperhelp .help -title "OSW Help" -modality none 
#	    -helpdir $env(OSW_DIR)/html


set topicOverride(+) add
set topicOverride(-) sub
set topicOverride(*) mul
set topicOverride(/) div
set topicOverride(%) mod
set topicOverride(^) pow
set topicOverride('+) sadd
set topicOverride('-) ssub
set topicOverride('*) smul
set topicOverride('/) sdiv
set topicOverride('%) smod
set topicOverride('^) spow
set topicOverride(Abs) math
set topicOverride(Acos) math
set topicOverride(Acot) math
set topicOverride(Asin) math
set topicOverride(Atan) math
set topicOverride(Acosh) math
set topicOverride(Asinh) math
set topicOverride(Atanh) math
set topicOverride(Asec) math
set topicOverride(Acsc) math
set topicOverride(Acsc) math
set topicOverride(Acsch) math
set topicOverride(Acoth) math
set topicOverride(Ceil) math
set topicOverride(Cos) math
set topicOverride(Cosh) math
set topicOverride(Cot) math
set topicOverride(Coth) math
set topicOverride(Csc) math
set topicOverride(Csch) math
set topicOverride(Exp) math
set topicOverride(Floor) math
set topicOverride(Frac) math
set topicOverride(Log) math
set topicOverride(Log10) math
set topicOverride(Sgn) math
set topicOverride(Sec) math
set topicOverride(Sech) math
set topicOverride(Sin) math
set topicOverride(Sinc) math
set topicOverride(Sinh) math
set topicOverride(Sinhc) math
set topicOverride(Sqrt) math
set topicOverride(Tan) math
set topicOverride(Tanh) math
set topicOverride(==) eq
set topicOverride(!=) eq
set topicOverride('==) seq
set topicOverride('!=) seq
set topicOverride(>) compare
set topicOverride(>=) compare
set topicOverride(<) compare
set topicOverride(<=) compare
set topicOverride('>) scompare
set topicOverride('>=) scompare
set topicOverride('<) scompare
set topicOverride('<=) scompare
set topicOverride(>>) bitwise
set topicOverride(<<) bitwise
set topicOverride(?&) bitwise
set topicOverride(?|) bitwise
set topicOverride(?^) bitwise
set topicOverride('>>) sbitwise
set topicOverride('<<) sbitwise
set topicOverride('?&) sbitwise
set topicOverride('?|) sbitwise
set topicOverride('?^) sbitwise
set topicOverride(&&) andor
set topicOverride(||) andor
set topicOverride('&&) sandor
set topicOverride('||) sandor
set topicOverride(Put*) Putmul
set topicOverride(Real) complex
set topicOverride(Imag) complex
set topicOverride(Norm) complex
set topicOverride(Conj) complex
set topicOverride(Angle) complex
set topicOverride(Fact) math
set topicOverride(Gamma) math
set topicOverride(Gcd) math

proc ShowHelpTopic {topic} {

    catch {set topic $::topicOverride($topic)}

    switch $::tcl_platform(platform) "windows" {
	set topic $::env(OSW_DIR)\\html\\$topic.html
	regsub -all "/" $topic "\\" topic
	exec EXPLORER.EXE $topic &
    } "unix" {
	set topic $::env(OSW_DIR)/html/$topic.html
	if {$::tcl_platform(os) == "Darwin"} {
	    #invoke browser via AppleScript
	    set osa [open "|osascript" w]
	    puts $osa "open location \"file://$topic\""
	    close $osa
	} else {
	    if {[catch {set ::osw_browser}] || ![string compare $::osw_browser "netscape"] } {
		if {[catch {exec netscape -remote "openURL($topic)"}]} {
		    exec netscape $topic &
		}
	    } else {
		exec $::osw_browser $topic &
	    }
	}
    }
}

proc CheckExistHelpTopic {topic} {
    catch {set topic $::topicOverride($topic)}
    if {[file exists $::env(OSW_DIR)/html/$topic.html]} {
        return $topic.html
    }
    return "NONE"
}

proc ShowHyperlink {link} {
    if {![regexp "^http" $link]} {
	set prefix "http://"
	append prefix $link
	set link $prefix
    }
    switch $::tcl_platform(platform) "windows" {
	exec EXPLORER.EXE $link &
    } "unix" {
	if {$::tcl_platform(os) == "Darwin"} {
	    #invoke browser via AppleScript
	    set osa [open "|osascript" w]
	    puts $osa "open location \"$link\""
	    close $osa
	} else {
	    if {[catch {set ::osw_browser}] || ![string compare $::osw_browser "netscape"] } {
		if {[catch {exec netscape -remote "openURL($link)"}]} {
		    exec netscape $link &
		}
	    } else {
		exec $::osw_browser $link &
	    }
	}
    }
}

proc ShowHelpPatch {topic} {
    set path [file dir $topic]
    set base [file root [file tail $topic]]
    if {[file exists $::env(OSW_DIR)/help_patches/$path/help_${base}.osw]} {
	oswSourcePatch $::env(OSW_DIR)/help_patches/$path/help_${base}.osw
    } else {
	catch {set topic $::topicOverride($topic)}
	set base [file root [file tail $topic]]
	if {[file exists $::env(OSW_DIR)/help_patches/$path/help_${base}.osw]} {
	    oswSourcePatch $::env(OSW_DIR)/help_patches/$path/help_${base}.osw
	}
    }
}

proc CheckExistHelpPatch {topic} {
    set path [file dir $topic]
    set base [file root [file tail $topic]]
    if {[file exists $::env(OSW_DIR)/help_patches/$path/help_${base}.osw]} {
	return $path/help_${base}.osw
    } else {
	catch {set topic $::topicOverride($topic)}
	set base [file root [file tail $topic]]
	if {[file exists $::env(OSW_DIR)/help_patches/$path/help_${base}.osw]} {
	    return $path/help_${base}.osw
	}
    }
    return "NONE"
}

proc BrowseHelpPatches {patch} {
    global defaultCount
    
    set patchFile [tk_getOpenFile -defaultextension "osw" -filetypes {{"OSW Patch" {.osw}}} -initialdir "$::env(OSW_DIR)/help_patches" -title "Browse Help Patches"]
    
    if {$patchFile == ""} {
	return
    }
    
    if {$patch != ""} {$patch Hide}
    oswReadPatchFromXML $patchFile [list "" "" "" $patchFile]
}

proc BrowseTutorials {patch} {
    global defaultCount
    
    set patchFile [tk_getOpenFile -defaultextension "osw" -filetypes {{"OSW Patch" {.osw}}} -initialdir "$::env(OSW_DIR)/tutorials" -title "Browse Tutorials"]
    
    if {$patchFile == ""} {
	return
    }
    
    if {$patch != ""} {$patch Hide}
    oswReadPatchFromXML $patchFile [list "" "" "" $patchFile]
}

proc BrowseDemos {patch} {
    global defaultCount
    
    set patchFile [tk_getOpenFile -defaultextension "osw" -filetypes {{"OSW Patch" {.osw}}} -initialdir "$::env(OSW_DIR)/demos" -title "Browse Demos"]
    
    if {$patchFile == ""} {
	return
    }

    if {$patch != ""} {$patch Hide}
    oswReadPatchFromXML $patchFile [list "" "" "" $patchFile]
}

proc SetupReferencePages {} {
    
    set transforms {}
    foreach dir [glob -nocomplain $::env(OSW_DIR)/html/*] {
	if {$dir == "CVS" || [regexp "_files" $dir]} {
	    continue
	}
	if {[file isdirectory $dir]} {
	    set packageName [file tail $dir]
	    
	    append packageName "::"
	    
	    set displayPackage $packageName
	    if {$packageName == "osw::" || $packageName == "standard::" || $packageName == "internal::"} {
		set displayPackage ""
	    }
	    
	    foreach f [glob -nocomplain $dir/*] {
		set transform [file root [file tail $f]]
		lappend transforms [list $displayPackage$transform $packageName$transform]
	    }
	}
    }

    set transforms [lsort $transforms]
    set chn [open $::env(OSW_DIR)/html/reference.html w]
    puts $chn "<h1>OSW Transform Reference</h1>"

    foreach pair $transforms {
	set path [lindex $pair 1]
	set transform [lindex $pair 0]
	regsub "::" $path "/" path
	puts $chn "<a href=$path.html>$transform</a><br>"
    }
    close $chn
}

#SetupReferencePages
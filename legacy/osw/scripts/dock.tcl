
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


set dockedList {}

set _currentdock ""

proc HideName {} {
    pack forget $::_currentdock.x.e -fill x
}

proc AddDockTransform {name transformCmd} {
    oswDockTransform $transformCmd $name 0
    AddDockTransformInt [.dock.dock childsite] $name $transformCmd
}

proc AddDockTransformInt {dock name transformCmd} {
    frame $dock.$name -bd 2 -relief raised
    frame $dock.$name.x
    set ::_currentdock $dock.$name
    set font [SelectFontFace "helvetica" 8]
    entry $dock.$name.x.e -font $font 
    $dock.$name.x.e insert 0 $transformCmd
    $dock.$name.x.e configure -state disabled
    frame $dock.$name.x.f -height 1
    set drawingSpace $dock.$name.x.f
    set nameSpace $dock.$name.x.e
    set transform /dock/$name
    set initializer $transformCmd
    pack $dock.$name.x.e -side top -fill x -expand 1
    pack $dock.$name.x.f
    eval [oswDrawTransform $transform $drawingSpace]    
    update idletasks
    if {$::tcl_platform(os) == "Darwin" && [winfo reqwidth $dock.$name.x.f] > 120} {
        $dock.$name.x.f configure -width 120        
    }
    pack $dock.$name.x.f
    pack $dock.$name.x -fill both -expand 1
    pack $dock.$name -fill both -expand 1

    bind $dock.$name.x.e <Button-1> "if {\$Patch::selected != \"\"} {\$Patch::selected DragDockedPrototype [list $transformCmd] $name}"
    bind $dock.$name.x <Button-1> "if {\$Patch::selected != \"\"} {\$Patch::selected DragDockedPrototype [list $transformCmd] $name}"
    foreach child [winfo children $dock.$name.x.f] {
        bind $child <Button> {}
        bind $child <Button-2> {}
        bind $child <Button-3> {}
        bind $child <Button-1> "if {\$Patch::selected != \"\"} {\$Patch::selected DragDockedPrototype [list $transformCmd] $name}"	
        foreach child [winfo children $child] {
            bind $child <Button> {}
            bind $child <Button-2> {}
            bind $child <Button-3> {}
            bind $child <Button-1> "if {\$Patch::selected != \"\"} {\$Patch::selected DragDockedPrototype [list $transformCmd] $name}"
        }
    }
    
    bind $dock.$name <Enter> [list oswShowBalloonHelp [oswDescription $transform] %x %y %W]    
    bind $dock.$name <Leave> oswHideBalloonHelp
    update idletasks
}

proc SetupDock {} {
    
    toplevel .dock -width 120 -height [expr [winfo screenheight .] - 150]
    wm geometry .dock +0+60
    iwidgets::scrolledframe .dock.dock \
	    -vscrollmode dynamic \
	    -hscrollmode none -width 120 \
	    -height [expr [winfo screenheight .] - 150]
    pack .dock.dock -side top -fill both -expand yes
    set dock [.dock.dock childsite]
    global dockedTransforms dockedList
    frame $dock.x  -bd 2 -relief raised
    label $dock.x.newxform -image [oswLoadBitmap photo newxform.gif] \
	    -bd 1 -relief raised
    pack $dock.x -fill x
    pack $dock.x.newxform
    #grid $dock.x.newxform -row 1 -sticky w
    bind $dock.x.newxform <Button-1> {if {$Patch::selected != ""} {$Patch::selected DragNewTransform blank}}
    bind $dock.x.newxform <Enter> {oswShowBalloonHelp "Blank Transform" %x %y %W}
    bind $dock.x.newxform <Leave> oswHideBalloonHelp
    
    foreach name $dockedList {
        set transformCmd $dockedTransforms($name)
        AddDockTransformInt $dock $name $transformCmd
    }
    if {$::tcl_platform(os) != "Darwin"} {
        pack $dock -side top -fill x
    }
    oswAttachDefaultBindings .dock
    bind .dock <Button> oswShowAllRelevant
    bind .dock <Destroy> {set _currentdock ""
    set showdock 0}
}


proc oswCreateDockingPatch {} {
    oswPatch "" "dock"
}

proc oswDockTransform {transformCmd instanceName askForArgs} {
    
    set oldCmd $transformCmd
    regsub -all -- \\$ $oldCmd "\\$" transformCmd

    set transformClass [lindex $transformCmd 0]
    
    set nameOption [lsearch $transformCmd "-name"]
    if {$nameOption >= 0} {
        incr nameOption
        set transformName [lindex $transformCmd $nameOption]
    } else {
        if {$instanceName != ""} {
            set transformName [lindex $instanceName 0]
        } else { 
            set transformName [GetUniqueName \
                                   [string tolower [oswGetNameWithoutPackage $transformClass]]]
        }
    }
    #all transforms are now "autonamed"
    set autonamed true 
    set savedCmd $transformCmd
    set transformCmd [linsert $transformCmd 1 $transformName]
    
    if {$transformClass == "Patch"} {
        oswAddSubpatch /dock $transformName
        set isPatch true
        set isSubpatch true
    } else {
        set transformStyle [oswFindTransform $transformClass]
        set transformCmd [lreplace $transformCmd 0 0 [oswGetPackage $transformClass]]
        if {$transformStyle == "none"} {
            bgerror "Couldn't make transform of type $transformClass"
            return
        }
        
        #special hack for TransformArrays
        if {$transformClass == "Array"} {
            oswFindTransform [lindex $transformCmd 3]
            set transformCmd [lreplace $transformCmd 3 3 [oswGetPackage $transformClass]]
        }
        if {$transformStyle != "osx" } {
            eval oswInstancePatch /dock $transformCmd $transformStyle
            set transformName [lindex $transformCmd 1]
            set isPatch true		
        } else {
            set transformName [eval /dock add $transformCmd]
        }
    }

    set transformCmd $savedCmd
    set ::dockedAutoNamed($instanceName) $autonamed
    set ::dockedTransforms($instanceName) $transformCmd
    lappend ::dockedList $instanceName
}

proc oswUndockTransform {instanceName} {
    /dock remove $instanceName
    unset ::dockedTransforms($instanceName)
    unset ::dockedAutoNamed($instanceName)
    global dockedList
    ldelete ::dockedList $instanceName
}


proc oswDefaultDocks {} {

    oswDockTransform "Button" button 0
    oswDockTransform "Toggle" toggle 0
    oswDockTransform "MessageBox" msgbox 0
    oswDockTransform "IntBox" intbox 0
    oswDockTransform "FloatBox" floatbox 0
    oswDockTransform "TimeMachine" tm 0
    #oswDockTransform "VSlider" vslider 0
    oswDockTransform "HSlider" hslider 0
    oswDockTransform "Knob" knob 0
    oswDockTransform "AudioOutput 1 2" out 1
    oswDockTransform "AudioInput 1 2" input 1
}


proc oswInitDock {} {
    
    oswCreateDockingPatch
    oswDefaultDocks
    SetupDock
}

proc oswCleanupDock {} {
    oswDelete /dock
}








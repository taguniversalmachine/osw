#!/usr/local/bin/wish


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


if {$tcl_platform(platform) == "windows"} {
    package require registry 1.0
    set osw_dir [registry get HKEY_LOCAL_MACHINE\\Software\\OSW Home]
    set env(OSW_DIR) [registry get HKEY_LOCAL_MACHINE\\Software\\OSW Home]
}


lappend auto_path $env(OSW_DIR)
package require Iwidgets
package require xml
catch {namespace import ::itcl::*}


#this one has to come first
source $env(OSW_DIR)/scripts/evalserver.tcl



switch $tcl_platform(platform) windows {
    load $env(OSW_DIR)/libosw.dll Osw
} unix {
    if {$tcl_platform(os) == "Darwin"} {
	load libosw.dylib OSW
    } else {
	load libosw.so Osw
    }
} default {
    error "Your platform is not supported!"
    exit
}




set oswversion [oswVersion]
source $env(OSW_DIR)/scripts/bugfix.tcl
source $env(OSW_DIR)/scripts/wirecolor.tcl
source $env(OSW_DIR)/scripts/ldelete.tcl
source $env(OSW_DIR)/scripts/commonStr.tcl
source $env(OSW_DIR)/scripts/changeColor.tcl
source $env(OSW_DIR)/scripts/extmanage.tcl
source $env(OSW_DIR)/scripts/balloon.tcl
source $env(OSW_DIR)/scripts/completionbox.tcl
source $env(OSW_DIR)/scripts/font.tcl
    
source $env(OSW_DIR)/scripts/transform.tcl
source $env(OSW_DIR)/scripts/connections.tcl
source $env(OSW_DIR)/scripts/stickynote.tcl
source $env(OSW_DIR)/scripts/patch.tcl
source $env(OSW_DIR)/scripts/oswxml.tcl
source $env(OSW_DIR)/scripts/dock.tcl
source $env(OSW_DIR)/scripts/oscserver.tcl




set defaultCursor {}
set PasteContextX 0
set PasteContextY 0
set PasteContextSeedX 0
set PasteContextSeedY 0


set oswGravityWell 10


###################


set Selection {}
set oswClipboard {}


global OldX OldY
proc StartObjectMotion {patch win tag x y shift} {
    
    
    global Selection OldX OldY
    
    
    set OldX [$win canvasx $x]
    set OldY [$win canvasy $y]
    #set OldX $x
    #set OldY $y
    $patch ObjectSelection $tag $shift
    $win bind $tag <B1-Motion> [list ::$tag MoveObject %x %y]
    $win bind $tag <ButtonRelease-1> [list StopObjectMotion $win $tag $shift]
}



proc MoveOthers {moved dx dy} {
    global Selection 
    
    foreach sel $Selection {
	if {"::$sel" != $moved} {
	    switch [$sel info class] "::TransformWidget" {
		$sel MoveObjectRel $dx $dy
	    } "::stickynote" {
		$sel MoveObjectRel $dx $dy              
	    } "::Connection" {
		#$sel Move $dx $dy
	    } default {
		$sel MoveObjectRel $dx $dy
	    }
	}
    }
}


proc StopObjectMotion {win tag shift} {
    global Selection
    $win bind $tag <B1-Motion> {}
    $win bind $tag <ButtonRelease-1> {}
    if {[llength $Selection] == 1 && [$tag info class] == "::TransformWidget"} {
	$Patch::selected SplitWire $tag 
    }
}



proc SetEditMenu {onoff} {
    global Selection oswClipboard
    if {$onoff && [llength $Selection] > 0} {
	.patchmenu.edit entryconfigure "Cut" -state normal
	.patchmenu.edit entryconfigure "Copy" -state normal
	.patchmenu.edit entryconfigure "Clear" -state normal
	#.patchmenu.edit entryconfigure "Collapse Subpatch" -state normal
	#.patchmenu.format configure -state normal
    } else {
	.patchmenu.edit entryconfigure "Cut" -state disabled
	.patchmenu.edit entryconfigure "Copy" -state disabled
	.patchmenu.edit entryconfigure "Clear" -state disabled
	#.patchmenu.edit entryconfigure "Collapse Subpatch" -state disabled
	#.patchmenu.format configure -state disabled
    }
    if {[llength $oswClipboard] > 0} {
	.patchmenu.edit entryconfigure "Paste" -state normal
    } else {
	.patchmenu.edit entryconfigure "Paste" -state disabled
    }
}


proc Help {} {
    global Selection
    foreach sel $Selection {
	#Try getting help for the first selected transform
	if {[$sel info class] == "::TransformWidget"} {
	    $sel HelpPatch
	    $sel Help
	    return
	}
    }    
}


proc HelpPatch {} {
    global Selection
    foreach sel $Selection {
	#Try getting help for the first selected transform
	if {[$sel info class] == "::TransformWidget"} {
	    $sel HelpPatch
	    return
	}
    }
}


proc ShowDock {} {
    global showdock
    global _currentdock

    if {$_currentdock != ""} {
        if {$showdock} {
            wm deiconify .dock
        } else {
            wm withdraw .dock
        }
    } else {
        SetupDock 
    }


}


proc GetTransformProperties {} {
    global Selection
    foreach sel $Selection {
	#Try getting help for the first selected transform
	if {[$sel info class] == "::TransformWidget"} {
	    $sel OpenDialog
	    return
	}
    }
    
    .help activate
}

proc StartExternalizerFromTransform {} {
    global Selection initCmd
    foreach sel $Selection {
	if {[$sel info class] == "::TransformWidget"} {
	    StartExternalizer [oswFindTransformSpecification \
		    [lindex $initCmd(::$sel) 0]]
	    return
	}
    }


    error "No transform selected"
}


proc StartExternalizer {args} {
    global tcl_platform env


    if {[llength $args] > 0} {
	switch $tcl_platform(platform) windows {
	    exec wish82 "$env(OSW_DIR)/externalizer/externalizer.tcl" \
		    [lindex $args 0] -calledFromOsw &
	} unix {
	    exec wish8.3 "$env(OSW_DIR)/externalizer/externalizer.tcl" \
		    [lindex $args 0] -calledFromOsw &
	}
    } else {
	switch $tcl_platform(platform) windows {
	    exec wish82 "$env(OSW_DIR)/externalizer/externalizer.tcl" &
	} unix {
	    exec wish8.3 "$env(OSW_DIR)/externalizer/externalizer.tcl" &
	} 
    }
}


##################

#this functions were originally for a project in an HCI class that involved
#recording and analyzing user input.  They might come in handy again someday.

set isRecording false
set logfile {}
proc RecordInput {} {



    if {$isRecording} {
	StopRecording
    }     


    set filename [tk_getSaveFile -defaultextension ".txt" -filetypes {{"Log File" {.txt}}} ]
    
    if {$filename != ""} {
	set logfile [open $filename w]
	bind all <Button> {catch {puts $logfile [list %t %T %W %b %x %y %s]}}
	bind all <Motion> {catch {puts $logfile [list %t %T %W %b %x %y %s]}}
	bind all <ButtonRelease> {catch {puts $logfile [list %t %T %W %b %x %y %s]}}
	bind all <KeyPress> {catch {puts $logfile [list %t %T %W %k %s]}}
	bind all <KeyRelease> {catch {puts $logfile [list %t %T %W %k %s]}}
	
	set isRecording true
    }
}


proc StopRecording {} {


    set isRecording false
    bind all <Button> {}
    bind all <Motion> {}
    bind all <ButtonRelease> {}
    bind all <KeyPress> {}
    bind all <KeyRelease> {}
    if {$logfile != {}} {
	close $logfile
    }
}


proc PlayInput {} {
    set filename [tk_getOpenFile -defaultextension "txt" -filetypes {{"Log File" {.txt}}} ]
    
    if {$filename != ""} {
	set playfile [open $filename r]
	while {![eof $playfile]} {
	    after 20
	    set line [gets $playfile]
	    switch {[lindex $line 1]} 2 {
		event generate [lindex $line 2] <KeyPress> \
			-keycode [lindex $line 3] -state [lindex $line 4]
	    } 3 {
		event generate [lindex $line 2] <KeyRelease> \
			-keycode [lindex $line 3] -state [lindex $line 4]
	    } 4 {
		event generate [lindex $line 2] <Button> \
			-button [lindex $line 3] -x [lindex $line 4] \
			-y [lindex $line 5] -state [lindex $line 6]
	    } 5 {
		event generate [lindex $line 2] <ButtonRelease> \
			-button [lindex $line 3] -x [lindex $line 4] \
			-y [lindex $line 5] -state [lindex $line 6]
	    } 6 {
		event generate [lindex $line 2] <Motion> \
			-button [lindex $line 3] -x [lindex $line 4] \
			-y [lindex $line 5] -state [lindex $line 6]
	    } 
	}
    }
}


###########################


proc oswSetFromGlobalVariable {toSet varName} {
    
    global $varName


    oswSet $toSet [set $varName]
}


set DeferredEvalVar ""
trace variable DeferredEvalVar w oswDoDeferred


proc oswDoDeferred {name1 name2 op} {
    eval $::DeferredEvalVar
}

proc oswSetWidgetProperty {widget property args} {
    $widget configure $property $args
}

proc oswSetFromTextBox {toSet textbox} {
    oswSet $toSet [$textbox get]
}


proc oswFixPatchTerminals {patch} {
    patchObj-$patch FixPatchTerminals
}


proc oswFixTransformTerminals {transform} {
    set patch [file dir $transform]
    set transformObj [patchObj-${patch} Map $transform]
    $transformObj AppendTerminals
}


####################


set showdock 1

proc oswAttachDefaultBindings {w} {
    if {$::tcl_platform(os) == "Darwin"} {	
	bind $w <Up> {MoveOthers "" 0 -1}
	bind $w <Right> {MoveOthers "" 1 0}
	bind $w <Left> {MoveOthers "" -1 0}
	bind $w <Down> {MoveOthers "" 0 1}
	bind $w <Home> {MoveOthers "" -1 -1}	
	#bind $w <End> {MoveOthers "" -1 1}
	bind $w <Prior> {MoveOthers "" 1 -1}
	bind $w <Next> {MoveOthers "" 1 1}
	bind $w <Command-n> {oswNewPatch}
	bind $w <Command-q> {FileQuit}
	bind $w <Command-s> {if {$Patch::selected != ""} {$Patch::selected Save}}
	bind $w <Command-o> {if {$Patch::selected != ""} {oswOpenPatch $Patch::selected}}
	bind $w <Command-w> {if {$Patch::selected != ""} {oswClosePatch $Patch::selected}}
	bind $w <Delete> {if {$Patch::selected != ""} {$Patch::selected DeleteSelection}}
	bind $w <Command-x> {if {$Patch::selected != ""} {$Patch::selected Cut}}
	bind $w <Command-c> {if {$Patch::selected != ""} {$Patch::selected Copy}}
	bind $w <Command-v> {if {$Patch::selected != ""} {$Patch::selected Paste}}
	bind $w <Command-a> {if {$Patch::selected != ""} {$Patch::selected SelectAll}}
	bind $w <Command-m> {if {$Patch::selected != ""} {$Patch::selected AddNewTransformFromKeyboard}}
	bind $w <Alt-Return> {GetTransformProperties}
	bind $w <Command-d> {if {$Patch::selected != ""} {$Patch::selected DockSelection}}
	bind $w <F5> {StartExternalizerFromTransform}
	bind $w <F1> Help
	bind $w <Shift-F1> HelpPatch
	bind $w <Command-Prior> {if {$Patch::selected != ""} {$Patch::selected ScaleUp}}
	bind $w <Command-Next> {if {$Patch::selected != ""} {$Patch::selected ScaleDown}}
    } else {
        if {$::tcl_platform(platform) == "windows"} {
            bind $w <Up> {MoveOthers "" 0 -1}
            bind $w <Right> {MoveOthers "" 1 0}
            bind $w <Left> {MoveOthers "" -1 0}
            bind $w <Down> {MoveOthers "" 0 1}
            bind $w <Home> {MoveOthers "" -1 -1}	
            bind $w <End> {MoveOthers "" -1 1}
            bind $w <Prior> {MoveOthers "" 1 -1}
            bind $w <Next> {MoveOthers "" 1 1}
        } else {
            bind $w <KP_Up> {MoveOthers "" 0 -1}
            bind $w <KP_Right> {MoveOthers "" 1 0}
            bind $w <KP_Left> {MoveOthers "" -1 0}
            bind $w <KP_Down> {MoveOthers "" 0 1}
            bind $w <KP_Home> {MoveOthers "" -1 -1}	
            bind $w <KP_End> {MoveOthers "" -1 1}
            bind $w <KP_Prior> {MoveOthers "" 1 -1}
            bind $w <KP_Next> {MoveOthers "" 1 1}
        }
	bind $w <Control-n> {oswNewPatch}
	bind $w <Control-q> {FileQuit}
	bind $w <Control-s> {if {$Patch::selected != ""} {$Patch::selected Save}}
	bind $w <Control-o> {if {$Patch::selected != ""} {oswOpenPatch $Patch::selected}}
	bind $w <Control-w> {if {$Patch::selected != ""} {oswClosePatch $Patch::selected}}
	bind $w <Delete> {if {$Patch::selected != ""} {$Patch::selected DeleteSelection}}
	bind $w <Control-x> {if {$Patch::selected != ""} {$Patch::selected Cut}}
	bind $w <Control-c> {if {$Patch::selected != ""} {$Patch::selected Copy}}
	bind $w <Control-v> {if {$Patch::selected != ""} {$Patch::selected Paste}}
	bind $w <Control-a> {if {$Patch::selected != ""} {$Patch::selected SelectAll}}
	bind $w <Control-m> {if {$Patch::selected != ""} {$Patch::selected AddNewTransformFromKeyboard}}
	bind $w <Alt-Return> {GetTransformProperties}
	bind $w <Control-d> {if {$Patch::selected != ""} {$Patch::selected DockSelection}}
	bind $w <F5> {StartExternalizerFromTransform}
	bind $w <F1> Help
	bind $w <Shift-F1> HelpPatch
	bind $w <Control-Prior> {if {$Patch::selected != ""} {$Patch::selected ScaleUp}}
	bind $w <Control-Next> {if {$Patch::selected != ""} {$Patch::selected ScaleDown}}
    }
}


proc oswInitWindowControl {} {
    bind . <Map> oswShowAllRelevant
    bind .dock <Map> oswShowAllRelevant
    if {$::tcl_platform(platform) == "unix"} {
	wm group .dock .
    }
}

set _inShowAllRelevant 0

if {$tcl_platform(platform) != "windows" && $tcl_platform(os) != "Darwin"} {
    proc oswShowAllRelevant {} {
	#do nothing under Linux (aws)
    }
} else {
    proc oswShowAllRelevant {} {
	if {!$::_inShowAllRelevant} {
	    set ::_inShowAllRelevant 1
	    wm deiconify .
	    if {$Patch::selected != ""} {
		raise [$Patch::selected cget -pane]
	    }
	    raise .
	    ShowDock
	    if {$::showdock} {
		raise .dock
	    }
	    set ::_inShowAllRelevant 0
	}
    }
}

proc AddAccelerator {accel} {
    if {$::tcl_platform(os) == "Darwin"} {
	return "Command-$accel"
    } else {
	return "Ctrl $accel"
    }
}

proc oswInitAppFrame {} {


    label .status -text ""
    label .status2 -text ""
    #pack .status2 .status -side bottom -fill x


    . configure -menu .patchmenu
    menu .patchmenu -tearoff 0
    .patchmenu add cascade -label "File" -menu .patchmenu.file -underline 0
    menu .patchmenu.file -tearoff 0
    .patchmenu.file add command -label "New" -command {oswNewPatch} \
	-accelerator [AddAccelerator N] -underline 0
    .patchmenu.file add command -label "Open" -command {oswOpenPatch $Patch::selected} \
	-accelerator [AddAccelerator O] -underline 0
    .patchmenu.file add command -label "Close" -command {oswClosePatch $Patch::selected} \
	-accelerator [AddAccelerator W] -underline 0
    .patchmenu.file add command -label "Save" -command {$Patch::selected Save} \
	-accelerator [AddAccelerator S] -underline 0
    .patchmenu.file add command -label "Save As..." \
	    -command {$Patch::selected SaveAs} -underline 6
    .patchmenu.file add separator
    
    .patchmenu.file add command -label "Quit" -command "FileQuit" \
	-accelerator [AddAccelerator Q] -underline 0
    oswInitRecentPatches
        
    .patchmenu add cascade -label "Edit" -menu .patchmenu.edit -underline 0
    menu .patchmenu.edit -tearoff 0
    .patchmenu.edit add command -label "Select All" -command {$Patch::selected SelectAll} \
	-accelerator [AddAccelerator A] -underline 8
    .patchmenu.edit add command -label "Cut" -command {$Patch::selected Cut} \
	-accelerator [AddAccelerator X] -state disabled
    .patchmenu.edit add command -label "Copy" -command {$Patch::selected Copy} \
	-accelerator [AddAccelerator C] -state disabled -underline 0
    .patchmenu.edit add command -label "Paste" -command {$Patch::selected Paste} \
	-accelerator [AddAccelerator V] -state disabled -underline 0
    .patchmenu.edit add command -label "Clear" -accelerator "Del" \
	    -command {$Patch::selected DeleteSelection} -state disabled
    .patchmenu.edit add separator
    .patchmenu.edit add command -label "Insert Blank Transform " \
	    -command {$Patch::selected AddNewTransformFromKeyboard} \
	-accelerator [AddAccelerator M]
    .patchmenu.edit add command -label "Insert Annotation" \
	    -command {$Patch::selected DragNewSticky} \


    #.patchmenu.edit add separator
    #.patchmenu.edit add command -label "Collapse Subpatch" \
    #	    -command {$Patch::selected Collapse} \
    #	    -accelerator "$ctrl K" 
    
    
    
    .patchmenu add cascade -label "View" -menu .patchmenu.view -underline 0
    menu .patchmenu.view -tearoff 0
    #.patchmenu.view add command -label "Zoom In" -command {$Patch::selected ScaleUp}
    #.patchmenu.view add command -label "Zoom Out" -command {$Patch::selected ScaleDown}
    #.patchmenu.view add command -label "Split" \
    #	    -command ShowAHiddenView
    #.patchmenu.view add separator
    .patchmenu.view add checkbutton -label "Show Dock" -variable showdock -command "ShowDock"
    
    #bind . <Control-Prior> {$Patch::selected ScaleUp}
    #bind . <Control-Next> {$Patch::selected ScaleDown}


    .patchmenu add cascade -label "Format" -menu .patchmenu.format -underline 2
    menu .patchmenu.format -tearoff 0
    
    .patchmenu.format add command -label "Color..." \
	    -command {$Patch::selected SelectTransformColor} -underline 0
    .patchmenu.format add cascade -label "Font Face" \
	    -menu .patchmenu.format.font -underline 6
    .patchmenu.format add cascade -label "Font Size" \
	    -menu .patchmenu.format.size -underline 6
    menu .patchmenu.format.font -tearoff 0
    foreach face [lsort [font families]] {
	.patchmenu.format.font add radiobutton -label $face -command {$Patch::selected SelectTransformFontFace $_oswFontFace} -variable _oswFontFace
    }
    
    menu .patchmenu.format.size -tearoff 0
    foreach size {8 10 12 14 16 18 20 24 26 28 32 40 48 56 64 72} {
	.patchmenu.format.size add radiobutton -label $size -command {$Patch::selected SelectTransformFontSize $_oswFontSize} -variable _oswFontSize
    }


    #.patchmenu add cascade -label "Transform" -menu .patchmenu.transform
    menu .patchmenu.transform -tearoff 0
    .patchmenu.transform add command -label "Properties..." \
	    -accelerator "Alt Enter" \
	    -command "GetTransformProperties"
    .patchmenu.transform add command -label "Dock" \
	-accelerator [AddAccelerator D] \
	    -command {$Patch::selected DockSelection}
    .patchmenu.transform add command -label "View Specification..." \
	    -command {StartExternalizerFromTransform} -accelerator "F5"
        
    .patchmenu add cascade -label "Patch" -menu .patchmenu.patch -underline 0
    menu .patchmenu.patch -tearoff 0
    #.patchmenu.patch add command -label "Arguments..." \
	    #       -command CreateArguments]
    .patchmenu.patch add command -label "Background..." \
	    -command {$Patch::selected SelectBackground}


    .patchmenu add cascade -label "Options" -menu .patchmenu.options -underline 0
    menu .patchmenu.options -tearoff 0
    .patchmenu.options add command -label "Audio Devices..." -command oswAudioPrefs
    .patchmenu.options add command -label "Editor..." -command oswEditorPrefs
    .patchmenu.options add separator
    .patchmenu.options add checkbutton -label "Profiling" -variable oswProfiling -command oswToggleProfiling -accelerator "Ctrl Shift P"
    .patchmenu.options add command -label "Dump Profile" -command oswDumpProfileToFile
    .patchmenu.options add separator
    .patchmenu.options add command -label "Reset Main Clock" -command oswResetMainClock
    #bind . <Control-P> {set ::oswProfiling [expr !$::oswProfiling] ; oswToggleProfiling}


    .patchmenu add cascade -label "Help" -menu .patchmenu.help -underline 0
    menu .patchmenu.help -tearoff 0
    .patchmenu.help add command -label "Help Topic" -accelerator "F1" \
	    -command Help
    #.patchmenu.help add command -label "Help Patch" -accelerator "Shift F1" \
#	    -command HelpPatch
    .patchmenu.help add separator
    .patchmenu.help add command -label "User Guide" -command {ShowHelpTopic UserGuide}
    .patchmenu.help add command -label "Transforms Sorted by Function" -command {ShowHelpTopic Transform_by_group}
    .patchmenu.help add command -label "Transforms Sorted Alphabetically" -command {ShowHelpTopic Transform_by_alpha}
    .patchmenu.help add command -label "Browse Help Patches..." -command {BrowseHelpPatches $Patch::selected}
    .patchmenu.help add command -label "Browse Tutorials..." -command {BrowseTutorials $Patch::selected}
    .patchmenu.help add command -label "Browse Demos..." -command {BrowseDemos $Patch::selected}
    .patchmenu.help add separator
    .patchmenu.help add checkbutton -label "Show Balloon Help" -variable oswShowBalloon
    .patchmenu.help add checkbutton -label "Show Command Completions" -variable oswShowCompletions
    .patchmenu.help add separator
    .patchmenu.help add command -label "About OSW..." -command {AboutBox 0}
    
    oswAttachDefaultBindings .
    bind . <Button> oswShowAllRelevant
    wm geometry . [expr [winfo screenwidth .] - 10]x0+0+0
    #bind . <Destroy> {if {!$_quitting} {oswQuit}}
    #iwidgets::panedwindow .pw  -orient vertical -width 800 -height 600
    #.pw add "dock"
    #.pw add "patches"


    #set ::patchArea [iwidgets::panedwindow [.pw childsite patches].pw \
	    #-orient horizontal]
    #pack $::patchArea -side top -fill both -expand yes
    
    #pack .pw -side top -fill both -expand yes


    #if {$::showdock} {
#	wm deiconify .dock
#    } else {
#	wm withdraw .dock
#    }
}


################### 


set oswShowBalloon 0
set oswShowCompletions 1

balloon TheBalloonWindow

proc oswShowBalloonHelp {text x y W} {
    if {$::oswShowBalloon} {
	TheBalloonWindow Show $x $y $text $W
    }
}

proc oswHideBalloonHelp {} {
   if {$::oswShowBalloon} {
	TheBalloonWindow Hide
    }
}


################### 


set recentPatches {}

proc oswInitRecentPatches {} {
    global recentPatches
    .patchmenu.file add separator
    for {set i 0} {[oswGetPreference File$i] != ""} {incr i} {
	set r [oswGetPreference File$i]
	.patchmenu.file add command \
		-label "[expr $i + 1]: $r" \
		-command "oswSourcePatchTopLevel [list $r]" \
		-underline 0
	set recentPatches [linsert $recentPatches 0 $r]
    }
}

proc oswSetRecentPatch {patch} {
    global recentPatches
    if {[lsearch $recentPatches $patch] == -1} {
	lappend recentPatches $patch
	if {[llength $recentPatches] > 8} {
	    set recentPatches [lrange $recentPatches [expr [llength $recentPatches] - 8] end]
	}
    }
    while {[.patchmenu.file type last] != "separator"} {
	.patchmenu.file delete last
    }
    
    for {set i 0} {$i < [llength $recentPatches]} {incr i} {
	set r [lindex $recentPatches [expr [llength $recentPatches] - $i - 1]]
	.patchmenu.file add command \
		-label "[expr $i + 1]: $r" \
		-command "oswSourcePatchTopLevel [list $r]" \
		-underline 0
	oswSetPreference File$i $r
    }
}

################### 

set defaultCount -1
proc oswNewPatch {} {
    global defaultCount

    incr defaultCount
    Patch patchObj-/default$defaultCount -patchClass default -patchInstance default$defaultCount 
    patchObj-/default$defaultCount Unlock
    #wm deiconify .default$defaultCount
    #focus .default$defaultCount
}


proc oswOpenPatch {patch} {
    global defaultCount

    set patchFile [tk_getOpenFile -defaultextension "osw" -filetypes {{"OSW Patch" {.osw}}} ]


    if {$patchFile == ""} {
	return
    }
    
    #$patch Hide
    oswReadPatchFromXML $patchFile [list "" "" "" $patchFile]
    oswSetRecentPatch $patchFile
}


proc oswReadPatchFromXML {patchFile instanceInfo} {
    # Get the document data, ensuring that UTF-8 characters are read
    
    set ch [open $patchFile]
    fconfigure $ch -encoding utf-8
    set data [read $ch]
    close $ch


    oswParseXML $data $instanceInfo
}


proc oswParseXML {data instanceInfo} {
    if {[llength $instanceInfo] > 0} {
	PushXMLState PATCHINSTANCE $instanceInfo
    }

    set xmlparser [CreateXMLParser]
    $xmlparser reset
    if {[catch {$xmlparser parse $data} err]} {
	tk_messageBox -parent . -type ok -title "Error" \
            -message "Unable to display patch due to \"$err\"" \
            -icon error
    }
    
    
    if {[llength $instanceInfo] > 0} {
	PopXMLState
    }
    
}


proc oswClosePatch {patch} {
    if {[$patch cget -parent] == ""} {
	if {[$patch cget -dirty] == 1} {
	    set yesno [tk_messageBox -title [$patch cget -patchInstance] \
		    -type yesno \
		    -message "Patch is not yet saved.  Save now?" \
		    -icon question]
	    if {$yesno == "yes"} {
		$patch Save
	    }
	}
	delete object $patch
	set Patch::selected $Patch::previousSelected
    } else {
	$patch Hide
    }
}


set NumPatches 0


proc oswInstancePatch {parent patchClass patchInstance patchCmd patchPath args} {
    global env

    #puts "$patchClass $patchInstance \"$patchCmd\" $patchPath"
    set patchArgs [lrange $patchCmd 2 end]
    if {[file exists $patchClass.osw]} {
	#puts "QQ $patchClass $patchInstance"
	oswReadPatchFromXML $patchClass.osw [list $parent $patchInstance \
		$patchArgs ""]
	return true
    }
    if {[file exists $patchPath]} {
	#puts "RR $patchClass $patchInstance"
	oswReadPatchFromXML $patchPath [list $parent $patchInstance \
		$patchArgs ""]
	return true
    }
    if {[file exists $env(OSW_DIR)/patches/$patchClass.osw]} {
	oswReadPatchFromXML $osw(OSW_DIR)/patches/$patchClass.osw \
		[list $parent $patchInstance $patchArgs ""]
	return true
    }


    return false
}


proc oswAddSubpatch {parent class} {
    oswParseXML "<PATCH CLASS=\"$class\"></PATCH>" [list $parent "" ""]
    patchObj-$parent/$class Unlock
}


proc oswSearchExternalPathAux {path fname} {
    if {[file exists "$path/$fname"]} {
	return $path/$fname
    } else {
	foreach maybeDir [glob -nocomplain "$path/*"] {
	    if {[file isdirectory $maybeDir]} {
		set result [oswSearchExternalPathAux "$maybeDir" $fname]
		if {$result != ""} {
		    return $result
		}
	    }
	}
	return ""
    }
}


proc oswSearchExternalPath {fname} {
    global env tcl_platform
    set local_osw_dir $env(OSW_DIR)
    if {$tcl_platform(platform) == "windows"} {
	regsub -all "\\\\" ${local_osw_dir} "/" local_osw_dir
    }
    set found [oswSearchExternalPathAux "${local_osw_dir}/externals" $fname]
    if {$found == ""} {
	set found [oswSearchExternalPathAux "${local_osw_dir}/tutorials" $fname]
    }           
    return $found
}


proc oswLinkTo {link} {
    set patchLink $link
    append patchLink ".osw"
    set found [oswSearchExternalPath $patchLink]
    if {$found != ""} {
	oswSourcePatch $found
	return
    }       
    ShowHyperlink $link
}



proc oswLoadBitmap {type fname args} {


    set name [file root [file tail $fname]]
    if {[llength $args] > 0} {
	global oswBitmap_$name
    }
    global env
        
    if {[file exists $fname]} {
	set oswBitmap_$name [image create $type -file $fname]
    } elseif {[file exists "$env(OSW_DIR)/bitmaps/$fname"]} {
	set oswBitmap_$name [image create $type -file "$env(OSW_DIR)/bitmaps/$fname"]
    } else {
	if {[catch {source [oswSearchExternalPath $scriptFile]}]} {
	    bgerror "Couldn't find script file $fname"
	    return
	}
    }
    return [set oswBitmap_$name]
}


proc oswLoadBitmapFromData {name type data} {
    
    
    global oswBitmap_$name env
    
    
    if {[info exists oswBitmap_$name]} {
	return [set oswBitmap_$name]
    }
    
    
    set oswBitmap_$name [image create $type -data $data]
    
    
    return set [set oswBitmap_$name]
}


proc oswSourcePatch {patchFile} {
    
    set patchName [file rootname [file tail $patchFile]]
    set instanceName $patchName
    #if {[info exists .$patchName]} {
	# there is already a toplevel patch $patchName
	#    append instanceName $defaultCount
	#    incr defaultCount
	#   }
    oswReadPatchFromXML $patchFile [list "" $instanceName {} $patchFile]
}
    
    
proc oswSourcePatchTopLevel {patchFile} {
    regsub "^OSW_DIR" $patchFile $::env(OSW_DIR) patchFile
    oswSourcePatch $patchFile
}


proc oswShowPatch {patchObj} {
    #wm deiconify [$window cget -win]
    $patchObj Show
}



set _scripts {}
proc oswLoadScript {scriptFile} {
    
    
    global _scripts env
    
    
    if {[lsearch $_scripts $scriptFile] != -1} {
	return
    }
    
    
    if {[file exists $scriptFile]} {
	source $scriptFile
    } elseif {[file exists $env(OSW_DIR)/scripts/$scriptFile]} {
	source $env(OSW_DIR)/scripts/$scriptFile
    } else {
	if {[catch {source [oswSearchExternalPath $scriptFile]}]} {
	    bgerror "Couldn't find script file $fname"
	    return
	}
    }
    
    
    lappend _scripts $scriptFile
}    

set _quitting 0
proc FileQuit {} {
    global _quitting
    if {!$_quitting} {
	set _quitting 1
	#oswDestroyAllPatches
	oswCleanupDock
	oswQuit
	exit
	#destroy .
    }
}


################


set oswProfiling 0


proc oswToggleProfiling {} {
    if {$::oswProfiling} {
	puts "Profiling On"
    } else {
	puts "Profiling Off"
    }
    oswProfile $::oswProfiling
}


proc oswDumpProfileToFile {} {
    set filename [tk_getSaveFile -defaultextension ".txt" -filetypes {{"Text File" {.txt}}} ]
    if {$filename != ""} {
	puts "Dumping Profile to $filename"
	oswDumpProfile $filename
    }
}


proc oswResetMainClock {} {
    oswSet /main_clock 0.0
}


################


source $env(OSW_DIR)/scripts/audioprefs.tcl
source $env(OSW_DIR)/scripts/editorprefs.tcl


oswLoadScript splash.tcl
wm withdraw .
wm title . "Open Sound World"
AboutBox 1
after 3000 destroy .aboutosw
oswInitAppFrame
wm deiconify .

if {$tcl_platform(os) == "Linux"} {
    source $env(OSW_DIR)/scripts/linuxbrowser.tcl
}

source $env(OSW_DIR)/scripts/help.tcl


oswInitDock

source $env(OSW_DIR)/scripts/welcome.tcl


set commandLineFiles [oswGetCommandLineFiles]
if {[llength $commandLineFiles] > 0} {
    foreach patch $commandLineFiles {	
	if {[file extension $patch] == ".osd"} {
	    StartExternalizer $patch
	} else {
	    if {[file exists $patch]} {
		oswSourcePatch $patch
		oswSetRecentPatch $patch		
	    } else {
		puts "Could not open $patch"
	    }
	}
    }
} else {
    oswNewPatch
}

oswInitWindowControl


oswMainLoop

if {$tcl_platform(platform) == "windows"} {
    oswQuit
    exit
}

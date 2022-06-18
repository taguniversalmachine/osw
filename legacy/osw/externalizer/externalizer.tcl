#!/usr/local/bin/wish8.3


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

if {$tcl_platform(platform) == "windows"} {
    package require registry 1.0
    set env(OSW_DIR) [registry get HKEY_LOCAL_MACHINE\\Software\\OSW Home]
}

package require Iwidgets
catch {namespace import ::itcl::*}

lappend auto_path $env(OSW_DIR)
package require xml


set fileToLoad ""
set BatchMode 0
set NoGUI 0

set AddLibs {}

for {set i 0} {$i < [llength $argv]} {incr i} {
    set arg [lindex $argv $i]
    if {$arg == "-calledFromOsw"} {
	set CalledFromOsw 1
    } elseif {$arg == "-osw_dir"} {
	incr i
	set env(OSW_DIR) [lindex $argv $i]
    } elseif {$arg == "-batch"} {
	set BatchMode 1
    } elseif {$arg == "-nogui"} {
	set NoGUI 1
    } elseif {$arg == "-lib"} {
	incr i
	set lib [lindex $argv $i]
	if {[file isfile $lib]} {
	    lappend AddLibs "[pwd]/$lib"
	} else {
	    lappend AddLibs $lib
	}
    } else {
	set fileToLoad $arg
    }
}



source $env(OSW_DIR)/externalizer/types.tcl
source $env(OSW_DIR)/externalizer/compilers.tcl
source $env(OSW_DIR)/externalizer/ldelete.tcl
source $env(OSW_DIR)/externalizer/includes.tcl
image create bitmap _exz_arrowup -file $env(OSW_DIR)/externalizer/arrowup.xbm
image create bitmap _exz_arrowdn -file $env(OSW_DIR)/externalizer/arrowdn.xbm

#do something about sucky appearance under OS X
if {$::tcl_platform(os) == "Darwin"} {
    tk_setPalette background "steel blue" foreground black
}
	


proc fixbackslash {stringToFix} {

    regsub -all -- "\\\\" $stringToFix "\\\\\\\\" result
    regsub -all -- "\"" $result "\\\"" result
    set result [string trimright $result]
    return $result
}

proc xmlfix {stringToFix} {
    regsub -all -- "&" $stringToFix "\\&amp;" result
    regsub -all -- "<" $result "\\&lt;" result
    regsub -all -- "{" $result "\\&#123;" result
    regsub -all -- "}" $result "\\&#125;" result
    regsub -all -- "\\$" $result "\\&#36;" result
    #regsub -all -- "\"" $result "!quot" result
    regsub -all -- "\\\[" $result "\\&#91;" result
    regsub -all -- "\\\]" $result "\\&#93;" result
    regsub -all -- "\\\\" $result "\\&#92;" result
    set result [string trimright $result]
    return $result
}

#############

class Externalizer {

    public variable name ""
    public variable packageName ""
    public variable description ""
    
    public variable inlets 
    public variable outlets 
    public variable states 
    public variable inheritedStates 
    public variable activations 

    public variable timedomain 0 
    public variable interactivedraw 0
    public variable autoname
    public variable sdif
    public variable datasource 0

    public variable drawCodeTcl ""
    public variable drawCodeCpp ""

    public variable Copyright ""
    public variable Announcement ""
    public variable TclDialog ""
    public variable Constructor ""
    public variable PreConstructor ""
    public variable Destructor ""
    public variable Private ""
    public variable Initialize ""
    public variable Global ""
    public variable TclStartup ""
    public variable AdvancedFeatures {Copyright Announcement TclDialog Constructor PreConstructor Destructor Private Initialize Global TclStartup}
    public variable AdvancedTab1 {Constructor PreConstructor Destructor Private Global Initialize}
    public variable AdvancedTab2 {Copyright Announcement TclDialog TclStartup}
 
    public variable persistentStates ""
    public variable synchronousStates ""

    public variable SourceFiles
    public variable Libraries {}
    public variable Includes {}
    public variable FileFeatures {SourceFiles Libraries Includes}

    public variable OnTheWeb ""
    public variable Syntax "default"
    public variable Links {}
    public variable Author ""
    public variable LongDescription ""
    public variable HTMLIncludes {}
    public variable HTMLIcon ""
    public variable Keywords {}
    public variable HelpPatch ""
    public variable HelpPatchSupportFiles ""
    public variable filename ""
    public variable currentCompiler ""

    private variable dirty false
    
    private variable shell
    private variable win
    private variable xgroup inlets
    private variable _persistence false
    private variable _synchronous false

    private variable logFile2Externalizer {}


    private variable Preferences {OSWSourceDir OSWInstallDir OSWStlDir OSWTclInclude OSWTkInclude OSWLibs}
    private variable OSWSourceDir ""
    private variable OSWInstallDir ""
    private variable OSWStlDir ""
    private variable OSWTclInclude ""
    private variable OSWTkInclude ""
    private variable OSWLibs ""

    constructor {args} {
	eval configure $args

	LoadPreferences
	
	set shell [toplevel .exz]
	set win $shell

	if {$::NoGUI} {
	    wm withdraw $shell
	}
	wm title $shell "Externalizer"

	iwidgets::entryfield $win.name -labeltext "Name:" -labelpos w \
		-textvariable [scope name]
	iwidgets::entryfield $win.package -labeltext "Package:" -labelpos w \
		-textvariable [scope packageName]
	iwidgets::entryfield $win.desc -labeltext "Description:" -labelpos w \
		-textvariable [scope description]

	#iwidgets::LabedWidget::alignlabels $win.name $win.desc
	
	frame $win.flags
	checkbutton $win.flags.timedomain -text "Time Domain" \
		-variable [scope timedomain] -command [list $this SetTimeDomain]
	checkbutton $win.flags.interactive -text "Interactive Drawing" \
		-variable [scope interactivedraw] \
		-command [list $this SetInteractive]
	checkbutton $win.flags.autoname -text "Data Source" \
		-variable [scope datasource] \
		-command [list $this SetDataSource]

	pack $win.flags.timedomain $win.flags.interactive $win.flags.autoname \
		-side left
	
	frame $win.ios
	
	foreach group {inlets outlets states inheritedStates} \
		title {Inlets Outlets "State Variables" "Inherited Variables"} {
	    iwidgets::scrolledlistbox $win.ios.$group \
		    -vscrollmode dynamic -hscrollmode none \
		    -textbackground white \
		    -labeltext $title -labelpos nw \
		    -selectioncommand [list $this ShowState $group]
	    pack $win.ios.$group -side left -padx 5
	}

	label $win.slabel1
	label $win.slabel2


	iwidgets::buttonbox $win.bb
	$win.bb add Up -image _exz_arrowup -command [list $this DecrOrder]
	$win.bb add Down -image _exz_arrowdn -command [list $this IncrOrder]
	$win.bb add Insert -text "Insert" -command [list $this InsertVariable]
	$win.bb add Edit -text "Edit" -command [list $this EditVariable]
	$win.bb add Delete -text "Delete" -command [list $this DeleteVariable]

	iwidgets::labeledframe  $win.aframe -labelpos nw -labeltext "Activations"
	set cs [$win.aframe childsite]
	iwidgets::scrolledlistbox $cs.activations \
		-vscrollmode dynamic -hscrollmode none \
		-textbackground white 
	iwidgets::buttonbox $cs.bb -orient vertical
	$cs.bb add Insert -text "Insert" -command [list $this InsertActivation]
	$cs.bb add Edit -text "Edit" -command [list $this EditActivation]
	$cs.bb add Delete -text "Delete" -command [list $this DeleteActivation]
	pack $cs.activations $cs.bb -side left
	
	iwidgets::labeledframe $win.misc -labelpos nw -labeltext "Additional Features"
	set cs [$win.misc childsite]
	iwidgets::buttonbox $cs.bb -orient vertical
	$cs.bb add Graphics -text "Graphics" -command "$this GraphicsCode"
	$cs.bb add Doc -text "Documentation" -command "$this EditDoc"
	$cs.bb add HelpPatch -text "Help Patch" -command [code $this ChooseHelpPatch]
	$cs.bb add Advanced -text "Advanced" -command "$this Advanced"
	pack $cs.bb -fill x

	frame $win.menu -relief raised
	menubutton $win.menu.file -text "File" -menu $win.menu.file.m 
	menu $win.menu.file.m -tearoff false
	$win.menu.file.m add command -label "New" -command "$this New" \
		-accelerator "Ctrl N"
	$win.menu.file.m add command -label "Open" -command "$this Open" \
		-accelerator "Ctrl O"
	$win.menu.file.m add command -label "Save" -command "$this Save" \
		-accelerator "Ctrl S"
	$win.menu.file.m add command -label "Save As..." -command "$this SaveAs"
	$win.menu.file.m add separator
	$win.menu.file.m add command -label "Quit" -command "$this Quit" \
		-accelerator "Ctrl Q"

	bind $shell <Control-n> "$this New"
	bind $shell <Control-o> "$this Open"
	bind $shell <Control-s> "$this Save"
	bind $shell <Control-q> "$this Quit"

	menubutton $win.menu.compile -text "Compile" -menu $win.menu.compile.m 
	menu $win.menu.compile.m -tearoff false
	$win.menu.compile.m add command -label "Externalize!" -command "$this Compile" \
		-accelerator "F9"
	$win.menu.compile.m add command -label "Just make the files" -command "$this MakeFiles"
	$win.menu.compile.m add separator
	foreach compiler $externalizer::compilers {
	    $win.menu.compile.m add radiobutton -label [lindex $compiler 0] \
		    -value [lindex $compiler 1] \
		    -variable [scope currentCompiler]
	}
	set currentCompiler [lindex [lindex $externalizer::compilers 0] 1]
	$win.menu.compile.m add separator
	$win.menu.compile.m add command -label "Batch Externalize" \
		-command "$this StartBatchExternalize"
	
	bind $shell <F9> "$this Compile"

	menubutton $win.menu.options -text "Options" -menu $win.menu.options.m
	menu $win.menu.options.m -tearoff false
	$win.menu.options.m add command -label "Preferences..." -command "$this EditPreferences"
	
	pack $win.menu.file $win.menu.compile $win.menu.options -side left
	pack $win.menu -fill x
	pack $win.name $win.package $win.desc $win.flags $win.ios $win.slabel1 \
		$win.slabel2 $win.bb \
		-side top -fill x -expand yes

	pack $win.aframe -side left 
	pack $win.misc
	#$shell activate
	if {!$::NoGUI} {
	    wm deiconify $shell
	    bind $shell <Deactivate> {if {!$CalledFromOsw} {exit}}
	}
	InitXML
    }

    destructor {
	destroy $shell
    }


    public method SetTimeDomain {} {
	if {$timedomain} {
	    set inheritedStates(SampleRate) {Float "Sample Rate" 44100}
	    set inheritedStates(NumberOfSamples) \
		    {"UInteger" "Number of samples processed at one" 128}
	    UpdateStates inheritedStates SampleRate NumberOfSamples
	} else {
	    unset inheritedStates(SampleRate)
	    unset inheritedStates(NumberOfSamples)
	    UpdateStates inheritedStates
	}

    }

    public method SetInteractive {} {
	if {$interactivedraw} {
	    set inheritedStates(drawInfo) {TkDrawingInfo "" none}
	    set inheritedStates(widget) \
		    {string "Tk Widget Name" none}
	    UpdateStates inheritedStates drawInfo widget
	} else {
	    unset inheritedStates(drawInfo)
	    unset inheritedStates(widget)
	    UpdateStates inheritedStates
	}
    }

    public method SetDataSource {} {
	if {$datasource} {
	    set inheritedStates(src) {string "External Data Source" none}
	    UpdateStates inheritedStates src
	} else {
	    unset inheritedStates(src)
	    UpdateStates inheritedStates
	}
    }
    

    public method ShowState {agroup} {
	set xgroup $agroup
	set stateVar [$win.ios.$xgroup getcurselection]
	set stateInfo [lindex [array get $xgroup $stateVar] 1]
	$win.slabel1 configure -text "$stateVar ([lindex $stateInfo 0])"
	$win.slabel2 configure -text [lindex $stateInfo 1]
    }

    private method UpdateStates {group args} {

	set stateVars [$win.ios.$group get 0 end]
	$win.ios.$group delete 0 end

	foreach stateVar $stateVars {
	    set stateInfo [lindex [array get $group $stateVar] 1]
	    if {[llength $stateInfo] > 0} {
		$win.ios.$group insert end $stateVar
	    }
	}
	
	foreach stateVar $args {
	    $win.ios.$group insert end $stateVar
	}
    }

    private method ShowVariableDialog {varname vardesc vartype vardef group noinsert} {
	set ds [iwidgets::dialogshell .ds#auto]
	$ds add OK -text "OK" -command \
	  "$this SaveVariableDialog $ds $group $noinsert"
	$ds add Cancel -text "Cancel" -command "destroy $ds" 
	$ds default Cancel
	set dsite [$ds childsite]
	iwidgets::entryfield $dsite.name -labeltext "Name:" -labelpos w 
	iwidgets::entryfield $dsite.desc -labeltext "Description:" -labelpos w
	iwidgets::combobox $dsite.type -labeltext "Type:" -labelpos w
	iwidgets::entryfield $dsite.def -labeltext "Default:" -labelpos w

	if {$varname == "" || [lsearch $persistentStates $varname] == -1} {
	    set _persistence 0
	} else {
	    set _persistence 1
	}
	checkbutton $dsite.persist -text "Persistent" -variable [scope _persistence]
	eval $dsite.type insert list end $::externalizer::types
	set typeIndex [lsearch $::externalizer::types $vartype]
	if {$typeIndex != -1} {
	    $dsite.type selection set $typeIndex
	}
	pack $dsite.name $dsite.desc $dsite.type $dsite.def $dsite.persist -side top
	if {$group == "outlets"} {
	    if {$varname == "" || [lsearch $synchronousStates $varname] == -1} {
		set _synchronous 0
	    } else {
		set _synchronous 1
	    }
	    checkbutton $dsite.sync -text "Synchronous" -variable [scope _synchronous]
	    pack $dsite.sync -side top
	}
	$dsite.name insert 0 $varname
	$dsite.desc insert 0 $vardesc
	$dsite.def insert 0 $vardef
	
	$ds activate
	
    }

    public method SaveVariableDialog {ds group noinsert} {
	set dsite [$ds childsite]
	if {$group != "inlets" && [$dsite.type getcurselection] == "Changeable"} {
	    bgerror "Changeable only allowed for inlets"
	    return
	}
	set varname [$dsite.name get]
	array set $group [list [$dsite.name get] \
		[list [$dsite.type getcurselection] [$dsite.desc get] \
		[$dsite.def get]]]
	if {$noinsert} {
	    UpdateStates $group
	} else {
	    UpdateStates $group [$dsite.name get]
	}
	if {$_persistence} {
	    if {[lsearch $persistentStates $varname] == -1} {
		lappend persistentStates $varname
	    }
	} else {
	    if {[lsearch $persistentStates $varname] > -1} {
		ldelete persistentStates $varname
	    }
	}
	if {$_synchronous} {
	    if {[lsearch $synchronousStates $varname] == -1} {
		lappend synchronousStates $varname
	    }
	} else {
	    if {[lsearch $synchronousStates $varname] > -1} {
		ldelete synchronousStates $varname
	    }
	}
	destroy $ds
	set dirty true
    }

    public method InsertVariable {} {
	ShowVariableDialog "" "" "" "" $xgroup false
    }

    public method EditVariable {} {
	if {$xgroup == "inheritedStates"} {
	    return
	}
	set stateVar [$win.ios.$xgroup getcurselection]
	set stateInfo [lindex [array get $xgroup $stateVar] 1]
	ShowVariableDialog $stateVar [lindex $stateInfo 1] \
		[lindex $stateInfo 0] [lindex $stateInfo 2] $xgroup true
    }

    public method DeleteVariable {} {
	if {$xgroup == "inheritedStates"} {
	    return
	}
	unset [set xgroup]([$win.ios.$xgroup get active])
	UpdateStates $xgroup
	set dirty true
    }

    public method DecrOrder {} {
	set order [$win.ios.$xgroup curselection]
	set selection [$win.ios.$xgroup get $order]
	if {$order == 0} {
	    return
	}
	$win.ios.$xgroup selection clear 0 end
	$win.ios.$xgroup delete $order
	incr order -1
	$win.ios.$xgroup insert $order $selection 
	$win.ios.$xgroup selection set $order
	set dirty true
    }

    public method IncrOrder {} {
	set order [$win.ios.$xgroup curselection]
	if {$order == ""} {
	    return
	}
	set selection [$win.ios.$xgroup get $order]
	$win.ios.$xgroup selection clear 0 end
	$win.ios.$xgroup delete $order
	incr order 1
	$win.ios.$xgroup insert $order $selection 
	$win.ios.$xgroup selection set $order
	set dirty true
    }

    public method New {} {
	set name ""
	set description ""
	set packageName ""

	set Copyright ""

	set timedomain 0 
	set interactivedraw 0
	set datasource 0
	set autoname 0
	
	set drawCodeTcl ""
	set drawCodeCpp ""

	foreach feature $AdvancedFeatures {
	    set $feature ""
	}
	
	set filename ""

	foreach group {inlets outlets states inheritedStates} {
	    foreach elem [array names $group] {
		unset [set group]($elem)
	    }
	    UpdateStates $group
	}

	$win.name delete 0 end
	$win.desc delete 0 end

	foreach elem [array names activations] {
	    unset activations($elem)
	}
	
	set persistentStates {}
	set synchronousStates {} 

	UpdateActivations

	foreach elem [array names SourceFiles] {
	    unset SourceFiles($elem)
	}
	set Libraries {}
	set Includes {}

	set OnTheWeb ""
	set LongDescription ""
	set Links {}
	set Syntax ""
	set Author ""
	set HTMLIncludes {}
	set HTMLIcon ""
	set Keywords {}
	set HelpPatch ""
	set HelpPatchSupportFiles {}

	set dirty false
    }

    public method Save {} {
	if {$filename != ""} {
	    SaveToFile $filename
	} else {
	    SaveAs
	}
    }

    public method SaveAs {} {
	SaveToFile \
		[tk_getSaveFile -defaultextension ".osd" -initialdir [pwd] \
		-filetypes {{"Externalizer Description" {.osd}}} ]
    }

    private method SaveToFile {afilename} {
	set filename $afilename
	cd [file dirname $filename]
	set chn [open $filename w]
	puts $chn "<?xml version=\"1.0\" ?>"
	puts $chn "<osw-external name=\"[xmlfix $name]\" package=\"[xmlfix $packageName]\">"
	puts $chn "<description>[xmlfix $description]</description>"
	puts $chn "<copyright>$Copyright</copyright>"

	if {$timedomain} {
	    puts $chn "<inherit class=\"timedomain\" />"
	}
	if {$interactivedraw} {
	    puts $chn "<inherit class=\"interactivedraw\" />"
	}
	if {$datasource} {
	    puts $chn "<inherit class=\"datasource\" />"
	}

	foreach elem [$win.ios.inlets get 0 end] {
	    set type [lindex $inlets($elem) 0]
	    set desc [lindex $inlets($elem) 1]
	    set def [lindex $inlets($elem) 2]
	    set persistent [lsearch $persistentStates $elem]
	    set tag "<inlet name=\"[xmlfix $elem]\" type=\"[xmlfix $type]\" "
	    if {$def != ""} {
		regsub -all -- "\"" $def "!quot" def
		append tag "default=\"[xmlfix $def]\" "
	    }
	    if {$desc != ""} {
		regsub -all -- "\"" $desc "!quot" desc
		append tag "description=\"[xmlfix $desc]\" "
	    }
	    if {$persistent >= 0} {
		append tag "persistent=\"1\" "
	    }
	    append tag "/>"
	    puts $chn $tag
	}

	foreach elem [$win.ios.outlets get 0 end] { 
	    set type [lindex $outlets($elem) 0]
	    set desc [lindex $outlets($elem) 1]
	    set def [lindex $outlets($elem) 2]
	    set persistent [lsearch $persistentStates $elem]
	    set tag "<outlet name=\"$elem\" type=\"$type\" "
	    if {$def != ""} {
		regsub -all -- "\"" $def "!quot" def
		append tag "default=\"[xmlfix $def]\" "
	    }
	    if {$desc != ""} {
		regsub -all -- "\"" $desc "!quot" desc
		append tag "description=\"[xmlfix $desc]\" "
	    }
	    if {$persistent >= 0} {
		append tag "persistent=\"1\" "
	    }
	    set sync [lsearch $synchronousStates $elem]
	    if {$sync >= 0} {
		append tag "synchronous=\"1\" "
	    }
	    append tag "/>"
	    puts $chn $tag
	}

	foreach elem [$win.ios.states get 0 end] {
	    set type [lindex $states($elem) 0]
	    set desc [lindex $states($elem) 1]
	    set def [lindex $states($elem) 2]
	    set persistent [lsearch $persistentStates $elem]
	    set tag "<state name=\"$elem\" type=\"$type\" "
	    if {$def != ""} {
		regsub -all -- "\"" $def "!quot" def
		append tag "default=\"[xmlfix $def]\" "
	    }
	    if {$desc != ""} {
		regsub -all -- "\"" $desc "!quot" desc
		append tag "description=\"[xmlfix $desc]\" "
	    }
	    if {$persistent >= 0} {
		append tag "persistent=\"1\" "
	    }
	    append tag "/>"
	    puts $chn $tag
	}

	foreach elem [$win.ios.inheritedStates get 0 end] {
	    set type [lindex $inheritedStates($elem) 0]
	    set desc [lindex $inheritedStates($elem) 1]
	    set def [lindex $inheritedStates($elem) 2]
	    set persistent [lsearch $persistentStates $elem]
	    set tag "<inherited-state name=\"$elem\" type=\"$type\" "
	    if {$def != ""} {
		append tag "default=\"$def\" "
	    }
	    if {$desc != ""} {
		append tag "description=\"[xmlfix $desc]\" "
	    }
	    if {$persistent >= 0} {
		append tag "persistent=\"1\" "
	    }
	    append tag "/>"
	    puts $chn $tag
	}

	puts $chn "<xblock name=\"drawCodeTcl\">[xmlfix $drawCodeTcl]</xblock>"
	puts $chn "<xblock name=\"drawCodeCpp\">[xmlfix $drawCodeCpp]</xblock>"

	foreach feature $AdvancedFeatures {
	    if {$feature != "Copyright"} {
		puts $chn "<xblock name=\"$feature\">[xmlfix [set $feature]]</xblock>"
	    }
	}

	foreach elem [array names activations] {
	    puts $chn "<activation name=\"$elem\" activators=\"[lindex $activations($elem) 1]\" order=\"[lindex $activations($elem) 3]\" >"
	    puts $chn [xmlfix [lindex $activations($elem) 2]]
	    puts $chn "</activation>"	
	}

	puts $chn "<xblock name=\"SourceFiles\">"
	foreach elem [array names SourceFiles] {
	    puts $chn [xmlfix "\{ $elem $SourceFiles($elem) \}"]
	}
	puts $chn "</xblock>"
	puts $chn "<xblock name=\"Libraries\">[xmlfix $Libraries]</xblock>"
	puts $chn "<xblock name=\"Includes\">[xmlfix $Includes]</xblock>"
	puts $chn "<xblock name=\"Syntax\">[xmlfix $Syntax]</xblock>"
	puts $chn "<xblock name=\"LongDescription\">[xmlfix $LongDescription]</xblock>"
	puts $chn "<xblock name=\"Links\">[xmlfix $Links]</xblock>"
	puts $chn "<xblock name=\"Author\">[xmlfix $Author]</xblock>"
	puts $chn "<xblock name=\"OnTheWeb\">[xmlfix $OnTheWeb]</xblock>"
	puts $chn "<xblock name=\"HTMLIncludes\">[xmlfix $HTMLIncludes]</xblock>"
	puts $chn "<xblock name=\"Keywords\">[xmlfix $Keywords]</xblock>"
	puts $chn "<xblock name=\"HTMLIcon\">[xmlfix $HTMLIcon]</xblock>"
	puts $chn "<xblock name=\"HelpPatch\">[xmlfix $HelpPatch]</xblock>"
	if {[llength $HelpPatchSupportFiles] > 0} {
	    puts $chn "<xblock name=\"HelpPatchSupportFiles\">[xmlfix $HelpPatchSupportFiles]</xblock>"
	}
	puts $chn "</osw-external>"
	close $chn
    }

    private method OldSaveToFile {afilename} {
	set filename $afilename
	cd [file dirname $filename]
	set chn [open $filename w]
	
	foreach prop {name packageName description timedomain interactivedraw \
		drawCodeTcl drawCodeCpp datasource} {
	    puts $chn "set $prop \{[set $prop]\}"
	}

	foreach feature $AdvancedFeatures {
	    puts $chn "set $feature \{[set $feature]\}"
	}

	foreach group {inlets outlets states inheritedStates} {
	    foreach elem [$win.ios.$group get 0 end] {
		puts $chn "set $group\([set elem]\) \{ [set [set group]($elem)] \}"
	    }
	    puts $chn "eval UpdateStates $group [ $win.ios.$group get 0 end ]"

	}

	foreach elem [array names activations] {
	    puts $chn "set activations($elem) \{ $activations($elem) \}"

	}
	puts $chn "UpdateActivations"

	set fixedPersistentStates {}
	#foreach s {$persistentStates} {
	#    if {[lsearch [array names inlets] $s] >= 0 || \
	#	    [lsearch [array names outlets] $s] >= 0 || \
	#	    [lsearch [array names states] $s] >= 0} {
	#	lappend fixedPersistentStates $s
	#    }
	#}
	#set persistentStates $fixedPersistentStates

	puts $chn "set persistentStates \{ $persistentStates \}"

	foreach elem [array names SourceFiles] {
	    puts $chn "set SourceFiles($elem) \{ $SourceFiles($elem) \}"
	}
	puts $chn "set Libraries \{ $Libraries \}"
	puts $chn "set Includes \{ $Includes \}"
	puts $chn "set Syntax \"$Syntax\""
	puts $chn "set LongDescription \{ $LongDescription \}"
	puts $chn "set Links \{ $Links \}"
	puts $chn "set Author \"$Author\""
	puts $chn "set OnTheWeb \"$OnTheWeb\""
	puts $chn "set HTMLIncludes \{ $HTMLIncludes \}"
	puts $chn "set Keywords \{ $Keywords \}"
	puts $chn "set HTMLIcon \"$HTMLIcon\""

	puts $chn "set HelpPatch {$HelpPatch}"

	close $chn
	set dirty false
    }

    public method Open {} {
	set afilename [tk_getOpenFile -defaultextension "osd" -initialdir [pwd] \
		-filetypes {{"Externalizer Description" {.osd}}} ]
	if {$afilename == ""} {
	    return
	}
	if {[file exists $afilename]} {
	    LoadFromFile $afilename
	} else {
	    error "Cannot open file $afilename"
	}
    }
    
    private variable _newhelppatch 0

    public method LoadFromFile {afilename} {
	set _newhelppatch 0
	cd [file dirname $afilename]
	New
	set chn [open $afilename r]
	set firstline [gets $chn]
	close $chn
	if {[regexp "\\?xml" $firstline]} {
	    ReadFromXML $afilename
	} else {
	    source $afilename
	}
	#legacy issue for older versions (one of many :P )
	if {![catch {set publicStates}]} {
	    set persistentStates $publicStates
	}
	if {$HelpPatch == ""} {
	    if {[file exists $::env(OSW_DIR)/help_patch_builds/$packageName/help_${name}.osw]} {
		LoadHelpPatch $::env(OSW_DIR)/help_patch_builds/$packageName/help_${name}.osw
		set _newhelppatch 1
	    }
	} else {
	    set helpbuild $::env(OSW_DIR)/help_patch_builds/$packageName/help_${name}.osw
	    if {[file exists $helpbuild] && [file mtime $helpbuild] > [file mtime $afilename]} {
		LoadHelpPatch $helpbuild
		set _newhelppatch 1
	    }
	}
	set filename $afilename
	set dirty false
    }

    public method Quit {} {
	delete object $this
	exit	
    }

    private method UpdateActivations {} {
	set cs [$win.aframe childsite]
	$cs.activations delete 0 end
	if {[array size activations] > 0} {
	    eval $cs.activations insert end [array names activations]
	}	
    }

    private method ShowActivationDialog {activation unused astates aexpr order} {
	set ds [iwidgets::dialogshell .ds#auto -title $activation]
	$ds add OK -text "OK" -command "$this SaveActivationDialog $ds"
	$ds add Cancel -text "Cancel" -command "destroy $ds" 
	set dsite [$ds childsite]
	iwidgets::entryfield $dsite.name -labeltext "Name:" -labelpos w 
	iwidgets::scrolledlistbox $dsite.activators \
		-vscrollmode dynamic -hscrollmode none \
		-textbackground white \
		-height 50 -width 90 \
		-labeltext "Depends on:" -labelpos nw -selectmode extended
	iwidgets::entryfield $dsite.order -labeltext "Order:" -labelpos w
	iwidgets::scrolledtext $dsite.expr -labeltext "Expression:" \
		-labelpos nw -height 80 -width 450 -textfont courier
	
	pack $dsite.expr -side bottom -fill both -expand yes
	pack $dsite.activators -side right
	pack $dsite.name $dsite.order -side top -fill x 
	$dsite.name insert 0 $activation
	$dsite.order insert 0 $order
	set activatorList [concat [array names inlets] \
		[array names states] [array names inheritedStates]]
	if {[llength $activatorList] > 0} {
	    eval $dsite.activators insert end $activatorList
	}
	foreach activator $astates {
	    $dsite.activators selection set [lsearch $activatorList $activator]
	}
	$dsite.expr insert end $aexpr
	bind $ds <Return> {}
	$ds activate
    }


    public method SaveActivationDialog {ds} {
	set dsite [$ds childsite]
	array set activations [list [$dsite.name get] [list unused \
		[$dsite.activators getcurselection] \
		[$dsite.expr get 0.0 end] \
		[$dsite.order get]]]

	UpdateActivations
	destroy $ds
	set dirty true
    }


    public method InsertActivation {} {
	ShowActivationDialog "" "" "" "" 0
    }

    public method EditActivation {} {
	set cs [$win.aframe childsite]
	set toEdit [$cs.activations getcurselection]
	ShowActivationDialog $toEdit [lindex $activations($toEdit) 0] \
		[lindex $activations($toEdit) 1] \
		[lindex $activations($toEdit) 2] \
		[lindex $activations($toEdit) 3]
    }


    public method DeleteActivation {} {
	unset activations([[$win.aframe childsite].activations getcurselection])
	UpdateActivations
	set dirty true
    }

    public method GraphicsCode {} {
	set ds [iwidgets::dialogshell .ds#auto \
		-title "$name Graphics"]
	$ds add OK -text "OK" -command "$this SaveGraphicsCode $ds"
	$ds add Cancel -text "Cancel" -command "destroy $ds" 
	set dsite [$ds childsite]
	iwidgets::scrolledtext $dsite.tcl -labeltext "Tcl graphics code:" \
		-labelpos nw -height 300 -width 600 -textfont courier
	iwidgets::scrolledtext $dsite.cpp -labeltext "C++ graphics code:" \
		-labelpos nw -height 250 -width 600 -textfont courier
	label $dsite.l -text "NOTE: C++ code will be executed before Tcl code."
	pack $dsite.cpp $dsite.tcl $dsite.l -fill x
	bind $ds <Return> {}
	$dsite.tcl insert end $drawCodeTcl
	$dsite.cpp insert end $drawCodeCpp

	$ds activate
    }

    public method SaveGraphicsCode {ds} {
	set dsite [$ds childsite]	
	set drawCodeTcl [$dsite.tcl get 0.0 end]
	set drawCodeCpp [$dsite.cpp get 0.0 end]
	set dirty true
	destroy $ds
    }

    public method EditDoc {} {
	set ds [iwidgets::dialogshell .ds#auto \
		-title "$name Graphics"]
	$ds add OK -text "OK" -command "$this SaveDoc $ds"
	$ds add Cancel -text "Cancel" -command "destroy $ds" 
	set dsite [$ds childsite]
	iwidgets::entryfield $dsite.syntax -labeltext "Syntax:" -labelpos nw
	iwidgets::scrolledtext $dsite.desc -labeltext "Comments:" \
		-labelpos nw -height 300 -width 500
	iwidgets::entryfield $dsite.links -labeltext "Links:" -labelpos nw
	iwidgets::entryfield $dsite.keyword -labeltext "Keywords:" -labelpos nw
	iwidgets::entryfield $dsite.author -labeltext "Author Info:" -labelpos nw

	iwidgets::entryfield $dsite.webpage -labeltext "Web page:" -labelpos nw
	frame $dsite.icon
	iwidgets::entryfield $dsite.icon.fname -labeltext "Include graphic from file: " -labelpos w
	button $dsite.icon.b -text "Choose File..." -command [code $this ChooseIconFile $ds]
	label $dsite.icon.icon 
	pack $dsite.icon.fname $dsite.icon.b $dsite.icon.icon -side left -fill x

	pack $dsite.syntax $dsite.desc $dsite.links $dsite.keyword -fill x
	pack $dsite.author $dsite.icon $dsite.webpage -fill x

	bind $ds <Return> {}
	if {$Syntax == "default" || $Syntax == ""} {
	    #if {$autoname} {
		set Syntax "$name <i>options</i>"
	    #} else {
	    #	set Syntax "$name <i>name options</i>"
	    #}
	}
	$dsite.syntax insert end $Syntax
	$dsite.desc insert end $LongDescription
	$dsite.links insert end $Links
	$dsite.keyword insert end $Keywords
	$dsite.webpage insert end $OnTheWeb
	$dsite.icon.fname insert end [file tail $HTMLIcon]
	$ds activate
    }

    public method SaveDoc {ds} {
	set dsite [$ds childsite]	
	set Syntax [$dsite.syntax get]
	set LongDescription [$dsite.desc get 0.0 end]
	set Links [$dsite.links get]
	set Author [$dsite.author get]
	set OnTheWeb [$dsite.webpage get]
	set Keywords [$dsite.keyword get]
	set dirty true
	destroy $ds
    }

    private method ChooseIconFile {ds} {
	set dsite [$ds childsite]	
	set newfile [tk_getOpenFile \
		-defaultextension "gif" -initialdir [pwd] \
		-filetypes {{"GIF image" {.gif}} {"JPEG image" {.jpg}}} ]
	if {$newfile != ""} {
	    set HTMLIcon $newfile
	    $dsite.icon.fname delete 0 end	    
	    $dsite.icon.fname insert 0 [file tail $newfile]
	    if {[$dsite.icon.icon cget -image] != ""} {
		set toDelete [$dsite.icon.icon cget -image]
		$dsite.icon.icon configure -image ""
		image delete $toDelete
	    }
	    $dsite.icon.icon configure -image \
		    [image create photo -file $newfile]
	}
    }

    private method ChooseHelpPatch {} {
	set newfile  [tk_getOpenFile \
		-defaultextension "osw" -initialdir [pwd] \
		-filetypes {{"OSW Patch" {.osw}}}]	
	if {$newfile != ""} {
	    LoadHelpPatch $newfile
	}
    }

    private method LoadHelpPatch {newfile} {	
	set chn [open $newfile "r"]
	set HelpPatch ""
	set newfiledir [file dirname $newfile]
	while {![eof $chn]} {
	    set line [gets $chn]
	    append HelpPatch $line
	    append HelpPatch "\n"
	    if {[regexp "src=\"(\[^\"\]+)" $line unused f]} {
		lappend HelpPatchSupportFiles $f
		if {$newfiledir != [pwd]} {
		    if {[regexp "^\[A-Z\]\:" $f] || [regexp "^/" $f]} {
			file copy -force $f .
		    } else {
			file copy -force $newfiledir/$f .
		    }
		}
	    }
	    if {[regexp "<osw" $line unused]} {
		append HelpPatch "<!-- Generated by OSW Externalizer -->\n"
	    }
	    if {[regexp "<patch" $line unused]} {
		append HelpPatch "<!-- Copyright Message -->\n"
	    }
	}
    }

    public method Advanced {} {
	set ds [iwidgets::dialogshell .ds#auto \
		-title "$name Advanced Features"]
	$ds add OK -text "OK" -command "$this SaveAdvanced $ds"
	$ds add Cancel -text "Cancel" -command "destroy $ds" 
	set dsite [$ds childsite]
	iwidgets::tabset $dsite.ts -tabpos n -font {"MS Sans Serif" 8} \
		-command "$this ShowAdvanced $ds 1 "
	iwidgets::tabset $dsite.ts2 -tabpos n -font {"MS Sans Serif" 8} \
		-command "$this ShowAdvanced $ds 2"

	foreach advancedFeature $AdvancedTab1 {
	    $dsite.ts add -label $advancedFeature
	    iwidgets::scrolledtext $dsite.x$advancedFeature \
		    -labeltext "$advancedFeature :" \
		    -labelpos nw -height 300 -width 600 -textfont courier
	    $dsite.x$advancedFeature insert end [set [set advancedFeature]]
	}

	foreach advancedFeature $AdvancedTab2 {
	    $dsite.ts2 add -label $advancedFeature
	    iwidgets::scrolledtext $dsite.x$advancedFeature \
		    -labeltext "$advancedFeature :" \
		    -labelpos nw -height 300 -width 600 -textfont courier
	    $dsite.x$advancedFeature insert end [set [set advancedFeature]]
	}


	#additional files
	foreach ff $FileFeatures {
	    $dsite.ts2 add -label $ff
	    frame $dsite.x$ff
	    if {$ff == "SourceFiles"} {
		#options for source code files
		iwidgets::entryfield $dsite.xSourceFiles.dep \
			-labeltext "Dependencies:" -labelpos nw
		iwidgets::entryfield $dsite.xSourceFiles.opt \
			-labeltext "Compile Options:" -labelpos nw
		button $dsite.xSourceFiles.set -text "Set" -state disabled \
			-command "$this SetSourceFileOptions $dsite"
		pack $dsite.xSourceFiles.set -side bottom
		pack $dsite.xSourceFiles.opt $dsite.xSourceFiles.dep -side bottom -fill x
		bind [$dsite.xSourceFiles.opt component entry] <Key> \
			"$dsite.xSourceFiles.set configure -state active"
		bind [$dsite.xSourceFiles.dep component entry] <Key> \
			"$dsite.xSourceFiles.set configure -state active"

	    }
	    iwidgets::scrolledlistbox $dsite.x$ff.lb \
		    -vscrollmode dynamic -hscrollmode none \
		    -textbackground white \
		    -selectioncommand [list $this Show$ff $dsite]		    
	    iwidgets::buttonbox $dsite.x$ff.bb -orient vertical
	    $dsite.x$ff.bb add Insert -text "Insert" -command [list $this Insert$ff $dsite]
	    $dsite.x$ff.bb add Delete -text "Delete" -command [list $this Delete$ff $dsite]
	    pack $dsite.x$ff.lb $dsite.x$ff.bb -side left
	    switch $ff SourceFiles {
		catch {eval $dsite.xSourceFiles.lb insert end [array names SourceFiles]}
	    } Libraries {
		catch {eval $dsite.xLibraries.lb insert end $Libraries}
	    }
	}

	pack $dsite.ts $dsite.ts2 $dsite.xTclDialog -side top -fill x
	bind $ds <Return> {}
	$ds activate
	return $ds
    }

    public method ShowAdvanced {ds tabmajor tabminor} {
	set dsite [$ds childsite]	
	foreach af $AdvancedFeatures {
	    pack forget $dsite.x$af	    
	}
	foreach ff $FileFeatures {
	    pack forget $dsite.x$ff	    
	}
	set tabset AdvancedTab$tabmajor
	if {$tabminor >= [llength [set $tabset]]} {
	    set tab [lindex $FileFeatures [expr $tabminor - [llength [set $tabset]]]]
	} else {
	    set tab [lindex [set $tabset] $tabminor]
	}

	pack $dsite.x$tab -after $dsite.ts2 -fill x
    }

    public method ShowAdvancedQuick {tab} {
	set tabminor [lsearch $AdvancedTab1 $tab]
	if {$tabminor > -1} {
	    ShowAdvanced [Advanced] 1 $tabminor
	} else {
	    set tabminor [lsearch $AdvancedTab2 $tab]
	    if {$tabminor > -1} {
		ShowAdvanced [Advanced] 2 $tabminor
	    } else {
		error "Can't find feature $tab"
	    }
	}
    }

    public method SaveAdvanced {ds} {
	set dsite [$ds childsite]	
	foreach af $AdvancedFeatures {
	    set $af [$dsite.x$af get 0.0 end]
	}
	set dirty true
	destroy $ds
    }

    public method InsertSourceFiles {dsite} {
	set newsource [tk_getOpenFile -initialdir [pwd] \
		-filetypes {{"C/C++ Source Files" {.c .cpp .cxx .cc .c++ .C}}}]
	$dsite.xSourceFiles.lb insert end [file tail $newsource]
	if {$newsource == ""} {
	    return
	}
	set SourceFiles([file tail $newsource]) {}
	$dsite.xSourceFiles.lb selection set end
	CopyFile $newsource
	set dirty true
    }

    public method InsertLibraries {dsite} {
	set newsource [tk_getOpenFile -initialdir [pwd] \
		-filetypes {{"Library files" {.lib .a}} {"Object files" {.obj .o}}}]
	if {$newsource == ""} {
	    return
	}
	$dsite.xLibraries.lb insert end [file tail $newsource]
	lappend Libraries [file tail $newsource]
	$dsite.xLibraries.lb selection set end
	CopyFile $newsource
	set dirty true
	CopyFile $newsource
    }

    public method InsertIncludes {dsite} {
	set newsource [tk_getOpenFile -initialdir [pwd] \
		-filetypes {{"Include files" {.h}}}]
	if {$newsource == ""} {
	    return
	}
	$dsite.xIncludes.lb insert end [file tail $newsource]
	lappend Includes $newsource
	$dsite.xIncludes.lb selection set end
	set dirty true
	CopyFile $newsource
    }

    public method DeleteSourceFiles {dsite} {
	set toDelete [$dsite.xSourceFiles.lb getcurselection]
	$dsite.xSourceFiles.lb delete 0 end
	foreach sf [array names SourceFiles] {
	    if {$sf != $toDelete} {
		$dsite.xSourceFiles.lb insert end $sf
	    } else {
		unset SourceFiles($sf)
	    }
	}
	set dirty true
	CopyFile $newsource
    }

    public method DeleteLibraries {dsite} {
	set toDelete [$dsite.xLibraries.lb getcurselection]
	$dsite.xLibraries.lb delete 0 end
	foreach lib $Libraries {
	    if {$lib != $toDelete} {
		$dsite.xLibraries.lb insert end $lib
	    } 
	}
	ldelete Libraries $toDelete
    }

    public method IncludeLibraries {dsite} {
	set toDelete [$dsite.xIncludes.lb getcurselection]
	$dsite.xLibraries.lb delete 0 end
	foreach lib $Includes {
	    if {$lib != $toDelete} {
		$dsite.xIncludes.lb insert end $lib
	    } 
	}
	ldelete Includes $toDelete
	set dirty true
    }

    public method ShowSourceFiles {dsite} {
	catch {
	    set sourcefile [$dsite.xSourceFiles.lb getcurselection]
	    $dsite.xSourceFiles.dep clear
	    $dsite.xSourceFiles.opt clear
	    $dsite.xSourceFiles.dep insert end [lindex $SourceFiles($sourcefile) 0]
	    $dsite.xSourceFiles.opt insert end [lindex $SourceFiles($sourcefile) 1]
	    $dsite.xSourceFiles.set configure -state disabled
	}
	set dirty true
    }
 
    public method ShowLibraries {dsite} {
    }

    public method SetSourceFileOptions {dsite} {
	set sourcefile [$dsite.xSourceFiles.lb getcurselection]
	set SourceFiles($sourcefile) [list \
		[$dsite.xSourceFiles.dep get] \
		[$dsite.xSourceFiles.opt get]]
	$dsite.xSourceFiles.dep clear
	$dsite.xSourceFiles.opt clear
	$dsite.xSourceFiles.set configure -state disabled
    }


    public method CopyFile {fileToCopy} {
	if {$filename == ""} {
	    tk_messageBox -type ok -title "Need to save" -message "You need to save your transform specification at least once before using this feature."
	    SaveAs
	}
	
	set dirname [file dirname $filename]
	if {![file exists $dirname/[file tail $fileToCopy]]} {
	    file copy $fileToCopy $dirname
	}
    }


    public method SaveInteractive {ds} {
	set persistentStates [[$ds childsite].states getcurselection]
	destroy $ds
    }

    
    public method Externalize {} {
	set ds [iwidgets::dialogshell .ds#auto \
		-title "$name Interactive States"]
	$ds add OK -text "OK" -command "$this SaveInteractive $ds"
	$ds add Cancel -text "Cancel" -command "destroy $ds" 
	set dsite [$ds childsite]		
    }

    private variable xerrors false
    private variable xbatchmode false
    private variable xshouldquit false

    public method Compile {args} {
	global env
	
	if {!$::NoGUI} {
	    set ds [iwidgets::dialogshell .ds#auto \
			-title "Externalizing $name"]
	    set dsite [$ds childsite]
	    label $dsite.status -text "Creating code files..."
	    iwidgets::feedback $dsite.fb -steps 4
	    pack $dsite.status $dsite.fb -fill x -side top
	    $ds add OK -text "OK" -command "destroy $ds"
	    $ds add View -text "View Log" -command "destroy $ds ; $this ViewLog" -state disabled
	    
	    $ds activate
	} else {
	    puts "Externalizing $name"
	    puts "Creating code files..."
	}
	set xerrors false

	MakeCPP

	if {!$::NoGUI} { 
	    $dsite.fb step
	}

	MakeMake
	if {![file exists $env(OSW_DIR)/externals/$packageName]} {
	    file mkdir $env(OSW_DIR)/externals/$packageName
	}

	if {!$::NoGUI} {
	    $dsite.fb step
	    $dsite.status configure -text "Compiling..."
	    update
	} else {
	    puts "Compiling..."
	}
	
	set result [::externalizer::[set currentCompiler]Compile $name]
	
	if {!$::NoGUI} {
	    $dsite.fb step
	    $dsite.status configure -text "Generating HTML Help..."
	    update
	} else {
	    puts "Generating HTML Help..."
	}
 
	MakeHTML
	CopyHTML

	if {$HelpPatch != ""} {
	    if {!$::NoGUI} {
		$dsite.status configure -text "Generating Help Patch..."
		update
	    } else {
		puts "Generating Help Patch..."
	    }
	    MakeHelpPatch
	    CopyHelpPatch
	}
	
	if {$::NoGUI} {
	    if {!$result} {
		puts "Success!"
	    } else {
		puts "There were errors"
		set log [open $name.log "r"]
		while {![eof $log]} {
		    puts [gets $log]
		}
		set xerrors true
	    }
	} else {
	    $dsite.fb step
	    $ds buttonconfigure OK -state normal
	    $ds buttonconfigure View -state normal
	    
	    if {!$result} {
		$dsite.status configure -text "Success!"
		if {$xbatchmode} {
		    destroy $ds
		}
	    } else {
		$dsite.status configure -text "There were errors"
		set xerrors true
	    }
	}
    }


    public method MakeFiles {} {
	MakeCPP
	MakeMake
    }

    public method MakeCPP {}
    public method MakeMake {}
    public method MakeHTML {}

    private method FixActivations {activationExpr}


    public method CopyHTML {} {
	global env
	if {![file exists $env(OSW_DIR)/html/$packageName]} {
	    file mkdir $env(OSW_DIR)/html/$packageName
	}
	file copy -force $name.html $env(OSW_DIR)/html/$packageName
	if {$HTMLIcon != ""} {
	    file copy -force $HTMLIcon $env(OSW_DIR)/html/$packageName
	}
    }
    
    public method MakeHelpPatch {}
    public method CopyHelpPatch {} {
	global env
	if {![file exists $env(OSW_DIR)/help_patches]} {
	    file mkdir $env(OSW_DIR)/help_patches
	}
	if {![file exists $env(OSW_DIR)/help_patches/$packageName]} {
	    file mkdir $env(OSW_DIR)/help_patches/$packageName
	}
	file copy -force help_${name}.osw $env(OSW_DIR)/help_patches/$packageName
	foreach f $HelpPatchSupportFiles {
	    file copy -force $f $env(OSW_DIR)/help_patches/$packageName
	}
    }

    public method ViewLog {} {
    	set ds [iwidgets::dialogshell .ds#auto \
		-title "Messages for $name"]
	set dsite [$ds childsite]
	iwidgets::scrolledtext $dsite.msg -height 300 -width 600 \
		-textfont courier
	$ds add Dismess -text "Dismiss" -command "destroy $ds"
	pack $dsite.msg -fill both -expand yes
	bind [$dsite.msg component text] <Double-Button-1> [list $this ParseErrorMessage $dsite.msg %x %y]
	
	
	set logtext {}
	set log [open $name.log "r"]
	while {![eof $log]} {
	    lappend logtext [gets $log]
	}
	$dsite.msg insert end [join $logtext "\n"]
	#$dsite.msg configure -state disabled
	$ds activate
    }

    public method ParseErrorMessage {w x y} {
	set linenum [::externalizer::[set currentCompiler]ParseErrorMsg \
		[$w get "@$x,$y linestart" "@$x,$y lineend"]]
	if {$linenum > 0} {
	    set foundLink {}
	    set prevLink {}
	    foreach link $logFile2Externalizer {
		if {[lindex $link 0] > $linenum} {
		    set foundLink $prevLink
		    break
		}
		set prevLink $link
	    }
	    if {[llength $foundLink] > 0} {
		eval [lindex $foundLink 1]
	    }
	}
	    
    }

    private method MakeLinkToVariable {linenum group varname} {
	set varRecord [lindex [array get $group $varname] 1]
	lappend logFile2Externalizer [list $linenum \
		[list ShowVariableDialog $varname \
		[lindex $varRecord 1] \
		[lindex $varRecord 0] \
		[lindex $varRecord 2] $group]]
    }

    private method MakeLinkToActivation {linenum activation} {
	lappend logFile2Externalizer [list $linenum \
		[list ShowActivationDialog $activation \
		[lindex $activations($activation) 0] \
		[lindex $activations($activation) 1] \
		[lindex $activations($activation) 2]]]
    }

    private method MakeLinkToMainWindow {linenum entry} {
	lappend logFile2Externalizer [list $linenum \
		"raise $shell ; focus $win.$entry"]
    }


    public method StartBatchExternalize {}
    public method BatchExternalize {startdir}

    public method LoadPreferences {} {
	set chn [open $::env(OSW_DIR)/externalizer/preferences.txt r]
	if {$::tcl_platform(platform) == "windows"} {
	    while {![eof $chn]} {
		set pref [gets $chn]
		regsub -all "\\\\" $pref "\\\\\\\\" pref
		set [lindex $pref 0] [lindex $pref 1]
	    }
	} else {
	    while {![eof $chn]} {
		set pref [gets $chn]
		set [lindex $pref 0] [lindex $pref 1]
	    }
	}
	close $chn
	settypes $OSWSourceDir
    }

    public method SavePreferences {} {
	set chn [open $::env(OSW_DIR)/externalizer/preferences.txt w]
	foreach prefvar $Preferences {
	    set pref [list $prefvar [set $prefvar]]
	    puts $chn $pref
	}
	close $chn
	settypes $OSWSourceDir
    }

    public method EditPreferences {} {
	puts Hi
	set ds [iwidgets::dialogshell .ds#auto \
		-title "Preferences" ]
	$ds add OK -text "OK" -command "$this SavePreferences ; destroy $ds"
	$ds add Cancel -text "Cancel" -command "destroy $ds" 
	$ds default Cancel
	set dsite [$ds childsite]

	foreach prefvar $Preferences {
	    puts $prefvar
	    iwidgets::entryfield $dsite.x$prefvar -labeltext $prefvar -labelpos w \
		-textvariable [scope $prefvar]
	    pack $dsite.x$prefvar -side top
	}
	$ds activate
    }


    private variable xmlstate {}
    private variable xmlparser
    private variable orderedInlets {}
    private variable orderedOutlets {}
    private variable orderedStates {}
    private variable orderedInheritedStates {}

    private method GetXMLStateName {} {
    	return [lindex [lindex $xmlstate end] 0]
    }
    
    private method GetXMLStateData {} {
	return [lindex [lindex $xmlstate end] 1]
    }
    
    private method PushXMLState {name data} {
	lappend xmlstate [list $name $data]
    }
    
    private method PopXMLState {} {
	set xmlstate [lrange $xmlstate 0 [expr [llength $xmlstate] - 2]]
    }
    
    private method ClearXMLState {} {
	set xmlstate {}
    }    

    private method InitXML {} {
	set xmlparser [::xml::parser]
	$xmlparser configure -elementstartcommand [code $this ElementStart] \
		-elementendcommand [code $this ElementEnd] \
		-characterdatacommand [code $this CharacterData] \
		-reportempty 1	
    }
    
    private method ElementStart {tagname attList args} {
	
	switch $tagname osw-external {
	    foreach {attname value} $attList {
		if {$attname == "package"} {
		    set packageName $value
		} else {
		    set $attname $value
		}
	    }
	} description {
	    PushXMLState description ""
	} "inherit" {
	    foreach {attname value} $attList {
		if {$attname == "class"} {
		    set $value 1
		}
	    }
	} inlet {
	    set iname ""
	    set itype ""
	    set idescription ""
	    set idefault ""
	    foreach {attname value} $attList {
		if {$attname == "persistent"} {
		    lappend persistentStates $iname
		} else {
		    set i${attname} $value		    
		}
	    }
	    regsub -all -- "!quot" $idefault "\"" idefault
	    set inlets($iname) [list $itype $idescription $idefault]
	    lappend orderedInlets $iname
	} outlet {
	    set iname ""
	    set itype ""
	    set idescription ""
	    set idefault ""
	    foreach {attname value} $attList {
		if {$attname == "persistent"} {
		    lappend persistentStates $iname
		} elseif {$attname == "synchronous"} {
		    lappend synchronousStates $iname
		} else {
		    set i${attname} $value
		}
	    }
	    regsub -all -- "!&quot" $idefault "\"" idefault
	    set outlets($iname) [list $itype $idescription $idefault]
	    lappend orderedOutlets $iname
	} state {
	    set iname ""
	    set itype ""
	    set idescription ""
	    set idefault ""
	    foreach {attname value} $attList {
		if {$attname == "persistent"} {
		    lappend persistentStates $iname
		} else {
		    set i${attname} $value
		}
	    }
	    regsub -all -- "!quot" $idefault "\"" idefault
	    set states($iname) [list $itype $idescription $idefault]
	    lappend orderedStates $iname
	} inherited-state {
	    set iname ""
	    set itype ""
	    set idescription ""
	    set idefault ""
	    foreach {attname value} $attList {
		if {$attname == "persistent"} {
		    lappend persistentStates $iname
		} else {
		    set i${attname} $value
		}
	    }
	    regsub -all -- "!quot" $idefault "\"" idefault
	    set inheritedStates($iname) [list $itype $idescription $idefault]
	    lappend orderedInheritedStates $iname
	} activation {
	    set iname ""
	    set iactivators ""
	    set iorder 0
	    foreach {attname value} $attList {
		set i${attname} $value
	    }
	    PushXMLState activation [list $iname $iactivators $iorder]
	} xblock {
	    set iname ""
	    foreach {attname value} $attList {
		set i${attname} $value
	    }
	    PushXMLState xblock $iname
	} copyright {
	    set iname ""
	    foreach {attname value} $attList {
		set i${attname} $value
	    }
	    PushXMLState copyright $iname
	}  default {
	    PushXMLState unused ""
	}         
    } 

    private method ElementEnd {name args} {
	
	switch $name unused {
	    PopXMLState
	} xblock {
	    PopXMLState
	} description {
	    PopXMLState
	} activation {
	    PopXMLState
	} copyright {
	    PopXMLState
	}
    }    

    private method CharacterData {data} {
	if {$data != "" } {
	    switch [GetXMLStateName] description {
		append description $data		
	    } xblock {
		set blockname [GetXMLStateData]
		if {$blockname == "SourceFiles"} {
		    append _sourceFileString $data
		} elseif {$blockname == "Copyright"} {
		    append Announcement $data
		} else {
		    append $blockname $data
		}
	    } activation {
		set stuff [GetXMLStateData]
		set iname [lindex $stuff 0]
		set iactivators [lindex $stuff 1]
		set iorder [lindex $stuff 2]
		set iexpr ""
		catch {set iexpr [lindex $activations($iname) 2]}
		append iexpr $data 
		set activations($iname) [list "unused" $iactivators $iexpr $iorder]
	    } copyright {
		append Copyright $data
	    } default {
		#do nothing
	    }
	}
    }


    private variable _sourceFileString {}

    private method ReadFromXML {afilename} {
	set orderedInlets {}
	set orderedOutlets {}
	set orderedStates {}
	set orderedInheritedStates {}
	set _sourceFileString {} 
	ClearXMLState
	$xmlparser reset
	set chn [open $afilename]
	fconfigure $chn -encoding utf-8
	set data [read $chn]
	close $chn
	if {[catch {$xmlparser parse $data} error]} {
	    puts $error
	    if {$::NoGUI} {
		exit
	    }
	}
	foreach elem $_sourceFileString {
	    set SourceFiles([lindex $elem 0]) [lrange $elem 1 end]
	}
	eval UpdateStates inlets $orderedInlets
	eval UpdateStates outlets $orderedOutlets
	eval UpdateStates states $orderedStates
	eval UpdateStates inheritedStates $orderedInheritedStates
	UpdateActivations
    }
}


body Externalizer::MakeCPP {} {

    set logFile2Externalizer {}

    set externalTypes {} 
    set includes "tcl.h"
    if {$interactivedraw} {
	lappend includes "tk.h"
    }
    lappend includes "oswTcl.h"
    set includes [concat $includes $externalizer::defaultIncludes]
    if {$interactivedraw} {
	lappend includes "oswTkDraw.h"
    }
    if {$datasource} {
	lappend includes "oswDataSource.h"
    }
    foreach group {inlets outlets states inheritedStates} {
	foreach var [array names $group] {
	    set type [lindex [lindex [array get $group $var] 1] 0]
	    if {$type == "TkDrawingInfo"} {
		continue
	    }
	    if {$externalizer::typeLocation($type) != "internal"} {
		puts $externalizer::typeLocation($type)
		lappend includes $externalizer::typeLocation($type)
		lappend externalTypes $type
	    }
	}
    }

    set includes [concat $includes $Includes]

    foreach activation [array names activations] {
	set expr [lindex $activations($activation) 2]
	set effects($activation) {}
	foreach outlet [array names outlets] {
	    if {[regexp "[set outlet]\ *=" $expr]} {
		lappend effects($activation) $outlet
		#puts "$activation affects $outlet"
	    }
	}
    }
    
    set linenum 1
    set chn [open $name.cpp w]
    puts $chn "/*\n $name.cpp: Generated by OSW Externalizer\n*/"
    puts $chn "/*"
    puts $chn $Copyright
    puts $chn "*/"

    foreach include $includes {
	puts $chn "#line $linenum"
	puts $chn "#include \"$include\""
	incr linenum
    }
    puts $chn "#line $linenum"
    puts $chn "#include \"oswExternal.h\""
    incr linenum
    
    puts $chn "#line [incr linenum]"
    puts $chn "\nusing namespace std;"
    puts $chn "#line [incr linenum]"
    puts $chn "#define _IN_OSW_NAMESPACE"
    puts $chn "namespace osw \{"
    puts $chn "#line [incr linenum]"
    lappend logFile2Externalizer [list $linenum "ShowAdvancedQuick Global"]
    #linenum to global
    puts $chn $Global
    incr linenum [llength [split $Global "\n"]]
    
    puts $chn "#line [incr linenum]"
    set classdecl "class _oswexport ${name}Transform : public "    
    if {$timedomain} {
	lappend classdecl TimeDomainTransform
    } else {
	lappend classdecl Transform
    }
    if {$interactivedraw} {
	append classdecl ", public TkInteractiveDrawing "
    }
    if {$datasource} {
	append classdecl ", public DataSource "
    }
    append classdecl " \{"
    puts $chn $classdecl
    puts $chn "#line [incr linenum]"
    puts $chn "public:"
    
    foreach inlet [array names inlets] {
	puts $chn "#line [incr linenum]"
	MakeLinkToVariable $linenum inlets $inlet
	set type [lindex $inlets($inlet) 0]
	if {$type == "Changeable"} {
	    puts $chn "ChangeableInlet $inlet ;"
	} else {
	    puts $chn "Inlet<$type> $inlet ;"
	}
    }
    foreach outlet [array names outlets] {
	puts $chn "#line [incr linenum]"
	MakeLinkToVariable $linenum outlets $outlet
	set type [lindex $outlets($outlet) 0]
	puts $chn "Outlet<$type> $outlet ;"
    }
    foreach state [array names states] {
	puts $chn "#line [incr linenum]"
	MakeLinkToVariable $linenum states $state
	set type [lindex $states($state) 0]
	puts $chn "State<$type> $state ;"
    }


    #the constructor
    set constext ""
    append constext "${name}Transform (const string &aname, Container *acontainer, int argc, char *argv\[\]) : \n"
    if {$timedomain} {
	append constext " TimeDomainTransform(aname,acontainer,argc,argv)"
    } else { 
	append constext " Transform(aname,acontainer,argc,argv)"
    }
    if {$interactivedraw} {
	append constext ",\n TkInteractiveDrawing(this)"
    }
    if {$datasource} {
	append constext ",\n DataSource(this,acontainer,argc,argv)"
    }

    incr linenum 10

    foreach inlet [array names inlets] {
	set type [lindex $inlets($inlet) 0]
	set desc [lindex $inlets($inlet) 1]
	append constext ",\n#line [incr linenum]\n $inlet (\"$inlet\",this,\"$desc\""
	set defval [lindex $inlets($inlet) 2]
	if {[llength $defval] > 0 && $type != "Changeable"} {
	    append constext \
		    ",ScanArguments<$type>(\"-$inlet\",$defval,acontainer,argc,argv)"
	}
	append constext ")"
	MakeLinkToVariable $linenum inlets $inlet
    }
    foreach outlet [array names outlets] {
	set type [lindex $outlets($outlet) 0]
	set desc [lindex $outlets($outlet) 1]
	append constext ",\n#line [incr linenum]\n $outlet (\"$outlet\",this,\"$desc\""
	set defval [lindex $outlets($outlet) 2]
	if {[llength $defval] > 0} {
	    append constext ",$defval"
	}
	append constext ")"
	MakeLinkToVariable $linenum outlets $outlet
    }
    foreach state [array names states] {
	set type [lindex $states($state) 0]
	set desc [lindex $states($state) 1]
	append constext ",\n#line [incr linenum]\n $state (\"$state\",this,\"$desc\""
	set defval [lindex $states($state) 2]
	if {[llength $defval] > 0} {
	    append constext \
		    ",ScanArguments<$type>(\"-$state\",$defval,acontainer,argc,argv)"
	}
	append constext ")"
	MakeLinkToVariable $linenum states $state
    }

    foreach activation [array names activations] {
	set time [lindex $activations($activation) 0]
	set numstates [llength [lindex $activations($activation) 1]]
	set order [lindex $activations($activation) 3]
	if {$order == ""} {
	    set order 0
	}
	append constext ",\n#line [incr linenum]\n $activation ($numstates,$order,"
	append constext "this,&[set name]Transform::[set activation]Expr)"
	MakeLinkToActivation $linenum $activation
    }
    if {[llength $PreConstructor] > 0} {
	incr linenum 2
	append constext ",\n#line $linenum\n"
	append constext $PreConstructor
	lappend logFile2Externalizer [list $linenum "ShowAdvancedQuick PreConstructor"]
	incr linenum 20
    }
    append constext " \{"
    incr linenum
    puts $chn $constext
    foreach activation [array names activations] {
	set activators [lindex $activations($activation) 1]
	foreach activator $activators {
	    incr linenum
	    puts $chn "$activation.AddActivator(&$activator);"
	}
	foreach effect $effects($activation) {
	    incr linenum
	    puts $chn "$activation.AddEffect(&$effect);"
	}
    }

    set orderedInlets [$win.ios.inlets get 0 end]
    for {set i 0} {$i < [llength $orderedInlets]} {incr i} {
	incr linenum
	puts $chn "[lindex $orderedInlets $i].SetOrder($i);"
    }
    set orderedOutlets [$win.ios.outlets get 0 end]
    for {set i 0} {$i < [llength $orderedOutlets]} {incr i} {
	incr linenum
	puts $chn "[lindex $orderedOutlets $i].SetOrder($i);"
    }

    foreach sync $synchronousStates {
	incr linenum
	puts $chn "$sync.NoParallel();"
    }

    puts $chn "#line [incr linenum]"
    puts $chn $Constructor
    lappend logFile2Externalizer [list $linenum "ShowAdvancedQuick Constructor"]
    incr linenum [llength [split $Constructor "\n"]]
    puts $chn "\}\n"

    #ok that bit of nastiness is now behind us.  Go on to the destructor

    puts $chn "~${name}Transform () \{"
    puts $chn "#line [incr linenum 10]"
    puts $chn $Destructor
    lappend logFile2Externalizer [list $linenum "ShowAdvancedQuick Destructor"]
    incr linenum [llength [split $Destructor "\n"]]
    puts $chn "\}\n"

    #that was much easier :)

    puts $chn "const char *Description () const \{"
    puts $chn "#line [incr linenum 10]"
    MakeLinkToMainWindow $linenum desc
    puts $chn " return \"$description\";"
    puts $chn "\}"    
    puts $chn "string PersistentState () const \{"
    puts $chn "#line [incr linenum 10]"
    lappend logFile2Externalizer [list $linenum InteractiveStates]
    puts $chn "  return \"$persistentStates\";"
    puts $chn "\}"
    if {[llength $TclDialog] > 0} {
	puts $chn "const char *Dialog () \{"
	puts $chn " return "
	puts $chn "#line [incr linenum 10]"
	lappend logFile2Externalizer [list $linenum "ShowAdvancedQuick TclDialog"]
	foreach line [split [fixbackslash $TclDialog] "\n"] {
	    incr linenum
	    puts $chn "\"[string trimright $line]\\n\""
	}
	puts $chn ";\n";
	puts $chn "\}"
    }

    puts $chn "const char *Draw(void *DrawState) \{"
    puts $chn "#line [incr linenum 10]"
    lappend logFile2Externalizer [list $linenum GraphicsCode]
    puts $chn $drawCodeCpp
    incr linenum [llength [split $drawCodeCpp "\n"]]    
    if {[llength $drawCodeTcl] > 0} {
	puts $chn "return "
	puts $chn "#line [incr linenum 10]"
	lappend logFile2Externalizer [list $linenum GraphicsCode]
	foreach line [split [fixbackslash $drawCodeTcl] "\n"] {
	    incr linenum
	    puts $chn "\"[string trimright $line]\\n\""
	}

	puts $chn ";"
    } else {
	puts $chn "return \"\";"
    }
    puts $chn "\}"

    puts $chn "private:"
    puts $chn "#line [incr linenum 20]"
    lappend logFile2Externalizer [list $linenum "ShowAdvancedQuick Private"]
    puts $chn $Private
    incr linenum [llength [split $Private "\n"]]
    puts $chn "\n"
    foreach activation [array names activations] {
	puts $chn "#line [incr linenum 10]"
	MakeLinkToActivation $linenum $activation
	puts $chn "Activation<${name}Transform> $activation ;"
	puts $chn "void [set activation]Expr () \{ "
	set activationExpr \
		[FixActivations [lindex $activations($activation) 2]]
	puts $chn $activationExpr
	incr linenum [llength [split $activationExpr "\n"]]
	puts $chn "\}"
    }

    puts $chn "\};"

    puts $chn "#line [incr linenum 10]"
    lappend logFile2Externalizer [list $linenum "ShowAdvancedQuick Announcement"]
    puts $chn "static const char *copyright = \"\""
    if {[llength $Announcement] > 0} {
	foreach line [split $Announcement "\n"] {
	    incr linenum
	    puts $chn "\"[string trimright $line]\\n\""
	}
    }
    puts $chn ";\n";
    
    puts $chn "static TransformFactory<${name}Transform> [set name]Factory"
    set localPackagePrefix ""
    if {$packageName != ""} {
	set localPackagePrefix "$packageName"
	append localPackagePrefix "::"
    }

    #if {$autoname} {
    #	puts $chn "\t(\"$localPackagePrefix$name\",true,copyright);\n"
    #} else {
	puts $chn "\t(\"$localPackagePrefix$name\",false,copyright);\n"
    #}
    
    puts $chn "\} //namespace osw\n"

    puts $chn "extern \"C\" {"
    puts $chn "_oswexport char *g_Name = \"$name\";\n"
    puts $chn "_oswexport char *g_Package = \"$packageName\";\n"
    puts $chn "_oswexport char *g_PrefixedName = \"$localPackagePrefix$name\";\n"
    puts $chn "}"
    puts $chn "\n"
    puts $chn "extern \"C\" _oswexport int [set name]_Init (Tcl_Interp *interp) \{"
    puts $chn "#line [incr linenum 100]"
    lappend logFile2Externalizer [list $linenum "ShowAdvancedQuick Initialize"]
    puts $chn $Initialize
    incr linenum [llength [split $Initialize "\n"]]    
    #puts $chn "cout << osw::copyright << endl;"
    if {[llength $TclStartup] > 0} {
	puts $chn "Tcl_GlobalEval (interp,"
	puts $chn "#line [incr linenum 10]"
	lappend logFile2Externalizer [list $linenum "ShowAdvancedQuick TclStartup"]
	foreach line [split [fixbackslash $TclStartup] "\n"] {
	    puts $chn "\"[string trimright $line]\\n\""
	    incr linenum
	}
	puts $chn ");\n";
    }

    puts $chn " return TCL_OK;"
    puts $chn "\}"

    puts $chn "extern \"C\" _oswexport int [set name]_SafeInit (Tcl_Interp *interp) \{"
    puts $chn "   return [set name]_Init (interp);"
    puts $chn "\}"
    
    close $chn

    set chn [open $name.osl w]
    foreach xtype $externalTypes {
	puts $chn $xtype
    }
    close $chn
}

body Externalizer::MakeMake {} {

    global env
    set chn [open "Makefile" w]
    set template [open "$env(OSW_DIR)/externalizer/$currentCompiler.mak" r]

    switch $::tcl_platform(platform) windows {
	if {$currentCompiler == "cygwin"} {
	    puts $chn "include $OSWSourceDir/win/System.mak.gcc"
	} else {
	    puts $chn "include $OSWSourceDir/win/System.mak"
	}
    } default {
	if {$OSWTclInclude != ""} {
	    puts $chn "TCL_INCLUDE_DIR=$OSWTclInclude"
	} 
	if {$OSWTkInclude != ""} {
	    puts $chn "TK_INCLUDE_DIR=$OSWTkInclude"
	}
	puts $chn "OSWLIBS=$OSWLibs"
    }
    puts $chn "OSXNAME = $name"
    puts $chn "PACKAGE = $packageName"
    if {$::tcl_platform(platform) == "windows" && $currentCompiler == "msvc"} {
	regsub -all "/" $OSWInstallDir "\\" backslashed
	puts $chn "OSWDIR = ${backslashed}\\src"
	puts $chn "OSWLIBDIR = ${backslashed}\\lib"
	regsub -all "/" $OSWInstallDir "\\" backslashed
	puts $chn "INSTALL_DIR = ${backslashed}"
	regsub -all "/" $OSWStlDir "\\" backslashed
	puts $chn "STLDIR = ${backslashed}"
    } else {
	puts $chn "OSWDIR = $OSWInstallDir/src"
	puts $chn "INSTALL_DIR = $OSWInstallDir/"
	puts $chn "STLDIR = $OSWStlDir"
    }


    set includes {$(OSWDIR)/include/osw.h }
    append includes {\$(OSWDIR)/include/oswList.h }
    append includes {\$(OSWDIR)/include/oswTable.h }
    

    foreach group {inlets outlets states inheritedStates} {
	foreach var [array names $group] {
	    set type [lindex [lindex [array get $group $var] 1] 0]
	    if {$type == "TkDrawingInfo"} {
		continue
	    }
	    if {$externalizer::typeLocation($type) != "internal"} {
		#puts $externalizer::typeLocation($type)
		set local {$(OSWDIR)}
		append local "/types/$externalizer::typeLocation($type)"
		lappend includes $local
	    }
	}
    
    }
    puts $chn "DOT_H = [join $includes]"

    puts $chn "USERLIB = [join [concat $Libraries $::AddLibs]]"

    set objsuffix [::externalizer::[set currentCompiler]ObjectSuffix]
    set userobj {}

    foreach sf [array names SourceFiles] {  
	lappend userobj [file root $sf].$objsuffix
    }
    

    puts $chn "USEROBJ = [join $userobj]"
    
    while {![eof $template]} {
	puts $chn [gets $template]
    }

    #catch {
	foreach sf [array names SourceFiles] obj $userobj {
	    puts $chn "$obj : $sf [eval concat [lindex $SourceFiles($sf) 0]]"
	    if {[regexp "\\.c$" $sf]} {
		puts $chn "\t\$(OLDC) \$(CFLAGS) [eval concat [lindex $SourceFiles($sf) 1]] -c $sf\n"
	    } else {
		puts $chn "\t\$(CC) \$(CFLAGS) [eval concat [lindex $SourceFiles($sf) 1]] -c $sf\n"
	    }
	}
    #}

    close $template
    close $chn
}


body Externalizer::FixActivations {activationExpr} {

    foreach group {inlets outlets states inheritedStates} {
	foreach statevar [array names $group] {
	    set type [lindex [lindex [array get $group $statevar] 1] 0]
	    if {$type != "Changeable"} {
		#puts "$statevar $type"
		regsub -all -- "(^|\[^a-zA-Z0-9_\\.\\&\])$statevar\(\\-\\>\)" $activationExpr "\\1(*\\&$statevar)\\2" activationExpr
		regsub -all -- "(^|\[^a-zA-Z0-9_\\.\\&\])$statevar\(\[ \]*=)" $activationExpr "\\1(*\\&$statevar)\\2" activationExpr
		regsub -all -- "(\\>\\>\[ \]*)$statevar\((\[^a-zA-Z0-9_\\.\]))" \
			$activationExpr "\\1(*\\&$statevar)\\2" activationExpr
		regsub -all -- "(\\-\\>\[ \]*)$statevar\((\[^a-zA-Z0-9_\\.\]))" \
			$activationExpr "\\1\#\#\#$statevar\\2" activationExpr
		regsub -all -- "(^|\[^a-zA-Z0-9_\\.\\&\\-\\#])$statevar\((\[^a-zA-Z0-9_\\.\]))" \
			$activationExpr \
			"\\1(($type \\&) $statevar)\\2" activationExpr
		regsub -all -- "\\(\\*\\&(\[^\)\]+)\\)" $activationExpr "\\1" activationExpr
		regsub -all -- "\\#\\#\\#" $activationExpr "" activationExpr
	    }	
	}
    }
    return $activationExpr
}


body Externalizer::StartBatchExternalize {} {

    iwidgets::fileselectiondialog .fsd -modality application -dirson true \
	    -fileson false 
    .fsd buttonconfigure OK -command "$this BatchExternalize \[.fsd get\]"
    .fsd activate

}

body Externalizer::BatchExternalize {startdir} {

    if {!$::NoGUI} {
	destroy .fsd
    }
    set numOSD 0
    foreach extdir [glob -nocomplain $startdir/*] {
	if {[file exists $startdir/$extdir.osd]} {
	    incr numOSD
	} elseif {[file isdirectory $startdir/$extdir]} {
	    foreach osd [glob -nocomplain $startdir/$extdir/*.osd] {
		incr numOSD
	    }
	}
    }

    incr numOSD
    set xbatchmode true
    set xshouldquit false
    
    if {!$::NoGUI} {
	set ds [iwidgets::dialogshell .ds#auto \
		    -title "Externalizing $name"]
	set dsite [$ds childsite]
	label $dsite.status -text ""
	iwidgets::feedback $dsite.fb -steps $numOSD
	pack $dsite.status $dsite.fb -fill x -side top
	$ds add cancel -text "Cancel" -command "set [scope xshouldquit] true"
	$ds activate
	update
    }
	
    foreach extdir [glob -nocomplain $startdir/*] {
	if {[file extension $extdir] == "osd"} {
	    set osdfile [file tail $extdir]
	    set osdcpp [file root $osdfile].cpp
	    set osdosx [file root $osdfile].osx
	    set curdir [pwd]
	    cd $startdir
	    if {![file exists $osdosx] || [file mtime $osdfile] > [file mtime $osdosx]} {	    
		
		#New
		#source $osdfile
		#set filename $osdfile
		#set dirty false
		LoadFromFile $osdfile
		if {$_newhelppatch} {
		    SaveToFile $osdfile
		}	
		if {!$::NoGUI} {
		    $dsite.status configure -text [file root $osdfile]
		    $dsite.fb step
		    update	    
		}
		Compile
	    }
	    cd $curdir
	} elseif {[file isdirectory $extdir]} {
	    foreach osd [glob -nocomplain $extdir/*.osd] {
		set osdfile [file tail $osd]
		set osdcpp [file root $osdfile].cpp
		set osdosx [file root $osdfile].osx
		set curdir [pwd]
		cd [file dirname $osd]
		if {![file exists $osdosx] || [file mtime $osdfile] > [file mtime $osdosx]} {				
		    #New
		    #source $osdfile
		    #set filename $osdfile
		    #set dirty false
		    LoadFromFile $osdfile
		    if {$_newhelppatch} {
			SaveToFile $osdfile
		    }
		    if {!$::NoGUI} {
			$dsite.status configure -text [file root $osdfile]
			$dsite.fb step
			update
		    }
		    Compile
		}
		cd $curdir
		if {$xerrors || $xshouldquit} {
		    break
		}
	    }
	}
	if {$xerrors || $xshouldquit} {
	    break
	}
    }
    if {!$::NoGUI} {
	destroy $ds
    }
    set xbatchmode false
    if {$::BatchMode} {
	exit
    }
}

body Externalizer::MakeHTML {} {

    set chn [open $name.html w]    
    puts $chn "<html>"
    puts $chn "<head>"
    puts $chn "\t<meta name=\"Author\" content=\"$Author\">"
    puts $chn "\t<meta name=\"Generator\" content=\"OSW Externalizer\">"
    puts $chn "\t<meta name=\"KeyWords\" content=\"[string trim $Keywords { \t}]\">"
    puts $chn "<link href=\"../css/text.css\" rel=\"stylesheet\" type=\"text/css\">"

    puts $chn "</head>"
    puts $chn "<body bgcolor=\"white\">"

    puts $chn {
<!-- BEGIN header -->
<table border="0" width="100%">
<tr>
<td>
<a class="mantle" target="_top" href="../UserGuide.html"><img src="../oswmantle.png" width="453" height="94" border="0" /></a>
</td>
<td align="left">
<h3 class="title">Transform Reference Guide</h3>
</td>
</tr>
<tr>
<td colspan="2" class="menubar">
<a href="UserGuide.html" target="_top" class="menu">User Guide</a> 
|
<a href="Transform_by_group.html" target="_top" class="menu">Transform Guide</a> 
|
<a href="http://osw.sourceforge.net" target="_top" class="menu">OSW on the Web</a> 
</td>
</tr>
</table>
<!-- END header -->
    }

    if {$HTMLIcon != ""} {
	puts $chn "<img src=$HTMLIcon />"
    }
    set helppatchurl ""
    if {$HelpPatch != ""} {
	set helppatchurl " <a href=\"http://localhost:7775/OSW_DIR/help_patches/"
	if {$packageName != ""} {
	    append helppatchurl "$packageName/"
	}
	append helppatchurl "help_$name.osw\">"
	append helppatchurl "<font size=\"-1\">\[Help Patch\]</font></a>"
    }
    if {$packageName != "" && $packageName != "standard" && $packageName != "osw"} {
	puts $chn "<h1 class=\"title\">$packageName\:\:$name$helppatchurl</h1>"
    } else {
	puts $chn "<h1 class=\"title\">$name$helppatchurl</h1>"
    }
    puts $chn "<p>$description</p>"
    puts $chn "<h3 class=\"title\">Syntax</h3>"

    if {$Syntax == "default" || $Syntax == ""} {
	    set Syntax "$name <i>options</i>"
    }
    puts $chn "<p>$Syntax</p>"

    set options {"<dt><b>-name</b></dt><dd>Optional user-defined name for the transform</dd>"}
    foreach inlet [array names inlets] {
	set type [lindex $inlets($inlet) 0]
	set desc [lindex $inlets($inlet) 1]
	set defval [lindex $inlets($inlet) 2]
	if {[llength $defval] > 0 && $type != "Changeable"} {
	    lappend options "<dt><b>-$inlet</dt></b><dd>$desc</dd><dd>Type: $type</dd><dd>Default: $defval</dd>"
	}
    }    
    foreach state [array names states] {
	set type [lindex $states($state) 0]
	set desc [lindex $states($state) 1]
	set defval [lindex $states($state) 2]
	if {[llength $defval] > 0 && $type != "Changeable"} {
	    lappend options "<dt><b>-$state</b></dt><dd>$desc</dd><dd>Type: $type</dd><dd>Default: $defval</dd>"
	}
    }    
    puts $chn "<h3 class=\"title\">Options</h3>"
    if {[llength $options] > 0} {
	puts $chn "<ul><dl>"
	foreach option $options {
	    puts $chn $option
	}
	puts $chn "</dl></ul>"
    } else {
	puts $chn "<p>none</p>"
    }

    if {[array size inlets] > 0} {
	puts $chn "<h3 class=\"title\">Inlets</h3>"
	puts $chn "<ul><dl>"
	foreach inlet [array names inlets] {
	    set type [lindex $inlets($inlet) 0]
	    set desc [lindex $inlets($inlet) 1]
	    set defval [lindex $inlets($inlet) 2]
	    puts $chn "<dt><b>$inlet</b></dt><dd>$desc</dd><dd>Type: $type</dd>"
	    if {[llength $defval] > 0} {
		puts $chn "<dd>Default: $defval</dd>"
	    }
	}
	puts $chn "</dl></ul>"
    }

    if {[array size outlets] > 0} {
	puts $chn "<h3 class=\"title\">Outlets</h3>"
	puts $chn "<ul><dl>"    
	foreach outlet [array names outlets] {
	    set type [lindex $outlets($outlet) 0]
	    set desc [lindex $outlets($outlet) 1]
	    puts $chn "<dt><b>$outlet</b></dt><dd>$desc</dd><dd>Type: $type</dd>"
	}
	puts $chn "</dl></ul>"
    }

    if {[array size states] > 0} {
	puts $chn "<h3 class=\"title\">State Variables</h3>"
	puts $chn "<ul><dl>"
	foreach state [array names states] {
	    if {[lsearch $persistentStates $state] < 0} {
		continue
	    }
	    set type [lindex $states($state) 0]
	    set desc [lindex $states($state) 1]
	    set defval [lindex $states($state) 2]
	    puts $chn "<dt><b>$state</b></dt><dd>$desc</dd><dd>Type: $type</dd>"
	    if {[llength $defval] > 0} {
		puts $chn "<dd>Default: $defval</dd>"
	    }
	}
	puts $chn "</dl></ul>"
    }

    puts $chn "<h3 class=\"title\">Comments</h3>"
    puts $chn "$LongDescription"

    if {[llength $OnTheWeb] > 0} {
	puts $chn "<h3 class=\"title\">OnTheWeb</h3>"
	puts $chn "<p><a href=$OnTheWeb>$OnTheWeb</a></p>"
    }
    if {[llength $Announcement] > 0} {
	puts $chn "<h3 class=\"title\">Announcement / Credits</h3>"
	puts $chn "<p>$Announcement </p>"
    }
    if {[llength $Author] > 0} {
	puts $chn "<h3 class=\"title\">Author</h3>"
	puts $chn "<p>$Author </p>"
    }
    if {[llength $Links] > 0} {
	puts $chn "<h3 class=\"title\">See Also</h3>"
	puts $chn "<dl>"
	foreach link $Links {
	    set link [string trimright $link ","]
	    regsub "^(.*/)+" $link "" cleanlink
	    puts $chn "<dt><a href=$link.html>$cleanlink</a></dt>"
	}
	puts $chn "</dl>"
    }
    if {[llength $Keywords] > 0} {
	puts $chn "<h3 class=\"title\">Keywords</h3>"
	puts $chn "<p>$Keywords </p>"
    }
    if {$Copyright != ""} {
	puts $chn "<h3 class=\"title\">Copyright / Licence</h3>"
	puts $chn "<pre>$Copyright\n</pre>"
    }

    puts $chn {
<!-- BEGIN footer -->
<hr />
<a target="_top" href="http://osw.sourceforge.net">Open Sound World</a> User Guide<br />
&copy; 2000-2004 Amar Chaudhary.  All rights reserved.
<!-- END footer -->
    }

    puts $chn "</body>"
    puts $chn "</html>"
    close $chn
}

body Externalizer::MakeHelpPatch {} {

    set chn [open help_${name}.osw w]    
    puts $chn $HelpPatch
    close $chn
}


set CalledFromOsw 0

wm withdraw .

set ext [Externalizer #auto]
if {$BatchMode} {
    $ext BatchExternalize $fileToLoad
} elseif {$fileToLoad != ""} {
    $ext LoadFromFile $fileToLoad
}





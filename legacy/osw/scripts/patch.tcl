
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

source $env(OSW_DIR)/scripts/stack.tcl
source $env(OSW_DIR)/scripts/ldelete.tcl
source $env(OSW_DIR)/scripts/manhattan.tcl

class View {

    private common paneID 0

    public common viewList {}
    public variable tbar ""
    public variable pane
    public variable patch ""
    
    public variable isHidden false

    private variable _destroying 0

    public method constructor {args} {
        eval configure $args
        MakeView
        lappend viewList $this
    }

    destructor {
        set _destroying 1
        destroy $tbar
        destroy $pane
        ldelete viewList $this
    }

    private method MakeView {} {

        set width [expr [winfo screenwidth .] - 240]
        set height [expr [winfo screenheight .] - 160]
        set pane [toplevel .p$paneID]
        set xoffset [expr 120 + ($paneID % 5) * 20]
        set yoffset [expr 80 + ($paneID % 5) * 20]
        wm geometry $pane ${width}x${height}+${xoffset}+${yoffset}
        oswAttachDefaultBindings $pane
        incr paneID

        #$::patchArea add $pane

        set tbar [iwidgets::toolbar $pane.tbar -orient horizontal]

        pack $tbar -fill x
        $tbar add button new -balloonstr "New Patch" -image \
		[oswLoadBitmap photo new.gif true] \
		-command {oswNewPatch}
        $tbar add button open -balloonstr "Open Patch" -image \
		[oswLoadBitmap photo open.gif true] \
		-command [list oswOpenPatch $patch]
        $tbar add button close -balloonstr "Close Patch" -image \
		[oswLoadBitmap photo close.gif true] \
		-command [list $this Close]
        $tbar add button save -balloonstr "Save Patch" -image \
		[oswLoadBitmap photo piggy.gif true] \
		-command [list $this Save]
        $tbar add button lock -balloonstr "Lock" -image \
		[oswLoadBitmap photo pencil.gif true] \
		-command [list $this LockUnlockButton]
        $tbar add button txtsticky -balloonstr "Sticky Note" -image \
		[oswLoadBitmap photo txtsticky.gif true] \
		-command [list $this DragNewSticky]
        $tbar add button title -balloonstr "Title Text" -image \
		[oswLoadBitmap photo title.gif true] \
		-command [list $this DragNewTitle]
        $tbar add button link -balloonstr "Hyperlink" -image \
		[oswLoadBitmap photo link.gif true] \
		-command [list $this DragNewLink]
        $tbar add button img -balloonstr "Image" -image \
		[oswLoadBitmap photo smiley.gif true] \
		-command [list $this DragNewImage]

        wm protocol $pane WM_DELETE_WINDOW [list $this CloseMe]
    }
    
    public method Show {} {
        wm deiconify $pane
        set isHidden false
        focus $pane
    }

    public method Hide {} {
        wm withdraw $pane
        set isHidden true
    }

    public method Maximize {} {
        foreach view $viewList {
            if {$view != $this} {
                $view Hide
            }
        }
    }

    public method CloseMe {} {
        oswClosePatch $patch
    }

    
    public method DragNewSticky {} {
	$patch DragNewSticky
    }
    
    public method DragNewTitle {} {
	$patch DragNewTitle
    }
    
    public method DragNewLink {} {
	$patch DragNewLink
    }

    public method DragNewImage {} {
	$patch DragNewImage
    }

    public method Save {} {
	$patch Save
    }

    public method Close {} {	
	oswClosePatch $patch
    }

    public method DelayClose {} {
	if {$_destroying} {
	    return
	}
	set _destroying 1
	oswClosePatch $patch
	if {$Patch::selected == $this} {
	    set Patch::selected ""
	}
	set _destroying 0
    }
    
    public method IsDestroyingView {} {
	return $_destroying
    }

    public method LockUnlockButton {} {
	$patch LockUnlockButton
    }
    
    public method ToggleLockButton {locked} {
	if {$locked} {
	    $tbar itemconfigure lock -balloonstr "Unlock" -image \
		    [oswLoadBitmap photo nopencil.gif true] \
		    -command [list $this LockUnlockButton]
	} else {
	    $tbar itemconfigure lock -balloonstr "Lock" -image \
		    [oswLoadBitmap photo pencil.gif true] \
		    -command [list $this LockUnlockButton]
	}
    }
}

class Patch {

    private common patchObjList {}
    private common availableViews {}
    private common patchCount 0

    public common selected ""
    public common previousSelected ""

    public variable view

    public variable patchInstance
    public variable patchClass
    public variable sourceFile ""
    public variable parent ""
    
    public variable version 0.0
    public variable bg {#CCCCCC}

    public variable transforms {}
    public variable transformMap
    public variable stickies {}
    public variable titles {}
    public variable links {}
    public variable imgs {}
    public variable scripts {}
    
    public variable patchArgs {}
    
    public variable patchcanvas
    public variable statusbar
    public variable win

    private variable lastdrawX 
    private variable lastdrawY 
    private variable firstdrawX 
    private variable firstdrawY 
    private variable gestureNum 0

    private variable PasteContextX 0
    private variable PasteContextY 0
    private variable PasteContextSeedX 0
    private variable PasteContextSeedY 0

    public variable dir
    public variable pane

    public variable dirty 0

    public method constructor {args} {
	eval configure $args
	
	incr patchCount

	AssignView
	set pane [$view cget -pane]

	wm title $pane $parent/$patchInstance

	set patchcanvas [iwidgets::scrolledcanvas \
		$pane.patchc$patchCount \
		-vscrollmode dynamic \
		-hscrollmode dynamic -autoresize true]
	pack $patchcanvas  -side top -expand yes -fill both

	set statusbar [label $pane.l -relief sunken -borderwidth 1]
	pack $statusbar -side bottom -fill both

	RegisterPatch

	set win [$patchcanvas component canvas]
	focus $patchcanvas
	bind $win <Button> "$this ButtonOverNothing %x %y" 

	Background $bg
	if {$parent != ""} {
	    $view Hide
	} else {
	    set previousSelected $selected
	    set selected $this
	}

	oswPatch $parent $patchInstance -window $this

	# Sometimes need to pass over a graph twice because
	# of dynamically-typed connections
	# (We could just do something smart about ordering the nodes,
	#  but that would take too much effort ;)

	global _oswSecondChanceConnections
	set _oswSecondChanceConnections {}
	
	set dir [file dir $sourceFile]
	if {$dir == "."} {
	    set dir [pwd]
	}

	InitArguments
	
	incr ::NumPatches
    }
    
    private method AssignView {} {
	set view [View view#auto]
	$view configure -patch $this
	$view ToggleLockButton $locked
    }
    
    private method RegisterPatch {} {
	
	lappend patchObjList $this
	

    }

    public method UnregisterPatch {} {

	ldelete patchObjList $this
    }

    public method Hide {} {
	$view Hide
    }

    destructor {
	#puts "Destroying patch..."
	UnregisterPatch
	foreach transform $transforms {
	    delete object $transform
	}

	destroy $win
	if {![$view IsDestroyingView]} {
	    #puts "Destroying view..."
	    delete object $view
	    #puts "Done destroying view..."
	}
	#these are things the parent patch would usually take care of
	if {$parent == ""} {
	    oswDelete $parent/$patchInstance
	}
    }

    public method FinishSetup {} {
	global _oswSecondChanceConnections
	foreach connection $_oswSecondChanceConnections {
	    set outlet [lindex $connection 0]
	    set inlet [lindex $connection 1]
	    set internalPts [lindex $connection 2]
	    while {[MakeConnectionSecondChance $outlet $inlet $internalPts]} {
		update idletasks
	    }
	}
	set selected $this
	if {$parent == ""} {
	    wm deiconify $pane
	}
	oswSet $parent/$patchInstance/load 0
    }

    public method SetVersion {aversion} {
	set version $aversion
    }

    public method ButtonOverNothing {x y} {

	set selected $this
	oswShowAllRelevant

	set x [$patchcanvas canvasx $x]
	set y [$patchcanvas canvasy $y]

	set PasteContextSeedX [expr int($x)]
	set PasteContextSeedY [expr int($y)]
	set selected $this
	
	if {[$win gettags current] == ""} {
	    focus $patchcanvas
	    ForgetSelection
	    update idletasks
	    global oswGravityWell
	    set overlaps [$win find overlapping [expr $x - $oswGravityWell] \
		    [expr $y - $oswGravityWell] \
		    [expr $x + $oswGravityWell] [expr $y + $oswGravityWell]]
	    foreach id $overlaps {
		if {[lsearch [$win gettags $id] outlet] != -1} {
		    set outlet [lindex [$win gettags $id] 1]
		    regsub "box_" $outlet "" outlet
		    [::outlet-$outlet cget -transform] StartConnection $outlet
		    return
		}
	    }      
	    set lastdrawX $x
	    set lastdrawY $y
	    set firstdrawX $x
	    set firstdrawY $y
	    incr gestureNum
	    bind $win <B1-Motion> "$this Draw %x %y"
	    bind $win <ButtonRelease-1> "$this InterpretDrawing" 
	}
    }

    public method AddBlankTransform {x y} {
	#set x [$win canvasx $x]
	#set y [$win canvasy $y]

	while {1} {
	    set name [GetUniqueName]
	    
	    if {![catch {
		TransformWidget ::$name -win $win -x $x -y $y -name $name -patch $parent/$patchInstance \
			-patchWindow $this
	    }]} {		
		break
	    }
	}
	::$name AutoWidgetSize true
	lappend transforms ::$name
	ObjectSelection $name false
	return $name
    }

    public method MakeTransform {x y w h transformCmd args} {
	
	set x [expr $x + $PasteContextX]
	set y [expr $y + $PasteContextY]
	set transObj [AddBlankTransform $x $y]

	global initCmd
	set initCmd(::$transObj) $transformCmd
	::$transObj CreateTransform $args
	::$transObj AutoWidgetSize false
	if {$w <= 0 || $h <= 0} {
	} else {
	    ::$transObj WidgetSize $w $h
	}
	if {$locked} {
	    $transObj Lock
	} else {
	}
	return $transObj
    }

    public method DeleteSelection {} {
	global Selection
	
	if {$locked} {
	    return
	}

	foreach sel $Selection {
	    switch [$sel info class] "::TransformWidget" {
		delete object ::$sel
		ldelete transforms ::$sel
	    } "::stickynote" {
		delete object ::$sel
		ldelete stickies ::$sel
	    } "::Title" {
		delete object ::$sel
		ldelete titles ::$sel
	    } "::Hyperlink" {
		delete object ::$sel
		ldelete links ::$sel
	    } "::Image" {
		delete object ::$sel
		ldelete imgs ::$sel
	    } default {
		delete object $sel
	    }
	}
	set dirty 1
	set Selection {}
    }

    public method Show {} {
	catch {$view Show}
	#catch {$view SetSelection "$parent/$patchInstance"}
    }

    private variable uniqueNum 0
    private variable usedNames {}

    public method GetUniqueName {args} {
	if {[llength $args] > 0} {
	    set name $args
	    for {set i 0} {1} {incr i} {
		if {[lsearch $usedNames $name$i] == -1 &&  
		[lsearch $transforms $name$i] == -1} {
		    lappend usedNames $name$i
		    return $name$i
		}
	    }
	} else {
	    incr uniqueNum
	    return $parent/$patchInstance-x$uniqueNum
	}
    }

    public method DragConnection {win wire x y constrained} {
	set x [$win canvasx $x]
	set y [$win canvasy $y]
	
	if {$constrained} {
	    global ForceX ForceY OldX OldY
	    if {$ForceX == 0 && $ForceY == 0} {
		if {[expr abs($x - $OldX)] > [expr abs($y - $OldY)]} {
		    set ForceX 1
		    set ForceY 0
		    set y $OldY
		    set x $OldX
		} else {
		    set ForceX 0
		    set ForceY 1
		    set x $OldX
		    set y $OldY
		}
	    } elseif {$ForceX} {
		set y $OldY
	    } else {
		set x $OldX
	    }
	}
	set coords [$win coords $wire]
	eval $win coords $wire [lrange $coords 0 [expr [llength $coords] - 3]] $x $y
	set OldX $x
	set OldY $y
    }
    
    
    public method MakeInternalConnection {win outlet inlet internalPts} {
	set wire [GetUniqueName _wire]
	eval $win create line [$outlet ConnectX] [$outlet ConnectY] \
		$internalPts [$inlet ConnectX] [$inlet ConnectY] \
		-width 2 -tag $wire \
		[GetTypeStyle [oswType [$outlet cget -outlet]]]
	
	set connection [Connection [GetUniqueName] -wire $wire -inlet $inlet -outlet $outlet -win $win -internalPts $internalPts]
	$win bind $wire <Enter> [list $this EnterWire $win $wire]
	$win bind $wire <Leave> [list $this LeaveWire $win $wire]
	$win bind $wire <Button-1> "$this SelectWire $win $connection false"
	$win bind $wire <Shift-Button-1> "$this SelectWire $win $connection true"
    }

    
    public method MakeConnectionRelative {outlet inlet internalPts} {
	MakeConnection ::outlet-$parent/$patchInstance/$outlet \
		::inlet-$parent/$patchInstance/$inlet \
		$internalPts 1
    }

    public method MakeConnection {outlet inlet internalPts args} {
	if {[catch {oswConnect [$outlet cget -outlet] [$inlet cget -inlet]}]} {
	    if {[llength $args] != 0} {
		global _oswSecondChanceConnections
		lappend _oswSecondChanceConnections \
			[list $outlet $inlet $internalPts]
		return
	    } else {
		global errorInfo
		bgerror $errorInfo
		return
	    }
	}
	MakeInternalConnection $win $outlet $inlet $internalPts
	update idletasks
    }

    public method MakeConnectionSecondChance {outlet inlet internalPts} {
	if {[catch {oswConnect [$outlet cget -outlet] [$inlet cget -inlet]}]} {
	    return 1
	} else {
	    MakeInternalConnection $win $outlet $inlet $internalPts
	    update idletasks
	    return 0
	}
    }

    public method EstablishConnection {win wire transform x y constrained} {
	
	global _whackyEventFlag
	if {$_whackyEventFlag} {
	    set _whackyEventFlag 0
	    return
	}
	global oswConnectionSource oswConnectionSink
	global oswGravityWell

	if {[llength $oswConnectionSink] == 0} {
	    set x [$win canvasx $x]
	    set y [$win canvasy $y]
	    set overlaps [$win find overlapping [expr $x - $oswGravityWell] \
		    [expr $y - $oswGravityWell] \
		    [expr $x + $oswGravityWell] [expr $y + $oswGravityWell]]
	    foreach id $overlaps {
		if {[lsearch [$win gettags $id] inlet] != -1} {
		    set oswConnectionSink [lindex [$win gettags $id] 0]
		}
	    }      
	    if {[llength $oswConnectionSink] == 0} {
		#a vertex in the wire
		global ForceX ForceY OldX OldY
		if {$ForceX} {
		    set OldX $x
		} else {
		    set OldY $y
		}
		set ForceX 0
		set ForceY 0
		eval $win coords $wire [$win coords $wire] $x $y
		return
	    }
	}

	bind $win <Motion> {}
	bind $win <Button-1> {}
	bind $win <Shift-Motion> {}
	bind $win <Shift-Button-1> {}
	
	if {[catch [oswConnect $oswConnectionSource $oswConnectionSink]]} {
	    global errorInfo
	    bgerror $errorInfo
	    $win delete $wire
	    return
	}
	
	set coords [$win coords $wire]
	set internalPts [lrange $coords 2 [expr [llength $coords] - 3]]
	$win delete $wire
	
	if {[::inlet-$oswConnectionSink cget -connection] != {}} {
	    delete object [::inlet-$oswConnectionSink cget -connection]
	}
	set outlet ::outlet-$oswConnectionSource
	set inlet ::inlet-$oswConnectionSink

	if {[$outlet cget -transform] == [$inlet cget -transform]} {
	    set transform [$outlet cget -transform]
	    set x [expr [$transform cget -x] + [$transform cget -width] + 20]
	    set internalPts [list $x [$outlet ConnectY] $x [$inlet ConnectY]]
	} else {
	    set internalPts [ManhattanizeWire [$outlet ConnectX] [$outlet ConnectY] \
		    [$inlet ConnectX] [$inlet ConnectY] $internalPts]
	}
	MakeInternalConnection $win $outlet $inlet $internalPts
	
    }

    public method CancelConnection {win wire} {
	$win delete $wire
	bind $patchcanvas <Button-1> "$this ButtonOverNothing %x %y" 

    }

    private method MakeTransformLine {transform} {
	global initCmd
	set transformCmd [list MakeTransform \
		[$transform cget -x] [$transform cget -y] \
		[$transform WidgetWidth] [$transform WidgetHeight] \
		$initCmd($transform)]
	if {[$transform cget -autonamed]} {
	    lappend transformCmd [$transform cget -transformName]
	}
	return $transformCmd
    }
    
    private method MakeTransformXMLSetXY {transform x y} {
	global initCmd
	if {[$transform cget -isSubpatch]} {
	    set patchAttr " x=\"$x\""
	    append patchAttr " y=\"$y\""
	    append patchAttr " width=\"[$transform WidgetWidth]\""
	    append patchAttr " height=\"[$transform WidgetHeight]\""
	    append patchAttr " color=\"[$transform cget -color]\""
	    return [::patchObj-[$transform cget -transformPath] SaveXML $patchAttr]
	}
	set transformLine "<TRANSFORM "
	set command $initCmd($transform)
	set commandElem ""
	if {[xmlfix $command] == $command} {
	    append transformLine " command=\"$command\""
	} else {
	    set commandElem [xmlfix $command]
	}
	if {[$transform cget -autonamed]} {
	    set mytransformName [$transform cget -transformName]
	    regsub "_inlet" $mytransformName "" mytransformName
	    regsub "_outlet" $mytransformName "" mytransformName
	    append transformLine " name=\"[xmlvalfix $mytransformName]\""
	}
	append transformLine " x=\"$x\""
	append transformLine " y=\"$y\""
	append transformLine " width=\"[$transform WidgetWidth]\""
	append transformLine " height=\"[$transform WidgetHeight]\""
	append transformLine " color=\"[$transform cget -color]\""
	append transformLine " fontface=\"[$transform cget -fontface]\""
	append transformLine " fontsize=\"[$transform cget -fontsize]\""
	
	append transformLine ">\n"
	if {$commandElem != ""} {
	    append transformLine "<COMMAND>$commandElem</COMMAND>\n"
	}
	set transformPath [$transform cget -transformPath]
	if {[oswIsDataSource $transformPath]} {
	    set src [oswGet $transformPath/src]
	    if {$src != ""} {
		if {[string first $dir $src] == 0} {
		    set src [string range $src [expr [string length $dir] + 1] end]
		}
		append transformLine "\t<DATA src=\"[xmlfix $src]\" />\n";
	    } else {
		set data [oswGetData $transformPath]	
		if {[llength $data] > 0} {
		    append transformLine "\t<DATA>\n\t\t[xmlfix $data] \n\t</DATA>\n"
		}
	    }
	}
	foreach stateLine [$transform EnumPersistentStatesXML] {
	    regsub -all -- $parent/$patchInstance/ \
		    $stateLine "" fixedLine
	    append transformLine "\n$fixedLine\n"
	}
	append transformLine "</TRANSFORM>"
	return $transformLine
    }

    private method MakeTransformXML {transform} {
	global initCmd
	if {[$transform cget -isSubpatch]} {
	    set patchAttr " x=\"[$transform cget -x]\""
	    append patchAttr " y=\"[$transform cget -y]\""
	    append patchAttr " width=\"[$transform WidgetWidth]\""
	    append patchAttr " height=\"[$transform WidgetHeight]\""
	    append patchAttr " color=\"[$transform cget -color]\""

	    return [::patchObj-[$transform cget -transformPath] SaveXML $patchAttr]
	}
	set transformLine "<TRANSFORM "
	set command $initCmd($transform)
	set commandElem ""
	if {[xmlfix $command] == $command} {
	    append transformLine " command=\"$command\""
	} else {
	    set commandElem [xmlfix $command]
	}
	if {[$transform cget -autonamed]} {
	    set mytransformName [$transform cget -transformName]
	    regsub "_inlet" $mytransformName "" mytransformName
	    regsub "_outlet" $mytransformName "" mytransformName
	    append transformLine " name=\"[xmlvalfix $mytransformName]\""
	}
	append transformLine " x=\"[$transform cget -x]\""
	append transformLine " y=\"[$transform cget -y]\""
	append transformLine " width=\"[$transform WidgetWidth]\""
	append transformLine " height=\"[$transform WidgetHeight]\""
	append transformLine " color=\"[$transform cget -color]\""
	append transformLine " fontface=\"[$transform cget -fontface]\""
	append transformLine " fontsize=\"[$transform cget -fontsize]\""
	append transformLine ">\n"
	if {$commandElem != ""} {
	    append transformLine "<COMMAND>$commandElem</COMMAND>\n"
	}
	set transformPath [$transform cget -transformPath]
	if {[oswIsDataSource $transformPath]} {
	    set src [oswGet $transformPath/src]
	    if {$src != ""} {
		if {[string first $dir $src] == 0} {
		    set src [string range $src [expr [string length $dir] + 1] end]
		}
		append transformLine "\t<DATA src=\"[xmlfix $src]\" />\n";
	    } else {
		set data [oswGetData $transformPath]	
		if {[llength $data] > 0} {
		    append transformLine "\t<DATA>\n\t\t[xmlfix $data] \n\t</DATA>\n"
		}
	    }
	}
	foreach stateLine [$transform EnumPersistentStatesXML] {
	    regsub -all -- $parent/$patchInstance/ \
		    $stateLine "" fixedLine
	    append transformLine "\n$fixedLine\n"
	}
	append transformLine "</TRANSFORM>"
	return $transformLine
    }


    public method EnumTransforms {} {
	global initCmd
	set result {}
	foreach transform $transforms {
	    lappend result [MakeTransformXML $transform]
	}
	return $result
    }

    public method EnumSelectedTransforms {} {
	global Selection

	set result {}
	set minx 100000
	set miny 100000
	
	foreach selection $Selection {
	    if {[$selection info class] == "::TransformWidget"} {
		set x [$selection cget -x]
		set y [$selection cget -y]
		if {$x < $minx} {
		    set minx $x
		}
		if {$y < $miny} {
		    set miny $y
		}
	    }
	}

	foreach selection $Selection {
	    if {[$selection info class] == "::TransformWidget"} {
		lappend result [MakeTransformXMLSetXY ::$selection \
			[expr [$selection cget -x] - $minx] \
			[expr [$selection cget -y] - $miny]]
	    }
	}
	return $result
    }

    public method EnumSelectedConnections {} {
	global Selection
	set result {}
	foreach selection $Selection {
	    if {[$selection info class] == "::TransformWidget"} {
		set connections [::$selection EnumOutletConnectionsXML $Selection]
		if {[llength $connections] > 0} {
		    lappend result $connections
		}
	    }
	}
	return $result
    }

    public method EnumConnections {} {
	set result {}
	foreach transform $transforms {
	    set connections [$transform EnumOutletConnectionsXML {}]
	    if {[llength $connections] > 0} {
		lappend result $connections
	    }
	}
	return $result
    }

    public method EnumStickies {} {
	set result {}
	foreach sticky $stickies {
	    set stickyLine "<STICKY "
	    append stickyLine " x=\"[$sticky cget -x]\""
	    append stickyLine " y=\"[$sticky cget -y]\""
	    append stickyLine " width=\"[$sticky cget -width]\""
	    append stickyLine " height=\"[$sticky cget -height]\""
	    append stickyLine " color=\"[$sticky cget -color]\""
	    append stickyLine " fontface=\"[$sticky cget -fontface]\""
	    append stickyLine " fontsize=\"[$sticky cget -fontsize]\""
	    append stickyLine ">[xmlfix [$sticky GetText]]</STICKY>\n"
	    lappend result $stickyLine
	}
	
	return $result
    }

    public method EnumTitles {} {
	set result {}
	foreach title $titles {
	    set titleText [$title GetText]
	    set titleFix [xmlfix $titleText]
	    if {$titleText != $titleFix} {
		set titleLine "<TITLE "
		append titleLine " x=\"[$title cget -x]\""
		append titleLine " y=\"[$title cget -y]\""
		append titleLine " color=\"[$title cget -color]\""
		append titleLine " fontface=\"[$title cget -fontface]\""
		append titleLine " fontsize=\"[$title cget -fontsize]\""
		append titleLine " >"
		append titleLine $titleFix
		append titleLine "</TITLE>\n"
	    } else {
		set titleLine "<TITLE "
		append titleLine " x=\"[$title cget -x]\""
		append titleLine " y=\"[$title cget -y]\""
		append titleLine " color=\"[$title cget -color]\""
		append titleLine " text=\"$titleText\""
		append titleLine " fontface=\"[$title cget -fontface]\""
		append titleLine " fontsize=\"[$title cget -fontsize]\""
		append titleLine " />\n"
	    }
	    lappend result $titleLine                    
	}
	
	return $result
    }

    public method EnumLinks {} {
	set result {}
	foreach link $links {
	    set titleLine "<HYPERLINK "
	    append titleLine " x=\"[$link cget -x]\""
	    append titleLine " y=\"[$link cget -y]\""
	    append titleLine " color=\"[$link cget -color]\""
	    append titleLine " link=\"[$link GetText]\""
	    append titleLine " fontface=\"[$link cget -fontface]\""
	    append titleLine " fontsize=\"[$link cget -fontsize]\""
	    append titleLine " />"
	    lappend result $titleLine
	}
	
	return $result
    }

    public method EnumImages {} {
	set result {}
	foreach link $imgs {
	    set titleLine "<IMG "
	    append titleLine " x=\"[$link cget -x]\""
	    append titleLine " y=\"[$link cget -y]\""
	    set src [$link cget -filename]
	    if {[string first $dir $src] == 0} {
		set src [string range $src [expr [string length $dir] + 1] end]
	    }
	    append titleLine " src=\"$src\""
	    append titleLine " />"
	    lappend result $titleLine
	}
	
	return $result
    }

    public method AddScript {script} {
	lappend scripts [list "text" $script]
    }

    public method AddScriptSrc {src} {
	lappend scripts [list "src" $src]
    }

    public method Save {} {

	if {$sourceFile == ""} {
	    SaveAs
	} else {
	    SaveToFile 
	}
	set dirty 0
    }

    public method SaveAs {} {

	set sourceFile [tk_getSaveFile -defaultextension ".osw" -filetypes {{"OSW Patch" {.osw}}} ]
	if {$sourceFile != ""} {
	    set patchClass [file root [file tail $sourceFile]]
	    if {![catch {SaveToFile}]} {
		oswSetRecentPatch $sourceFile
	    }
	    #wm title $win [list OSW $patchClass - $patchInstance]
	}
    }

    public method SaveToFile {} {
	# If the current file exists, make a backup in $sourceFile~1~
	# If that exists, move it to $sourceFile~2~ up to ~5~
	set backupNum 4
	while {[expr $backupNum > 0] && ![file exists $sourceFile]} {
	    incr backupNum -1
	}

	# Now $backupNum contains the highest numbered backup < 5 that
	# exists, or 0 (we don't need 5, because if 4 exists, 5 is
	# just overwritten)
	for {} {$backupNum > 0} {incr backupNum -1} {
	    # The if is just to avoid weird situations
	    if {[file exists "$sourceFile~$backupNum~"]} {
		file rename -force "$sourceFile~$backupNum~" "$sourceFile~[expr $backupNum + 1]~"
	    }
	}

	if {[file exists "$sourceFile"]} {
	    file rename -force "$sourceFile" "$sourceFile~1~"
	}
	
	set outputChn [open $sourceFile w]
	puts $outputChn [SaveXML]
	close $outputChn
    }

    public method SaveXML {args} {

	set patchXMLText ""

	SortTransforms
	
	if {[llength $args] == 0} {
	    #this is a "top-level" patch
	    append patchXMLText "<?xml version=\"1.0\" ?>\n"
	    append patchXMLText "<OSW Version=\"1.0.0\">\n"
	}
	append patchXMLText "<PATCH CLASS=\"$patchClass\" Version=\"$::oswversion\" WindowSize=\"[GetPatchWindowWidth] [GetPatchWindowHeight]\" Background=\"[$win cget -bg]\""
	foreach arg $args {
	    append patchXMLText " $arg "
	}
	append patchXMLText ">\n"

	if {$copyright != ""} {
	    append patchXMLText "<COPYRIGHT>\n"
	    append patchXMLText $copyright
	    append patchXMLText "\n</COPYRIGHT>\n"
	}

	foreach transformLine [EnumTransforms] {
	    regsub -all -- $parent/$patchInstance/ \
		    $transformLine "" fixedLine 
	    append patchXMLText "$fixedLine\n"
	}
	foreach connections [EnumConnections] {
	    foreach connectionLine $connections {
		regsub -all -- $parent/$patchInstance/ \
			$connectionLine "" fixedLine 
		regsub -all -- "::outlet-" \
			$fixedLine "" fixedLine 
		regsub -all -- "::inlet-" \
			$fixedLine "" fixedLine 
		append patchXMLText "$fixedLine\n"
	    }
	}
	
	foreach sticky [EnumStickies] {
	    append patchXMLText $sticky
	}
	foreach title [EnumTitles] {
	    append patchXMLText $title
	}
	foreach link [EnumLinks] {
	    append patchXMLText $link
	}
	foreach link [EnumImages] {
	    append patchXMLText $link
	}
	foreach script $scripts {
	    if {[lindex $script 0] == "src"} {
		append patchXMLText "<SCRIPT language=\"tcl\" src=\"[lindex $script 1]\" />\n"
	    } else {
		append patchXMLText "<SCRIPT language=\"tcl\">\n"
		append patchXMLText [xmlfix [lindex $script 1]]
		append patchXMLText "\n</SCRIPT>\n"
	    }
	}
	append patchXMLText "</PATCH>\n"
	if {[llength $args] == 0} {
	    append patchXMLText "</OSW>\n";
	}
	return $patchXMLText
    }

    private variable sortID 0

    public method SortAux {transform} {
        if {[$transform cget -visit] == $sortID} {
	    return {} 
	} else {
	    $transform configure -visit $sortID
	    if {[llength [$transform EnumConnectedToInlets]] == 0} {
		return $transform
	    } else {
		set prelist {} 
		foreach connected [$transform EnumConnectedToInlets] {
		    set prelist [concat $prelist [SortAux $connected]]
		}
		lappend prelist $transform
		return $prelist
	    }	    
	}
    }

    public method SortTransforms {} {
	if {[llength $transforms] > 0} {
	    incr sortID
	    return [SortAux [lindex $transforms 0]]
	}
    }

    public method SetPatchWindowSize {width height} {
	$patchcanvas configure -width $width -height $height
    }

    public method GetPatchWindowWidth {} {
	return [$patchcanvas cget -width] 
    }
    
    public method GetPatchWindowHeight {} {
	return [$patchcanvas cget -height] 
    }

    public method AddNewTransformFromKeyboard {} {
	AddBlankTransform [expr rand() * [$win cget -width]] \
		[expr rand() * [$win cget -height]]
    }

    public method DragNewTransform {type args} {
	if {$locked} {
	    return
	}

	if {[llength $args] != 0} {
	    set defaultname [lindex $args 0]
	} else {
	    set defaultname [string tolower $type]
	}
	$pane configure -cursor icon
	.dock configure -cursor icon
	bind $win <Button-1> [list $this DropNewTransform $type %x %y $defaultname]
	bind $win <Button-3> [list $this CancelNewTransform]
    }

    public method DropNewTransform {type x y defaultname} {
	bind $win <Button-1> [list $this ButtonOverNothing %x %y]
	bind $win <Button-3> {}
	$pane configure -cursor $::defaultCursor
	.dock configure -cursor $::defaultCursor
	switch $type Button {
	    MakeTransform $x $y 32 32 [list Button [GetUniqueName button]]
	} blank {
	    AddBlankTransform $x $y
	} default {
	    MakeTransform $x $y -1 -1 [linsert $type 1 \
		    [GetUniqueName $defaultname]]
	}
    }

    public method CancelNewTransform {} {
	bind $win <Button-1> [list $this ButtonOverNothing %x %y]
	bind $win <Button-3> {}
	$pane configure -cursor $::defaultCursor
	.dock configure -cursor $::defaultCursor
    }

    public method DragNewSticky {} {
	if {$locked} {
	    return
	}
	$pane configure -cursor icon
	bind $win <Button-1> [list $this DropNewSticky %x %y]
    }

    public method DropNewSticky {x y} {
	bind $win <Button-1> {}
	$pane configure -cursor $::defaultCursor
	set dirty 1
	MakeSticky $x $y 75 50 "#FFFF77" ""
    }


    public method MakeSticky {x y width height color txt} {
	set newsticky [stickynote ::#auto -win $win \
		-x $x -y $y -color $color\
		-width $width -height $height -patchWindow $this]
	lappend stickies $newsticky
	$newsticky SetText $txt
	ObjectSelection [string trimleft $newsticky ":"] false
	if {$locked} {
	    $newsticky Lock
	}
	return $newsticky
    }

    public method DragNewTitle {} {
	if {$locked} {
	    return
	}
	$pane configure -cursor icon
	bind $win <Button-1> [list $this DropNewTitle %x %y]
    }

    public method DropNewTitle {x y} {
	bind $win <Button-1> {}
	$pane configure -cursor $::defaultCursor
	set dirty 1
	MakeTitle $x $y "#FFFFFF" ""
    }


    public method MakeTitle {x y color txt} {
	set newtitle [Title ::#auto -win $win \
		-x $x -y $y -color $color \
		-patchWindow $this]
	lappend titles $newtitle
	$newtitle SetText $txt
	ObjectSelection [string trimleft $newtitle ":"] false
	if {$locked} {
	    $newtitle Lock
	}
	return $newtitle
    }

    public method DragNewLink {} {
	if {$locked} {
	    return
	}
	$pane configure -cursor icon
	bind $win <Button-1> [list $this DropNewLink %x %y]
    }

    public method DropNewLink {x y} {
	bind $win <Button-1> {}
	$pane configure -cursor $::defaultCursor
	set dirty 1
	MakeLink $x $y "#0000FF" ""
    }


    public method MakeLink {x y color txt} {
	set newtitle [Hyperlink ::#auto -win $win \
		-x $x -y $y -color $color \
		-patchWindow $this]
	lappend links $newtitle
	$newtitle SetText $txt
	ObjectSelection [string trimleft $newtitle ":"] false
	if {$locked} {
	    $newtitle Lock
	}
	return $newtitle
    }

    public method DragNewImage {} {
	if {$locked} {
	    return
	}
	$pane configure -cursor icon
	bind $win <Button-1> [list $this DropNewImage %x %y]
    }

    public method DropNewImage {x y} {
	bind $win <Button-1> {}
	$pane configure -cursor $::defaultCursor
	set dirty 1
	MakeImage $x $y ""
    }

    public method MakeImage {x y filename} {
	set newimage [Image ::x#auto -win $win \
		-x $x -y $y -filename $filename \
		-patchWindow $this]
	lappend imgs $newimage
	ObjectSelection [string trimleft $newimage ":"] false
	if {$locked} {
	    $newimage Lock
	}
	return $newimage
    }

    public method DragDockedPrototype {transformCmd instanceName} {
	if {$locked} {
	    return
	}
	set oldname $instanceName
	set instanceName [GetUniqueName [string trimright $oldname "0123456789"]]
	$pane configure -cursor icon
	.dock configure -cursor icon
        bind $win <Button-1> [list $this DropPrototype %x %y $transformCmd $instanceName]
	bind $win <Button-3> [list $this CancelPrototype]

    }

    public method DropPrototype {x y transformCmd instanceName} {
	bind $win <Button-1> [list $this ButtonOverNothing %x %y]
	bind $win <Button-3> {}
	$pane configure -cursor $::defaultCursor
	.dock configure -cursor $::defaultCursor
	set dirty 1
	MakeTransform $x $y -1 -1 $transformCmd $instanceName
    }

    public method CancelPrototype {} {
	bind $win <Button-1> [list $this ButtonOverNothing %x %y]
	bind $win <Button-3> {}
	$pane configure -cursor $::defaultCursor
	.dock configure -cursor $::defaultCursor
    }

    
    public method EnterWire {win wire} {
	$win itemconfigure $wire -width [expr [$win itemcget $wire -width] + 2]
    }

    public method LeaveWire {win wire} {
	if {[catch {$win itemconfigure $wire -width [expr [$win itemcget $wire -width] - 2]}]} {
	    $win itemconfigure $wire -width 2
	}
    }

    public method SelectWire {win connection shift} {
	global Selection
	
	if {$shift} {
	    if {[lsearch $Selection $connection] == -1} {
		lappend Selection $connection
		$connection Select
	    }
	} else {
	    ForgetSelection
	    set Selection $connection
	    $connection Select
	}
	SetEditMenu true
    }

    public method SelectBackground {} {

	$win configure -bg \
		[tk_chooseColor -initialcolor \
		[[$patchcanvas component canvas] cget -bg] \
		-title "Patch Background Color"]
    }

    public method Background {newcolor} {
	#protected against Unix seeing Windows default colors
	# (The Tcl folks should really just fix this!)
	catch {$win configure -bg $newcolor}
    }

    public method SelectTransformColor {} {
	

	set newcolor [tk_chooseColor -initialcolor darkgrey]

	foreach sel $::Selection {
	    if {[$sel info class] == "::TransformWidget"} {
		$sel WidgetColor $newcolor
	    } elseif {[$sel info class] == "::stickynote"} {
		$sel WidgetColor $newcolor
	    } elseif {[$sel info class] == "::Title"} {
		$sel WidgetColor $newcolor
	    } elseif {[$sel info class] == "::Hyperlink"} {
		$sel WidgetColor $newcolor
	    }
	}
    }

    public method SelectTransformFontFace {face} {

	foreach sel $::Selection {
	    if {[$sel info class] == "::TransformWidget"} {
		$sel WidgetFontFace $face
	    } elseif {[$sel info class] == "::stickynote"} {
		$sel WidgetFontFace $face
	    } elseif {[$sel info class] == "::Title"} {
		$sel WidgetFontFace $face
	    } elseif {[$sel info class] == "::Hyperlink"} {
		$sel WidgetFontFace $face
	    }
	}
    }

    public method SelectTransformFontSize {size} {

	foreach sel $::Selection {
	    if {[$sel info class] == "::TransformWidget"} {
		$sel WidgetFontSize $size
	    } elseif {[$sel info class] == "::stickynote"} {
		$sel WidgetFontSize $size
	    } elseif {[$sel info class] == "::Title"} {
		$sel WidgetFontSize $size
	    } elseif {[$sel info class] == "::Hyperlink"} {
		$sel WidgetFontSize $size
	    }
	}
    }

    public method RegisterTransform {transformObj transformPath} {
	set transformMap($transformPath) $transformObj
	lappend usedNames [file tail $transformPath]

    }

    public method Map {transformPath} {
	return $transformMap($transformPath)
    }

    public method UnregisterTransform {transformObj transformPath} {
    }

    private method TransformProperty {transformPath args} {
	eval $transformMap($transformPath) $args
    }

    public method SelectAll {} {
	ForgetSelection
	foreach transform $transforms {
	    ObjectSelection [string trimleft $transform ":"] true
	}
	foreach sticky $stickies {
	    ObjectSelection [string trimleft $sticky ":"] true
	}
	foreach title $titles {
	    ObjectSelection [string trimleft $title ":"] true
	}
	foreach link $links {
	    ObjectSelection [string trimleft $link ":"] true
	}
	foreach img $imgs {
	    ObjectSelection [string trimleft $img ":"] true
	}
    }		

    public method Copy {} {
	global oswClipboard Selection
	
	if {$::isRecording} {
	    puts $::logfile "<<ToClipboard>>"
	}

	set clippedXML ""

	foreach transformLine [EnumSelectedTransforms] {
	    regsub -all -- $parent/$patchInstance/ \
		    $transformLine "" fixedLine 
	    append clippedXML "$fixedLine\n"
	}
	foreach connections [EnumSelectedConnections] {
	    foreach connectionLine $connections {
		regsub -all -- $parent/$patchInstance/ \
			$connectionLine "" fixedLine 
		regsub -all -- "::outlet-" \
			$fixedLine "" fixedLine 
		regsub -all -- "::inlet-" \
			$fixedLine "" fixedLine 
		append clippedXML "$fixedLine\n"
	    }
	}
	set clippedTransforms {}
	foreach sel $Selection {
	    if {[$sel info class] == "::TransformWidget"} {
		lappend clippedTransforms [$sel cget -transformName]
	    }
	}
	set oswClipboard [list $clippedTransforms $clippedXML]
	clipboard clear
	clipboard append $clippedXML
	SetEditMenu 0
    }

    public method Cut {} {
	Copy
	DeleteSelection
    }

    public method Paste {} {
	global oswClipboard Selection
	set PasteContextX $PasteContextSeedX
	set PasteContextY $PasteContextSeedY
	incr PasteContextSeedX 100
	incr PasteContextSeedY 100

	if {$::isRecording} {
	    puts $::logfile "<<FromClipboard>>"
	}
	ForgetSelection 

	set clipboardXML [lindex $oswClipboard 1]
	set newnames {}
	foreach transform [lindex $oswClipboard 0] {
	    set adjustedName [GetUniqueName \
		    [string trimright $transform "0123456789"]]
	    lappend newnames $adjustedName
	    regsub -all $transform $clipboardXML $adjustedName clipboardXML
	}
	
	PushXMLState PATCH $this
	oswParseXML $clipboardXML {}
	PopXMLState
	set PasteContextX 0
	set PasteContextY 0
	ForgetSelection
	foreach newname $newnames {
	    ObjectSelection \
		    [string trimleft $transformMap($parent/$patchInstance/$newname) ":"] \
		    true
	}
	focus $patchcanvas
	SetEditMenu 0
    }

    public method Collapse {} {
	

	if {$::isRecording} {
	    puts $::logfile "<<Collapse>>"
	}

	set x 10000
	set y 10000
	foreach sel $Selection {
	    if {[$sel info class] == "::TransformWidget"} {		
		set transform ::$sel 
		if {[$transform cget -x] < $x} {
		    set x [$transform cget -x]
		}
		if {[$transform cget -y] < $y} {
		    set y [$transform cget -y]
		}
	    }
	}

	Cut
	set newPatchName [GetUniqueName "patch"]
	MakeTransform $x $y 80 20 [list Patch $newPatchName]
	::patchObj-$parent/$patchInstance/$newPatchName Paste
	::patchObj-$parent/$patchInstance/$newPatchName ExportUnconnected	
    }

    public method ExportUnconnected {} {
	set inletX 10
	set outletX 10
	foreach transform $transforms {
	    foreach inlet [$transform EnumUnconnectedInlets] {
		set inletName [$transform cget -transformName]_[file tail $inlet]
		MakeTransform $inletX 20 60 40 [list Inlet $inletName -type [oswType $inlet]]
		incr inletX 60
		MakeConnection ::outlet-$parent/$patchInstance/[set inletName]_inlet/out \
			::inlet-$inlet {}
	    }
	    foreach outlet [$transform EnumUnconnectedOutlets] {
		set outletName [$transform cget -transformName]_[file tail $outlet]
		MakeTransform $outletX 500 60 40 [list Outlet $outletName -type [oswType $outlet]]
		incr outletX 60
		MakeConnection ::outlet-$outlet \
			::inlet-$parent/$patchInstance/[set outletName]_outlet/in {}
	    }
	}
    }


    private variable argumentVariables {}
    private variable argDesc
    private variable argDefault
    private variable argType

    public method ShowArgument {dsite} {
	set argName [$dsite.name getcurselection]
	if {$argName == ""} {
	    return
	}
	if {[catch {set argDesc($argName)}]} {
	    set argDesc($argName) ""
	    set argDefault($argName) ""
	    set argType($argName) ""
	}
	$dsite.desc configure -textvariable [scope argDesc($argName)]
	$dsite.type configure -textvariable [scope argType($argName)]
	$dsite.def configure -textvariable [scope argDefault($argName)]
    }

    
    public method CreateArguments {} {
	set ds [iwidgets::dialogshell .ds#auto -modality application]
	$ds add OK -text "OK" -command [list $this DoneArguments $ds]
	$ds add Cancel -text "Cancel" -command "destroy $ds" 
	set dsite [$ds childsite]
	iwidgets::combobox $dsite.name -labeltext "Name:" -labelpos w \
		-selectioncommand [list $this ShowArgument $dsite]
	iwidgets::entryfield $dsite.desc -labeltext "Description:" -labelpos w
	iwidgets::entryfield $dsite.type -labeltext "Type:" -labelpos w
	iwidgets::entryfield $dsite.def -labeltext "Default:" -labelpos w
	bind $dsite.name <Delete> "$dsite.name delete list \[ $dsite.name curselection \]"
	if {[llength $argumentVariables] > 0} {
	    eval $dsite.name insert list end $argumentVariables
	}
	pack $dsite.name $dsite.desc $dsite.type $dsite.def -side top
	$ds activate
    }

    public method DoneArguments {ds} {
	set dsite [$ds childsite]
	set argumentVariables [[$dsite.name component list] get 0 end]
	destroy $ds
    }

    public method InitArguments {} {
	
	for {set i 0} {$i < [llength $patchArgs]} {incr i} {
	    if {[string match -* [lindex $patchArgs $i]]} {
		set argName [string trimleft [lindex $patchArgs $i] "-"]
		incr i
		$parent/$patchInstance addargument $argName Any ""
		oswSet $parent/$patchInstance/$argName [lindex $patchArgs $i]
	    }
	}
    }
    public method RegisterArgument {aname adesc atype adef} {
	lappend argumentVariables $aname
	set argDesc($aname) $adesc
	set argType($aname) $atype
	set argDefault($aname) $adef

	oswSet $parent/$patchInstance/$aname $adef
    }


    private variable locked true
    public method LockUnlockButton {} {
	if {$locked} {
	    Unlock
	} else {
	    Lock 
	}
    }

    public method Lock {} {
	set locked true
	foreach transform $transforms {
	    $transform Lock
	}
	foreach sticky $stickies {
	    $sticky Lock
	}
	foreach title $titles {
	    $title Lock
	}
	foreach link $links {
	    $link Lock
	}
	foreach img $imgs {
	    $img Lock
	}
	$view ToggleLockButton $locked
    }

    public method Unlock {} {
	set locked false
	foreach transform $transforms {
	    $transform Unlock
	}
	foreach sticky $stickies {
	    $sticky Unlock
	}
	foreach title $titles {
	    $title Unlock
	}
	foreach link $links {
	    $link Unlock
	}
	foreach img $imgs {
	    $img Unlock
	}
	$view ToggleLockButton $locked
    }


    public method ScaleUp {} {
    }

    public method ScaleDown {} {	
    }

    private variable _currentdock

    
    
    private method HideName {} {
	pack forget $_currentdock.x.e
    }

    public method DockSelection {} {
	global Selection

	if {$::isRecording} {
	    puts $::logfile "<<Dock>>"
	}

	foreach sel $Selection {
	    if {[$sel info class] == "::TransformWidget"} {
		$sel Dock
	    }
	}
    }

    public method Draw {x y} {
	set x [$patchcanvas canvasx $x]
	set y [$patchcanvas canvasy $y]

	$win  create line $lastdrawX $lastdrawY $x $y \
		-tag gesture$gestureNum -stipple gray75
	
	set lastdrawX $x
	set lastdrawY $y
    }

    private method CountCrossings {crossings region} {
	set numcrossings 0
	foreach crossing $crossings {
	    if {[lsearch [$win gettags $crossing] $region] > -1} {
		incr numcrossings
	    }
	}
	return $numcrossings
    }

    private method IsVertexInRegion {vertex region} {
	set bbox [$win bbox $region]
	set crossings [$win find overlap [lindex $bbox 0] \
		[lindex $vertex 1] [lindex $vertex 0] [lindex $vertex 1]]
	
	set warning false
	if {[CountCrossings $crossings $region] % 2 == 0} {
	    set warning true
	}
	set crossings [$win find overlap [lindex $bbox 2] \
		[lindex $vertex 1] [lindex $vertex 2] [lindex $vertex 1]]
	if {[CountCrossings $crossings $region] % 2 == 0} {
	    if {$warning} {
		return false
	    }
	}
	set warning false
	set crossings [$win find overlap [lindex $vertex 0] \
		[lindex $bbox 1] [lindex $vertex 0] [lindex $vertex 1]]
	if {[CountCrossings $crossings $region] % 2 == 0} {
	    set warning true
	}
	set crossings  [$win find overlap [lindex $vertex 0] \
		[lindex $bbox 3] [lindex $vertex 0] [lindex $vertex 3]]
	if {[CountCrossings $crossings $region] % 2 == 0} {
	    if {$warning} {
		return false
	    }
	}
	return true
    }

    public method InterpretDrawing {} {
	if {[llength [$win bbox gesture$gestureNum]] == 0} {
	    #we probably shouldn't even be here
	    return
	}
	
	if {abs($lastdrawX - $firstdrawX) > 50} {
	    $win delete gesture$gestureNum
	    bind $win <B1-Motion> {}
	    bind $win <ButtonRelease-1> {}
	    return
	}
	set enclosed [eval $win find enclosed [$win bbox gesture$gestureNum]]
	set numenclosed 0
	foreach transform [$win find withtag transform] {
	    if {[lsearch $enclosed $transform] > -1} {
		if {[IsVertexInRegion [$win coords $transform] \
			gesture$gestureNum]} {
		    set transformtag [$win gettags $transform]
		    ldelete transformtag $transform
		    ldelete transformtag transform
		    ObjectSelection $transformtag true
		    incr numenclosed
		}
	    }
	}
	foreach sticky [$win find withtag sticky] {
	    if {[lsearch $enclosed $sticky] > -1} {
		if {[IsVertexInRegion [$win coords $sticky] \
			gesture$gestureNum]} {
		    set stickytag [$win gettags $sticky]
		    ldelete stickytag $sticky
		    ldelete stickytag sticky
		    ObjectSelection $stickytag true
		    incr numenclosed
		}
	    }
	}
	foreach title [$win find withtag title] {
	    if {[lsearch $enclosed $title] > -1} {
		if {[IsVertexInRegion [$win bbox $title] \
			gesture$gestureNum]} {
		    set titletag [$win gettags $title]
		    ldelete titletag $title
		    ldelete titletag title
		    ObjectSelection $titletag true
		    incr numenclosed
		}
	    }
	}
	foreach link [$win find withtag link] {
	    if {[lsearch $enclosed $link] > -1} {
		if {[IsVertexInRegion [$win bbox $link] \
			gesture$gestureNum]} {
		    set linktag [$win gettags $link]
		    ldelete linktag $link
		    ldelete linktag link
		    ObjectSelection $linktag true
		    incr numenclosed
		}
	    }
	}
	foreach img [$win find withtag img] {
	    if {[lsearch $enclosed $img] > -1} {
		if {[IsVertexInRegion [$win bbox $img] \
			gesture$gestureNum]} {
		    set imgtag [$win gettags $img]
		    ldelete imgtag $img
		    ldelete imgtag img
		    ObjectSelection $imgtag true
		    incr numenclosed
		}
	    }
	}
	if {$numenclosed > 0} {
	    $win delete gesture$gestureNum
	} else {
	    set foundOutlets {}
	    set foundInlets {}
	    foreach object $enclosed {
		set tags [$win gettags $object]
		if {[lsearch $tags outlet_terminal] > -1} {
		    lappend foundOutlets [lindex $tags 0] 
		} elseif {[lsearch $tags inlet_terminal] > -1} {
		    lappend foundInlets [lindex $tags 0] 
		}
	    }
	    if {[llength $foundOutlets] > 0} {
		bind $win <B1-Motion> {}
		bind $win <ButtonRelease-1> {}
		$win delete gesture$gestureNum
		if {[llength $foundInlets] == [llength $foundOutlets]} {
		    foreach outlet $foundOutlets inlet $foundInlets {
			DrawAndMakeConnection $outlet $inlet
		    }
		} else {
		    foreach outlet $foundOutlets {
			[::outlet-$outlet cget -transform] StartConnection $outlet		    
		    }
		}
	    }
	}
	$win delete gesture$gestureNum
	bind $win <B1-Motion> {}
	bind $win <ButtonRelease-1> {}
    }

    public method FixTransformTerminals {transform} {
	catch {
	    set transformObj $transformMap($transform)
	    $transformObj FixTerminals
	}
    }

    public method FixPatchTerminals {} {
	if {$parent != ""} {
	    ::patchObj-$parent FixTransformTerminals $parent/$patchInstance
	}
    }

    public method  ObjectSelection {tag shift} {    
	global Selection
	
	if {[lsearch $Selection $tag] == -1} {
	    if {$shift} {
		if {[lsearch $Selection $tag] == -1} {
		    lappend Selection $tag
		}
	    } else {
		
		ForgetSelection
		set Selection $tag
	    }
	    if {[catch {
		if {![catch {::$tag info class}] && [::$tag info class] == "::TransformWidget"} {
		    if {[catch {.patchmenu type Transform}]} {
			.patchmenu insert Patch cascade -label "Transform" -menu .patchmenu.transform
		    }
		}
	    }]} {
		global errorInfo
		bgerror $errorInfo
	    }
	    
	}

	::$tag Select
	SetEditMenu 1
    }
    
    public method ForgetSelection {} {
	global Selection
	
	foreach sel $Selection {
	    if {[catch {::$sel UnSelect}]} {
		catch {$sel UnSelect}
	    }
	}
	set Selection {}
	if {![catch {$win.patchmenu type Transform}]} {
	    $win.patchmenu delete Transform
	}

	SetEditMenu 0
    }

    public method SplitWire {splitter} {
	
	set inlets [$splitter cget -inlets]
	if {[llength $inlets] < 1} {
	    return
	}
	set outlets [$splitter cget -outlets]
	if {[llength $outlets] < 1} {
	    return
	}
	set splitinlet ::inlet-[lindex $inlets 0]
	if {[$splitinlet cget -connection] != ""} {
	    return
	}
	if {[::outlet-[lindex $outlets 0] cget -connection] != ""} {
	    return
	}

	set splitx [$splitinlet cget -x]
	set splity [$splitinlet cget -y]
	set splitoutlet ::outlet-[lindex $outlets 0]
	foreach transform $transforms {
	    foreach outlet [$transform cget -outlets] {
		if {[outlet-$outlet cget -connection] != ""} {
		    set connection [outlet-$outlet cget -connection]
		    set bbox [$connection BoundingBox]
		    if {$splitx >= [expr [lindex $bbox 0] - 5] && \
			    $splitx <= [expr [lindex $bbox 2] + 5] && \
			    $splity >= [expr [lindex $bbox 1] - 5] && \
			    $splity <= [expr [lindex $bbox 3] + 5]} {
			set inlet [$connection cget -inlet]
			if {
			    [oswType [$splitinlet cget -inlet]] == 
			    [oswType $outlet] &&
			    [oswType [$splitoutlet cget -outlet]] ==
			    [oswType [$splitinlet cget -inlet]]
			} {
			    delete object $connection
			    DrawAndMakeConnection $outlet \
				    [$splitinlet cget -inlet]
			    DrawAndMakeConnection [$splitoutlet cget -outlet] \
				    [$inlet cget -inlet]
			}
		    }
		}
	    }
	}

    }
    
    private method DrawAndMakeConnection {outlet inlet} {
	set internalPts [ManhattanizeWire [::outlet-$outlet ConnectX] \
		[::outlet-$outlet ConnectY] \
		[::inlet-$inlet ConnectX] \
		[::inlet-$inlet ConnectY] {}]
	MakeConnection ::outlet-$outlet ::inlet-$inlet $internalPts
    }

    public variable copyright ""

    public method GetCopyright {} {
	return $copyright
    }
    
    public method SetCopyright {acopyright} {
	set copyright $acopyright
    }   
}



proc oswDestroyAllPatches {} {
    foreach patch $Patch::patchObjList {
	delete object $patch
    }
}

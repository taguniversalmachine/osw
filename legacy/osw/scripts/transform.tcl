
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

class TransformWidget {

    public variable name
    public variable patch
    public variable patchWindow
    public variable transformName "" 
    public variable transformPath ""
    public variable transformClass ""
    public variable packageName ""
    public variable inlets {}
    public variable outlets {}
    public variable win
    public variable x 
    public variable y 
    public variable width 0
    public variable height 0
    public variable color darkgrey
    public variable fontface ""
    public variable fontsize 8
    public variable autonamed false
    public variable isPatch false
    public variable isSubpatch false

    private variable inletConnections
    private variable outletConnections

    private variable locked false

    private variable three 3
    private variable five 5
    private variable ten 10
    private variable seven 7
    private variable fourteen 14
    private variable fifteen 15
    private variable twenty 20

    public variable alreadyInstantiated false
    private variable prevInitCmd ""

    public variable visit -1

    private variable tooltip ""

    private variable OldWidth 0
    private variable OldHeight 0

    constructor {args} {
        eval configure $args

        frame $win.$name -bg $color -bd 2 -relief raised
	if {$::tcl_platform(os) == "Darwin"} {
	    set font [SelectFontFace $::defaultFont 10]
	} else {
	    set font [SelectFontFace $::defaultFont 8]
	}
        entry $win.$name.e -textvariable ::initCmd($this) -font $font
        frame $win.$name.f -bg $color
        pack $win.$name.e -side top -fill x
        pack $win.$name.f -side top -fill both -expand yes

        update idletasks
        
        set x10 [expr $x + 10 + [winfo width $win.$name]]
        set y10 [expr $y + 10 + [winfo height $win.$name]]
        $win create window [expr $x + 5] [expr $y + 5] \
            -anchor nw -tag frame-$name -window $win.$name  
        $win create rectangle $x $y $x10 $y10 \
            -fill $color -tag $name
        $win create rectangle \
            [expr $x + 5 + [winfo width $win.$name]] \
            [expr $y + 5 + [winfo height $win.$name]] \
            $x10 $y10 \
            -fill white -tag scale-$name
        $win addtag transform withtag $name
        $win bind $name <Button-1> "StartObjectMotion $patchWindow $win $name %x %y false"
        $win bind $name <Shift-Button-1> "StartObjectMotion $patchWindow $win $name %x %y true"
        bind $win.$name.e <Return> [list $this CreateTransform]
        bind $win.$name.e <Escape> [list $this CommandCompletion]
        $win bind scale-$name <Enter> [list $win configure -cursor sizing]
        $win bind scale-$name <Leave> [list $win configure -cursor $::defaultCursor]
        $win bind scale-$name <Button-1> [list $this StartResize %x %y]
        
        BindFocusOut

        focus $win.$name.e
        #Scale 1.0
        UpdateCanvas
    }

    destructor {
        $patchWindow UnregisterTransform $this $transformPath
        ForgetInlets
        ForgetOutlets
        $win delete $name
        $win delete frame-$name
        $win delete scale-$name
        foreach child [winfo children $win.$name.f] {
            destroy $child
        }
        destroy $win.$name.e
        destroy $win.$name.f
        destroy $win.$name

        if {$transformName != ""} {
            $patch remove $transformPath
        }
    }


    public method Lock {} {
        set locked true
        $win bind $name <Button-1> "$patchWindow ObjectSelection $name false"
        $win bind $name <Shift-Button-1> "$patchWindow ObjectSelection $name true"
        bind $win.$name.e <Return> {}
        bind $win.$name.e <Escape> {}
        $win.$name.e configure -state disabled
        $win lower scale-$name $name
    }

    public method Unlock {} {
        set locked false
        $win bind $name <Button-1> "StartObjectMotion $patchWindow $win $name %x %y false"
        $win bind $name <Shift-Button-1> "StartObjectMotion $patchWindow $win $name %x %y true"
        bind $win.$name.e <Return> [list $this CreateTransform]
        bind $win.$name.e <Escape> [list $this CommandCompletion]
        $win.$name.e configure -state normal
        $win raise scale-$name $name
    }

    public method  MoveObject {nx ny} {

        global OldX OldY
        set nx [$win canvasx $nx]
        set ny [$win canvasy $ny]
        if {$nx < 0 || $ny < 0} {
            return
        }
        set dx [expr $nx - $OldX]
        set dy [expr $ny - $OldY]
        $win move $name $dx $dy
        $win move frame-$name $dx $dy
        $win move scale-$name $dx $dy
        foreach inlet $inlets {
            $win move $inlet $dx $dy
            $win move box_$inlet $dx $dy
            ::inlet-$inlet Move $dx $dy
        }
        foreach outlet $outlets {
            $win move $outlet $dx $dy
            $win move box_$outlet $dx $dy
            ::outlet-$outlet Move $dx $dy
        }
        set x [expr $x + $dx]
        set y [expr $y + $dy]
        UpdateCanvas

        set OldX $nx
        set OldY $ny
        MoveOthers $this $dx $dy
    }

    public method  MoveObjectRel {dx dy} {
	    

        $win move $name $dx $dy
        $win move frame-$name $dx $dy
        $win move scale-$name $dx $dy
        foreach inlet $inlets {
            $win move $inlet $dx $dy
            $win move box_$inlet $dx $dy
            ::inlet-$inlet Move $dx $dy
        }
        foreach outlet $outlets {
            $win move $outlet $dx $dy
            $win move box_$outlet $dx $dy
            ::outlet-$outlet Move $dx $dy
        }
        set x [expr $x + $dx]
        set y [expr $y + $dy]
        UpdateCanvas
    }

    public method SetInlets {ainlets} {
        set inletPairs $ainlets
        set inletPairs [lsort -integer -index 0 $inletPairs]
        set inlets {}
        foreach pair $inletPairs {
            set inlet [lindex $pair 1]
            lappend inlets $inlet
            #only in Tcl, folks!
            catch {Inlet ::inlet-$inlet -inlet $inlet -transform $this}
        }
    }

    public method SetOutlets {aoutlets} {
        set outletPairs $aoutlets
        set outletPairs [lsort -integer -index 0 $outletPairs]
        set outlets {}
        foreach pair $outletPairs {
            set outlet [lindex $pair 1]
            lappend outlets $outlet	
            catch {Outlet ::outlet-$outlet -outlet $outlet -transform $this}
        }
    }


    public method Redraw {} {
        ForgetInletWidgets
        ForgetOutletWidgets
        #Scale 1.0
        $win itemconfigure frame-$name -width 0 -height 0
        DrawTransform
        update idletasks
        set width [WidgetWidth]
        set height [WidgetHeight]
        ReWidgetSize
        #WidgetSize $xwidth $xheight
        RedrawInt
    }

    public method StartResize {nx ny} {
        if {$locked} {
            return
        }
        global OldX OldY

        set OldX [$win canvasx $nx]
        set OldY [$win canvasy $ny]
        set OldWidth $width
        set OldHeight $height
        
        if {$width == 0} {
            set width [winfo width $win.$name]
        } 
        if {$height == 0} {
            set height [winfo heigth $height.$name]
        }
        $win bind scale-$name <B1-Motion> [list $this DragResize %x %y]
        $win bind scale-$name <ButtonRelease-1> [list $this DoneResize]
    }

    public method DragResize {nx ny} {
        global OldX OldY
        set nx [$win canvasx $nx]
        set ny [$win canvasy $ny]
        if {$nx < 0 || $ny < 0} {
            return
        }
        
        set NewWidth [expr $OldWidth + $nx - $OldX]
        set NewHeight [expr $OldHeight + $ny - $OldY]

        if {$NewWidth < 5} {
            set NewWidth 5
        }

        if {$NewHeight < 5} {
            set NewHeight 5
        }

        WidgetSize $NewWidth $NewHeight

        #set dx [expr $nx - $OldX]
        #set dy [expr $ny - $OldY]

        #set xwidth $width
        #set xheight $height
        #if {$xwidth + $dx > 0} {
        #    set xwidth [expr $xwidth + $dx]
        #}
        #if {$xheight + $dy > 0} {
        #    set xheight [expr $xheight + $dy]
        #}
        
        #WidgetSize $xwidth $xheight

        #set OldX $nx
        #set OldY $ny
    }

    public method DoneResize {} {
        $win bind scale-$name <B1-Motion> {}
        $win bind scale-$name <ButtonRelease-1> {}
        SetBaseScale
    }

    public method WidgetWidth {} {
        return [winfo reqwidth $win.$name]
    }

    public method WidgetHeight {} {
        return [winfo reqheight $win.$name]
    }

    public method SetWidgetSize {awidth aheight} {
        WidgetSize $awidth $aheight
    }

    public method ReWidgetSize {} {
        if {$width < 15 * [llength $inlets]} {
            set width [expr 15 * [llength $inlets]]
        } 
        if {$width < 15 * [llength $outlets]} {
            set width [expr 15 * [llength $outlets]]
        } 
    }

    public method WidgetSize {awidth aheight} {
        $win coords frame-$name [expr $x + 5] [expr $y + 5] 
        $win itemconfigure frame-$name -width $awidth -height $aheight 
        $win coords $name $x $y [expr $x + 10 + $awidth] [expr $y + 10 + $aheight]
        $win coords scale-$name [expr $x + 5 + $awidth] [expr $y + 5 + $aheight] \
            [expr $x + 10 + $awidth] [expr $y + 10 + $aheight]

        if {[llength $inlets] > 1} {
            set dinletX [expr int(($awidth - 2) / ([llength $inlets] - 1))]
        } else {
            set dinletX 1
        }
        set inletX [expr $x +  1]
        foreach inlet $inlets {
            $win coords box_$inlet $inletX [expr $y - 7] \
                [expr $inletX + 10] [expr $y + 3] 
            $win coords ${inlet}_terminal  [expr $inletX + 3] [expr $y - 5] \
                [expr $inletX + 10 - 3] [expr $y - 5] \
                [expr $inletX + 5] [expr $y] 
            ::inlet-$inlet MoveAbsolute $inletX [expr $y - 5]
            set inletX [expr $dinletX + $inletX]
        }

        if {[llength $outlets] > 1} {
            set doutletX [expr int(($awidth - 2) / ([llength $outlets] - 1))]
        } else {
            set doutletX 1
        }

        set outletX [expr $x + 1]
        foreach outlet $outlets {
            $win coords box_$outlet $outletX [expr $y + $aheight + 10 - 3] \
                [expr $outletX + 10] [expr $y + $aheight + 10 + 7]
            $win coords ${outlet}_terminal \
                [expr $outletX + 3] [expr $y + $aheight + 10] \
                [expr $outletX + 10 - 3] [expr $y + $aheight + 10] \
                [expr $outletX + 5] [expr $y + 5 + $aheight + 10]
            ::outlet-$outlet MoveAbsolute $outletX [expr $y + 10 + $aheight]
            set outletX [expr $doutletX + $outletX]
        }

        UpdateCanvas
        set width $awidth
        set height $aheight
    }

    public method AutoWidgetSize {hack} {
        set width [winfo reqwidth $win.$name]
        if {$width < 32} {
            set $width 32
        }
        ReWidgetSize
        SetWidgetSize $width [winfo reqheight $win.$name]
    }

    public method FixTerminals {} {
        foreach child [winfo children $win.$name.f] {
            destroy $child
        }
        ForgetInlets
        ForgetOutlets
        SetInlets [oswInlets $transformPath]
        SetOutlets [oswOutlets $transformPath]
        ReWidgetSize
        Redraw
        #slimy hack to make wires appear properly when transform
        # terminals change
        foreach inlet $inlets {
            ::inlet-$inlet Move 0 0
        }
        foreach outlet $outlets {
            ::outlet-$outlet Move 0 0
        }
    }

    public method AppendTerminals {} {
        foreach child [winfo children $win.$name.f] {
            destroy $child
        }
        ForgetInletWidgets
        ForgetOutletWidgets
        SetInlets [oswInlets $transformPath]
        SetOutlets [oswOutlets $transformPath]
        ReWidgetSize
        Redraw
        #slimy hack to make wires appear properly when transform
        # terminals change
        foreach inlet $inlets {
            ::inlet-$inlet Move 0 0
        }
        foreach outlet $outlets {
            ::outlet-$outlet Move 0 0
        }
    }

    public method WidgetColor {newcolor} {
        changeColor $win.$name.f  $newcolor
        $win itemconfigure $name -fill $newcolor
        set color $newcolor
    }

    public method WidgetFontFace {face} {
        changeFontFace $win.$name.f  $face
        changeFontFace $win.$name.e  $face
        set fontface $face
    }

    public method WidgetFontSize {size} {
        changeFontSize $win.$name.f  $size
        changeFontSize $win.$name.e  $size
        set fontsize $size
    }

    private method RedrawInt {} {
        set xwidth $width
        set xheight $height
        $win itemconfigure frame-$name -width $xwidth -height $xheight
        $win coords $name $x $y [expr $x + 10 + $xwidth] [expr $y + 10 + $xheight]
        $win coords scale-$name [expr $x + 5 + $xwidth] [expr $y + 5 + $xheight] \
            [expr $x + 10 + $xwidth] [expr $y + 10 + $xheight]
        if {[llength $inlets] > 1} {
            set dinletX [expr int(($xwidth - 2) / ([llength $inlets] - 1))]
        } else {
            set dinletX 1
        }
        set inletX [expr $x + 1]
        foreach inlet $inlets {
            $win create rectangle $inletX [expr $y - 7] \
                [expr $inletX + 10] [expr $y + 3] \
                -tags [list inlet box_$inlet]
            $win create polygon [expr $inletX + 3] [expr $y - 5] \
                [expr $inletX + 10 - 3] [expr $y - 5] \
                [expr $inletX + 5] [expr $y] \
                -tags [list $inlet inlet inlet_terminal ${inlet}_terminal]
            ::inlet-$inlet configure -x $inletX
            ::inlet-$inlet configure -y [expr $y - 5]
            $win bind $inlet <Enter> \
                [list $this AnnounceTerminal $inlet true %x %y %W]
            $win bind $inlet <Leave> \
                [list $this LeaveTerminal $inlet true]
            $win bind box_$inlet <Enter> \
                [list $this AnnounceTerminal $inlet true %x %y %W]
            $win bind box_$inlet <Leave> \
                [list $this LeaveTerminal $inlet true]
            if {[oswIsActivator $inlet] && [llength [oswEffects $inlet]] > 0} {
                $win itemconfigure ${inlet}_terminal -fill green -outline green
            } else {
                $win itemconfigure ${inlet}_terminal -fill "#AAAAAA" -outline "#AAAAAA"
            }
            eval $win itemconfigure box_$inlet \
                [GetTypeStyle [oswType $inlet]] \
                -width 1
            set inletX [expr $dinletX + $inletX]
        }

        if {[llength $outlets] > 1} {
            set doutletX [expr int(($xwidth - 2) / ([llength $outlets] - 1))]
        } else {
            set doutletX 1
        }
        set outletX [expr $x + 1]
        foreach outlet $outlets {
            $win create rectangle $outletX [expr $y + $xheight + 10 - 3] \
                [expr $outletX + 10] [expr $y + $xheight + 10 + 7] \
                -tags [list outlet box_$outlet]
            $win create polygon [expr $outletX + 3] [expr $y + $xheight + 10] \
                [expr $outletX + 10 - 3] [expr $y + $xheight + 10] \
                [expr $outletX + 5] [expr $y + 5 + $xheight + 10] \
                -fill "#AAAAAA" -outline "#AAAAAA" \
                -tags [list $outlet outlet outlet_terminal ${outlet}_terminal]
            ::outlet-$outlet configure -x $outletX
            ::outlet-$outlet configure -y [expr $y + $xheight + 10]
            $win bind $outlet <Enter> \
                [list $this AnnounceTerminal $outlet false %x %y %W]
            $win bind $outlet <Leave> \
                [list $this LeaveTerminal $outlet false] 
            $win bind box_$outlet <Enter> \
                [list $this AnnounceTerminal $outlet false %x %y %W]
            $win bind box_$outlet <Leave> \
                [list $this LeaveTerminal $outlet false] 
            $win bind $outlet <Button-1> \
                [list $this StartConnection $outlet]

            eval $win itemconfigure box_$outlet \
                [GetTypeStyle [oswType $outlet]] \
                -width 1
            set outletX [expr $doutletX + $outletX]
        }
    }

    private method TransformNameFix {tofix} {
        regsub -all -- "&" $tofix "amp" tofix
        regsub -all -- ">" $tofix "gt" tofix
        regsub -all -- "<" $tofix "lt" tofix
        regsub -all -- "\\+" $tofix "add" tofix
        regsub -all -- "\\-" $tofix "sub" tofix
        regsub -all -- "\\*" $tofix "mul" tofix
        regsub -all -- "/" $tofix "div" tofix
        regsub -all -- "%" $tofix "mod" tofix
        regsub -all -- "=" $tofix "eq" tofix
        regsub -all -- "!" $tofix "not" tofix
        regsub -all -- "\'" $tofix "q" tofix
	regsub -all -- "\\|" $tofix "or" tofix
	regsub -all -- "\\^" $tofix "x" tofix
        return $tofix
    }

    public method CreateTransform {args} {

        if {$_cbox != ""} {
            catch {delete object $_cbox}
            set _cbox ""
        }

        if {$::initCmd($this) == $prevInitCmd} {
            return
        }
        
        regsub -all -- \\$ $::initCmd($this) "\\$" transformCmd

        #annoying hack to make list arguments work in an eval statement later on
        for {set i 0} {$i < [llength $transformCmd]} {incr i} {
            set elem [lindex $transformCmd $i]
            if {[llength $elem] > 1} {
                set transformCmd [lreplace $transformCmd $i $i [list $elem]]
            }
        }

        if {$transformName != ""} {
            $patch remove $transformPath
            $patchWindow UnregisterTransform $this $transformPath
            ForgetInlets
            ForgetOutlets
            SetInlets {}
            SetOutlets {}
            set transformName ""
            set transformPath ""
            set transformClass ""
            set packageName ""
            set isPatch false
            set autonamed false
        }

        set autonamed false

        set transformClass [lindex $transformCmd 0]
        
        set nameOption [lsearch $transformCmd "-name"]
        if {$nameOption >= 0} {
            incr nameOption
            set transformName [lindex $transformCmd $nameOption]
        } else {
            if {[llength $args] > 0} {
                set transformName [lindex $args 0]
            } else { 
                set transformName [$patchWindow GetUniqueName \
                                       [TransformNameFix [string tolower [oswGetNameWithoutPackage $transformClass]]]]
                
            }
        }

        #all transforms are now "autonamed" (except for Inlets and Outlets)
        set autonamed true 
        set transformCmd [linsert $transformCmd 1 $transformName]

        if {$transformClass == "Patch"} {
            oswAddSubpatch $patch $transformName
            set isPatch true
            set isSubpatch true
        } else {
            set transformStyle [oswFindTransform $transformClass]
            if {[catch {set transformCmd [lreplace $transformCmd 0 0 [oswGetPackage $transformClass]]}]} {
                set transformName ""
                bgerror "Could not find transform class $transformClass"
                set transformClass ""
                set transformPath ""
                return		
            }
            
            #special hack for TransformArrays
            if {$transformClass == "Array"} {
                set arrayClass [lindex $transformCmd 3]
                oswFindTransform $arrayClass
                set transformCmd [lreplace $transformCmd 3 3 [oswGetPackage $arrayClass]]
            }
            if {$transformStyle != "osx" } {
                set transformName [lindex $transformCmd 1]
                oswInstancePatch $patch $transformClass $transformName \
                    $transformCmd $transformStyle
                set isPatch true		
            } else {
                set toeval [concat $patch add $transformCmd]
                set transformName [eval $toeval]
            }
        }

        set packageName [oswGetPackage $transformClass]
        set transformClass [oswGetNameWithoutPackage $transformClass]
        set transformPath $patch/$transformName
        SetInlets [oswInlets $transformPath]
        SetOutlets [oswOutlets $transformPath]
        Redraw
        $win bind $name <Double-Button-1> [list $this OpenDialog]

        $patchWindow RegisterTransform $this $transformPath

        bind $win.$name <Enter> [list oswShowBalloonHelp [oswDescription $transformPath] %x %y %W]
        bind $win.$name <Leave> oswHideBalloonHelp
        UnbindFocusOut
        SetupMenu
    }

    public method Help {} {
        if {[llength $::initCmd($this)] > 0} {
            if {$packageName != ""} {
                regsub "::" $packageName "/" helppath
                ::ShowHelpTopic $helppath
            } else {
                ::ShowHelpTopic  internal/$transformClass
            }
        }
    }

    public method HelpPatch {} {
        if {[llength $::initCmd($this)] > 0} {
            if {$packageName != ""} {
                regsub "::" $packageName "/" helppath
                ::ShowHelpPatch $helppath
            } else {
                ::ShowHelpPatch  internal/$transformClass
            }
        }
    }

    public method AnnounceTerminal {terminal isInlet x y W} {

        [$patchWindow cget -statusbar] configure -text \
            [concat [file tail $terminal] [oswType $terminal] ":" \
                 [oswDescription $terminal]]
        if {[oswIsActivator $terminal]} {
            foreach effect [oswEffects $terminal] {
                $win itemconfigure $effect -fill green -outline green
                $win itemconfigure box_${effect} -outline green -width 3
            }
            foreach activator [oswCoactivators $terminal] {
                $win itemconfigure box_${activator} -outline green -width 3
            }

        }
        oswShowBalloonHelp [oswDescription $terminal] $x $y $W
    }

    public method LeaveTerminal {terminal isInlet} {
        .status configure -text ""
        .status2 configure -text ""
        if {[oswIsActivator $terminal]} {
            foreach effect [oswEffects $terminal] {
                $win itemconfigure $effect -fill "#AAAAAA" -outline "#AAAAAA"
                $win itemconfigure box_${effect} -outline black -width 1
            }
            foreach activator [oswCoactivators $terminal] {
                $win itemconfigure box_${activator} -outline black -width 1
            }
        }
        oswHideBalloonHelp
    }

    public method StartConnection {terminal} {

        if {$locked} {
            return
        }

        global oswConnectionSource oswConnectionSink

        if {[::outlet-$terminal cget -connection] != {}} {
            delete object [::outlet-$terminal cget -connection]
        }

        set oswConnectionSource $terminal
        set oswConnectionSink {}
        set wire [$patchWindow GetUniqueName]
        set wireX [outlet-$terminal ConnectX]
        set wireY [outlet-$terminal ConnectY]
        eval $win create line $wireX $wireY $wireX $wireY -width 2 -tag $wire \
            [GetTypeStyle [oswType $terminal]]
        #$win lower $terminal all
        
        #for constrained motion
        global ForceX ForceY OldX OldY
        set ForceX 0
        set ForceY 0
        set OldX $wireX
        set OldY $wireY
        
        bind $win <Motion> [list $patchWindow DragConnection %W $wire %x %y false]
        bind $win <Shift-Motion> [list $patchWindow DragConnection %W $wire %x %y true]
        #if we don't do this flag variable thing, Tk will run the whole
        # "EstablishConnection" routine since it already received a
        # Button-1 event
        global _whackyEventFlag
        set _whackyEventFlag 1
        bind $win <Button-1> [list $patchWindow EstablishConnection %W $wire $this %x %y false]
        bind $win <Shift-Button-1> [list $patchWindow EstablishConnection %W $wire $this %x %y true]
        $win bind $wire <Escape> [list $patchWindow CancelConnection %W $wire]
        bind $win <Button-3> [list $patchWindow CancelConnection %W $wire]

    }

    public method GetUniqueName {} {
        return [$patchWindow GetUniqueName]
    }


    public method EnumOutletConnectionsXML {selected} {
        set result {}
        foreach outlet $outlets {
            set connection [outlet-$outlet cget -connection]
            if {$connection != {}} {
                if {$selected != {}} {
                    set connected [[$connection cget -inlet] cget -transform]
                    if {[lsearch $selected [string trimleft $connected :]] == -1} {
                        break
                    }
                }
                set connectionItem "<CONNECTION "
                append connectionItem " outlet=\"[xmlfix [$connection cget -outlet]]\""
                append connectionItem " inlet=\"[xmlfix [$connection cget -inlet]]\""
                append connectionItem " vertices=\"[$connection cget -internalPts]\""
                append connectionItem " />"
                lappend result $connectionItem
            }
        }
        return $result
    }

    public method EnumUnconnectedOutlets {} {
        set result {}
        foreach outlet $outlets {
            if {[outlet-$outlet cget -connection] == {}} {
                lappend result $outlet
            }
        }
        return $result
    }

    public method EnumUnconnectedInlets {} {
        set result {}
        foreach inlet $inlets {
            if {[inlet-$inlet cget -connection] == {}} {
                lappend result $inlet
            }
        }
        return $result
    }


    public method EnumConnectedToInlets {} {
        # for use with depency checking
        set result {}
        foreach inlet $inlets {
            set connection [inlet-$inlet cget -connection]
            if {$connection != {}} {
                lappend result [[$connection cget -outlet] cget -transform]
            }
        }
        return $result
    }


    public method EnumPersistentStates {} {
        #for cut & copy
        set result {}
        foreach state [oswPersistentState $transformPath] {
            set state $transformPath/$state
            lappend result [list oswSet $state [oswGet $state]]
        }
        return $result
    }
    
    public method EnumPersistentStatesXML {} {
        #for saving
        set result {}
        foreach state [oswPersistentState $transformPath] {
            set stateAsParam "-$state"
            if {[lsearch $::initCmd($this) $stateAsParam] >= 0} {
                #puts "Skipping $state, used in parameter"
                continue
            }
            set state $transformPath/$state
            set stateVal [oswGet $state]
            set stateFix [xmlfix $stateVal]
            if {$stateVal == $stateFix} {
                lappend result "<STATE name=\"[xmlfix $state]\" value=\"$stateVal\" />"
            } else {
                lappend result "<STATE name=\"[xmlfix $state]\">$stateFix</STATE>"
            }
        }
        return $result
    }

    private method SetupMenu {} {
        if {[winfo exists $win.$name.menu]} {
            destroy $win.$name.menu
        }
        menu $win.$name.menu
        if {$isPatch} {
            $win.$name.menu add command -label "Open Patch" -command [list oswShowPatch [oswGet $transformPath/window]]
        } else {
            $win.$name.menu add command -label "Open..." -command [list $this OpenDialog]
        }
        $win.$name.menu add command -label "Help" -command "$this Help ; $this HelpPatch"
        $win.$name.menu add command -label "Properties..." -command [list $this StateDialog]
        foreach elem [oswTransformMenu $transformPath] {
            $win.$name.menu add command -label [lindex $elem 0] -command [lindex $elem 1]
        }
        $win bind $name <Button-3> [list tk_popup $win.$name.menu %X %Y]
    }

    public method OpenDialog {} {
	    
        if {$isPatch} {
            oswShowPatch [oswGet $transformPath/window]
        } else {
            set dialogCmd [oswDialog $transformPath]
            if {$dialogCmd == ""} {
                StateDialog
            } else {
                eval $dialogCmd
            }
        }
    }
    
    private variable xdirtyStates {}

    public method StateDialog {} {

        if {[catch [.statedialog-$transformPath activate]]} {
            iwidgets::dialogshell .statedialog-$transformPath -title $transformPath
            .statedialog-$transformPath add ok -text "Ok" -command [list $this ApplyStateDialog true]
            .statedialog-$transformPath add cancel -text "Cancel" -command [list $this CloseStateDialog]
            .statedialog-$transformPath add apply -text "Apply" -command [list $this ApplyStateDialog false]
            .statedialog-$transformPath default cancel
            bind .statedialog-$transformPath <Return> {}
            
            set dialog [.statedialog-$transformPath childsite]
            label $dialog.l -text [list $transformName [oswDescription $transformPath]] \
                -font 10
            grid $dialog.l -row 0 -column 1 -columnspan 4
            set publicState [oswPersistentState $transformPath]
            set row 1
            foreach stateVar $publicState {
                label $dialog.name$row -text $stateVar
                entry $dialog.value$row -textvariable textVar_$dialog$row
                $dialog.value$row delete 0 end
                $dialog.value$row insert 0 [oswGet $transformPath/$stateVar]
                
                label $dialog.type$row -text [oswType $transformPath/$stateVar]
                label $dialog.desc$row -text [oswDescription $transformPath/$stateVar]
                grid $dialog.name$row -row $row -column 1
                grid $dialog.value$row -row $row -column 2
                grid $dialog.type$row -row $row -column 3 -padx 10
                grid $dialog.desc$row -row $row -column 4
                lappend xdirtyStates false
                incr row
            }
            
            .statedialog-$transformPath activate
        }
    }

    public method CloseStateDialog {} {
        set row 1
        set dialog [.statedialog-$transformPath childsite]
        destroy .statedialog-$transformPath
    }

    public method ApplyStateDialog {shouldClose} {
        
        set dialog [.statedialog-$transformPath childsite]

        set row 1
        foreach stateVar [oswPersistentState $transformPath] {
            set newval [$dialog.value$row get]
            if {$newval != [oswGet $transformPath/$stateVar]} {
                oswSet $transformPath/$stateVar $newval
            }
            incr row
        }

        if {$shouldClose} {
            CloseStateDialog
        }
    }

    private method ForgetInlets {} {
        foreach inlet $inlets {
            $win delete $inlet
            $win delete box_$inlet
            delete object inlet-$inlet
        }
    }

    private method ForgetOutlets {} {
        foreach outlet $outlets {
            $win delete $outlet
            $win delete box_$outlet
            delete object outlet-$outlet
        }
    }

    private method ForgetInletWidgets {} {
        foreach inlet $inlets {
            $win delete box_$inlet
            $win delete $inlet
        }
    }
    
    private method ForgetOutletWidgets {} {
        foreach outlet $outlets {
            $win delete box_$outlet
            $win delete $outlet
        }
    }

    private method DrawTransform {} {
        set _width [string length $::initCmd($this)]
        if {$_width < 4} {
            set _width 4
        }
        $win.$name.e configure -width $_width 
        set drawingSpace $win.$name.f
        set nameSpace $win.$name.e
        set transform $transformPath
        set initializer $::initCmd($this)
        eval [oswDrawTransform $transformPath $drawingSpace]
        #$win.$name configure -width 0 -height 0
        #$win.$name.f configure -width 0 -height 0
    }

    private method HideName {} {
        pack forget $win.$name.e
    }

    private method UpdateCanvas {} {
        set bbox [$win bbox all]
        $win configure -scrollregion [list 0 0 [lindex $bbox 2] [lindex $bbox 3]]
    }	

    private variable _cbox ""

    public method CommandCompletion {} {
        #first, figure out if we're working on the Transform class
        #or the instance name
        if {$_cbox != ""} {
            delete object $_cbox
        }
        if {[regexp "\ +$" $::initCmd($this)]} {
        } else {
            set completions [oswGetCompletions $::initCmd($this)]
            if {[llength $completions] > 0} {
                if {[llength $completions] > 1} {
                    set ::initCmd($this) [commonString $completions]
                    if {$::oswShowCompletions} {
                        set _cbox [completionbox cb#auto -completions $completions -associated $win.$name -object $this]
                               }
                    } else {
                        set ::initCmd($this) $completions
                        append ::initCmd($this) " "
                    }
                }
            }
            $win.$name.e icursor end
        }

        public method SetCommand {newcmd} {
            set ::initCmd($this) $newcmd
            append ::initCmd($this) " "
            $win.$name.e icursor end
        }
        
        public method Dock {} {
            ::AddDockTransform $transformName $::initCmd($this)
        }

        public method BindFocusOut {} {
            bind $win.$name.e <FocusOut> [code $this _onFocusOut %d]
        }

        public method UnbindFocusOut {} {
            bind $win.$name.e <FocusOut> ""
        }

        private method _onFocusOut {detail} {
            if {$detail == "NotifyAncestor" && [focus] != ""} {
                CreateTransform
            }
        }

        public method Select {} {
            $win itemconfigure $name -width 2
        }

        public method UnSelect {} {
            $win itemconfigure $name -width 1
        }
    }
    

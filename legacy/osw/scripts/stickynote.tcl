
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

class stickynote {

    public variable win
    public variable x 5
    public variable y 5
    public variable width 50
    public variable height 75
    public variable color "#FFFF77"
    public variable fontface $::defaultFont
    public variable fontsize 8
    public variable style "text"
    public variable patchWindow

    private variable five 5
    private variable ten 10
    private variable name
    private variable locked false

    constructor {args} {
	eval configure $args
	
	set name [namespace tail $this]

	frame $win.$name
	switch $style text {
	    text $win.$name.sticky -relief flat -bg $color
	} graphics {
	    canvas $win.$name.sticky -relief flat -bg $color
	}
	pack $win.$name.sticky -fill both -expand yes
	
	$win create window [expr $x + $five] [expr $y + $five] \
		-anchor nw -tag sticky-$name -window $win.$name
	$win create rectangle $x $y \
		[expr $x + $ten + $width] \
		[expr $y + $ten + $height] \
		-fill $color -tag $name
	$win create rectangle \
		[expr $x + $five + $width] \
		[expr $y + $five + $height] \
		[expr $x + $ten + $width] \
		[expr $y + $ten + $height] \
		-fill white -tag scale-$name
	$win addtag sticky withtag $name
	WidgetSize $width $height

        bind $win.$name.sticky <FocusIn> "$patchWindow ForgetSelection"
	$win bind scale-$name <Enter> [list $win configure -cursor sizing]
	$win bind scale-$name <Leave> [list $win configure -cursor $::defaultCursor]
	$win bind scale-$name <Button-1> [list $this StartResize %x %y]
	
	$win bind $name <Button-1> "StartObjectMotion $patchWindow $win $name %x %y false"
	$win bind $name <Shift-Button-1> "StartObjectMotion $patchWindow $win $name %x %y true"
    }

    destructor {

	$win delete $name
	$win delete sticky-$name
	$win delete scale-$name
	destroy $win.$name.sticky
	destroy $win.$name
    }

    public method Lock {} {
	set locked true
	$win bind $name <Button-1> {}
	$win bind $name <Shift-Button-1> {}
	$win.$name.sticky configure -state disabled
	$win lower scale-$name $name
    }

    public method Unlock {} {
	set locked false
	$win bind $name <Button-1> "StartObjectMotion $patchWindow $win $name %x %y false"
	$win bind $name <Shift-Button-1> "StartObjectMotion $patchWindow $win $name %x %y true"
	$win.$name.sticky configure -state normal
	$win raise scale-$name $name
    }


    public method WidgetSize {awidth aheight} {
	$win coords sticky-$name [expr $x + $five] [expr $y + 5] 
	$win itemconfigure sticky-$name -width $awidth -height $aheight 
	$win coords $name $x $y [expr $x + $ten + $awidth] [expr $y + $ten + $aheight]
	$win coords scale-$name [expr $x + $five + $awidth] [expr $y + $five + $aheight] \
		[expr $x + $ten + $awidth] [expr $y + $ten + $aheight]
	set width $awidth
	set height $aheight
	UpdateCanvas
    }

    public method WidgetFontFace {face} {
	if {$face != ""} {
	    set fontface $face
	    $win.$name.sticky configure -font [list $fontface $fontsize]
	}
    }

    public method WidgetFontSize {size} {
	if {$size != ""} {
	    set fontsize $size
	    $win.$name.sticky configure -font [list $fontface $fontsize]
	}
    }

    private method UpdateCanvas {} {
	set bbox [$win bbox all]
	$win configure -scrollregion [list 0 0 [lindex $bbox 2] [lindex $bbox 3]]
    }	

    public method StartResize {nx ny} {
	if {$locked} {
	    return
	}
	global OldX OldY
	set OldX [$win canvasx $nx]
	set OldY [$win canvasy $ny]
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
	set dx [expr $nx - $OldX]
	set dy [expr $ny - $OldY]

	set xwidth $width
	set xheight $height
	set xwidth [expr $xwidth + $dx]
	set xheight [expr $xheight + $dy]

	WidgetSize $xwidth $xheight

	set OldX $nx
	set OldY $ny
    }

    public method DoneResize {} {
	$win bind scale-$name <B1-Motion> {}
	$win bind scale-$name <ButtonRelease-1> {}
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
	$win move sticky-$name $dx $dy
	$win move scale-$name $dx $dy
	set x [expr $x + $dx]
	set y [expr $y + $dy]
	UpdateCanvas
	set OldX $nx
	set OldY $ny
	MoveOthers $this $dx $dy
    }

    public method  MoveObjectRel {dx dy} {
	    
	$win move $name $dx $dy
	$win move sticky-$name $dx $dy
	$win move scale-$name $dx $dy
	set x [expr $x + $dx]
	set y [expr $y + $dy]
	UpdateCanvas
    }


    public method WidgetColor {newcolor} {
	$win itemconfigure $name -fill $newcolor
	$win.$name.sticky configure -bg $newcolor
	set color $newcolor
    }

    public method GetText {} {
	return [$win.$name.sticky get 0.0 end]
    }

    public method SetText {newtext} {
	$win.$name.sticky configure -state normal
	$win.$name.sticky delete 0.0 end	
	$win.$name.sticky insert end $newtext
	if {$locked} {
	    $win.$name.sticky configure -state disabled
	}
    }

    public method Select {} {
	$win itemconfigure $name -width 2
    }

    public method UnSelect {} {
	$win itemconfigure $name -width 1
    }
}


class Title {

    public variable win
    public variable x 5
    public variable y 5
    public variable color "#000000"
    public variable fontface $defaultFont
    public variable fontsize 10
    public variable patchWindow

    private variable name
    private variable locked false

    constructor {args} {
	eval configure $args
	
	set name [namespace tail $this]
	$win create text $x $y -tag ${name}_text
	$win create rectangle [expr $x - 2] [expr $y + 5] [expr $x + 3] [expr $y + 10] -fill white -tag $name
	$win bind ${name}_text <KeyPress> [list $this textInsert %A]
	$win bind ${name}_text <1> [list $this textB1Press %x %y]
	$win bind ${name}_text <B1-Motion> [list $this textB1Move %x %y]
	$win bind ${name}_text <Control-h> [list $this textBS]
	$win bind ${name}_text <BackSpace> [list $this textBS]
	$win bind $name <Button-1> "$this textB1Press %x %y ; StartObjectMotion $patchWindow $win $name %x %y false"
	$win bind $name <Shift-Button-1> "StartObjectMotion $patchWindow $win $name %x %y true"
	$win addtag title withtag $name
	$win icursor ${name}_text 0
	StartEdit
    }

    destructor {
	$win delete ${name}_text
	$win delete ${name}_box
	$win delete $name
    }

    public method StartEdit {} {
	$win focus ${name}_text
	focus $win
    }

    public method textB1Press {x y} {
	$win icursor ${name}_text @$x,$y
	StartEdit	
	$win focus ${name}_text
	$win select from ${name}_text @$x,$y
	$patchWindow ObjectSelection $name false
    }

    public method textB1Move {x y} {
	$win select to ${name}_text @$x,$y
	$patchWindow ObjectSelection $name false
    }

    public method textInsert {string} {
	if {$string == ""} {
	    return
	}
	catch {$win dchars ${name}_text sel.first sel.last}
	$win insert ${name}_text insert $string
	eval $win coords ${name}_box [$win bbox ${name}_text]
    }

    public method textBS {} {
	if ![catch {$win dchars ${name}_text sel.first sel.last}] {
	    return
	}
	set char [expr {[$win index ${name}_text insert] - 1}]
	if {$char >= 0} {$win dchar ${name}_text $char}
    }

    public method WidgetFontFace {face} {
	if {$face != ""} {
	    set fontface $face
	    $win itemconfigure ${name}_text -font [list $fontface $fontsize]
	}
    }
    
    public method WidgetFontSize {size} {
	if {$size != ""} {
	    set fontsize $size
	    $win itemconfigure ${name}_text -font [list $fontface $fontsize]
	}
    }
    
    public method WidgetColor {newcolor} {
	if {$color != ""} {
	    set color $newcolor
	    $win itemconfigure ${name}_text -fill $color
	}
    }

    public method SetText {newtext} {
	catch {$win dchars 0 end}
	$win insert ${name}_text insert $newtext
	catch {eval $win coords ${name}_box [$win bbox ${name}_text]}
    }
    
    public method GetText {} {
	return [$win itemcget ${name}_text -text]
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
	$win move ${name}_text $dx $dy
	$win move ${name}_box $dx $dy
	$win move $name $dx $dy
	set x [expr $x + $dx]
	set y [expr $y + $dy]
	UpdateCanvas
	set OldX $nx
	set OldY $ny
	MoveOthers $this $dx $dy
    }

    public method  MoveObjectRel {dx dy} {
	    
	$win move ${name}_text $dx $dy
	$win move ${name}_box $dx $dy
	$win mov $name $dx $dy
	set x [expr $x + $dx]
	set y [expr $y + $dy]
	UpdateCanvas
    }

    private method UpdateCanvas {} {
	set bbox [$win bbox all]
	$win configure -scrollregion [list 0 0 [lindex $bbox 2] [lindex $bbox 3]]
    }	

    public method Lock {} {
	set locked true
	$win delete $name 
	$win delete ${name}_box
	$win bind ${name}_text <KeyPress> {}
	$win bind ${name}_text <1> {}
	$win bind ${name}_text <B1-Motion> {}
	$win bind ${name}_text <Control-h> {}
	$win bind ${name}_text <BackSpace> {}
    }

    public method Unlock {} {
	set locked false
	$win bind ${name}_text <KeyPress> [list $this textInsert %A]
	$win bind ${name}_text <1> [list $this textB1Press %x %y]
	$win bind ${name}_text <B1-Motion> [list $this textB1Move %x %y]
	$win bind ${name}_text <Control-h> [list $this textBS]
	$win bind ${name}_text <BackSpace> [list $this textBS]
	$win create rectangle [expr $x - 2] [expr $y + 5] [expr $x + 3] [expr $y + 10] -fill white -tag $name
	$win bind $name <Button-1> "StartObjectMotion $patchWindow $win $name %x %y false"
	$win bind $name <Shift-Button-1> "StartObjectMotion $patchWindow $win $name %x %y true"
    }

    public method Select {} {
	if {$locked} {
	    return
	}
	StartEdit
	$win delete ${name}_box
	eval $win create rectangle [$win bbox ${name}_text] -tag ${name}_box
    }

    public method UnSelect {} {
	$win delete ${name}_box
    }
}

class Hyperlink {

    public variable win
    public variable x 5
    public variable y 5
    public variable color "blue"
    public variable fontface $defaultFont
    public variable fontsize 10
    public variable patchWindow

    private variable name
    private variable locked false

    private variable prevCursor ""

    constructor {args} {
	eval configure $args
	
	set name [namespace tail $this]
	$win create text $x $y -tag ${name}_text -fill $color
	$win create line $x [expr $y + 8] [expr $x + 1] [expr $y + 8] -tag ${name}_line -fill $color
	$win create rectangle [expr $x - 2] [expr $y + 5] [expr $x + 3] [expr $y + 10] -fill white -tag $name
	$win bind ${name}_text <KeyPress> [list $this textInsert %A]
	$win bind ${name}_text <3> [list $this textB1Press %x %y] 
	$win bind ${name}_text <B3-Motion> [list $this textB1Move %x %y]
	$win bind ${name}_text <1> "oswLinkTo \[ $this GetText \]"
	#$win bind ${name}_text <Enter> "$win configure -cursor hand2"
	#$win bind ${name}_text <Leave> "$win configure -cursor $::defaultCursor"
	#$win bind ${name}_text <ButtonRelease-3> [$win configure -cursor $prevCursor]
	$win bind ${name}_text <Control-h> [list $this textBS]
	$win bind ${name}_text <BackSpace> [list $this textBS]
	$win addtag link withtag $name
	$win bind $name <Button-1> "$this textB1Press %x %y ;  StartObjectMotion $patchWindow $win $name %x %y false"
	$win bind $name <Shift-Button-1> "StartObjectMotion $patchWindow $win $name %x %y true"
	$win icursor ${name}_text 0
	StartEdit
    }

    destructor {
	$win delete ${name}_text
	$win delete ${name}_line
	$win delete ${name}_box
	$win delete $name
    }

    public method StartEdit {} {
	$win focus ${name}_text
	focus $win
    }

    public method textB1Press {x y} {
	if {$locked} {
	    return
	}
	$win icursor ${name}_text @$x,$y
	StartEdit	
	$win focus ${name}_text
	$win select from ${name}_text @$x,$y
	$patchWindow ObjectSelection $name false
    }

    public method textB1Move {x y} {
	$win select to ${name}_text @$x,$y
	$patchWindow ObjectSelection $name false
    }

    public method textInsert {string} {
	if {$string == ""} {
	    return
	}
	catch {$win dchars ${name}_text sel.first sel.last}
	$win insert ${name}_text insert $string
	set box [$win bbox ${name}_text]
	$win coords ${name}_line [lindex $box 0] [lindex $box 3] [lindex $box 2] [lindex $box 3]
	$win coords ${name}_box [lindex $box 0] [lindex $box 3] [lindex $box 2] [lindex $box 3]
    }

    public method textBS {} {
	if ![catch {$win dchars ${name}_text sel.first sel.last}] {
	    return
	}
	set char [expr {[$win index ${name}_text insert] - 1}]
	if {$char >= 0} {$win dchar ${name}_text $char}
    }

    public method WidgetFontFace {face} {
	if {$face != ""} {
	    set fontface $face
	    $win itemconfigure ${name}_text -font [list $fontface $fontsize]
	    set box [$win bbox ${name}_text]
	    $win coords ${name}_line [lindex $box 0] [lindex $box 3] [lindex $box 2] [lindex $box 3]

	}
    }
    
    public method WidgetFontSize {size} {
	if {$size != ""} {
	    set fontsize $size
	    $win itemconfigure ${name}_text -font [list $fontface $fontsize]
	    set box [$win bbox ${name}_text]
	    $win coords ${name}_line [lindex $box 0] [lindex $box 3] [lindex $box 2] [lindex $box 3]	    
	}
    }
    
    public method WidgetColor {newcolor} {
	if {$color != ""} {
	    set color $newcolor
	    $win itemconfigure ${name}_text -fill $color
	}
    }

    public method SetText {newtext} {
	catch {$win dchars 0 end}
	$win insert ${name}_text insert $newtext
	catch {eval $win coords ${name}_box [$win bbox ${name}_text]}
    }
    
    public method GetText {} {
	return [$win itemcget ${name}_text -text]
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
	$win move ${name}_text $dx $dy
	$win move ${name}_line $dx $dy
	$win move ${name}_box $dx $dy
	$win move $name $dx $dy
	set x [expr $x + $dx]
	set y [expr $y + $dy]
	UpdateCanvas
	set OldX $nx
	set OldY $ny
	MoveOthers $this $dx $dy
    }

    public method  MoveObjectRel {dx dy} {
	    
	$win move ${name}_text $dx $dy
	$win move ${name}_line $dx $dy
	$win move ${name}_box $dx $dy
	$win move $name $dx $dy
	set x [expr $x + $dx]
	set y [expr $y + $dy]
	UpdateCanvas
    }

    private method UpdateCanvas {} {
	set bbox [$win bbox all]
	$win configure -scrollregion [list 0 0 [lindex $bbox 2] [lindex $bbox 3]]
    }	

    public method Lock {} {
	set locked true
	$win delete $name 
	$win bind ${name}_text <KeyPress> {}
	#$win bind ${name}_text <1> {}
	$win bind ${name}_text <B1-Motion> {}
	$win bind ${name}_text <Control-h> {}
	$win bind ${name}_text <BackSpace> {}
	$win delete ${name}_box 
    }

    public method Unlock {} {
	set locked false
	$win bind ${name}_text <KeyPress> [list $this textInsert %A]
	#$win bind ${name}_text <1> [list $this textB1Press %x %y]
	$win bind ${name}_text <B1-Motion> [list $this textB1Move %x %y]
	$win bind ${name}_text <Control-h> [list $this textBS]
	$win bind ${name}_text <BackSpace> [list $this textBS]
	$win create rectangle [expr $x - 2] [expr $y + 5] [expr $x + 3] [expr $y + 10] -fill white -tag $name
	$win bind $name <Button-1> "StartObjectMotion $patchWindow $win $name %x %y false"
	$win bind $name <Shift-Button-1> "StartObjectMotion $patchWindow $win $name %x %y true"
    }

    public method Select {} {
	if {$locked} {
	    return
	}
	StartEdit
	$win delete ${name}_box
	eval $win create rectangle [$win bbox ${name}_text] -tag ${name}_box \
		-outline $color
    }

    public method UnSelect {} {
    }
}


class Image {

    public variable win
    public variable x 5
    public variable y 5
    public variable patchWindow
    public variable filename ""

    private variable name
    private variable locked false
    private variable myimage false

    constructor {args} {
	eval configure $args
	
	set name [namespace tail $this]
	LoadImage false
	$win bind $name <Button-1> "StartObjectMotion $patchWindow $win $name %x %y false"
	$win bind $name <Shift-Button-1> "StartObjectMotion $patchWindow $win $name %x %y true"
	$win addtag img withtag $name
    }

    destructor {
	$win delete ${name}_text
	$win delete $name
    }

    private method LoadImage {reopen} {
	if {$filename == "" || $reopen} {
	    set filename [tk_getOpenFile -defaultextension "gif" -filetypes {{"GIF Image" {.gif}} {"PPM Image" {.ppm}} {"PGM Image" {.pgm}}}]
	}
	if {$filename != ""} {
	    if {![catch {set newimage [image create photo -file $filename]}]} {
		if {$myimage} {
		    image delete $myimage
		}
		set myimage $newimage
                $win create image $x $y -tag $name -image $myimage
	    }
	}
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
	set x [expr $x + $dx]
	set y [expr $y + $dy]
	UpdateCanvas
	set OldX $nx
	set OldY $ny
	MoveOthers $this $dx $dy
    }

    public method  MoveObjectRel {dx dy} {
	    
	$win mov $name $dx $dy
	set x [expr $x + $dx]
	set y [expr $y + $dy]
	UpdateCanvas
    }

    private method UpdateCanvas {} {
	set bbox [$win bbox all]
	$win configure -scrollregion [list 0 0 [lindex $bbox 2] [lindex $bbox 3]]
    }	

    public method Lock {} {
	set locked true
    }

    public method Unlock {} {
	set locked false
    }

    public method Select {} {
    }

    public method UnSelect {} {
    }
}
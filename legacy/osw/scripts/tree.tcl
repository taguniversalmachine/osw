#!/usr/bin/wish

# This is Amar Chaudhary's [incr Tcl] refactoring of D. Richard Hipp's 
# tree widget for tcl.  The original notices are posted below.
#
#
# I am D. Richard Hipp, the author of this code.  I hereby
# disavow all claims to copyright on this program and release
# it into the public domain. 
#
#                     D. Richard Hipp
#                     January 31, 2001
#
# As an historical record, the original copyright notice is
# reproduced below:
#
# Copyright (C) 1997,1998 D. Richard Hipp
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Library General Public
# License as published by the Free Software Foundation; either
# version 2 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Library General Public License for more details.
# 
# You should have received a copy of the GNU Library General Public
# License along with this library; if not, write to the
# Free Software Foundation, Inc., 59 Temple Place - Suite 330,
# Boston, MA  02111-1307, USA.
#
# Author contact information:
#   drh@acm.org
#   http://www.hwaci.com/drh/
#
# $Revision: 1.3 $
#

#option add *highlightThickness 0


package require Iwidgets

#
# Bitmaps used to show which parts of the tree can be opened.
#
set maskdata "#define solid_width 9\n#define solid_height 9"
append maskdata {
  static unsigned char solid_bits[] = {
   0xff, 0x01, 0xff, 0x01, 0xff, 0x01, 0xff, 0x01, 0xff, 0x01, 0xff, 0x01,
   0xff, 0x01, 0xff, 0x01, 0xff, 0x01
  };
}
set data "#define open_width 9\n#define open_height 9"
append data {
  static unsigned char open_bits[] = {
   0xff, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x7d, 0x01, 0x01, 0x01,
   0x01, 0x01, 0x01, 0x01, 0xff, 0x01
  };
}
image create bitmap Tree:openbm -data $data -maskdata $maskdata \
  -foreground black -background white
set data "#define closed_width 9\n#define closed_height 9"
append data {
  static unsigned char closed_bits[] = {
   0xff, 0x01, 0x01, 0x01, 0x11, 0x01, 0x11, 0x01, 0x7d, 0x01, 0x11, 0x01,
   0x11, 0x01, 0x01, 0x01, 0xff, 0x01
  };
}
image create bitmap Tree:closedbm -data $data -maskdata $maskdata \
  -foreground black -background white


class Tree {
    inherit itk::Widget    

    private variable Tree
    private variable w
    private variable selection
    private variable selidx
    
    public variable sort 0

    constructor {args} {
	itk_component add canvas {
	    canvas $itk_interior.c
	} {
	    usual Canvas
	    keep -width -height -background -yscrollcommand -xscrollcommand \
		    -borderwidth -selectborderwidth
	}
	pack $itk_interior.c -fill both -expand yes
	set w $itk_interior.c
	eval itk_initialize $args
	dfltconfig /
	buildwhenidle
	set Tree(selection) {}
	set Tree(selidx) {}
	switch $::tcl_platform(platform) {
	    unix {
		set Tree(font) \
			-adobe-helvetica-medium-r-normal-*-11-80-100-100-p-56-iso8859-1
	    }
	    windows {
		set Tree(font) \
			-adobe-helvetica-medium-r-normal-*-14-100-100-100-p-76-iso8859-1
	    }
	}
    }

    private method dfltconfig {v} {
	set Tree($v:children) {}
	set Tree($v:open) 0
	set Tree($v:icon) {}
	set Tree($v:tags) {}
    }

    public method newitem {v args} {
	set dir [file dirname $v]
	set n [file tail $v]
	if {![info exists Tree($dir:open)]} {
	    error "parent item \"$dir\" is missing"
	}
	set i [lsearch -exact $Tree($dir:children) $n]
	if {$i>=0} {
	    error "item \"$v\" already exists"
	}
	lappend Tree($dir:children) $n
	# sort only if the user specified the -sort option
	if {$sort} {
	    set Tree($dir:children) [lsort $Tree($dir:children)]
	}
	dfltconfig $v
	foreach {op arg} $args {
	    switch -exact -- $op {
		-image {set Tree($v:icon) $arg}
		-tags {set Tree($v:tags) $arg}
	    }
	}
	buildwhenidle
    }

    #
    # Delete element $v from the tree $w.  If $v is /, then the widget is
    # deleted.
    #

    public method delitem {w v} {
	global Tree
	if {![info exists Tree($v:open)]} return
	if {[string compare $v /]==0} {
	    destroy $this
	}
	foreach c $Tree($v:children) {
	    catch {delitem $v/$c}
	}
	unset Tree($v:open)
	unset Tree($v:children)
	unset Tree($v:icon)
	set dir [file dirname $v]
	set n [file tail $v]
	set i [lsearch -exact $Tree($dir:children) $n]
	if {$i>=0} {
	    set Tree($dir:children) [lreplace $Tree($dir:children) $i $i]
	}
	buildwhenidle
    }
 
    #
    # Change the selection to the indicated item
    #
    public method setselection {v} {
	set Tree(selection) $v
	drawselection
    }
    
    # 
    # Retrieve the current selection
    #
    public method getselection {} {
	return $Tree(selection)
    }


    # Internal use only.
    # Draw the tree on the canvas
    private method build {} {
	$w delete all
	catch {unset Tree(buildpending)}
	set Tree(y) 30
	buildlayer / 10
	$w config -scrollregion [$w bbox all]
	drawselection
    }
    
    # Internal use only.
    # Build a single layer of the tree on the canvas.  Indent by $in pixels
    private method buildlayer {v in} {
	if {$v=="/"} {
	    set vx {}
	} else {
	    set vx $v
	}
	set y $Tree(y)
	set start [expr $Tree(y)-10]
	foreach c $Tree($v:children) {
	    set y $Tree(y)
	    incr Tree(y) 17
	    $w create line $in $y [expr $in+10] $y -fill gray50 
	    set icon $Tree($vx/$c:icon)
	    set taglist x
	    foreach tag $Tree($vx/$c:tags) {
		lappend taglist $tag
	    }
	    set x [expr $in+12]
	    if {[string length $icon]>0} {
		set k [$w create image $x $y -image $icon -anchor w -tags $taglist]
		incr x 20
		set Tree(tag:$k) $vx/$c
	    }
	    set j [$w create text $x $y -text $c -font $Tree(font) \
		    -anchor w -tags $taglist]
	    set Tree(tag:$j) $vx/$c
	    set Tree($vx/$c:tag) $j
	    if {[string length $Tree($vx/$c:children)]} {
		if {$Tree($vx/$c:open)} {
		    set j [$w create image $in $y -image Tree:openbm]
		    $w bind $j <1> [code $this onclose $vx $c]
		    buildlayer $vx/$c [expr $in+18]
		} else {
		    set j [$w create image $in $y -image Tree:closedbm]
		    $w bind $j <1> [code $this onopen $vx $c]
		}
	    }
	}
	set j [$w create line $in $start $in [expr $y+1] -fill gray50 ]
	$w lower $j
    }

    # called when a branch is opened or closed
    private method onopen {vx c} {
	set Tree($vx/$c:open) 1
	build
    }

    private method onclose {vx c} {
	set Tree($vx/$c:open) 0
	build
    }


    # Open a branch of a tree
    #
    public method open {v} {
	if {[info exists Tree($v:open)] && $Tree($v:open)==0
	&& [info exists Tree($v:children)] 
	&& [string length $Tree($v:children)]>0} {
	    set Tree($v:open) 1
	    build
	}
    }
    
    public method close {v} {
	global Tree
	if {[info exists Tree($v:open)] && $Tree($v:open)==1} {
	    set Tree($v:open) 0
	    build
	}
    }
    
    # Internal use only.
    # Draw the selection highlight
    private method drawselection {} {
	if {[string length $Tree(selidx)]} {
	    $w delete $Tree(selidx)
	}
	set v $Tree(selection)
	if {[string length $v]==0} return
	if {![info exists Tree($v:tag)]} return
	set bbox [$w bbox $Tree($v:tag)]
	if {[llength $bbox]==4} {
	    set i [eval $w create rectangle $bbox -fill skyblue -outline {{}}]
	    set Tree(selidx) $i
	    $w lower $i
	} else {
	    set Tree(selidx) {}
	}
    }
    
    # Internal use only
    # Call build then next time we're idle
    private method buildwhenidle {} {
	if {![info exists Tree(buildpending)]} {
	    set Tree(buildpending) 1
	    after idle [code $this build]
	}
    }
    
    #
    # Return the full pathname of the label for widget $w that is located
    # at real coordinates $x, $y
    #
    public method labelat {x y} {
	set x [$w canvasx $x]
	set y [$w canvasy $y]
	global Tree
	foreach m [$w find overlapping $x $y $x $y] {
	    if {[info exists Tree(tag:$m)]} {
		return $Tree(tag:$m)
	    }
	}
	return ""
    }

    #forward the bind command to the internal canvas widget (AC 12/2001)

    public method bind {args} {
	eval $w bind $args
    }
    
}










#################
#
# The remainder is code that demonstrates the use of the Tree
# widget.  Uncomment the code if you want to test changes to the widget.
#
#. config -bd 3 -relief flat
#frame .f -bg white
#pack .f -fill both -expand 1
#image create photo idir -data {
#    R0lGODdhEAAQAPIAAAAAAHh4eLi4uPj4APj4+P///wAAAAAAACwAAAAAEAAQAAADPVi63P4w
#    LkKCtTTnUsXwQqBtAfh910UU4ugGAEucpgnLNY3Gop7folwNOBOeiEYQ0acDpp6pGAFArVqt
#    hQQAO///
#}
#image create photo ifile -data {
#    R0lGODdhEAAQAPIAAAAAAHh4eLi4uPj4+P///wAAAAAAAAAAACwAAAAAEAAQAAADPkixzPOD
#    yADrWE8qC8WN0+BZAmBq1GMOqwigXFXCrGk/cxjjr27fLtout6n9eMIYMTXsFZsogXRKJf6u
#    P0kCADv/
#}
#frame .f.mb -bd 2 -relief raised
#pack .f.mb -side top -fill x
#menubutton .f.mb.file -text File -menu .f.mb.file.menu
#catch {
#  menu .f.mb.file.menu
#  .f.mb.file.menu add command -label Quit -command exit
#}
#menubutton .f.mb.edit -text Edit
#menubutton .f.mb.view -text View
#menubutton .f.mb.help -text Help
#pack .f.mb.file .f.mb.edit .f.mb.view .f.mb.help -side left -padx 10
#Tree .f.w -width 150 -height 400 -yscrollcommand {.f.sb set}
#scrollbar .f.sb -orient vertical 
#pack .f.w -side left -fill both -expand 1 -padx 5 -pady 5
#pack .f.sb -side left -fill y
#frame .f.c -height 400 -width 400 -bg white
#pack .f.c -side left -fill both -expand 1
#label .f.c.l -width 40 -text {} -bg [.f.c cget -bg]
#pack .f.c.l -expand 1
#foreach z {1 2 3} {
#  .f.w newitem /dir$z
#  foreach x {1 2 3 4 5 6} {
#    .f.w newitem /dir$z/file$x
#  }
#  .f.w newitem /dir$z/subdir
#  foreach y {1 2} {
#    .f.w newitem /dir$z/subdir/file$y
#  }
#  foreach zz {1 2 3 4} {
#    .f.w newitem /dir$z/subdir/ssdir$zz 
#    .f.w newitem /dir$z/subdir/ssdir$zz/file1 
#    .f.w newitem /dir$z/subdir/ssdir$zz/file2 
#  }
#}
#.f.w bind x <1> {
#  set lbl [.f.w labelat %x %y]
#  .f.w setselection $lbl
#  .f.c.l config -text $lbl
#}
#.f.w bind x <Double-1> {
#  .f.w open [.f.w labelat  %x %y]
#}
#update

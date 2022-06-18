
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

class Connection {

    public variable inlet
    public variable outlet
    public variable wire
    public variable win
    public variable internalPts
    public variable frozen 0

    constructor {args} {
	eval configure $args
	$inlet configure -connection $this
	$outlet configure -connection $this
	if {[$inlet cget -transform] == [$outlet cget -transform]} {
	    set frozen 1
	}
    }

    destructor {
	$outlet configure -connection {}
        oswConnect [$outlet cget -outlet] none
	$inlet configure -connection {}
	$win delete $wire
    }

    public method MoveOutlet {x y} {

	set coords [$win coords $wire]
	set inletx [lindex $coords [expr [llength $coords] - 2]]
	set inlety [lindex $coords [expr [llength $coords] - 1]]
	if {$frozen} {
	    set outletx [lindex $coords 0]
	    set outlety [lindex $coords 1]
	    set dx [expr $x - $outletx]
	    set dy [expr $y - $outlety]
	    set i 0
	    set newPts {}
	    foreach pt $internalPts {
		if {$i} {
		    lappend newPts [expr $pt + $dy]
		} else {
		    lappend newPts [expr $pt + $dx]
		}
		set i [expr !$i]
	    }
	    set internalPts $newPts
	} elseif {$inlety < $y} {
	    set internalPts [ManhattanizeWire2 $x $y $inletx $inlety [list $x [expr $y + 10] $inletx [expr $inlety - 10]]]
	} else {
	    set internalPts [ManhattanizeWire $x $y $inletx $inlety {}]
	}
	eval $win coords $wire $x $y $internalPts $inletx $inlety
    }

    public method MoveInlet {x y} {

	set coords [$win coords $wire]
	set outletx [lindex $coords 0]
	set outlety [lindex $coords 1]
	if {$frozen} {
	} elseif {$y < $outlety} {
	    set internalPts [ManhattanizeWire2 $outletx $outlety $x $y [list $outletx [expr $outlety + 10] $x [expr $y - 10]]]
	} else {
	    set internalPts [ManhattanizeWire $outletx $outlety $x $y {}]
	}
	eval $win coords $wire $outletx $outlety $internalPts $x $y
    }

    public method Move {dx dy} {
	set coords [$win coords $wire] 
	set newcoords {} 
	for {set i 0} {$i < [llength $coords]} {incr i} {
	    lappend newcoords [expr [lindex $coords $i] + $dx]
	    incr i
	    lappend newcoords [expr [lindex $coords $i] + $dy]
	}
	eval $win coords $wire $newcoords
    }

    public method BoundingBox {} {
	return [$win bbox $wire]
    }

    public method Select {} {
	$win itemconfigure $wire -stipple gray75 \
		-width [expr [$win itemcget $wire -width] + 2]
    }

    public method UnSelect {} {
	set mywidth [$win itemcget $wire -width]
	if {$mywidth > 2} {
	    $win itemconfigure $wire -stipple "" \
		    -width [expr $mywidth - 2]
	} else {
	    $win itemconfigure $wire -stipple "" -width 2
	}
    }
}

class Inlet {
    public variable inlet
    public variable transform
    public variable connection {}
    public variable x
    public variable y

    constructor {args} {
	eval configure $args
    }

    destructor {
	if {[llength $connection] != 0} {
	    delete object $connection
	}
    }

    public method Move {dx dy} {
	set x [expr $x + $dx]
	set y [expr $y + $dy]
	if {$connection != {}} {
	    $connection MoveInlet [$this ConnectX] [$this ConnectY]
	}
    }

    public method MoveAbsolute {nx ny} {
	set x $nx
	set y $ny
	if {$connection != {}} {
	    $connection MoveInlet [$this ConnectX] [$this ConnectY]
	}
    }

    public method ConnectX {} {
	return [expr $x + 5]
    }
    public method ConnectY {} {
	return [expr $y]
    }
}

class Outlet {
    public variable outlet
    public variable transform
    public variable connection {}
    public variable x
    public variable y

    constructor {args} {
	eval configure $args
    }

    destructor {
	if {[llength $connection] != 0} {
	    delete object $connection	
	}
    }

    public method Move {dx dy} {
	set x [expr $x + $dx]
	set y [expr $y + $dy]
	if {$connection != {}} {
	    $connection MoveOutlet [$this ConnectX] [$this ConnectY]
	}
    }

    public method MoveAbsolute {nx ny} {
	set x $nx
	set y $ny
	if {$connection != {}} {
	    $connection MoveOutlet [$this ConnectX] [$this ConnectY]
	}
    }

    public method ConnectX {} {
	return [expr $x + 5]
    }
    public method ConnectY {} {
	return [expr $y + 5]
    }
}

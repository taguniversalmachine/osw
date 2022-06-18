
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

package require Iwidgets

itk::usual knob {
    keep -background -cursor -labelfont -foreground
}

class knob {

    inherit itk::Widget


    itk_option define -knobcolor knobColor KnobColor black
    itk_option define -notchcolor notchColor NotchColor white
    itk_option define -from from From 0.0
    itk_option define -to to To 1.0
    itk_option define -value value Value 0.0
    itk_option define -velocity value Value 0.01
    itk_option define -command command Command {}

    constructor {args} {}
    destructor {}

    #
    # Public methods
    #
    public {
	method knob {args}
    }

    #
    # Private methods
    #
    private {

	method _displayKnob {}
	method _setangle {}
	method _startmotion {x y}
	method _stopmotion {}
	method _move {x y}

	variable angle
	variable _oldX
	variable _oldY
	common PI [expr 2*asin(1.0)]
	common TWOPI [expr 4*asin(1.0)]
    }
}



# -----------------------------------------------------------------------------
#                        CONSTRUCTOR
# -----------------------------------------------------------------------------
body knob::constructor { args } {
    #
    # Add back to the hull width and height options and make the
    # borderwidth zero since we don't need it.
    #
    set _interior $itk_interior

    itk_option add hull.width hull.height
    component hull configure -borderwidth 0
    grid propagate $itk_component(hull) no


    itk_component add canvas {
	canvas $itk_interior.canvas
    } {
	usual
	keep -width -height -background
    }

    bind $itk_component(canvas) <Map> +[code $this _displayKnob]
    bind $itk_component(canvas) <Configure> +[code $this _displayKnob]

    eval itk_initialize $args

    _setangle
    knob create oval 0 0 2 2 -fill $itk_option(-knobcolor) -tags knob
    knob create line 0 0 2 2 -fill $itk_option(-notchcolor) -tags notch
    pack $itk_component(canvas) -fill both -expand yes

    knob bind knob <Button-1> [code $this _startmotion %x %y]
    knob bind kinb <ButtonRelease-1> [code $this _stopmotion]
    #grid $itk_component(canvas) -row 1 -column 0 -sticky nsew

    #grid rowconfigure    $itk_interior 0 -weight 1
    #grid columnconfigure $itk_interior 0 -weight 1

}


# -----------------------------------------------------------------------------
#                            METHODS
# -----------------------------------------------------------------------------


body knob::knob {args} {
    return [eval $itk_component(canvas) $args]
}


body knob::_displayKnob {} {


    #
    # Compute the center coordinates for the Knob based on the
    # with and height of the canvas.
    #
    set width [winfo width $itk_component(canvas)]
    set height [winfo height $itk_component(canvas)]
    set x0 [expr $width/2]
    set y0 [expr $height/2]

    #
    # Set the radius of the knob, pivot, hour, minute and second items.
    #
    
    if {$x0 < $y0} {
	set radius [expr $x0 - 5]
    } else {
	set radius [expr $y0 - 5]
    }
    knob coords knob [expr $x0 - $radius] [expr $y0 - $radius] [expr $x0 + $radius] [expr $y0 + $radius]
    #knob coords knob [expr $x0 - $radius] [expr $y0 - $radius] \
    #	    [expr $x0 + $radius] [expr $y0 + $radius]

    set notchwidth [expr int($width * 0.02) + 1]
    knob coords notch $x0 $y0 [expr $x0 - $radius * sin($angle)] \
	    [expr $y0 + $radius * cos($angle)]
    knob itemconfigure notch -width $notchwidth

}

body knob::_setangle {} {
    set angle [expr $PI * (0.166666667 + 1.6666666 * \
	    ($itk_option(-value) - $itk_option(-from)) / \
	    ($itk_option(-to) - $itk_option(-from)))]
}

body knob::_startmotion {x y} {
    set _oldX $x
    set _oldY $y

    knob bind knob <B1-Motion> [code $this _move %x %y]
}

body knob::_stopmotion {} {
    knob bind knob <B1-Motion> {}
}

body knob::_move {x y} {
    set width [winfo width $itk_component(canvas)]
    set height [winfo height $itk_component(canvas)]
    set x0 [expr $width/2]
    set y0 [expr $height/2]
    
    set oldangle $angle
    set angle [expr atan2($y0 - $y,$x0 - $x) + $PI * 0.5]
    if {$angle < 0} {
	set angle [expr $TWOPI + $angle]
    } 
    set oldvalue $itk_option(-value)
    set itk_option(-value) [expr 0.6 * ($angle * 0.318309886184 - 0.16666666666667) \
	    * ($itk_option(-to) - $itk_option(-from)) + $itk_option(-from)]
    #puts "$itk_option(-to) $itk_option(-from)"

    if {$itk_option(-value) < $itk_option(-from) || $itk_option(-value) > $itk_option(-to)} {
	set itk_option(-value) $oldvalue
	set $angle $oldangle
	return
    }
    if {[llength $itk_option(-command)] > 0} {
	eval $itk_option(-command) $itk_option(-value)
    }
    _displayKnob
    set _oldX $x
    set _oldY $y
}

# -----------------------------------------------------------------------------
#                             OPTIONS
# -----------------------------------------------------------------------------

# ------------------------------------------------------------------
# OPTION: state
#
# Configure the editable state of the widget.  Valid values are
# normal and disabled.  In a disabled state, the hands of the 
# knob are not selectabled.
# ------------------------------------------------------------------
configbody knob::knobcolor {

    knob itemconfigure knob -fill $itk_option(-knobcolor)
}

configbody knob::value {
    _setangle
    _displayKnob
}

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
#namespace import ::itcl::*


class keyboard {
    inherit itk::Widget

    constructor {args} {
	
	itk_component add keyboard {

	    canvas $itk_interior.k  -width 360 -height 80
	} {
	    usual
	}

	$itk_interior.k create rectangle 0 0 24 80 -tag d0  -fill white
	$itk_interior.k create rectangle 24 0 48 80 -tag d2 -fill white
	$itk_interior.k create rectangle 48 0 72 80 -tag d4 -fill white
	$itk_interior.k create rectangle 72 0 96 80 -tag d5 -fill white
	$itk_interior.k create rectangle 96 0 120 80 -tag d7 -fill white
	$itk_interior.k create rectangle 120 0 144 80 -tag d9 -fill white
	$itk_interior.k create rectangle 144 0 168 80 -tag d11 -fill white
	$itk_interior.k create rectangle 168 0 192 80 -tag d12 -fill white
	$itk_interior.k create rectangle 192 0 216 80 -tag d14 -fill white
	$itk_interior.k create rectangle 216 0 240 80 -tag d16 -fill white
	$itk_interior.k create rectangle 240 0 264 80 -tag d17 -fill white
	$itk_interior.k create rectangle 264 0 288 80 -tag d19 -fill white
	$itk_interior.k create rectangle 288 0 312 80 -tag d21 -fill white
	$itk_interior.k create rectangle 312 0 336 80 -tag d23 -fill white
	$itk_interior.k create rectangle 336 0 360 80 -tag d24 -fill white
	
	$itk_interior.k create rectangle 16 0 32 50 -tag d1 -fill black
	$itk_interior.k create rectangle 40 0 56 50 -tag d3 -fill black
	$itk_interior.k create rectangle 88 0 104 50 -tag d6 -fill black
	$itk_interior.k create rectangle 112 0 128 50 -tag d8 -fill black
	$itk_interior.k create rectangle 136 0 152 50 -tag d10 -fill black
	$itk_interior.k create rectangle 184 0 200 50 -tag d13 -fill black
	$itk_interior.k create rectangle 208 0 224 50 -tag d15 -fill black
	$itk_interior.k create rectangle 256 0 272 50 -tag d18 -fill black
	$itk_interior.k create rectangle 280 0 296 50 -tag d20 -fill black
	$itk_interior.k create rectangle 304 0 320 50 -tag d22 -fill black

	for {set i 0} {$i < 25} {incr i} {
	    $itk_interior.k bind d$i <Button-1> [list [code $this _keypressed] $i]
	}

	eval itk_initialize $args

	pack $itk_interior.k
    }

    itk_option define -octave octave Octave 5
    itk_option define -degree degree Degree 0
    itk_option define -notein   notein   Notein   -1
    itk_option define -command command Command {}

    private variable arrangement {white black white black white white black white black white black white white black white black white white black white black white black white white }

    private method _keypressed {keynum} {
	if {[llength $itk_option(-command)] > 0} {
	    eval $itk_option(-command) [expr $itk_option(-octave) * 12 + $keynum]
	} 
	configure -degree $keynum
    }
    private variable olddegree 0
}

configbody keyboard::notein {
    set noteOctave [expr $itk_option(-notein) / 12]
    if {$noteOctave == $itk_option(-octave)} {
	configure -degree [expr $itk_option(-notein) % 12]
    } elseif {$noteOctave - 1 == $itk_option(-octave)} {
	configure -degree [expr $itk_option(-notein) % 24]
    } else {
	$itk_interior.k itemconfigure d[set itk_option(-degree)] \
		-fill [lindex $arrangement $itk_option(-degree)]
    }
}

configbody keyboard::degree {
    $itk_interior.k itemconfigure d$olddegree -fill [lindex $arrangement $olddegree]
    $itk_interior.k itemconfigure d[set itk_option(-degree)] -fill gray
    set olddegree $itk_option(-degree)
}

keyboard .keyb
pack .keyb

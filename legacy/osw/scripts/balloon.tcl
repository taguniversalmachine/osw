
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


#package require Iwidgets

class balloon {

    public variable delay 1000
    public variable background "#FFFF66"

    private variable _hintWindow 
    private variable _balloonAfterID 0
    private variable top
    private variable left
    private variable text

    constructor {args} {

	eval configure $args

	set _hintWindow [toplevel .balloonHintWindow]
	wm withdraw $_hintWindow
	label $_hintWindow.l -bg $background -relief raised \
		-borderwidth 1
	pack $_hintWindow.l
    }

    destructor {
	if {$_balloonAfterID != 0} {after cancel $_balloonAfterID}
	destroy $_hintWindow
    }

    public method Show {x y atext window} {
	DoHide
	set top $y
	set left $x
	set text $atext
	if {$_balloonAfterID != 0} {
	    after cancel $_balloonAfterID
	}
	set _balloonAfterID [after $delay [code $this DoShow $window]]
    }

    private method DoShow {window} {
	set top [expr int($top)]
	set left [expr int($left)]
	set xtop [expr $top + [winfo rooty $window]]
	set xleft [expr $left + [winfo rootx $window]]
	$_hintWindow.l configure -text $text 
	wm overrideredirect $_hintWindow 0
	wm geometry $_hintWindow "+$xleft+$xtop"
	wm overrideredirect $_hintWindow 1
	wm deiconify $_hintWindow
	raise $_hintWindow	
    }

    public method Hide {} {
	if { $_balloonAfterID != 0 } {
	    after cancel $_balloonAfterID
	    set _balloonAfterID 0
	    DoHide
	} else {
	    DoHide
	}
    }

    private method DoHide {} {
	wm withdraw $_hintWindow
    }
}



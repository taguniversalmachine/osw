
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

namespace eval midiIn {
    set statusList {all noteoff noteon polyaftertouch control program \
	    aftertouch pitchbend}
}


proc midiInSelect {transformPath} {

    if {[catch [.mididialog-$transformPath activate]]} {
	iwidgets::dialogshell .mididialog-$transformPath -title $transformPath
	.mididialog-$transformPath add ok -text "Ok" \
		-command "SaveMidiInDialog $transformPath"
	.mididialog-$transformPath add cancel -text "Cancel" \
		-command "destroy .mididialog-$transformPath"
	

	set childSite [.mididialog-$transformPath childsite]

	iwidgets::combobox $childSite.driver -labeltext "Driver"
	iwidgets::combobox $childSite.channel -labeltext "Channel"
	iwidgets::combobox $childSite.status -labeltext "Status"
	iwidgets::combobox $childSite.parameter -labeltext "Parameter"
	pack $childSite.driver $childSite.channel $childSite.status \
		$childSite.parameter
	
	set currentDriver [oswGet $transformPath/driver]
	set currentChannel [oswGet $transformPath/channel]
	set currentStatus [oswGet $transformPath/status]
	set currentParameter [oswGet $transformPath/parameter]
	
	set numDrivers [oswGet /midiIn/count]
	set selection -1
	for {set i 0} {$i < $numDrivers} {incr i} {
	    $childSite.driver insert list end [list /midiIn/${i}: [oswGet /midiIn/$i/full_name]]
	    if {$currentDriver == "/midiIn/$i"} {
		set selection $i
	    }
	}
	if {$selection >= 0} {
	    $childSite.driver selection set $selection
	}

	eval $childSite.status insert list end $::midiIn::statusList
	$childSite.status selection set \
		[lsearch -exact $::midiIn::statusList $currentStatus]

	$childSite.channel insert list end all
	for {set i 1} {$i <= 16} {incr i} {
	    $childSite.channel insert list end $i
	}
	$childSite.channel selection set \
		[expr $currentChannel + 1]

	$childSite.parameter insert list end all
	for {set i 0} {$i <= 127} {incr i} {
	    $childSite.parameter insert list end $i
	}
	$childSite.parameter selection set \
		[expr $currentParameter + 1]

    }
    .mididialog-$transformPath activate
}

proc SaveMidiInDialog {transformPath} {

    set childSite [.mididialog-$transformPath childsite]
    oswSet $transformPath/driver /midiIn/[$childSite.driver curselection]
    oswSet $transformPath/channel [expr [$childSite.channel curselection] - 1]
    oswSet $transformPath/status [$childSite.status getcurselection]
    oswSet $transformPath/parameter \
	    [expr [$childSite.parameter curselection] - 1]

    destroy .mididialog-$transformPath
}



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


set oswAudioDeviceList {}

set _selectedAudioDevice ""

proc oswAudioPrefs {} {
    global oswAudioDeviceList _selectedAudioDevice

    set _selectedAudioDevice ""

    if {[llength $oswAudioDeviceList] == 0} {
	for {set i 0} {$i < 10} {incr i} {
	    if {![catch {oswGet /audio/$i/sample_rate}]} {
		lappend oswAudioDeviceList /audio/$i
	    }
	}
    }

    set ds [iwidgets::dialogshell .ds#auto -title "Audio Device Preferences"]
    set dsite [$ds childsite]
    $ds add OK -text "OK" -command "_checkSelectedAudioDevice $dsite ; destroy $ds"
    $ds add Apply -text "Apply" -command "_checkSelectedAudioDevice $dsite"
    $ds add Cancel -text "Cancel" -command "destroy $ds"

    iwidgets::combobox $dsite.cb -labeltext "Device: " -labelpos w \
	    -selectioncommand "_selectAudioDevice $dsite"
    eval $dsite.cb insert list end $oswAudioDeviceList
    iwidgets::entryfield $dsite.srate -labeltext "Sample Rate: " -labelpos w
    label $dsite.ochannels -text "Output channels: "
    label $dsite.ichannels -text "Input channels: "
    iwidgets::entryfield $dsite.buffer -labeltext "Vector Size: " -labelpos w
    iwidgets::entryfield $dsite.latency -labeltext "Target Latency: " -labelpos w
    pack $dsite.cb $dsite.srate $dsite.ochannels $dsite.ichannels \
	    $dsite.buffer $dsite.latency -side top -fill both

    $ds activate
}
    
proc _checkSelectedAudioDevice {dsite} {
    global _selectedAudioDevice

    set srate [$dsite.srate get]
    set buffer [$dsite.buffer get]
    set latency [$dsite.latency get]

    if {[oswGet $_selectedAudioDevice/sample_rate] != $srate} {
	oswSet $_selectedAudioDevice/sample_rate $srate
    }
    if {[oswGet $_selectedAudioDevice/buffer_size] != $buffer} {
	oswSet $_selectedAudioDevice/buffer_size $buffer
    }
    if {[oswGet $_selectedAudioDevice/hi_water] != $latency} {
	oswSet $_selectedAudioDevice/hi_water $latency
    }
}

proc _selectAudioDevice {dsite} {
    global _selectedAudioDevice

    set _selectedAudioDevice [$dsite.cb getcurselection]

    $dsite.srate delete 0 end
    $dsite.srate insert end [oswGet $_selectedAudioDevice/sample_rate]
    $dsite.buffer delete 0 end
    $dsite.buffer insert end [oswGet $_selectedAudioDevice/buffer_size]
    $dsite.latency delete 0 end
    $dsite.latency insert end [oswGet $_selectedAudioDevice/hi_water]
    $dsite.ochannels configure -text "Output channels: [oswGet $_selectedAudioDevice/output_channels]"
    $dsite.ichannels configure -text "Input channels: [oswGet $_selectedAudioDevice/input_channels]"

}

#just for the demo :)

#catch {oswSet /audio/1/hi_water 2200}

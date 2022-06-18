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

set browserList {
    galeon
    opera
    mozilla
    w3m
    netscape
    konqueror
    lynx
}
set osw_browser [oswGetPreference Browser]
if {$osw_browser == ""} {
    puts "No browser preference set, autodetecting..."
    foreach b $browserList {
	puts "trying $b..."
	if {![catch [list exec which $b]]} {
	    puts "Setting browser to $b"
	    set osw_browser $b
	    oswSetPreference Browser $b
	    break
	}
    }
}

.patchmenu.options add separator
.patchmenu.options add command -label "Set Help Browser..." -command oswChooseHelpBrowser

proc oswChooseHelpBrowser {} {
    set ds [iwidgets::dialogshell .ds#auto -title "Select Help Browser"]
    set dsite [$ds childsite]
    $ds add OK -text "OK" -command "_selectHelpBrowser $dsite ; destroy $ds"
    $ds add Apply -text "Apply" -command "_selectHelpBrowser $dsite"
    $ds add Cancel -text "Cancel" -command "destroy $ds"
    iwidgets::entryfield $dsite.browser -labeltext "Browser: " -labelpos w
    $dsite.browser insert end $::osw_browser

    pack $dsite.browser
    $ds activate
}

proc _selectHelpBrowser {dsite} {

    set new_browser [$dsite.browser get]
    if {$new_browser != $::osw_browser} {
	set ::osw_browser $new_browser
	oswSetPreference Browser $::osw_browser
    }
}




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


proc DoneWelcome {} {
    global _dontshowwelcome
    grab release .welcomeosw
    destroy .welcomeosw


    if {$_dontshowwelcome} {
        oswSetPreference Welcome 1
    }
}

if {[oswGetPreference Welcome] != "1"} {
    set _dontshowwelcome 0

    toplevel .welcomeosw -background blue
    
    label .welcomeosw.logo -image [oswLoadBitmap photo littlepatch.ppm]
    label .welcomeosw.welcome -text "Welcome to Open Sound World!" -font {{helvetica} 14} -background blue -foreground white
    
    label .welcomeosw.userguide -text "View Users Guide" -font {{helvetica} 12} \
            -background blue -foreground white
    label .welcomeosw.help -text "View Help Patches" -font {{helvetica} 12} \
            -background blue -foreground white
    label .welcomeosw.tutorial -text "View Tutorial Patches" -font {{helvetica} 12} \
            -background blue -foreground white
    label .welcomeosw.demo -text "View Demo Patches" -font {{helvetica} 12} \
            -background blue -foreground white
    label .welcomeosw.cont -text "Continue" -font {{helvetica} 12} \
            -background blue -foreground white
    label .welcomeosw.info -text "These options are availabe from the\nHelp menu at any time" -background blue 
    bind .welcomeosw.cont <1> {DoneWelcome}
    bind .welcomeosw.userguide <1> {DoneWelcome ; ShowHelpTopic UserGuide}
    bind .welcomeosw.help <1> {DoneWelcome ; BrowseHelpPatches "" }
    bind .welcomeosw.demo <1> {DoneWelcome ; BrowseDemos ""}
    bind .welcomeosw.tutorial <1> {DoneWelcome ; BrowseTutorials ""}
    checkbutton .welcomeosw.dontshow -text "Do not show this message again" \
            -background blue  -variable _dontshowwelcome
    pack .welcomeosw.logo -side right
    pack .welcomeosw.welcome .welcomeosw.userguide .welcomeosw.help \
            .welcomeosw.tutorial .welcomeosw.demo .welcomeosw.cont \
            .welcomeosw.info .welcomeosw.dontshow \
            -side top
    raise .welcomeosw
    
    grab .welcomeosw
    tkwait window .welcomeosw

}





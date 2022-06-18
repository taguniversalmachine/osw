
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

proc GetTypeStyle {typename} {

    global TypeStyles
    set foundStyle [array get TypeStyles $typename]

    if {[llength $foundStyle] == 0} {
	return "-fill black"
    } else {
	return [lindex $foundStyle 1]
    }
}


proc AddTypeStyle {typename style} {

    global TypeStyles

    set TypeStyles($typename) $style
}


AddTypeStyle Samples "-fill red"
AddTypeStyle IntegerSamples "-fill magenta"
AddTypeStyle Integer16Samples "-fill darkgreen"
AddTypeStyle Time "-fill purple"
AddTypeStyle Sinusoids "-fill blue"
AddTypeStyle Spectrum "-fill darkcyan"
AddTypeStyle Bundle "-width 7 -fill black"
AddTypeStyle Score "-fill brown"
AddTypeStyle Blob "-fill darkblue"
AddTypeStyle Note "-fill darkblue"
AddTypeStyle MidiMessage "-fill orange"




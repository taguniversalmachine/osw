
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

namespace eval osw {

    proc doeval {evalstr} {
	if {[catch {namespace eval ::osw $evalstr}]} {
            if {[regexp "invalid command name" $::errorInfo] && \
                    [regexp "this" $::errorInfo]} {
                #do nothing
            } else {             
                puts $::errorInfo
                bgerror $::errorInfo
            }
	}
    }
    
    proc evalserver {chn host portnum} {
	fileevent $chn readable "osw::doeval \[ gets $chn \]"
    }
    
    if {[catch {set theEvalServer [socket -server "osw::evalserver" 7776]}]} {
	if {[llength $argv] > 0} {
	    set sendPatch [socket localhost 7775]
	    puts $sendPatch [lindex $argv 0]
	    flush $sendPatch
	    close $sendPatch
	    exit
	}
    }

    set thePatchServer [socket -server "osw::patchserver" 7775]

    proc dopatch {chn patch} {
	if {[eof $chn]} {
	    fileevent $chn readable {}
	    return
	}
	if {$patch != ""} {
	    #puts $patch
	    if {[regsub "GET +/" $patch "" patch]} {
		regsub " +HTTP.*$" $patch "" patch
		while {![eof $chn]} {
		    set line [gets $chn]
		    if {$line == ""} {
			break
		    }
		}
		puts $chn "HTTP/1.1 200 OK\n\n<HTML><HEAD></HEAD><BODY>"
		puts $chn "<SCRIPT>\nhistory.back()\n</SCRIPT>"
		puts $chn "<NOSCRIPT><H3>Please hit the back button on your browser, and switch to your OSW Window</H3></NOSCRIPT></BODY></HTML>"
		close $chn
	    }
	    oswSourcePatchTopLevel $patch
	}
    }

    proc patchserver {chn host portnum} {
	fileevent $chn readable "osw::dopatch $chn \[ gets $chn \]"
    }
}



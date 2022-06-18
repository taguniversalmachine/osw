
#Copyright (c) 2001-2002 Amar Chaudhary. All rights reserved.
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

# OSC Server sets up a "built-in" patch that reads OSC messages on port 7017

# OSW does not setup the builtin server on its own when invoked via TCL

#oswPatch {} osc_server
#/osc_server add OSCDispatch oscdispatch0
#/osc_server add OSCCommunicator osccommunicator0
#oswConnect /osc_server/osccommunicator0/messageOut /osc_server/oscdispatch0/messageIn
#oswConnect /osc_server/oscdispatch0/messageOut /osc_server/osccommunicator0/messageIn

#puts "OSC Server initialized"

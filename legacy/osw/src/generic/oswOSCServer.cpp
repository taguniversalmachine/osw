/* 
Copyright (c) 2002-2004 Andrew Schmeder, Amar Chaudhary.  All rights reserved.
Copyright (c) 2001-2002 Amar Chaudhary. All rights reserved.
Copyright (c) 1998-2001 Regents of the University of California.
All rights reserved.

     ``The contents of this file are subject to the Open Sound World Public
     License Version 1.0 (the "License"); you may not use this file except in
     compliance with the License. A copy of the License should be included
     in a file named "License" or "License.txt" in the distribution from 
     which you obtained this file. 

     Software distributed under the License is distributed on an "AS IS"
     basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
     License for the specific language governing rights and limitations
     under the License.

     The Original Code is Open Sound World (OSW) software.

     The Initial Developer of the Original Code is Amar Chaudhary.
     Portions created by Amar Chaudhary are Copyright (C) 1998-2001 
     Regents of the University of California and Amar Chaudhary. 
     All Rights Reserved.

     Contributor(s):

  
From the UC Regents:

Permission to use, copy, modify, and distribute this software and its
documentation, without fee and without a signed licensing agreement, is hereby
granted, provided that the above copyright notice, this paragraph and the
following two paragraphs appear in all copies, modifications, and
distributions.  Contact The Office of Technology Licensing, UC Berkeley, 2150
Shattuck Avenue, Suite 510, Berkeley, CA 94720-1620, (510) 643-7201, for
commercial licensing opportunities.

Written by Amar Chaudhary, The Center for New Music and Audio Technologies, 
University of California, Berkeley.

     IN NO EVENT SHALL REGENTS BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT,
     SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST
     PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS
     DOCUMENTATION, EVEN IF REGENTS HAS BEEN ADVISED OF THE POSSIBILITY OF
     SUCH DAMAGE.

     REGENTS SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT
     LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
     FOR A PARTICULAR PURPOSE. THE SOFTWARE AND ACCOMPANYING
     DOCUMENTATION, IF ANY, PROVIDED HEREUNDER IS PROVIDED "AS IS".
     REGENTS HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES,
     ENHANCEMENTS, OR MODIFICATIONS.
     
*/

/* oswOSCServer.cpp
   Implementation of OpenSound Control (OSC) server transforms for OSW
   Andrew Schmeder, Amar Chaudhary
*/

#include "oswTcl.h"
#include "oswOSCPacket.h"
#include "oswOSCTypes.h"
#include "oswPatternMatch.h"
#include "oswList.h"
#include "oswSock.h"
#include "oswDataSource.h"
#include "oswClock.h"
#ifdef _WINDOWS
/* needed for htonl */
#include <winsock.h>
#undef GetMessage
#else
#include <netinet/in.h>
#endif
#include "oswCast.h"
#include "oswPatternMatch.h"
#include "oswInterface.h"

namespace osw {

  //REGISTER_CAST(Raw,OSCPacket);

  /*********************/

  // OSC Server transforms:

  // OSCCommunicator

  // OSCListen

  // OSCRoute
  // OSCRoute should be almost just like Route, except it is capable of partial address matching

  // OSCDispatch

  /**
    * OSCCommunicator is used for bi-directional OSC communication.  It is capable of both sending and recieving messages
    * via UDP networking.  It is intended to be connected with OSCDispatch.
    *
    * The following additional features are on the wishlist for OSCCommunicator:
    *  - given a base port number, use broadcast messages to 'discover' other instances of OSCCommunicator,
    *    automatically move to a free port pair when collisions are detected.  This makes it possible to have mulitple OSW servers.
    *  - implement a basic stack of return addresses.  Assuming every message has a response, route the return messages 
    *    directly to the client which initiated the request.
    */
   
  // TODO: Handle #bundle messages correctly!
  
  class OSCCommunicator : public OSCParser, public Transform {

    public: 

      void MessageActivate(char *address, char *typeTags, char *data, char *boundary) {
        List alist;
        char *thisType;
        alist.append(new Datum<String>(address));
        for (thisType = typeTags + 1; *thisType != 0; ++thisType) {
          if(*thisType != 'N') {
            BaseDatum *datum = OSCToDatum(data,thisType,boundary);
            if (datum) {
              alist.append(datum);
            }
          }
        }
        messageOut = alist;
      }

      State<Integer> basePort;
      Inlet<List>    messageIn;
      Outlet<List>   messageOut;

      OSCCommunicator (const String &aname, Container *acontainer, int argc, char *argv[]) : 
       Transform(aname,acontainer,argc,argv),
       messageIn("messageIn", this, "OSC message in list format to send"),
       messageOut("messageOut", this, "OSC message recieved on listener port"),
       basePortChanged (&basePort,0,this,&OSCCommunicator::basePortChangedExpr),
       sendMessage(&messageIn,this,&OSCCommunicator::sendMessageExpr), 
       basePort ("basePort",this,"Listen on this port, send on this port + 1",ScanArguments<Integer>("-port",7707,acontainer,argc,argv)) {
        AddUDPServer(basePort,&communicator_handler_friend,this);
        old_port = (Integer) basePort;
        sock = OpenUDPSocket("255.255.255.255", basePort+1);
        xmaxbuf = ScanArguments<int>("-maxbuf",1024,acontainer,argc,argv);
        xbuffer = new char [xmaxbuf];
        OSC_initBuffer(&xoscbuf,xmaxbuf,xbuffer);
      }

      ~OSCCommunicator () {
        RemoveUDPServer(basePort,&communicator_handler_friend,this);
        CloseSocket(sock);
	delete [] xbuffer;
      }

      const char *Description () {
        return "Bi-directional OSC communication via UDP.";
      }

     string PersistentState () const {
       return "basePort";
     }
    protected:

      char    *xbuffer;
      OSCbuf  xoscbuf;
      int     xmaxbuf;
      void    *sock;

      int old_port;
      Activation<OSCCommunicator> basePortChanged, sendMessage;

      void basePortChangedExpr () {
        RemoveUDPServer(old_port,&communicator_handler_friend,this);
        CloseSocket(sock);
        AddUDPServer(basePort,&communicator_handler_friend,this);
        sock = OpenUDPSocket("255.255.255.255", basePort+1);
        old_port = (Integer) basePort;
      }

      void sendMessageExpr () {
        if(messageIn->size() == 0) {
          return;
        }
        // Convert messageIn to OSC then write to socket.
	String address = GetElementFromList<String>(messageIn,0);
        String typetag = ",";

        for (int i = 1; i < messageIn->size(); ++i) {
          typetag += GetTypeTagFromDatum(messageIn->GetUntypedElement(i));
        }
        OSC_resetBuffer(&xoscbuf);
        OSC_writeAddressAndTypes(&xoscbuf,
    	    const_cast<char *>(address.c_str()),
 	        const_cast<char *>(typetag.c_str()));
#ifdef OSW_LAME_FOR_LOOP
  	    // Ugh, I hate this stupid MSVC problem!! (AC)
        for (i = 1; i < messageIn->size(); ++i) {
#else
        for (int i = 1; i < messageIn->size(); ++i) {
#endif
          DatumToOSC(&xoscbuf, messageIn->GetUntypedElement(i));
        }
        
        //Alert("Sending OSC response");
        UDPSocketSend(sock, OSC_getPacket(&xoscbuf), OSC_packetSize(&xoscbuf));
      }

      friend int communicator_handler_friend (char *buf, int n, ClientAddr returnAddr, void *extra) {
        return ((OSCCommunicator *) extra)->handler(buf,n,returnAddr);
      }

      int handler (char *buf, int n, ClientAddr returnAddr) {
        // FIXME: This method cannot deal with multiple clients simultaneously.  (oh well)
        //Alert("Received OSC message");
        UDPSocketSetDest(sock, &returnAddr->cl_addr, returnAddr->clilen, basePort+1);
        ReadOSCMessage(buf, buf + n);
        return 0;
      }

  };

  static TransformFactory<OSCCommunicator> OSCCommunicatorFactory("OSCCommunicator",true);

  /**
   * OSCListen binds to a port, listens for UDP packets and parses them into OSC list-format messages.
   *
   * OSW does not need this object for interacting with the native OSW namespace, it is provided mainly
   * for interaction with external programs which have known, fixed namespaces.  (Also OSCRoute, OSCMessage, etc)
   */
  class OSCListen : public OSCParser, public Transform {

    public: 

      void MessageActivate(char *address, char *typeTags, char *data, char *boundary) {
        List alist;
        char *thisType;
        alist.append(new Datum<String>(address));
        for (thisType = typeTags + 1; *thisType != 0; ++thisType) {
          if(*thisType != 'N') {
            BaseDatum *datum = OSCToDatum(data,thisType,boundary);
            if (datum) {
              alist.append(datum);
            }
          }
        }
        messageOut = alist;
      }

      State<Integer> port;
      Outlet<List>   messageOut;

      OSCListen (const String &aname, Container *acontainer, int argc, char *argv[]) : 
       Transform(aname,acontainer,argc,argv),
       messageOut("messageOut", this, "Received OSC message in list format"),
       portChanged (1,0,this,&OSCListen::portChangedExpr),
       port ("port",this,"port number",ScanArguments<Integer>("-port",7005,acontainer,argc,argv)) {
        AddUDPServer(port,&handler_friend,this);
        old_port = (Integer) port;
      }

      ~OSCListen () {
        RemoveUDPServer(port,&handler_friend,this);
      }

      const char *Description () const {
        return "Listens for UDP packets and parses them into OSC list-format messages.";
      }
      
      string PersistentState () const {
	return "port";
      }

    protected:

      int old_port;
      Activation<OSCListen> portChanged;

      void portChangedExpr () {
        RemoveUDPServer(old_port,&handler_friend,this);
        AddUDPServer(port,&handler_friend,this);
        old_port = (Integer) port;
      }

      friend int handler_friend (char *buf, int n, ClientAddr returnAddr, void *extra) {
        return ((OSCListen *) extra)->handler(buf,n,returnAddr);
      }

      int handler (char *buf, int n, ClientAddr returnAddr) {
        ReadOSCMessage(buf, buf + n);
        return 0;
      }
    

  };

  static TransformFactory<OSCListen> OSCListenFactory("OSCListen",true);


  /*
   * OSCRoute
   * Input is an OSC message in list format
   * Arguments are OSC patterns
   * OSCRoute attempts to match the incoming address against one of its arguments.
   * If the match is a success, OSCRoute will remove the matching part of the address and pass on the rest of the list.
   * If the address matches fully, the address part of the list will be removed entirely and the rest passed on.
   * Globbing operators are not supported!
   */
  class OSCRoute : public Transform {

  public:

    Inlet<List> messageIn;
    stl::vector<Outlet<List> *> outputs;

    OSCRoute (const String &aname, Container *acontainer,
		 int argc, char *argv[]) :
      Transform (aname,acontainer,argc,argv),
      messageIn("messageIn",this,"Incoming OSC Message in List Format"),
      dumpOSC(&messageIn,this,&OSCRoute::dumpOSCExpr) {
      outputs.reserve(argc);
      addresses.reserve(argc);
      char buffer[32];
      for (int i = 0; i < argc; ++i) {
        sprintf(buffer,"out%d",i+1);
        outputs.push_back(new Outlet<List>(buffer, this, String("Output for message") + argv[i]));
        dumpOSC.AddEffect(outputs[i]);
        addresses.push_back(argv[i]);
      }
    }

    ~OSCRoute () {
      for (int i = 0; i < outputs.size(); ++i) {
        if (outputs[i]) {
          delete outputs[i];
        }
      }
    }

    const char *Description () const {
      return "Route OSC messages to different outlets based on address, remove matching part of address.";
    }

  protected:

    Activation<OSCRoute> dumpOSC;
    stl::vector<String>  addresses;
    void dumpOSCExpr () {
      String pattern = GetElementFromList<String>(messageIn,0);
      for (int i = 0; i < addresses.size(); ++i) {
	if (MatchesPattern(addresses[i],pattern)) {
	  Debug (pattern+" matches "+addresses[i]);
	  List outList;
	  List::iterator p;
	  for (p = messageIn->begin(), ++p;
	       p != messageIn->end();
	       ++p) {
	    outList.append(*p);
	  }
	  *outputs[i] = outList;
	}
      }
    }

  };

  static TransformFactory<OSCRoute> OSCRouteFactory("OSCRoute",true);

  /**
   * OSCDispatch expects input to be a OSC message in list format
   * The message is 'dispatched' against the OSW namespace of objects.
   * This implementation fully supports namespace, documentation, type-signature and current-value queries,
   * as well as processing set-value commands.
   *
   * Each input message will return exactly one response message (also an OSC message in list format)
   * The response message will have the same address of the input message, followed by a 'return number'.
   * If the return number is less than 0 then an error occured while attempting to dispatch the message.
   * Otherwise no error occured.  Return codes greater than zero may convey other information, for example
   * the return code for a namespace query is the number of nodes found below the query address.
   *
   * TODO: Relpace MatchesPattern() with more efficient standard library strcmp. (or not, MatchesPattern does what it's supposed to do, AC)
   */

  class _oswexport OSCDispatch : public Transform {

    public:

      Inlet<List> messageIn ;
      Outlet<List> messageOut ;
      OSCDispatch(const String &aname, Container *acontainer, int argc, char *argv[]) : 
       Transform(aname,acontainer,argc,argv),
       messageIn ("messageIn",this,"OSC message in list format, /address arg1 arg2 ..."),
       messageOut ("messageOut",this,"OSC reply-message in list format, /address return-code ..."),
       dispatch_query (1,0,this,&OSCDispatch::dispatch_queryExpr) {
        dispatch_query.AddActivator(&messageIn);
        dispatch_query.AddEffect(&messageOut);
        messageIn.SetOrder(0);
        messageOut.SetOrder(0);
      }

      ~OSCDispatch () { }
      
      const char *Description () const {
        return "Dispatch and respond to OSC messages using the OSW namespace";
      }

    private :


      Activation<OSCDispatch> dispatch_query ;
      void dispatch_queryExpr () { 
        if(messageIn->size() == 0) {
          // Invalid message (ignore)
          return;
        }
        
        // Extract address into a String.
        String osc_address = GetElementFromList<String>(messageIn,0);
	// Deal with the simplist and most common cast first;
        // When the address points directly to an OSC-nameable object.
        if(osc_address.size() > 1 && IsValidNameable(osc_address)) {
	  // Add the address to the return message.
          List alist;
          alist.append(new Datum<String>(osc_address));
          // Ignore the root node
          if(osc_address.size() != 1) {
            // Find what is there
            Nameable *thing = Nameable::Find(osc_address, NULL);
	    // Remove header (address) from ((List &) messageIn)
	    messageIn->erase(messageIn->begin());
	    // See if it is a state...
	    BaseState *state = dynamic_cast<osw::BaseState *>(thing);
	    if (state != NULL) {
	      // Handle the list types seperately
	      if(state->TypeOf() == typeid(List)) {
		((State<List> *)state)->SetFromDatum(new Datum<List>(((List &) messageIn)));
		alist.append(new Datum<int>(messageIn->size()));
	      }
	      else if(state->TypeOf() == typeid(Any)) {
		if ((messageIn->size() > 1 || messageIn->size() == 0)) {
		  *((State<Any> *)state) = new Datum<List>(((List &) messageIn));
		  alist.append(new Datum<int>(messageIn->size()));
		} else {
		  *((State<Any> *)state) = messageIn->GetUntypedElement(0);
		  alist.append(new Datum<int>(messageIn->size()));
		}
	      }
	      else {
		// If there are no args supplied, then we cannot set anything (there are no intrisic defaults)
		if(messageIn->size() == 0) {
		  alist.append(new Datum<int>(-1));
		  alist.append(new Datum<String>("Set-value query requires one or more values."));
		} else {
		  // otherwise import the state via SetFromDatum()
		  // Make sure the types match...
                  string messageInTypeTag = GetTypeTagFromDatum(messageIn->GetUntypedElement(0));
                  string stateTypeTag = GetTypeTagFromState(state);
                  if(stateTypeTag != messageInTypeTag) {
                    if ((stateTypeTag == "T" && messageInTypeTag == "F") ||
                        (stateTypeTag == "F" && messageInTypeTag == "T")) {
                      state->SetFromDatum(messageIn->GetUntypedElement(0));
                      alist.append(new Datum<int>(messageIn->size()));
                    } else {
                      alist.append(new Datum<int>(-1));
                      alist.append(new Datum<String>("The type of input does not match the target state."));
                    }
		  } else {
		    state->SetFromDatum(messageIn->GetUntypedElement(0));
		    alist.append(new Datum<int>(messageIn->size()));
		  }
		}
	      }
	    } else {
	      // The target object is not a state (therefore not set-able)
	      alist.append(new Datum<int>(-1));
	      alist.append(new Datum<String>("Target object must be an OSW State"));
	    }
	  } else {
	    // Root node is not a state
	    alist.append(new Datum<int>(-1));
	    alist.append(new Datum<String>("Target object must be an OSW State"));
	  }
	  // All done.
	  messageOut = alist;
	}
        // Rename, other object-specific methods
        else if(IsValidExtendedMethodQuery(osc_address)) {
          if(IsValidState(OSCStripLastName(osc_address)) && 
           (MatchesPattern(osc_address, "*/set-name") ||
            MatchesPattern(osc_address, "*/get-coactivators") ||
            MatchesPattern(osc_address, "*/get-effects"))) {
            try {
              if(MatchesPattern(osc_address, "*/set-name")) {
                if(messageIn->size() < 2) {
                  throw Exception("Not enough args to rename.");
                }
                StateInterface::Rename(OSCStripLastName(osc_address), GetElementFromList<String>(messageIn,1));
                List alist;
                alist.append(new Datum<String>(osc_address));
                alist.append(new Datum<int>(0));
                alist.append(new Datum<String>(GetElementFromList<String>(messageIn,1)));
                messageOut = alist;
                return;
              }
              else if(MatchesPattern(osc_address, "*/get-coactivators")) {
                List *alist = StateInterface::CoActivators(OSCStripLastName(osc_address));
                alist->prepend(new Datum<int>(alist->size()));
                alist->prepend(new Datum<String>(osc_address));
                messageOut = *alist;
                delete alist;
                return;
              }
              else if(MatchesPattern(osc_address, "*/get-effects")) {
                List *alist = StateInterface::Effects(OSCStripLastName(osc_address));
                alist->prepend(new Datum<int>(alist->size()));
                alist->prepend(new Datum<String>(osc_address));
                messageOut = *alist;
                delete alist;
                return;
              }
            } catch(Exception e) {
              List alist;
              alist.append(new Datum<String>(osc_address));
              alist.append(new Datum<int>(-1));
              alist.append(new Datum<String>(e.GetMessage()));
              messageOut = alist;
              return;
            }
            // Default response... no error
            List alist;
            alist.append(new Datum<String>(osc_address));
            alist.append(new Datum<int>(0));
            messageOut = alist; 
            return;
          }
          else if(IsValidOutlet(OSCStripLastName(osc_address)) &&
            (MatchesPattern(osc_address, "*/connect") ||
            MatchesPattern(osc_address, "*/disconnect"))) {
            try {
              if(MatchesPattern(osc_address, "*/connect")) {
                if(messageIn->size() < 2) {
                  throw Exception("Not enough args to connect");
                }
                OutletInterface::Connect(OSCStripLastName(osc_address), GetElementFromList<String>(messageIn,1));
              } else if(MatchesPattern(osc_address, "*/disconnect")) {
                if(messageIn->size() < 2) {
                  throw Exception("Not enough args to disconnect");
                }
                OutletInterface::Disconnect(OSCStripLastName(osc_address), GetElementFromList<String>(messageIn,1));
              }
            } catch (Exception e) {
              List alist;
              alist.append(new Datum<String>(osc_address));
              alist.append(new Datum<int>(-1));
              alist.append(new Datum<String>(e.GetMessage()));
              messageOut = alist;
              return;
            }
            // Default response... no error
            List alist;
            alist.append(new Datum<String>(osc_address));
            alist.append(new Datum<int>(0));
            alist.append(new Datum<String>(GetElementFromList<String>(messageIn,1)));
            messageOut = alist;
            return;
          }
          else if(IsValidTransform(OSCStripLastName(osc_address)) &&
           (MatchesPattern(osc_address, "*/set-name") ||
            MatchesPattern(osc_address, "*/get-inlets") ||
            MatchesPattern(osc_address, "*/get-outlets") ||
            MatchesPattern(osc_address, "*/persistent-states")
           )) 
          {
            try {
              if(MatchesPattern(osc_address, "*/set-name")) {
                if(messageIn->size() < 2) {
                  throw Exception("Not enough args to rename.");
                }
                TransformInterface::Rename(OSCStripLastName(osc_address), GetElementFromList<String>(messageIn,1));
                List alist;
                alist.append(new Datum<String>(osc_address));
                alist.append(new Datum<int>(0));
                alist.append(new Datum<String>(GetElementFromList<String>(messageIn,1)));
                messageOut = alist;
                return;
              } else if(MatchesPattern(osc_address, "*/get-inlets")) {
                List *alist = TransformInterface::Inlets(OSCStripLastName(osc_address));
                alist->prepend(new Datum<int>(alist->size()));
                alist->prepend(new Datum<String>(osc_address));
                messageOut = *alist;
                delete alist;
                return;
              } else if(MatchesPattern(osc_address, "*/get-outlets")) {
                List *alist = TransformInterface::Outlets(OSCStripLastName(osc_address));
                alist->prepend(new Datum<int>(alist->size()));
                alist->prepend(new Datum<String>(osc_address));
                messageOut = *alist;
                delete alist;
                return;
              } else if(MatchesPattern(osc_address, "*/persistent-states")) {
                List alist;
                alist.append(new Datum<String>(osc_address));
                alist.append(new Datum<int>(0));
                alist.append(new Datum<String>(TransformInterface::PersistentState(OSCStripLastName(osc_address))));
                messageOut = alist;
                return;
              }
            } catch (Exception e) {
              List alist;
              alist.append(new Datum<String>(osc_address));
              alist.append(new Datum<int>(-1));
              alist.append(new Datum<String>(e.GetMessage()));
              messageOut = alist;
              return;
            }
          }
          else if(IsValidDataSource(OSCStripLastName(osc_address)) &&
           (MatchesPattern(osc_address, "*/get-data") ||
            MatchesPattern(osc_address, "*/set-data")))
          {
            try {
              if(MatchesPattern(osc_address, "*/get-data")) {
                List alist;
                alist.append(new Datum<String>(osc_address));
                alist.append(new Datum<int>(0));
                alist.append(new Datum<String>(TransformInterface::GetData(OSCStripLastName(osc_address))));
                messageOut = alist;
                return;
              } else if(MatchesPattern(osc_address, "*/set-data")) {
                if(messageIn->size() < 2) {
                  throw Exception("Not enough args to set-data");
                }
                TransformInterface::SetData(OSCStripLastName(osc_address), GetElementFromList<String>(messageIn,1));
                List alist;
                alist.append(new Datum<String>(osc_address));
                alist.append(new Datum<int>(0));
                messageOut = alist;
                return;
              }
            } catch (Exception e) {
              List alist;
              alist.append(new Datum<String>(osc_address));
              alist.append(new Datum<int>(-1));
              alist.append(new Datum<String>(e.GetMessage()));
              messageOut = alist;
              return;
            }
          }
          else if(IsValidPatch(OSCStripLastName(osc_address)) &&
           (MatchesPattern(osc_address, "*/add-transform") ||
            MatchesPattern(osc_address, "*/delete-transform")))
          {
            try {
              if(MatchesPattern(osc_address, "*/add-transform")) {
                if(messageIn->size() < 3) {
                  throw Exception("Not enough args for add-transform");
                }
                int argc = messageIn->size() - 3;
                char **argv = new char *[argc];
                for (int i = 0; i < argc; ++i) {
                  ostringstream os;
                  os << *(messageIn->GetUntypedElement(i+3));
                  argv[i] = strdup(os.str().c_str());
                }
                List alist;
                alist.append(new Datum<String>(osc_address));
                alist.append(new Datum<int>(0));
                alist.append(new Datum<String>(PatchInterface::Create(OSCStripLastName(osc_address), GetElementFromList<String>(messageIn,1), GetElementFromList<String>(messageIn,2), argc, argv)));
                messageOut = alist;
                for (int j = 0; j < argc; ++j) {
                  free(argv[j]);
                }
                delete [] argv;
                return;
              }
              else if(MatchesPattern(osc_address, "*/delete-transform")) {
                if(messageIn->size() < 2) {
                  throw Exception("Not enough args for delete-transform");
                }
                PatchInterface::Destroy(OSCStripLastName(osc_address) + 
					"/" + 
					(String)GetElementFromList<String>(messageIn,1));
                List alist;
                alist.append(new Datum<String>(osc_address));
                alist.append(new Datum<int>(0));
                alist.append(new Datum<String>(GetElementFromList<String>(messageIn,1)));
                messageOut = alist;
                return;
              }
            } catch(Exception e) {
              List alist;
              alist.append(new Datum<String>(osc_address));
              alist.append(new Datum<int>(-1));
              alist.append(new Datum<String>(e.GetMessage()));
              messageOut = alist;
              return;
            }
          }
        }
        
        // transform-types, root level functions, etc
        else if(IsValidExtendedRootQuery(osc_address)) {
          try {
            if(osc_address == "/transform-types") {
              List *alist = RootInterface::TransformTypes();
              alist->prepend(new Datum<int>(alist->size()));
              alist->prepend(new Datum<String>(osc_address));
              messageOut = *alist;
              delete alist;
              return;
            } else if(osc_address == "/osw-quit") {
              RootInterface::Quit();
              // No point in returning... this thread will be killed!
            } else if(osc_address == "/osw-restart") {
              RootInterface::Restart();
              // Again no point in returning a message...
            } else if(osc_address == "/is-auto-nameable") {
              if(messageIn->size() < 2) {
                throw Exception("Not enough args for is-auto-nameable");
              }
              List alist;
              alist.append(new Datum<String>(osc_address));
              alist.append(new Datum<int>(RootInterface::IsAutoNameable(GetElementFromList<String>(messageIn,1))));
              messageOut = alist;
              return;
            } else if(osc_address == "/load-osx") {
              if(messageIn->size() < 2) {
                throw Exception("Not enough args for load-osx");
              }
              RootInterface::LoadOSX(GetElementFromList<String>(messageIn,1));
              List alist;
              alist.append(new Datum<String>(osc_address));
              alist.append(new Datum<int>(0));
              messageOut = alist;
              return;
            } else if(osc_address == "/advance-clock") {
              MainClock = ++MainClock;
              List alist;
              alist.append(new Datum<String>(osc_address));
              alist.append(new Datum<int>(0));
              messageOut = alist;
              return;
            }
          } catch(Exception e) {
            List alist;
            alist.append(new Datum<String>(osc_address));
            alist.append(new Datum<int>(-1));
            alist.append(new Datum<String>(e.GetMessage()));
            messageOut = alist;
            return;
          }
        }
        
        // OSC current-value query
        else if(osc_address.size() >= 14 && MatchesPattern(osc_address, "*/current-value")) {
          List alist;
          alist.append(new Datum<String>(osc_address));
          // Strip the trailing "/current-value"
          String target = osc_address.erase(osc_address.size()-14,14);
          if(target.size() != 0) {
            // Find what is there
            Nameable *thing = Nameable::Find(target, NULL);
            if(thing != NULL) {
              // See if it is a state...
              BaseState *state = dynamic_cast<osw::BaseState *>(thing);
              if (state != NULL) {
                alist.append(new Datum<int>(0));
                alist.append(state->MakeDatum());
              } else {
                // it is not a state...  and therefore has no readable value!  (has nil (N) type-signature)
                alist.append(new Datum<int>(0));
              }
            } else if(IsValidExtendedMethodQuery(target) || IsValidExtendedRootQuery(target)) {
              // these objects have no readable value... but it is not an error to look at their current value.
              alist.append(new Datum<int>(-1));
              alist.append(new Datum<String>("Target is a Write-only path"));
            } else {
              alist.append(new Datum<int>(-1));
              alist.append(new Datum<String>("Invalid address"));
            }
          } else {
            // The root object has no readable value.
            alist.append(new Datum<int>(0));
          }
          messageOut = alist;
        }
        
        // OSC type-signature query
        else if(osc_address.size() >= 15 && MatchesPattern(osc_address, "*/type-signature")) {
          List alist;
          alist.append(new Datum<String>(osc_address));
          // Strip the trailing "/type-signature"
          String target = (osc_address).erase(osc_address.size()-15,15);
          if(target.size() != 0) {
            // Find what is there
            Nameable *thing = Nameable::Find(target, NULL);
            if(thing != NULL) {
              // See if it is a state...
              BaseState *state = dynamic_cast<osw::BaseState *>(thing);
              if (state != NULL) {
                String ttag = GetTypeTagFromState(state);
                if(ttag.size() == 0) { // Non-OSC-typeable state.
                  alist.append(new Datum<int>(0));
                  alist.append(new Datum<String>("!"));
                } else {
                  alist.append(new Datum<int>(0));
                  alist.append(new Datum<String>(ttag));
                }
                alist.append(new Datum<String>("State"));
                alist.append(new Datum<String>(RegisteredTypeName(state->TypeOf())));
                if(state->IsActivator()) {
                  alist.append(new Datum<String>("Activator"));
                }
              } else {
                // This is object is not a state, (e.g. a transform) and therefore cannot hold data.
                alist.append(new Datum<int>(0));
                alist.append(new Datum<String>("N"));
                // Try to figure out if its a Transform, Patch or Container, else resort to "Unknown"
                if(dynamic_cast<Patch *>(thing) != NULL) {
                  alist.append(new Datum<String>("Patch"));
                } else if(dynamic_cast<Transform *>(thing) != NULL) {
                  alist.append(new Datum<String>("Transform"));
                  alist.append(new Datum<String>((dynamic_cast<Transform *>(thing))->ClassName()));
                  if(dynamic_cast<DataSource *>(thing) != NULL) {
                  	alist.append(new Datum<String>("DataSource"));
                  }
                  if(dynamic_cast<TimeDomainTransform *>(thing) != NULL) {
                  	alist.append(new Datum<String>("TimeDomainTransform"));
                  }
                } else if(dynamic_cast<Container *>(thing) != NULL) {
                  // There are various extensions to Container but distinguishing them is not particularly important.
                  alist.append(new Datum<String>("Container"));
                } else {
                  alist.append(new Datum<String>("Unknown"));
                }
              }
            } else if(IsValidExtendedRootQuery(target)) {
                // These all have N nil type
                alist.append(new Datum<int>(0));
                alist.append(new Datum<String>(RootQueryType(target)));
            } else if(IsValidExtendedMethodQuery(target)) {
              alist.append(new Datum<int>(0));
              alist.append(new Datum<String>(ExtendedMethodQueryType(target)));
            } else {
              alist.append(new Datum<int>(-1));
              alist.append(new Datum<String>("Invalid address"));
            }
          } else {
            // The root object is not a state
            alist.append(new Datum<int>(0));
            alist.append(new Datum<String>("N"));
            alist.append(new Datum<String>("Root"));
          }
          messageOut = alist;
        }

        // OSC documentation query
        // Replace MatchesPattern with strcmp
        else if(osc_address.size() >= 14 && MatchesPattern(osc_address, "*/documentation")) {
          List alist;
          alist.append(new Datum<String>(osc_address));
          // Strip the trailing "/documentation"
          String target = osc_address.erase(osc_address.size()-14,14);
          if(target.size() != 0) {
            // Find what is there
            Nameable *thing = Nameable::Find(target, NULL);
            if(thing != NULL) {
              alist.append(new Datum<int>(0));
              alist.append(new Datum<String>(thing->Documentation()));
            } else if(IsValidExtendedRootQuery(target)) {
              alist.append(new Datum<int>(0));
              alist.append(new Datum<String>(RootQueryDocumentation(target)));             
            } else if(IsValidExtendedMethodQuery(target)) {
              alist.append(new Datum<int>(0));
              alist.append(new Datum<String>(ExtendedMethodQueryDocumentation(target)));              
            } else {
              alist.append(new Datum<int>(-1));
              alist.append(new Datum<String>("Invalid address"));
            }
          } else {
            // Documentation on the root returns a program/version id String
            alist.append(new Datum<int>(0));
            ostringstream os;
            os << "Open Sound World (Version " << GetVersionString() << ")";
            alist.append(new Datum<String>(os.str()));
          }
          messageOut = alist;
        }

        // OSC namespace exploration query
        else if(osc_address.c_str()[osc_address.size() - 1] == (char)('/')) {
          List alist;
          //alist.append(new Datum<String>(osc_address));
          // Remove the trailing slash.
          String target = osc_address;
          target.erase(osc_address.size()-1,1);
          if(target.size() == 0) {
            // then we want to look at the root node
            for (Container::enumerator toplevels = Container::BeginTopLevelEnumerate(); !Container::EndTopLevelEnumerate(toplevels); ++toplevels) {
              alist.append(new Datum<String>(Container::GetNameableFromEnumerator(toplevels)->Name()));
            }
            // Root methods
            alist.append(new Datum<String>("transform-types"));
            alist.append(new Datum<String>("osw-restart"));
            alist.append(new Datum<String>("load-osx"));
            alist.append(new Datum<String>("osw-quit"));
            alist.append(new Datum<String>("is-auto-nameable"));
            alist.append(new Datum<String>("advance-clock"));
            alist.prepend(new Datum<int>(alist.size()));
            alist.prepend(new Datum<String>(osc_address));
            messageOut = alist;
          } else {
            Nameable *thing = Nameable::Find(target);
            if(thing != NULL) {
              Container* node = dynamic_cast<Container*>(Nameable::Find(target));
              if(node != NULL) {
                // List children by name... put into a list.
                //alist.append(new Datum<int>(node->NumMembers()));
                for(Container::MemberIterator children = node->BeginMembers(); !node->EndMembers(children); node->NextMember(children)) {
                  Nameable *child = dynamic_cast<Nameable *>(node->MemberSelf(children));
                  if(child == NULL) {
                    Error("Found NULL child in container!!");
                  } else {
                    alist.append(new Datum<String>(child->Name()));
                  }
                }
                // Add transform or patch extended methods if applicable.
                if(IsValidPatch(target)) {
                  alist.append(new Datum<String>("add-transform"));
                  alist.append(new Datum<String>("delete-transform"));
                }
                if(IsValidTransform(target)) {
                  alist.append(new Datum<String>("get-inlets"));
                  alist.append(new Datum<String>("get-outlets"));
                  alist.append(new Datum<String>("persistent-states"));
                }
                if(IsValidDataSource(target)) {
                  alist.append(new Datum<String>("get-data"));
                  alist.append(new Datum<String>("set-data"));
                }
              }
              else {
                if(IsValidState(target)) {
                  alist.append(new Datum<String>("get-coactivators"));
                  alist.append(new Datum<String>("get-effects"));
                }
                if(IsValidOutlet(target)) {
                  alist.append(new Datum<String>("connect"));
                  alist.append(new Datum<String>("disconnect"));
                }
              }
              // Everything has a rename method
              alist.append(new Datum<String>("set-name"));
              alist.prepend(new Datum<int>(alist.size()));
              alist.prepend(new Datum<String>(osc_address));
              messageOut = alist;
            }
            else if(IsValidExtendedRootQuery(target) || IsValidExtendedMethodQuery(target)) {
              alist.append(new Datum<String>(osc_address));
              alist.append(new Datum<int>(0));
              messageOut = alist;
            }
            else {
              alist.append(new Datum<String>(osc_address));
              alist.append(new Datum<int>(-1));
              alist.append(new Datum<String>("Invalid address"));
              messageOut = alist;
            }
          }
        }
        else {
  	      // Last try, pattern match for states
          List alist;
  	      StateList matchedStates;
  	      MatchStates(const_cast<char *>(osc_address.c_str()+1), NULL, matchedStates);
  	      if (matchedStates.size() == 0) {
      	    // No states were matched.  Time to just give up
      	    alist.append(new Datum<String>(osc_address));
      	    alist.append(new Datum<int>(-1));
      	    alist.append(new Datum<String>("Invalid address"));
      	    messageOut = alist;
      	  } else {
      	    messageIn->erase(messageIn->begin());
      	    alist.append(new Datum<String>(osc_address));
            for (StateList::iterator p = matchedStates.begin(); p != matchedStates.end(); ++p) { 
         	   	BaseState *state = *p;
  	          // Handle the list types seperately
  	          if(state->TypeOf() == typeid(List)) {
  	            ((State<List> *)state)->SetFromDatum(new Datum<List>(((List &) messageIn)));
  	            alist.append(new Datum<int>(0));
  	          }
  	          else if(state->TypeOf() == typeid(Any)) {
  	            if ((messageIn->size() > 1 || messageIn->size() == 0)) {
  	              *((State<Any> *)state) = new Datum<List>(((List &) messageIn));
  	              alist.append(new Datum<int>(0));
  	            } else {
  	              *((State<Any> *)state) = messageIn->GetUntypedElement(0);
  	              alist.append(new Datum<int>(0));
  	            }
  	          }
      	      else {
  	            // If there are no args supplied, then we cannot set anything (there are no intrisic defaults)
  	            if(messageIn->size() == 0) {
  	              alist.append(new Datum<int>(-1));
  	              alist.append(new Datum<String>("Set-value query requires one or more values."));
  	            } else {
  	              // otherwise import the state via SetFromDatum()
  	              // Make sure the types match...
                      string messageInTypeTag = GetTypeTagFromDatum(messageIn->GetUntypedElement(0));
                      string stateTypeTag = GetTypeTagFromState(state);
                      if(stateTypeTag != messageInTypeTag) {
                        if ((stateTypeTag == "T" && messageInTypeTag == "F") ||
                            (stateTypeTag == "F" && messageInTypeTag == "T")) {
                          state->SetFromDatum(messageIn->GetUntypedElement(0));
                          alist.append(new Datum<int>(messageIn->size()));
                        } else {
                          alist.append(new Datum<int>(-1));
                          alist.append(new Datum<String>("The type of input does not match the target state."));
                        }
  	              } else {
  	                state->SetFromDatum(messageIn->GetUntypedElement(0));
  	                alist.append(new Datum<int>(messageIn->size()));
  	              }
  	            }
              }
            }
      	    messageOut = alist;
          }
        }
      }
  };

  static TransformFactory<OSCDispatch> OSCDispatchFactory("OSCDispatch",true);

  /*
   * Setup builtin OSC server and patch
   */

    static osw::Patch *user_patch = NULL;
    static osw::Patch *server_patch = NULL;

  void InitializeOSCServer() {
    // This is the 'user patch'
    user_patch = new osw::Patch ("patch", NULL, 0, NULL);
    
    // The builtin osc_server patch
    server_patch = new osw::Patch ("osc_server",NULL,0,NULL);
  
    // Add OSCDispatch object
    OSCDispatch *server_dispatcher = 
     dynamic_cast<OSCDispatch *>(osw::MakeTransform("OSCDispatch","dispatcher",server_patch->PathName(),0,NULL));
    OSCCommunicator *server_communicator = 
     dynamic_cast<OSCCommunicator *>(osw::MakeTransform("OSCCommunicator","communicator",server_patch->PathName(),0,NULL));

    Transform *xform = dynamic_cast<Transform *>(server_dispatcher);
    BaseState *asink = dynamic_cast<BaseState *>(&server_dispatcher->messageIn);

    // Connect the objects together
    if(! xform->Connect(dynamic_cast<BaseOutlet *>(&server_communicator->messageOut),asink)) {
      Error("Connection from communicator/messageOut -> dispatcher/messageIn failed.");
    }

    xform = dynamic_cast<Transform *>(server_communicator);
    asink = dynamic_cast<BaseState *>(&server_communicator->messageIn);

    // Connect the objects together
    if(! xform->Connect(dynamic_cast<BaseOutlet *>(&server_dispatcher->messageOut),asink)) {
      Error("Connection from dispatcher/messageOut -> communicator/messageIn failed.");
    }

  }

  //! This cleanup handler removes OSC server transforms & patches
  BEGIN_CLEANUP_HANDLER_ORDER(OSCServerCleaner,1) 
  {
    delete user_patch;
    delete server_patch;
  }
  END_CLEANUP_HANDLER(OSCServerCleaner)


}

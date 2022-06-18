/* 
Copyright (c) 2001-2004 Amar Chaudhary. All rights reserved.
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

/* oswOSCClient.cpp
   Implementation of OpenSound Control (OSC) client services
   for OSW
*/

#include "oswTcl.h"
#include "oswOSCPacket.h"
#include "oswOSCTypes.h"
#include "oswList.h"
#include "oswSock.h"

// Set the max buffer size to be large... so that big stuff can fit in the UDP packets.
#define MAX_OSC_BUF 16536

namespace osw {

  // OSC Client transforms:

  // OSCSend

  // OSCBundle
  // TODO: Test OSCBundle?
  
  /*
   * Convert individual messages to OSC packets and transmit via UDP to target host/port
   */

  // TODO: Handle #bundle messages outgoing...

  class OSCSend : public Transform {

  public:

    Inlet<List>       messageIn;
    State<Integer>    port;
    State<String>     host;

    OSCSend(const string &aname, Container *acontainer, int argc, char *argv[]) :
     Transform (aname,acontainer,argc,argv),
     messageIn("messageIn",this,"OSC message as list, /address arg1 arg2 ..."),
     port ("port",this,"port number",ScanArguments<Integer>("-port",7005,acontainer,argc,argv)),
     host ("host",this,"name or IP number of destination",ScanArguments<String>("-host","localhost",acontainer,argc,argv)),
     portChanged (&port,0,this,&OSCSend::portChangedExpr),
     hostChanged (&host,0,this,&OSCSend::hostChangedExpr),
     sendMessage(&messageIn,this,&OSCSend::sendMessageExpr) {
      xmaxbuf = ScanArguments<int>("-maxbuf",1024,acontainer,argc,argv);
      xbuffer = new char [xmaxbuf];
      OSC_initBuffer(&xoscbuf,xmaxbuf,xbuffer);
      sock = OpenUDPSocket(const_cast<char *>(host->c_str()), port);
    }
    
    ~OSCSend() {
      CloseSocket(sock);
      if (xbuffer) {
	delete [] xbuffer;
      }
    }

    const char *Description () const {
      return "Turn a OSC message list into a packet and transmit via UDP.";
    }

    string PersistentState () const {
      return "host port";
    }

  protected:
    char    *xbuffer;
    OSCbuf  xoscbuf;
    int     xmaxbuf;
    void    *sock;

    Activation<OSCSend> sendMessage, portChanged, hostChanged;

    void sendMessageExpr () {
      if(messageIn->size() == 0) {
        return;
      }
      // Convert messageIn to OSC then write to socket.
#ifdef OSW_NO_EXPLICIT_MEMBER_TEMPLATES
      string address = *((Datum<String> *)messageIn->GetUntypedElement(0));
#else
      string address = messageIn->GetElement<String>(0);
#endif
      string typetag = ",";
      for (int i = 1; i < messageIn->size(); ++i) {
        typetag += GetTypeTagFromDatum(messageIn->GetUntypedElement(i));
      }
      OSC_resetBuffer(&xoscbuf);
      OSC_writeAddressAndTypes(&xoscbuf,
			       const_cast<char *>(address.c_str()),
			       const_cast<char *>(typetag.c_str()));
#ifdef OSW_LAME_FOR_LOOP // stupid VC++ problem (AC)
      for (i = 1; i < messageIn->size(); ++i) {
#else
      for (int i = 1; i < messageIn->size(); ++i) {
#endif
        DatumToOSC(&xoscbuf, messageIn->GetUntypedElement(i));
      }

      UDPSocketSend(sock, OSC_getPacket(&xoscbuf), OSC_packetSize(&xoscbuf));
    }

    void portChangedExpr () {
      CloseSocket(sock);
      sock = OpenUDPSocket(const_cast<char *>(host->c_str()),port);
    }
 
    void hostChangedExpr () {
      CloseSocket(sock);
      sock = OpenUDPSocket(const_cast<char *>(host->c_str()),port);
    }



  };

  static TransformFactory<OSCSend> OSCSendFactory("OSCSend",true);
  


 
  /*****************/

  class OSCBundle : public Transform {

  public:

    Inlet<List>       messageIn;
    Inlet<Unit>       sendBundle;
    State<Integer>    port;
    State<String>     host;

    OSCBundle(const string &aname, Container *acontainer, int argc, char *argv[]) :
     Transform (aname,acontainer,argc,argv),
     messageIn("messageIn",this,"OSC message as list, /address arg1 arg2 ..."),
     sendBundle("sendBundle",this,"Close bundle and send."),
     port ("port",this,"port number",ScanArguments<Integer>("-port",7005,acontainer,argc,argv)),
     host ("host",this,"name or IP number of destination",ScanArguments<String>("-host","localhost",acontainer,argc,argv)),
     portChanged (&port,0,this,&OSCBundle::portChangedExpr),
     hostChanged (&host,0,this,&OSCBundle::hostChangedExpr),
     addMessage(&messageIn,this,&OSCBundle::addMessageExpr),
     sendBundleNow(&sendBundle,this,&OSCBundle::sendBundleNowExpr) {
      xinBundle = false;
      xmaxbuf = ScanArguments<int>("-maxbuf",1024,acontainer,argc,argv);
      xbuffer = new char [xmaxbuf];
      OSC_initBuffer(&xoscbuf,xmaxbuf,xbuffer);
      sock = OpenUDPSocket(const_cast<char *>(host->c_str()), port);
    }
    
    ~OSCBundle() {
      CloseSocket(sock);
      if (xbuffer) {
	delete [] xbuffer;
      }
    }

    const char *Description () const {
      return "Construct a bundle of OpenSound Control and transmit via UDP.";
    }

    string PersistentState () const {
      return "host port";
    }

  protected:
    char    *xbuffer;
    OSCbuf  xoscbuf;
    int     xmaxbuf;
    bool    xinBundle;
    void    *sock;

    Activation<OSCBundle> addMessage, sendBundleNow, portChanged, hostChanged;

    void addMessageExpr () {
      if (!xinBundle) {
	OSC_resetBuffer(&xoscbuf);
	OSC_openBundle(&xoscbuf,OSCTT_Immediately());
	xinBundle = true;
      }
      Untyped head = messageIn->nth(0);
      if (head->TypeOf() != typeid(string)) {
	Error("First element of list must be a valid address string");
      } else {
	string address = Extract<string>(head);
	if (address[0] != '/') {
	  Error("OSC address must begin with a \"/\"");
	} else {
	  string typetag = ",";
	  for (List::iterator index = messageIn->begin()+1;
	       index != messageIn->end();
	       ++index) {
	    typetag += GetTypeTagFromDatum(*index);
	  }
	  OSC_writeAddressAndTypes(&xoscbuf,
			       const_cast<char *>(address.c_str()),
			       const_cast<char *>(typetag.c_str()));
	  for (List::iterator index2 = messageIn->begin()+1;
	       index2 != messageIn->end();
	       ++index2) {
	    DatumToOSC(&xoscbuf,*index2);
	  }
	}
      }
    }

    void sendBundleNowExpr () {
      OSC_closeBundle(&xoscbuf);
      UDPSocketSend(sock, OSC_getPacket(&xoscbuf), OSC_packetSize(&xoscbuf));
      xinBundle = false;
    }

    void portChangedExpr () {
      CloseSocket(sock);
      sock = OpenUDPSocket(const_cast<char *>(host->c_str()),port);
    }
 
    void hostChangedExpr () {
      CloseSocket(sock);
      sock = OpenUDPSocket(const_cast<char *>(host->c_str()),port);
    }

  };

  static TransformFactory<OSCBundle> OSCBundleFactory("OSCBundle",true);
  
  /*************************/

  OSW_DECLARE_USE(OSCSendFactory);
  OSW_DECLARE_USE(OSCBundleFactory);

  /*************************/


  /*
  class OSCMessageTransform : public Transform {
    
  public:

    Outlet<List>                   osc_message;
    Inlet<string>                  address;
    std::vector<BaseState *>       inputs;

    OSCMessageTransform (const string &aname, Container *acontainer,
			 int argc, char *argv[]) :
      Transform (aname,acontainer,argc,argv),
      address("address",this,"OSC address",
	      ScanArguments<string>("-address",(argc > 0) ? argv[0] : "",
				    acontainer,argc,argv)),
      packetOut("packetOut",this,"OSC Packet"),
      activation1(ScanArguments<int>("-inputs",1,acontainer,argc,argv),
		  this,&OSCMessageTransform::activation1Expr) {      
      int NumInputs = ScanArguments<int>("-inputs",1,acontainer,argc,argv);
      inputs.reserve(NumInputs);
      
      char buffer[32];
      for (int i = 0; i < NumInputs; ++i) {
	sprintf (buffer,"in%d",i+1);
	inputs.push_back(new Inlet<Any>(buffer,this,""));
	inputs[i]->SetOrder(i+1);
	//activation1.AddActivator(inputs[i]);
      }
      activation1.AddEffect(&packetOut);
      packetOut.SetOrder(0);
      numconnected = 0;
     
      xmaxbuf = ScanArguments<int>("-maxbuf",1024,acontainer,argc,argv);
      xbuffer = new char [xmaxbuf];
      OSC_initBuffer(&xoscbuf,xmaxbuf,xbuffer);
    }
    
    ~OSCMessageTransform() {
      for (int i = 0; i < inputs.size(); ++i) {
	//cout << "Deleting input " << i << endl;
	delete inputs[i];
      }
      if (xbuffer) {
	delete [] xbuffer;
      }
    }
    
    virtual bool Connect (BaseOutlet *source, BaseState *&sink) {
      BaseState *sourceAsState = dynamic_cast<BaseState *>(source);

      if (sourceAsState->TypeOf() != sink->TypeOf()) {
      
	int index;
	for (index = 0; index < inputs.size(); ++index) {
	  if (inputs[index] == sink) {
	    break;
	  }
	}
	string inputName = sink->Name();
	delete sink;
	sink = inputs[index] = 
	  sourceAsState->CloneInlet(inputName,this,"");
	sink->SetOrder(index);
      }
      activation1.AddActivator(sink);

      ++numconnected;
      if (numconnected >= inputs.size()) {
        char buffer[32];
        sprintf(buffer,"in%d",numconnected+1);
        inputs.push_back(sink->CloneInlet(buffer,this));
        inputs[numconnected]->SetOrder(numconnected);	
      }
      ostringstream os;
      os << "oswFixTransformTerminals " << PathName() << endl;
      oswTclEvalString(os.str().c_str());
      
      if (!Transform::Connect(source,sink)) {
	--numconnected;
	return false;
      }
      return true;	      
    }
    
    bool BackDisconnect(BaseState *sink) {
      activation1.RemoveActivator(sink,true);
      --numconnected;
      return Transform::BackDisconnect(sink);
    }

    const char *Description () const {
      return "Construct OpenSound Control (OSC) message from arguments.";
    }

  protected:

    Activation<OSCMessageTransform> activation1;
    int numconnected;
    char *xbuffer;
    OSCbuf  xoscbuf;
    int    xmaxbuf;

    void activation1Expr () {
      
      string typetag = ",";
      for (int i = 0; i < inputs.size(); ++i) {
	if (inputs[i]->IsActivator()) {
	  if (inputs[i]->TypeOf() == typeid(Any)) {
	    typetag += GetTypeTagFromDatum((*((State<Any> *)inputs[i]))->GetDatum());
	  } else {
	    typetag += GetTypeTagFromState(inputs[i]);
	  }
	}
      }
      OSC_resetBuffer(&xoscbuf);
      //OSC_openBundle(&xoscbuf,OSCTT_Immediately());
      OSC_writeAddressAndTypes(&xoscbuf,
			       const_cast<char *>(address->c_str()),
			       const_cast<char *>(typetag.c_str()));
      for (int j = 0; j < inputs.size(); ++j) {
	if (inputs[j]->IsActivator()) {
	  StateToOSC(&xoscbuf,inputs[j]);
	}
      }
      //OSC_closeBundle(&xoscbuf);
      packetOut = OSCPacket(OSC_getPacket(&xoscbuf),OSC_packetSize(&xoscbuf));
    }
    
  };

  static TransformFactory<OSCMessageTransform> 
  OSCMessageTransformFactory("OSCMessage",true);

  */

  /*************************/

  //REGISTER_TYPE(OSCPacket);
}


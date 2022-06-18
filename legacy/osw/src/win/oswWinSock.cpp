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

/*
   oswWinSock.cpp
   Win32 implementation of  TCP/IP and UDP sockets

   Amar Chaudhary
*/

#include "oswSock.h"
#include "oswSelect.h"
#include <list>
#include <iostream>
#include <string>

using namespace std;

namespace osw {

  extern void Error(const string &msg);

  typedef int socklen_t;

  static int xnumsocks = 0;

  void _oswexport *OpenTCPSocket (char *host,int portnum) {

      WSADATA wsaData;
      
      if (xnumsocks == 0) {
	WSAStartup(0x202,&wsaData);
      }

      char *server_name = host;
      struct hostent *hp;
      struct sockaddr_in server;
      
      if (isalpha(server_name[0])) {   /* server address is a name */ 
	hp = gethostbyname(server_name); 
      } 
      else  { /* Convert nnn.nnn address to a usable one */ 
	unsigned int addr = inet_addr(server_name); 
	hp = gethostbyaddr((char *)&addr,4,AF_INET); 
      } 
      
      // 
      // Copy the resolved information into the sockaddr_in structure 
      // 
      memset(&server,0,sizeof(server)); 
      memcpy(&(server.sin_addr),hp->h_addr,hp->h_length); 
      server.sin_family = hp->h_addrtype; 
      server.sin_port = htons(portnum); 
      
      SOCKET sock = socket(AF_INET,SOCK_STREAM,0);
      
      if (connect(sock,(struct sockaddr*)&server,sizeof(server)) 
	  == SOCKET_ERROR) { 
	fprintf(stderr,"connect() failed: %d\n",WSAGetLastError()); 
       
	return NULL;
      }

      return (void *) sock;
  }

  struct UDPSendSocketInfo {
    SOCKET sock;
    struct sockaddr_in server;
    struct sockaddr_in client;
  };
    

  void _oswexport *OpenUDPSocket (char *host,int portnum) {

    WSADATA wsaData;
      
    if (xnumsocks == 0) {
      WSAStartup(0x202,&wsaData);
    }
    
    UDPSendSocketInfo *info = new UDPSendSocketInfo;

    char *server_name = host;
    struct hostent *hp;
    struct sockaddr_in server;
    
    BOOL bBroadcast = FALSE;
    
    if (isalpha(server_name[0])) {   /* server address is a name */ 
      hp = gethostbyname(server_name); 
    } 
    else  { /* Convert nnn.nnn address to a usable one */ 
      unsigned int addr = inet_addr(server_name); 
      hp = gethostbyaddr((char *)&addr,4,AF_INET); 
    } 
    if (!hp && string("255.255.255.255") != server_name) {
      fprintf(stderr,"host not found %s\n",server_name);
      delete info;
      return NULL;
    }

    // 
    // Copy the resolved information into the sockaddr_in structure 
    // 

    memset(&(info->server),0,sizeof(server)); 
    if (!hp) {
      info->server.sin_family = AF_INET;
      // Slimy hack :P (AC)
      info->server.sin_addr.S_un.S_addr = INADDR_BROADCAST;
      bBroadcast = TRUE;
    } else {
      memcpy(&(info->server.sin_addr),hp->h_addr,hp->h_length); 
      info->server.sin_family = hp->h_addrtype; 
    }
    info->server.sin_port = htons(portnum); 
    
    if ((info->sock = socket(AF_INET,SOCK_DGRAM,0)) == SOCKET_ERROR ) {
      fprintf(stderr,"socket() failed: %d\n",WSAGetLastError()); 
      delete info;
      return NULL;
    }
    if (bBroadcast) {
      setsockopt(info->sock,SOL_SOCKET,SO_BROADCAST,(char *)&bBroadcast,sizeof(bBroadcast));
    }
    return (void *) info;
  }

  /*********************/

  struct UDPReplySocketInfo {
    SOCKET sock;
    int inport;
    int outport;
    socklen_t recv_len;
    struct sockaddr_in server;
    struct sockaddr_in client;
  };
    
  void _oswexport *OpenUDPReplySocket (int in_portnum, int out_portnum) {

    UDPReplySocketInfo *info = new UDPReplySocketInfo;    
    
    info->inport = in_portnum;
    info->outport = out_portnum;

    // Source is any interface
    info->client.sin_addr.s_addr = htonl(INADDR_ANY);
    info->client.sin_family = AF_INET;
    info->client.sin_port = htons(in_portnum);
    
    if ((info->sock = socket(AF_INET,SOCK_DGRAM,0)) < 0 ) {
      perror("OpenUDPReplySocket failed at data gram socket creation");
      delete info;
      return NULL;
    }
    
    //int one = 1;
    //if(setsockopt(info->sock, SOL_SOCKET, SO_BROADCAST, &one, sizeof(one)) < 0) {
    //	fprintf(stderr, "Can't set socket broadcast option.\n");
    //}
    
    if(bind(info->sock, (const sockaddr*)&(info->client), sizeof(info->client))) {
      Error("Bind failure");
    }
    
    fprintf(stderr, "UDP Reply Socket initialized, incoming port %d, outgoing port %d\n", in_portnum, out_portnum);
    
    return (void *) info;
  }



  /*********************/

  void _oswexport CloseSocket (void *toClose) {
    closesocket((SOCKET)toClose);
  }

  /*******************/

  int  _oswexport TCPSocketSend (void *sock, const char *bytesToSend, int length) {
    return send ((SOCKET)sock,bytesToSend,length,0);
  }

  int  _oswexport TCPSocketReceive (void *sock, char *bytesToReceive, int length) {
    return recv ((SOCKET)sock,bytesToReceive,length,0);
  }

  /*******************/

  int  _oswexport UDPSocketSend (void *sock, const char *bytesToSend, int length) {
    UDPSendSocketInfo *info = (UDPSendSocketInfo *)sock;
    return sendto (info->sock,bytesToSend,length,0,(const sockaddr *)&(info->server),sizeof(info->server));
  }


  int  _oswexport UDPSocketReceive (void *sock, char *bytesToReceive, int length) {
    UDPSendSocketInfo *info = (UDPSendSocketInfo *)sock;
    return recv (info->sock,bytesToReceive,length,0);
  }
  
  int  _oswexport UDPUnSocketSend (void *sock, const char *bytesToSend, int length, ClientAddr addr) {
    UDPSendSocketInfo *info = (UDPSendSocketInfo *)sock;
    return sendto (info->sock,bytesToSend,length,0,(struct sockaddr *)&addr->cl_addr,addr->clilen); 
  }

  void _oswexport UDPSocketBind(void *sock, int portnum) {
    UDPSendSocketInfo *info = (UDPSendSocketInfo *)sock;
    info->client.sin_port = htons(portnum);
    if(bind(info->sock, (const sockaddr*)&(info->client), sizeof(info->client))) {
      Error("Bind failure");
    }
  }
  
  void _oswexport UDPSocketSetDest(void *sock, struct sockaddr_in* dest, int len, int port) {
    UDPSendSocketInfo *info = (UDPSendSocketInfo *)sock;
    memcpy(&info->server, dest, len);
    info->server.sin_port = htons(port);
  }
  

  void _oswexport UDPSocketSetTimeout(void *sock, int sec, int usec) {
    struct timeval tv;
    tv.tv_sec = sec;
    tv.tv_usec = usec;
    UDPSendSocketInfo *info = (UDPSendSocketInfo *)sock;
    setsockopt (info->sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv, sizeof tv);
  }
  
  // --------------------------------------------------
  
  int _oswexport UDPReplySocketReceive (void *infoptr, char *bytesToReceive, int length) {
    UDPReplySocketInfo *info = (UDPReplySocketInfo *)infoptr;
    int r = recvfrom(info->sock,bytesToReceive,length,0,(sockaddr *)&(info->server),&info->recv_len);
    info->server.sin_port = htons(info->outport);
    return r;
  }
  int _oswexport UDPReplySocketSend (void *infoptr, char *bytesToSend, int length) {
    UDPReplySocketInfo *info = (UDPReplySocketInfo *)infoptr;
    return sendto (info->sock,bytesToSend,length,0,(const sockaddr *)&(info->server),sizeof(info->server));
  }
  
  // -----------------------------------------
  
  void _oswexport UDPReplySocketSetTimeout(void *infoptr, int sec, int usec) {
    struct timeval tv;
    tv.tv_sec = sec;
    tv.tv_usec = usec;
    UDPReplySocketInfo *info = (UDPReplySocketInfo *)infoptr;
    setsockopt (info->sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv, sizeof tv);
  }
    

  /**********************************/


  static int initudp(int port) {
    struct sockaddr_in serv_addr;
    int n, sockfd, optval, optlen;

    WSADATA wsaData; 
    if (xnumsocks == 0) {
      WSAStartup(0x202,&wsaData);
    }    

    if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
      cerr << "initudp: Oh Shit (1)\n";
      return sockfd;
    }
    
    memset((char *)&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(port);
    
    if(bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
      cerr <<  "initudp: Oh Shit (2)\n";      
      return -1;
    }
    
    unsigned long nbio = 1;
    if (ioctlsocket(sockfd,FIONBIO,&nbio) < 0) {
      cerr <<  "initudp: Oh Shit (3)\n";      
      return -1;
    }
    //fcntl(sockfd, F_SETFL, FNDELAY); 
    
    return sockfd;
  }

  struct socket_handler {
    ServerReceiveProc  proc;
    void               *extra;

    socket_handler (ServerReceiveProc aproc,
		    void *aextra) :
      proc(aproc),extra(aextra) {
    }
  };

  struct active_socket {
    SOCKET  sock;
    int     port;
    std::list<socket_handler>  handlers;

    explicit active_socket(int aport) {
      port = aport;
      sock = NULL;
    }

    ~active_socket() {
      if (sock != NULL) {
	closesocket(sock);
      }
    }

    void open () {
      sock = initudp(port);
    }

    void close () {
	closesocket(sock);
	sock = NULL;
    }

    bool operator == (int aport) const {
      return port == aport;
    }
  };


  typedef std::list<active_socket>  socket_list;
  static socket_list   TheSockets;

#define MAXMESG 32768
  static char mbuf[MAXMESG];
  
  static void ReceivePacket(int sockfd, socket_list::iterator index) {
    struct ClientAddressStruct returnAddress;
    int maxclilen=sizeof(returnAddress.cl_addr);
    int n,x,i;
    
    
    returnAddress.clilen = maxclilen;
    while( (n = recvfrom(sockfd, mbuf, MAXMESG, 0, (sockaddr *) &(returnAddress.cl_addr),
			 (int *)&(returnAddress.clilen))) >0)  {
      for (std::list<socket_handler>::iterator h_index = index->handlers.begin();
	   h_index != index->handlers.end();
	   ++h_index) {
	h_index->proc(mbuf,n,&returnAddress,h_index->extra);
      }
      returnAddress.clilen = maxclilen;
    }
  }


  void _oswexport AddUDPServer (int portnum, ServerReceiveProc aproc, void *extra) {
    for (socket_list::iterator  index = TheSockets.begin();
	 index != TheSockets.end();
	 ++index) {
      if (index->port == portnum) {
	index->handlers.push_back(socket_handler(aproc,extra));
	return;
      }
    }
    TheSockets.push_back(active_socket(portnum));
    TheSockets.back().open();
    //cerr << TheSockets.back().sock << endl;
    TheSockets.back().handlers.push_back(socket_handler(aproc,extra));
  }


  void _oswexport RemoveUDPServer (int portnum, ServerReceiveProc aproc, void *extra) {
    for (socket_list::iterator  index = TheSockets.begin();
	 index != TheSockets.end();
	 ++index) {
      if (index->port == portnum) {
	for (std::list<socket_handler>::iterator h_index = index->handlers.begin();
	     h_index != index->handlers.end();
	     ++h_index) {
	  if (h_index->proc == aproc && h_index->extra == extra) {
	    index->handlers.erase(h_index);
	    break;
	  }
	}
	if (index->handlers.empty()) {
	  TheSockets.erase(index);
	}
	return;
      }
    }
  }


  static void UDPServerSetupProc (SelectEventManager &eventManager) {
    if (TheSockets.empty()) {
      return;
    }
    for (socket_list::iterator  index = TheSockets.begin();
	 index != TheSockets.end();
	 ++index) {    
      eventManager.AddRead(index->sock);   
    }
  }

  static void UDPServerDispatchProc (SelectEventManager &eventManager) {
    for (socket_list::iterator index = TheSockets.begin();
	 index != TheSockets.end();
	 ++index) {
      if (eventManager.IsSetRead(index->sock)) {
	//cerr << "Receiving packet on socket " << index->port << endl;
	ReceivePacket(index->sock,index);
      }
    }
  }
  
  void _oswexport InitUDPServers() {
    AddSelectGroup(UDPServerSetupProc,UDPServerDispatchProc);
  }

}


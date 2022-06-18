
/* 
Copyright (c) 2001 Amar Chaudhary. All rights reserved.
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
   oswSock.h
   Abstractions for TCP/IP and UDP sockets

   Amar Chaudhary
*/

#ifndef _OSWSOCK
#define _OSWSOCK

#include "oswCfg.h"
#ifdef OSW_NO_STDC_HEADERS
#include <string.h>
#include <stdio.h>
#else
#include <cstring>
#include <cstdio>
#endif 

#if defined(__WIN32__) || defined(_WINDOWS)
#include <winsock.h>
#else
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <rpc/rpc.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/times.h>
#include <sys/param.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pwd.h>
#include <signal.h>
#include <grp.h>
#include <sys/file.h>
#endif

#define OSW_SOCKET_BROADCAST 1
#define OSW_SOCKET_NONE 2

namespace osw {

  typedef struct ClientAddressStruct {
    struct sockaddr_in  cl_addr;
    unsigned int clilen;
    int sockfd;
  } *ClientAddr;
  
  void _oswexport *OpenTCPSocket (char *host,int portnum);
  void _oswexport *OpenUDPSocket (char *host,int portnum);
  void _oswexport *OpenUDPReplySocket (int in_portnum, int out_portnum);
  void _oswexport CloseSocket (void *toClose);
  int  _oswexport TCPSocketSend (void *sock, const char *bytesToSend, int length);
  int  _oswexport TCPSocketReceive (void *sock, char *bytesToReceive, int length);
  int  _oswexport UDPSocketSend (void *sock, const char *bytesToSend, int length);
  int  _oswexport UDPSocketReceive (void *sock, char *bytesToReceive, int length);
  void _oswexport UDPSocketBind(void* sock, int portnum);
  void _oswexport UDPSocketSetTimeout (void *sock, int sec, int usec);
  void _oswexport UDPSocketSetDest (void *sock, struct sockaddr_in* cl_addr, int clilen, int port);
  int  _oswexport UDPReplySocketReceive (void *sock, char *bytesToReceive, int length);
  int  _oswexport UDPReplySocketSend (void *sock, char *bytesToSend, int length);
  void _oswexport UDPReplySocketSetTimeout (void *sock, int sec, int usec);

  typedef ClientAddr NetworkAddress;

  int  _oswexport UDPUnSocketSend (void *sock, const char *bytesToSend, int length, ClientAddr addr);


  typedef int (*ServerReceiveProc) (char *buf, int n, ClientAddr returnAddr, void *extra);

  void _oswexport AddUDPServer (int portnum, ServerReceiveProc aproc, void *extra);
  void _oswexport RemoveUDPServer (int portnum, ServerReceiveProc aproc, void *);
  void _oswexport InitUDPServers();

}
#endif // _OSWSOCK


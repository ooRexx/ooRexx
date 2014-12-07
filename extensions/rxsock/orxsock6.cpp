/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2014-2014 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* http://www.oorexx.org/license.html                                         */
/*                                                                            */
/* Redistribution and use in source and binary forms, with or                 */
/* without modification, are permitted provided that the following            */
/* conditions are met:                                                        */
/*                                                                            */
/* Redistributions of source code must retain the above copyright             */
/* notice, this list of conditions and the following disclaimer.              */
/* Redistributions in binary form must reproduce the above copyright          */
/* notice, this list of conditions and the following disclaimer in            */
/* the documentation and/or other materials provided with the distribution.   */
/*                                                                            */
/* Neither the name of Rexx Language Association nor the names                */
/* of its contributors may be used to endorse or promote products             */
/* derived from this software without specific prior written permission.      */
/*                                                                            */
/* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS        */
/* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT          */
/* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS          */
/* FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT   */
/* OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,      */
/* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED   */
/* TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,        */
/* OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY     */
/* OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING    */
/* NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS         */
/* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.               */
/*                                                                            */
/*----------------------------------------------------------------------------*/


#ifdef HAVE_CONFIG_H
    #include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <ctype.h>
#include <setjmp.h>
/*------------------------------------------------------------------
 * tcp/ip includes
 *------------------------------------------------------------------*/
#include <sys/types.h>
#include <errno.h>

#if !defined(WIN32)
    #include <netdb.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #ifdef __APPLE__
        // need to define this for Mac OSX 10.2
        #define _BSD_SOCKLEN_T_
    #endif
    #include <sys/socket.h>
    #include <sys/ioctl.h>
    #include <sys/time.h>
    #include <unistd.h>
    #if defined( HAVE_SYS_SELECT_H )
        #include <sys/select.h>
    #endif
    #if defined( HAVE_SYS_FILIO_H )
        #include <sys/filio.h>
    #endif
   #define sock_errno() errno
   #define psock_errno(s) printf("\nrxsock6 error %s\n",s)
   #define ORXSOCKET int
   #define INVALID_SOCKET -1  //defined in Windows
   #define closesocket close
   #define SOCKOPTION int
#else
   #include <malloc.h>
   #include <winsock2.h>
   #include <ws2tcpip.h>
   #define psock_errno(s) fprintf(stderr, "\nrxsock6 Error: %s\n", s)
   #define ORXSOCKET uintptr_t
   #define sock_errno WSAGetLastError
   #define inet_ntop InetNtop
   #define inet_pton InetPton
   #define SOCKOPTION char
#endif

#if !defined(HOST_NAME_MAX)
   #define HOST_NAME_MAX 255
#endif

/*------------------------------------------------------------------
 * rexx includes
 *------------------------------------------------------------------*/
# include "oorexxapi.h"



// a helper routine for transformations
ORXSOCKET getSocket(RexxMethodContext* context)
{
    uintptr_t temp;
    // get the socket file descriptor
    RexxObjectPtr rxsockfd = context->GetObjectVariable("socketfd");
    context->Uintptr(rxsockfd, &temp);
    return (ORXSOCKET)temp;
}


// Another helper routine.
RexxObjectPtr socketToObject(RexxMethodContext* context, ORXSOCKET socket)
{
#ifdef WIN32
    return context->UintPtr((uintptr_t)socket);
#else
    return context->Int32((int)socket);
#endif
}


// RKM:  Common routine for setting the error condition
void setErrno(RexxMethodContext* context, bool hasError)
{
    context->SetObjectVariable("errno", context->Int32(hasError ? sock_errno() : 0));
}


/*----------------------------------------------------------------------------*/
/* Method: init                                                               */
/* Description: instance initialization                                       */
/* Arguments:                                                                 */
/*         domain   - socket domain, like PF_INET6                            */
/*         type     - socket type, like SOCK_STREAM                           */
/*         protocol - socket protocol, usually zero                           */
/*----------------------------------------------------------------------------*/

// RKM:  You shouldn't need different signatures here.  A uintptr_t can hold
// an int value just fine.
RexxMethod3(RexxObjectPtr,                 // Return type
            orxSocket6,                // Object_method name
            int, domain,               // protocol family
            int, type,                 // socket type
            int, protocol)             // protocol
{
    ORXSOCKET socketfd;
    int zero = 0;

#ifdef WIN32
    WORD wVersionRequested;
    WSADATA wsaData;
    wVersionRequested = MAKEWORD( 1, 1 );
    int rc = WSAStartup( wVersionRequested, &wsaData );
#endif
    // perform function and return
    socketfd = socket(domain, type, protocol);
    if (socketfd == INVALID_SOCKET)
    {
        context->SetObjectVariable("retc", context->Int32(-1));
        context->SetObjectVariable("errno", context->Int32(sock_errno()));
    }
    else
    {
        context->SetObjectVariable("retc", context->Int32(0));
        context->SetObjectVariable("errno", context->Int32(0));
    }

#if defined(WIN32)
    // Windows sockets are not dual-stack by default, so force it to be dual-stack
    setsockopt(socketfd, SOL_SOCKET, IPV6_V6ONLY, (char *)&zero, sizeof(zero));
#endif
    return socketToObject(context, socketfd);
}


/*----------------------------------------------------------------------------*/
/* Method: accept                                                             */
/* Description: accept a connection                                           */
/* Arguments:                                                                 */
/*         inetaddr - Inetaddr instance uninitialized (optional)              */
/*----------------------------------------------------------------------------*/

RexxMethod1(RexxObjectPtr,             // Return type
            orxAccept6,                // Object_method name
            OPTIONAL_RexxObjectPtr, inetaddr) // INetaddr instance
{
    RexxObjectPtr newrxsockfd;
    RexxClassObject classinst;
    ORXSOCKET socketfd, retc;
    struct sockaddr_storage myaddr;
    struct sockaddr_in * myaddr4 = (struct sockaddr_in *)&myaddr;
    struct sockaddr_in6 * myaddr6 = (struct sockaddr_in6 *)&myaddr;
    socklen_t len = sizeof(struct sockaddr_storage);
    char str[INET6_ADDRSTRLEN];

    socketfd = getSocket(context);
    memset(&myaddr, 0, sizeof(struct sockaddr_storage));
    // perform function and return
    retc = accept(socketfd, (struct sockaddr *)&myaddr, &len);

    context->SetObjectVariable("retc", socketToObject(context, retc));

    // RKM: use common routine for setting the errno value
    setErrno(context, retc == INVALID_SOCKET);

    if (retc == INVALID_SOCKET)
    {
        return socketToObject(context, retc);
    }

    if (inetaddr != NULLOBJECT)
    {
        if (!context->IsOfType(inetaddr, "InetAddress"))
        {
            RexxArrayObject arrobj;
            const char *msg = "Argument 1 must be of type .InetAddress.";
            context->ArrayAppendString(arrobj, msg, strlen(msg));
            context->RaiseException(88900, arrobj);
            return context->String("-1");
        }
        // fill out the caller's inetaddress
        if (myaddr.ss_family == AF_INET)
        {
            context->SendMessage1(inetaddr, "family=",
                                  context->UnsignedInt64((uint64_t)myaddr4->sin_family));
            context->SendMessage1(inetaddr, "port=",
                                  context->UnsignedInt64((uint64_t)myaddr4->sin_port));

            // RKM:  This used a lot and the only difference is the routine name.
            // create a macro for the name.
            inet_ntop(AF_INET, myaddr4, str, INET6_ADDRSTRLEN);
        }
        else
        {
            context->SendMessage1(inetaddr, "family=",
                                  context->UnsignedInt64((uint64_t)myaddr6->sin6_family));
            context->SendMessage1(inetaddr, "port=",
                                  context->UnsignedInt64((uint64_t)myaddr6->sin6_port));
            inet_ntop(AF_INET6, myaddr4, str, INET6_ADDRSTRLEN);
        }
        context->SendMessage1(inetaddr, "address=", context->String(str));
    }
    classinst = context->FindClass("Socket");
    newrxsockfd = context->SendMessage1(classinst, "new", socketToObject(context, retc));
    return newrxsockfd;
}


/*----------------------------------------------------------------------------*/
/* Method: bind                                                               */
/* Description: bind a socket to an address.                                  */
/* Arguments:                                                                 */
/*         inetaddr - Inetaddr instance initialized with the address          */
/*----------------------------------------------------------------------------*/

RexxMethod1(int,                       // Return type
            orxBind6,                  // Object_method name
            RexxObjectPtr, inetaddr)   // Inetaddr instance
{
    ORXSOCKET socketfd;
    struct sockaddr_storage myaddr;
    struct sockaddr_in * myaddr4 = (struct sockaddr_in *)&myaddr;
    struct sockaddr_in6 * myaddr6 = (struct sockaddr_in6 *)&myaddr;
    char str[INET6_ADDRSTRLEN];
    socklen_t len;
    RexxObjectPtr obj;
    uint32_t tmp;

    socketfd = getSocket(context);

    // perform function and return
    memset(&myaddr, 0, sizeof(struct sockaddr_storage));

    // RKM:  This is done a lot...got place for a helper function.
    if (!context->IsOfType(inetaddr, "InetAddress"))
    {
        RexxArrayObject arrobj;
        const char *msg = "Argument 1 must be of type .InetAddress.";
        context->ArrayAppendString(arrobj, msg, strlen(msg));
        context->RaiseException(88900, arrobj);
        return -1;
    }
    obj = context->SendMessage0(inetaddr, "family");
    context->UnsignedInt32(obj, &tmp);
    if (tmp == AF_INET)
    {
        myaddr4->sin_family = (uint16_t) tmp;
        obj = context->SendMessage0(inetaddr, "port");
        context->UnsignedInt32(obj, &tmp);
        myaddr4->sin_port = (uint16_t) tmp;
        obj = context->SendMessage0(inetaddr, "address");

        // another helper macro opportunity
        inet_pton(myaddr4->sin_family, context->CString(obj), str);
        len = sizeof(struct sockaddr_in);
    }
    else
    {
        myaddr6->sin6_family = (uint16_t) tmp;
        obj = context->SendMessage0(inetaddr, "port");
        context->UnsignedInt32(obj, &tmp);
        myaddr6->sin6_port = (uint16_t) tmp;
        obj = context->SendMessage0(inetaddr, "address");
        inet_pton(myaddr6->sin6_family, context->CString(obj), str);
        len = sizeof(struct sockaddr_in6);
    }
    int retc = bind(socketfd, (struct sockaddr *)&myaddr, len);
    context->SetObjectVariable("retc", context->Int32(retc));

    setErrno(context, retc == -1);
    return retc;
}

/*----------------------------------------------------------------------------*/
/* Method: close                                                              */
/* Description: shutdown and close a socket                                   */
/* Arguments: none                                                            */
/*----------------------------------------------------------------------------*/

// RKM:  use uintptr every where
RexxMethod0(uintptr_t,                 // Return type
            orxClose6)                 // Object_method name
{
    ORXSOCKET socketfd, retc;

    socketfd = getSocket(context);
    // perform function and return
    retc = shutdown(socketfd, 2);
    // RKM:  Don't much mind the conditional here because it is only used in one place.
    // however, a #define for the Linux case to map closesocket to close would
    // simplify this in the inline code.  Probably worth doing, since it appears
    // using the conditional here might have introduced a bug.
    retc = closesocket(socketfd);
    // RKM:  Shouldn't this variable also be set for the Linux case?
    context->SetObjectVariable("retc", context->Uintptr(retc));

    setErrno(context, retc == -1);

    // RKM:  Need a cast here.
    return (uintptr_t)retc;
}


/*----------------------------------------------------------------------------*/
/* Method: connect                                                            */
/* Description: connect a socket to a remote address                          */
/* Arguments:                                                                 */
/*         inetaddr - Inetaddr instance initialized with the address          */
/*----------------------------------------------------------------------------*/

RexxMethod1(int,                       // Return type
            orxConnect6,               // Object_method name
            RexxObjectPtr, inetaddr)   // Inetaddr instance
{
    ORXSOCKET socketfd;
    struct sockaddr_storage myaddr;
    struct sockaddr_in * myaddr4 = (struct sockaddr_in *)&myaddr;
    struct sockaddr_in6 * myaddr6 = (struct sockaddr_in6 *)&myaddr;
    socklen_t len = sizeof(myaddr);
    RexxObjectPtr obj;
    uint32_t tmp;

    socketfd = getSocket(context);

    // perform function and return
    memset(&myaddr, 0, sizeof(struct sockaddr_storage));
    if (!context->IsOfType(inetaddr, "InetAddress"))
    {
        RexxArrayObject arrobj;
        const char *msg = "Argument 1 must be of type .InetAddress.";
        context->ArrayAppendString(arrobj, msg, strlen(msg));
        context->RaiseException(88900, arrobj);
        return -1;
    }


    // RKM:  This is also done a lot...make a single helper function
    obj = context->SendMessage0(inetaddr, "family");
    context->UnsignedInt32(obj, &tmp);
    if (tmp == AF_INET)
    {
        myaddr4->sin_family = (uint16_t) tmp;
        obj = context->SendMessage0(inetaddr, "port");
        context->UnsignedInt32(obj, &tmp);
        myaddr4->sin_port = (uint16_t) tmp;
        obj = context->SendMessage0(inetaddr, "address");
        inet_pton(AF_INET, context->CString(obj), &myaddr4->sin_addr.s_addr);
    }
    else
    {
        myaddr6->sin6_family = (uint16_t) tmp;
        obj = context->SendMessage0(inetaddr, "port");
        context->UnsignedInt32(obj, &tmp);
        myaddr6->sin6_port = (uint16_t) tmp;
        obj = context->SendMessage0(inetaddr, "address");
        inet_pton(AF_INET6, context->CString(obj), &myaddr6->sin6_addr.s6_addr);
    }
    int retc = connect(socketfd, (struct sockaddr *)&myaddr, len);

    // RKM:  Setting the error stuff is done a lot.  Make a helper here too.

    context->SetObjectVariable("retc", context->Int32(retc));

    setErrno(context, retc == -1);
    return retc;
}


/*----------------------------------------------------------------------------*/
/* Method: getAddrInfo                                                        */
/* Description: get the address info for a host.                              */
/* Arguments:                                                                 */
/*         nodename - the host name or ip address                             */
/*         servname - the service name or number                              */
/*         hints    - an Inetaddr for the search hints                        */
/*         rea      - Rexx array variable (empty)                             */
/*----------------------------------------------------------------------------*/

RexxMethod4(int,                       // Return type
            orxGetAddrinfo6,           // Object_method name
            RexxStringObject, nodename,// the host name or ip address
            RexxStringObject, servname,// the service name or number
            RexxObjectPtr, hints,      // an Inetaddr for the search hints
            RexxArrayObject, rea)      // the name of a Rexx variable for the returned array
{
    CSTRING snodename = context->StringData(nodename);
    CSTRING sservname = context->StringData(servname);
    struct addrinfo shints, *rrea, *startrrea;
    RexxObjectPtr obj;
    int tmp, retc;
    unsigned int utmp;
    struct sockaddr_storage myaddr;
    struct sockaddr_in * myaddr4 = (struct sockaddr_in *)&myaddr;
    struct sockaddr_in6 * myaddr6 = (struct sockaddr_in6 *)&myaddr;
    char str[INET6_ADDRSTRLEN];

    // fill out the shints struct
    memset(&shints, 0, sizeof(shints));
    obj = context->SendMessage0(hints, "ai_flags");
    context->Int32(obj, &tmp);
    shints.ai_flags = (int32_t) tmp;
    obj = context->SendMessage0(hints, "ai_family");
    context->Int32(obj, &tmp);
    shints.ai_family = (int32_t) tmp;
    obj = context->SendMessage0(hints, "ai_socktype");
    context->Int32(obj, &tmp);
    shints.ai_socktype = (int32_t) tmp;
    obj = context->SendMessage0(hints, "ai_protocol");
    context->Int32(obj, &tmp);
    shints.ai_protocol = tmp;
    obj = context->SendMessage0(hints, "ai_canonname");
    shints.ai_canonname = (char *) context->StringData((RexxStringObject)obj);
    obj = context->SendMessage0(hints, "sa_family");
    context->UnsignedInt32(obj, &utmp);
    if (tmp == AF_INET)
    {
        myaddr4->sin_family = (uint16_t) utmp;
        obj = context->SendMessage0(hints, "sa_addr");
        inet_pton(AF_INET, context->CString(obj), &myaddr4 ->sin_addr.s_addr);
        shints.ai_addrlen = sizeof(struct sockaddr_in);
    }
    else
    {
        myaddr6->sin6_family = (uint16_t) tmp;
        obj = context->SendMessage0(hints, "sa_addr");
        inet_pton(AF_INET6, context->CString(obj), &myaddr6 ->sin6_addr.s6_addr);
        shints.ai_addrlen = sizeof(struct sockaddr_in6);
    }
    shints.ai_addr = (struct sockaddr *) &myaddr;
    //perform function
    retc = getaddrinfo(snodename, sservname, &shints, &rrea);
    context->SetObjectVariable("retc", context->Int32(retc));
    setErrno(context, retc == -1);

    if (retc != 0 || rrea == NULL)
    {
        return retc;
    }
    // create the output array
    startrrea = rrea;
    while (rrea != NULL)
    {
        RexxClassObject cobj = context->FindClass("AddrInfo");
        obj = context->SendMessage0(cobj, "new");
        context->SendMessage1(obj, "ai_flags=",
                              context->Int32ToObject(rrea->ai_flags));
        context->SendMessage1(obj, "ai_family=",
                              context->Int32ToObject(rrea->ai_family));
        context->SendMessage1(obj, "ai_socktype=",
                              context->Int32ToObject(rrea->ai_socktype));
        context->SendMessage1(obj, "ai_protocol=",
                              context->Int32ToObject(rrea->ai_protocol));
        if (rrea->ai_canonname != NULL)
        {
            context->SendMessage1(obj, "ai_canonname=",
                                  context->String(rrea->ai_canonname));
        }
        else
        {
            context->SendMessage1(obj, "ai_canonname=",
                                  context->String(""));
        }
        if (rrea->ai_addr != NULL)
        {
            context->SendMessage1(obj, "sa_family=",
                                  context->UnsignedInt32((uint32_t)rrea->ai_addr->sa_family));
            if (utmp == AF_INET)
            {
                rrea->ai_addr->sa_family = AF_INET;
                inet_ntop(AF_INET, rrea->ai_addr->sa_data, str, INET6_ADDRSTRLEN);
            }
            else
            {
                rrea->ai_addr->sa_family = AF_INET6;
                inet_ntop(AF_INET6, rrea->ai_addr->sa_data, str, INET6_ADDRSTRLEN);
            }
            context->SendMessage1(obj, "sa_addr=", context->String(str));
        }
        else
        {
            context->SendMessage1(obj, "sa_family=", context->UnsignedInt32(0));
            context->SendMessage1(obj, "sa_addr=", context->String(""));
        }
        context->ArrayAppend(rea, obj);
        rrea = rrea->ai_next;
    }
    freeaddrinfo(startrrea);
    return retc;
}


/*----------------------------------------------------------------------------*/
/* Method: gai_strerror                                                       */
/* Description: get the error text associated with an error code from         */
/*              getaddrinfo method.                                           */
/* Arguments:                                                                 */
/*         errcode - error code                                               */
/*----------------------------------------------------------------------------*/

RexxMethod1(RexxObjectPtr,             // Return type
            orxGetStrerror6,           // Object_method name
            int, err)                  // error code
{

    // perform function and return
    return context->String(gai_strerror(err));
}


/*----------------------------------------------------------------------------*/
/* Method: getHostName                                                        */
/* Description: get the host name of the local machine.                       */
/* Arguments:                                                                 */
/*----------------------------------------------------------------------------*/

RexxMethod0(RexxObjectPtr,             // Return type
            orxGetHostName6)           // Object_method name
{
    char host[HOST_NAME_MAX + 1];

    // perform function and return
    int retc = gethostname(host, sizeof(host));
    context->SetObjectVariable("retc", context->Int32(retc));

    setErrno(context, retc == -1);

    // RKM:  Suspect the host needs to be initialized to a null string.  This will
    // return garbage if there is an error.
    return context->String(host);
}


/*----------------------------------------------------------------------------*/
/* Method: getPeerName                                                        */
/* Description: get the peer name connected to a socket                       */
/* Arguments:                                                                 */
/*         inetaddr - Inetaddr instance                                       */
/*----------------------------------------------------------------------------*/

RexxMethod1(int,                       // Return type
            orxGetPeerName6,           // Object_method name
            RexxObjectPtr, inetaddr)   // Inetaddr instance
{
    ORXSOCKET socketfd;
    struct sockaddr_storage myaddr;
    struct sockaddr_in * myaddr4 = (struct sockaddr_in *)&myaddr;
    struct sockaddr_in6 * myaddr6 = (struct sockaddr_in6 *)&myaddr;
    char str[INET6_ADDRSTRLEN];
    socklen_t len = sizeof(myaddr);
    int retc;

    // get the socket file descriptor
    socketfd = getSocket(context);

    // perform function and return
    memset(&myaddr, 0, sizeof(struct sockaddr_storage));
    if (!context->IsOfType(inetaddr, "InetAddress"))
    {
        RexxArrayObject arrobj;
        const char *msg = "Argument 1 must be of type .InetAddress.";
        context->ArrayAppendString(arrobj, msg, strlen(msg));
        context->RaiseException(88900, arrobj);
        return -1;
    }
    retc = getpeername(socketfd, (struct sockaddr *)&myaddr, &len);
    context->SetObjectVariable("retc", context->Int32(retc));
    setErrno(context, retc == -1);

    if (retc == -1)
    {
        return retc;
    }

    if (myaddr.ss_family == AF_INET)
    {
        context->SendMessage1(inetaddr, "family=",
                              context->UnsignedInt64((uint64_t)myaddr4->sin_family));
        context->SendMessage1(inetaddr, "port=",
                              context->UnsignedInt64((uint64_t)myaddr4->sin_port));
        inet_ntop(AF_INET, myaddr4, str, INET6_ADDRSTRLEN);
    }
    else
    {
        context->SendMessage1(inetaddr, "family=",
                              context->UnsignedInt64((uint64_t)myaddr6->sin6_family));
        context->SendMessage1(inetaddr, "port=",
                              context->UnsignedInt64((uint64_t)myaddr6->sin6_port));
        inet_ntop(AF_INET6, myaddr4, str, INET6_ADDRSTRLEN);
    }
    context->SendMessage1(inetaddr, "address=", context->String(str));
    return retc;
}


/*----------------------------------------------------------------------------*/
/* Method: getProtoByName                                                     */
/* Description: get the protocol by its name.                                 */
/* Arguments:                                                                 */
/*         proto - protocol name                                              */
/*----------------------------------------------------------------------------*/

RexxMethod1(int,                       // Return type
            orxGetProtocolByName6,     // Object_method name
            RexxStringObject, proto)   // Protocol name
{
    CSTRING protoname = context->StringData(proto);
    struct protoent *ent;

    // perform function and return
    ent = getprotobyname(protoname);
    if (ent == NULL)
    {
        context->SetObjectVariable("retc", context->Int32(-1));
        context->SetObjectVariable("errno", context->Int32(22));
        return -1;
    }
    context->SetObjectVariable("retc", context->Int32(0));
    context->SetObjectVariable("errno", context->Int32(0));
    return ent->p_proto;
}


/*----------------------------------------------------------------------------*/
/* Method: getProtoByNumber                                                   */
/* Description: get the protocol by its name.                                 */
/* Arguments:                                                                 */
/*         proto - protocol number                                            */
/*----------------------------------------------------------------------------*/

RexxMethod1(RexxStringObject,          // Return type
            orxGetProtocolByNumber6,   // Object_method name
            int, proto)                // Protocol number
{
    struct protoent *ent;

    // perform function and return
    ent = getprotobynumber(proto);
    if (ent == NULL)
    {
        return context->String("-1");
    }
    context->SetObjectVariable("retc", context->Int32(0));
    context->SetObjectVariable("errno", context->Int32(0));
    return context->String(ent->p_name);
}


/*----------------------------------------------------------------------------*/
/* Method: getSockName                                                        */
/* Description: get the socket name of the socket.                            */
/* Arguments:                                                                 */
/*         inetaddr - Inetaddr instance                                       */
/*----------------------------------------------------------------------------*/

RexxMethod1(int,                       // Return type
            orxGetSockName6,           // Object_method name
            RexxObjectPtr, inetaddr)   // Inetaddr instance
{
    ORXSOCKET socketfd;
    struct sockaddr_storage myaddr;
    struct sockaddr_in * myaddr4 = (struct sockaddr_in *)&myaddr;
    struct sockaddr_in6 * myaddr6 = (struct sockaddr_in6 *)&myaddr;
    char str[INET6_ADDRSTRLEN];
    socklen_t len = sizeof(myaddr);
    int retc;

    // get the socket file descriptor
    socketfd = getSocket(context);

    // perform function and return
    memset(&myaddr, 0, sizeof(struct sockaddr_storage));
    if (!context->IsOfType(inetaddr, "InetAddress"))
    {
        RexxArrayObject arrobj;
        const char *msg = "Argument 1 must be of type .InetAddress.";
        context->ArrayAppendString(arrobj, msg, strlen(msg));
        context->RaiseException(88900, arrobj);
        return -1;
    }
    retc = getsockname(socketfd, (struct sockaddr *)&myaddr, &len);
    context->SetObjectVariable("retc", context->Int32(retc));
    setErrno(context, retc == -1);

    if (retc == -1)
    {
        return retc;
    }

    if (myaddr.ss_family == AF_INET)
    {
        context->SendMessage1(inetaddr, "family=",
                              context->UnsignedInt64((uint64_t)myaddr4->sin_family));
        context->SendMessage1(inetaddr, "port=",
                              context->UnsignedInt64((uint64_t)myaddr4->sin_port));
        inet_ntop(AF_INET, myaddr4, str, INET6_ADDRSTRLEN);
    }
    else
    {
        context->SendMessage1(inetaddr, "family=",
                              context->UnsignedInt64((uint64_t)myaddr6->sin6_family));
        context->SendMessage1(inetaddr, "port=",
                              context->UnsignedInt64((uint64_t)myaddr6->sin6_port));
        inet_ntop(AF_INET6, myaddr4, str, INET6_ADDRSTRLEN);
    }
    context->SendMessage1(inetaddr, "address=", context->String(str));
    return retc;
}


/*----------------------------------------------------------------------------*/
/* Method: getsockopt                                                         */
/* Description: get a socket option.                                          */
/* Arguments:                                                                 */
/*         option - socket option                                             */
/*----------------------------------------------------------------------------*/

RexxMethod1(RexxObjectPtr,             // Return type
            orxGetSockOpt6,            // Object_method name
            int, option)               // socket option
{
    ORXSOCKET socketfd;
    int sockval_int, retc;
    socklen_t len;

    // get the socket file descriptor
    socketfd = getSocket(context);

    // perform function and return
    switch (option)
    {
        case SO_DEBUG:
        case SO_REUSEADDR:
        case SO_TYPE:
        case SO_ERROR:
        case SO_DONTROUTE:
        case SO_BROADCAST:
        case SO_SNDBUF:
        case SO_RCVBUF:
#ifdef SO_SNDBUFFORCE
        case SO_SNDBUFFORCE:
#endif
#ifdef SO_RCVBUFFORCE
        case SO_RCVBUFFORCE:
#endif
        case SO_KEEPALIVE:
        case SO_OOBINLINE:
#ifdef SO_NO_CHECK
        case SO_NO_CHECK:
#endif
#ifdef SO_PRIORITY
        case SO_PRIORITY:
#endif
#ifdef SO_BSDCOMPAT
        case SO_BSDCOMPAT:
#endif
#ifdef SO_REUSEPORT
        case SO_REUSEPORT:
#endif
        case SO_RCVLOWAT:
        case SO_SNDLOWAT:
        case SO_RCVTIMEO:
        case SO_SNDTIMEO:
#ifdef SO_SECURITY_AUTHENTICATION
        case SO_SECURITY_AUTHENTICATION:
#endif
#ifdef SO_SECURITY_ENCRYPTION_TRANSPORT
        case SO_SECURITY_ENCRYPTION_TRANSPORT:
#endif
#ifdef SO_SECURITY_ENCRYPTION_NETWORK
        case SO_SECURITY_ENCRYPTION_NETWORK:
#endif
#ifdef SO_ATTACH_FILTER
        case SO_ATTACH_FILTER:
#endif
#ifdef SO_DETACH_FILTER
        case SO_DETACH_FILTER:
#endif
#ifdef SO_TIMESTAMP
        case SO_TIMESTAMP:
#endif
        case SO_ACCEPTCONN:
#ifdef SO_PEERSEC
        case SO_PEERSEC:
#endif
#ifdef SO_PASSSEC
        case SO_PASSSEC:
#endif
#ifdef SO_TIMESTAMPNS
        case SO_TIMESTAMPNS:
#endif
#ifdef SO_MARK
        case SO_MARK:
#endif
#ifdef SO_TIMESTAMPING
        case SO_TIMESTAMPING:
#endif
#ifdef SO_PROTOCOL
        case SO_PROTOCOL:
#endif
#ifdef SO_DOMAIN
        case SO_DOMAIN:
#endif
#ifdef SO_RXQ_OVFL
        case SO_RXQ_OVFL:
#endif
#ifdef SO_RXQ_WIFI_STATUS
        case SO_WIFI_STATUS:
#endif
#ifdef SO_PEEK_OFF
        case SO_PEEK_OFF:
#endif
#ifdef SO_NOFCS
        case SO_NOFCS:
#endif
#ifdef SO_LOCK_FILTER
        case SO_LOCK_FILTER:
#endif
#ifdef SO_SELECT_ERR_QUEUE
        case SO_SELECT_ERR_QUEUE:
#endif
#ifdef SO_BUSY_POLL
        case SO_BUSY_POLL:
#endif
#ifdef SO_PASSCRED
        case SO_PASSCRED:
#endif
        {
            // boolean/int options
            len = (int) sizeof(int);

// RKM:  I suspect this one is ok to leave too...you could probably hide the differences
// with a macro if needed.
            retc = getsockopt(socketfd, SOL_SOCKET, option, (SOCKOPTION *)&sockval_int, &len);
            context->SetObjectVariable("retc", context->Int32(retc));
            setErrno(context, retc == -1);

            // RKM: Fixed a bug here...should be "==" not "="
            if (retc == -1)
            {
                return context->Int64((int64_t)retc);
            }
            return context->Int64((int)sockval_int);
        }
        case SO_LINGER:
        {
            struct linger so_linger;
            len = (int) sizeof(so_linger);
            retc = getsockopt(socketfd, SOL_SOCKET, option, (SOCKOPTION *)&so_linger, &len);
            context->SetObjectVariable("retc", context->Int32(retc));
            setErrno(context, retc == -1);

            // RKM:  Another comparison bug
            if (retc == 0)
            {
                RexxClassObject l_class= context->FindClass("Linger");
                RexxObjectPtr l_obj = context->SendMessage0(l_class, "new");
                context->SendMessage1(l_obj, "l_onoff=", context->Int64((int64_t)so_linger.l_onoff));
                context->SendMessage1(l_obj, "l_linger=", context->Int64((int64_t)so_linger.l_linger));
            }
            return context->Int64((int64_t)retc);
        }
#ifdef SO_BINDTODEVICE
        case SO_BINDTODEVICE:
        {
            char sockval_str[512];
            len = sizeof(sockval_str);
            retc = getsockopt(socketfd, SOL_SOCKET, option, &sockval_str, &len);
            context->SetObjectVariable("retc", context->Int32(retc));
            setErrno(context, retc == -1);

            return context->String(sockval_str, len);
        }
#endif
#ifdef SO_PEERNAME
        case SO_PEERNAME:    // there is a better way to do this
#endif
#ifdef SO_PEERCRED
        case SO_PEERCRED:    // we do not support credentials
#endif
        default:
            context->SetObjectVariable("retc", context->Int32(-1));
            context->SetObjectVariable("errno", context->Int32(22));
            return context->Int64(-1);
    }
}


/*----------------------------------------------------------------------------*/
/* Method: listen                                                             */
/* Description: listen on a socket.                                           */
/* Arguments:                                                                 */
/*         backlog - number of possible waiting clients.                      */
/*----------------------------------------------------------------------------*/

// RKM:  Just use uintptr_t always
RexxMethod1(uintptr_t,                 // Return type
            orxListen6,                // Object_method name
            int, backlog)              // backlog
{
    ORXSOCKET socketfd, retc;

    // get the socket file descriptor
    socketfd = getSocket(context);

    // perform function and return
    retc = listen(socketfd, backlog);
    context->SetObjectVariable("retc", socketToObject(context, retc));

    setErrno(context, retc == INVALID_SOCKET);
    return retc;
}


/*----------------------------------------------------------------------------*/
/* Method: recv                                                               */
/* Description: read a block of bytes from a socket.                          */
/* Arguments:                                                                 */
/*         len - length of bytes to read                                      */
/*----------------------------------------------------------------------------*/

RexxMethod1(RexxObjectPtr,             // Return type
            orxRecv6,                  // Object_method name
            size_t, len)               // number of bytes to be read
{
    ORXSOCKET socketfd;

    // RKM:  Can probably fixed just by defining alloca() to _malloca() on Windows.
    // However, I don't think this is a good idea.  If reading a lot of data from the
    // socket, there's a good chance this will fail, particularly if not running on the
    // main thread.  Also, I think this is completely wrong, since this will only allocate
    // Consider using a BufferString for this.

    RexxBufferStringObject buffer = context->NewBufferString(len);
    char *cblock = (char *)context->BufferStringData(buffer);
    ssize_t lenread;

    // get the socket file descriptor
    socketfd = getSocket(context);

    // perform function and return
    // RKM:  The cast to (int) could be a problem in 64-bit.  Is that correct?
    lenread = recv(socketfd, cblock, (int)len, 0);
    // RKM:  This could potentially truncate in 64-bit, so changed the conversion function
    context->SetObjectVariable("retc", context->Int64((int64_t)lenread));
    setErrno(context, lenread == -1);

    // RKM:  Not sure this is an appropriate return value for errors...null string
    // or maybe .nil might be a better return.  The data read might actually be
    // the string "-1"
    if (lenread == -1)
    {
        return context->Nil();
    }

    // set to the actual read length and return
    context->FinishBufferString(buffer, lenread);
    return buffer;
}


/*----------------------------------------------------------------------------*/
/* Method: recvFrom                                                           */
/* Description: recieve data on a socket from a specified address             */
/* Arguments:                                                                 */
/*         len         - the maximum amount of data to recieve in bytes       */
/*         inetaddress - initialized InetAddress instance                     */
/*----------------------------------------------------------------------------*/

RexxMethod2(RexxObjectPtr,             // Return type
            orxRecvFrom6,              // Object_method name
            int, len,                  // number of bytes to be read
            RexxObjectPtr, inetaddr)   // Inetaddr instance
{
    RexxObjectPtr obj;
    ORXSOCKET socketfd;
    RexxBufferStringObject buffer = context->NewBufferString(len);
    char *cblock = (char *)context->BufferStringData(buffer);
    ssize_t lenread;

    struct sockaddr_storage myaddr;
    struct sockaddr_in * myaddr4 = (struct sockaddr_in *)&myaddr;
    struct sockaddr_in6 * myaddr6 = (struct sockaddr_in6 *)&myaddr;
    char str[INET6_ADDRSTRLEN];
    socklen_t slen = sizeof(myaddr);
    uint32_t tmp;

    // get the socket file descriptor
    socketfd = getSocket(context);

    // perform function and return
    memset(&myaddr, 0, sizeof(struct sockaddr_storage));
    if (!context->IsOfType(inetaddr, "InetAddress"))
    {
        RexxArrayObject arrobj;
        const char *msg = "Argument 2 must be of type .InetAddress.";
        context->ArrayAppendString(arrobj, msg, strlen(msg));
        context->RaiseException(88900, arrobj);
        return (RexxObjectPtr) context->NullString();
    }
    obj = context->SendMessage0(inetaddr, "family");
    context->UnsignedInt32(obj, &tmp);
    if (tmp == AF_INET)
    {
        myaddr4->sin_family = (uint16_t) tmp;
        obj = context->SendMessage0(inetaddr, "port");
        context->UnsignedInt32(obj, &tmp);
        myaddr4->sin_port = (uint16_t) tmp;
        obj = context->SendMessage0(inetaddr, "address");
        inet_pton(myaddr4->sin_family, context->CString(obj), str);
    }
    else
    {
        myaddr6->sin6_family = (uint16_t) tmp;
        obj = context->SendMessage0(inetaddr, "port");
        context->UnsignedInt32(obj, &tmp);
        myaddr6->sin6_port = (uint16_t) tmp;
        obj = context->SendMessage0(inetaddr, "address");
        inet_pton(myaddr6->sin6_family, context->CString(obj), str);
    }

    lenread = recvfrom(socketfd, cblock, len, 0, (struct sockaddr *)&myaddr, &slen);
    context->SetObjectVariable("retc", context->Int64((int64_t)lenread));
    setErrno(context, lenread == -1);

    if (lenread == -1)
    {
        return context->Nil();
    }

    // set to the actual read length and return
    context->FinishBufferString(buffer, lenread);
    return buffer;
}

#if !defined(WIN32)
/*----------------------------------------------------------------------------*/
/* Method: select                                                             */
/* Description: find out if file operations are available.                    */
/* Arguments:                                                                 */
/*         maxfd    - max file descriptor + 1                                 */
/*         readfds  - an array of read file descriptors to check              */
/*         writefds - an array of read file descriptors to check              */
/*         excptfds - an array of exception file descriptors to check         */
/*         timeout  - timeout in milliseconds                                 */
/*----------------------------------------------------------------------------*/

RexxMethod4(int,                       // Return type
            orxSelect6,                // Object_method name
            RexxArrayObject, readfds,  // array of read file descriptors
            RexxArrayObject, writefds, // array of write file descriptors
            RexxArrayObject, excpfds,  // array of read file descriptors
            int, timeout)              // timeout in milliseconds
{
    RexxObjectPtr rxsockfd1, rxsockfd2;
    int i, retc;
    size_t items;
    int socketfd, maxsocketfd = 0;
    fd_set *pread_set = NULL, *pwrite_set = NULL, *pexcp_set = NULL;
    struct timeval sel_timeout;

    // get the read set
    if (readfds != context->Nil())
    {
        pread_set = (fd_set *)alloca(sizeof(fd_set));
        FD_ZERO(pread_set);
        items = context->ArrayItems(readfds);
        for (i = 1; i <= items; i++)
        {
            rxsockfd1 = context->ArrayAt(readfds, i);
            rxsockfd2 = context->SendMessage0(rxsockfd1, "socketfd");
            context->Int32(rxsockfd2, &socketfd);
            if (socketfd > maxsocketfd)
            {
                maxsocketfd = socketfd;
            }
            FD_SET(socketfd, pread_set);
        }
    }
    // get the write set
    if (writefds != context->Nil())
    {
        pwrite_set = (fd_set *)alloca(sizeof(fd_set));
        FD_ZERO(pwrite_set);
        items = context->ArrayItems(writefds);
        for (i = 1; i <= items; i++)
        {
            rxsockfd1 = context->ArrayAt(writefds, i);
            rxsockfd2 = context->SendMessage0(rxsockfd1, "socketfd");
            context->Int32(rxsockfd2, &socketfd);
            if (socketfd > maxsocketfd)
            {
                maxsocketfd = socketfd;
            }
            FD_SET(socketfd, pwrite_set);
        }
    }
    // get the exception set
    if (excpfds != context->Nil())
    {
        pexcp_set = (fd_set *)alloca(sizeof(fd_set));
        FD_ZERO(pexcp_set);
        items = context->ArrayItems(excpfds);
        for (i = 1; i <= items; i++)
        {
            rxsockfd1 = context->ArrayAt(excpfds, i);
            rxsockfd2 = context->SendMessage0(rxsockfd1, "socketfd");
            context->Int32(rxsockfd2, &socketfd);
            if (socketfd > maxsocketfd)
            {
                maxsocketfd = socketfd;
            }
            FD_SET(socketfd, pexcp_set);
        }
    }
    // get the timeout
    sel_timeout.tv_sec = timeout / 1000;
    sel_timeout.tv_usec = timeout % 1000;
    // perform the select() operation
    retc = select(maxsocketfd + 1, pread_set, pwrite_set, pexcp_set, &sel_timeout);
    context->SetObjectVariable("retc", context->Int32(retc));
    if (retc == -1)
    {
        context->SetObjectVariable("errno", context->Int32(sock_errno()));
    }
    else
    {
        context->SetObjectVariable("errno", context->Int32(0));
    }
    // see if at least one file descriptor changed
    if (retc > 0)
    {
        // set the read array
        if (readfds != context->Nil())
        {
            items = context->ArrayItems(readfds);
            for (i = 1; i <= items; i++)
            {
                rxsockfd1 = context->ArrayAt(readfds, i);
                rxsockfd2 = context->SendMessage0(rxsockfd1, "socketfd");
                if (!FD_ISSET(socketfd, pread_set))
                {
                    context->ArrayPut(readfds, context->Nil(), i);
                }
            }
        }
        // set the write array
        if (writefds != context->Nil())
        {
            items = context->ArrayItems(writefds);
            for (i = 1; i <= items; i++)
            {
                rxsockfd1 = context->ArrayAt(writefds, i);
                rxsockfd2 = context->SendMessage0(rxsockfd1, "socketfd");
                if (!FD_ISSET(socketfd, pwrite_set))
                {
                    context->ArrayPut(writefds, context->Nil(), i);
                }
            }
        }
        // set the exception array
        if (excpfds != context->Nil())
        {
            items = context->ArrayItems(excpfds);
            for (i = 1; i <= items; i++)
            {
                rxsockfd1 = context->ArrayAt(excpfds, i);
                rxsockfd2 = context->SendMessage0(rxsockfd1, "socketfd");
                if (!FD_ISSET(socketfd, pexcp_set))
                {
                    context->ArrayPut(excpfds, context->Nil(), i);
                }
            }
        }
    }
    // return
    return retc;
}
#endif


/*----------------------------------------------------------------------------*/
/* Method: send                                                               */
/* Description: write a block of text to the socket.                          */
/* Arguments:                                                                 */
/*         text - the bytes to be written to the socket                       */
/*----------------------------------------------------------------------------*/

RexxMethod1(int,                       // Return type
            orxSend6,                  // Object_method name
            RexxStringObject, block)   // bytes to be written
{
    ORXSOCKET socketfd;
    CSTRING cblock;

    // get the socket file descriptor
    socketfd = getSocket(context);

    // perform function and return
    cblock = context->CString(block);
    int retc = send(socketfd, cblock, (int)context->StringLength(block), 0);
    context->SetObjectVariable("retc", context->Int32(retc));
    setErrno(context, retc == -1);

    return retc;
}


/*----------------------------------------------------------------------------*/
/* Method: sendTo                                                             */
/* Description: send data on a socket to specified address                    */
/* Arguments:                                                                 */
/*         data        - the data to be sent                                  */
/*         inetaddress - initialized InetAddress instance                     */
/*----------------------------------------------------------------------------*/

RexxMethod2(int,                       // Return type
            orxSendTo6,                // Object_method name
            RexxObjectPtr, data,       // data to be sent
            RexxObjectPtr, inetaddr)   // Inetaddr instance
{
    RexxObjectPtr obj;
    ORXSOCKET socketfd;
    ssize_t lenwritten;
    struct sockaddr_storage myaddr;
    struct sockaddr_in * myaddr4 = (struct sockaddr_in *)&myaddr;
    struct sockaddr_in6 * myaddr6 = (struct sockaddr_in6 *)&myaddr;
    char str[INET6_ADDRSTRLEN];
    socklen_t slen = sizeof(myaddr);
    uint32_t tmp;
    CSTRING strdata;
    int len;

    // get the socket file descriptor
    socketfd = getSocket(context);

    // perform function and return
    if (!context->IsOfType(inetaddr, "InetAddress"))
    {
        RexxArrayObject arrobj;
        const char *msg = "Argument 2 must be of type .InetAddress.";
        context->ArrayAppendString(arrobj, msg, strlen(msg));
        context->RaiseException(88900, arrobj);
        return -1;
    }
    obj = context->SendMessage0(inetaddr, "family");
    context->UnsignedInt32(obj, &tmp);
    if (tmp == AF_INET)
    {
        myaddr4->sin_family = (uint16_t) tmp;
        obj = context->SendMessage0(inetaddr, "port");
        context->UnsignedInt32(obj, &tmp);
        myaddr4->sin_port = (uint16_t) tmp;
        obj = context->SendMessage0(inetaddr, "address");
        inet_pton(myaddr4->sin_family, context->CString(obj), str);
    }
    else
    {
        myaddr6->sin6_family = (uint16_t) tmp;
        obj = context->SendMessage0(inetaddr, "port");
        context->UnsignedInt32(obj, &tmp);
        myaddr6->sin6_port = (uint16_t) tmp;
        obj = context->SendMessage0(inetaddr, "address");
        inet_pton(myaddr6->sin6_family, context->CString(obj), str);
    }
    strdata = context->CString(data);
    len = (int) context->StringLength((RexxStringObject)data);
    lenwritten = sendto(socketfd, str, len, 0, (struct sockaddr *)&myaddr, slen);
    context->SetObjectVariable("retc", context->Int32((int32_t)lenwritten));

    setErrno(context, lenwritten == -1);
    return (int)lenwritten;
}


/*----------------------------------------------------------------------------*/
/* Method: setsockopt                                                         */
/* Description: set a socket option.                                          */
/* Arguments:                                                                 */
/*         option - socket option                                             */
/*         val   - value for the option                                       */
/*----------------------------------------------------------------------------*/

RexxMethod2(int,                       // Return type
            orxSetSockOpt6,            // Object_method name
            int, option,               // socket option
            RexxObjectPtr, val)        // socket option value
{
    ORXSOCKET socketfd;
    int sockval_int;
    socklen_t len;
    int retc;

    // get the socket file descriptor
    socketfd = getSocket(context);

    // perform function and return
    switch (option)
    {
        case SO_DEBUG:
        case SO_REUSEADDR:
        case SO_TYPE:
        case SO_ERROR:
        case SO_DONTROUTE:
        case SO_BROADCAST:
        case SO_SNDBUF:
        case SO_RCVBUF:
#ifdef SO_SNDBUFFORCE
        case SO_SNDBUFFORCE:
#endif
#ifdef SO_RCVBUFFORCE
        case SO_RCVBUFFORCE:
#endif
        case SO_KEEPALIVE:
        case SO_OOBINLINE:
#ifdef SO_NO_CHECK
        case SO_NO_CHECK:
#endif
#ifdef SO_PRIORITY
        case SO_PRIORITY:
#endif
#ifdef SO_BSDCOMPAT
        case SO_BSDCOMPAT:
#endif
#ifdef SO_REUSEPORT
        case SO_REUSEPORT:
#endif
        case SO_RCVLOWAT:
        case SO_SNDLOWAT:
        case SO_RCVTIMEO:
        case SO_SNDTIMEO:
#ifdef SO_SECURITY_AUTHENTICATION
        case SO_SECURITY_AUTHENTICATION:
#endif
#ifdef SO_SECURITY_ENCRYPTION_TRANSPORT
        case SO_SECURITY_ENCRYPTION_TRANSPORT:
#endif
#ifdef SO_SECURITY_ENCRYPTION_NETWORK
        case SO_SECURITY_ENCRYPTION_NETWORK:
#endif
#ifdef SO_ATTACH_FILTER
        case SO_ATTACH_FILTER:
#endif
#ifdef SO_DETACH_FILTER
        case SO_DETACH_FILTER:
#endif
#ifdef SO_TIMESTAMP
        case SO_TIMESTAMP:
#endif
        case SO_ACCEPTCONN:
#ifdef SO_PEERSEC
        case SO_PEERSEC:
#endif
#ifdef SO_PASSSEC
        case SO_PASSSEC:
#endif
#ifdef SO_TIMESTAMPNS
        case SO_TIMESTAMPNS:
#endif
#ifdef SO_MARK
        case SO_MARK:
#endif
#ifdef SO_TIMESTAMPING
        case SO_TIMESTAMPING:
#endif
#ifdef SO_PROTOCOL
        case SO_PROTOCOL:
#endif
#ifdef SO_DOMAIN
        case SO_DOMAIN:
#endif
#ifdef SO_RXQ_OVFL
        case SO_RXQ_OVFL:
#endif
#ifdef SO_RXQ_WIFI_STATUS
        case SO_WIFI_STATUS:
#endif
#ifdef SO_PEEK_OFF
        case SO_PEEK_OFF:
#endif
#ifdef SO_NOFCS
        case SO_NOFCS:
#endif
#ifdef SO_LOCK_FILTER
        case SO_LOCK_FILTER:
#endif
#ifdef SO_SELECT_ERR_QUEUE
        case SO_SELECT_ERR_QUEUE:
#endif
#ifdef SO_BUSY_POLL
        case SO_BUSY_POLL:
#endif
#ifdef SO_PASSCRED
        case SO_PASSCRED:
#endif
        {
            // boolean/int options
            context->Int32(val, &sockval_int);
            len = sizeof(int);
            retc = setsockopt(socketfd, SOL_SOCKET, option, (SOCKOPTION *)&sockval_int, len);
            context->SetObjectVariable("retc", context->Int32(retc));

            setErrno(context, retc == -1);
            return retc;
        }
        case SO_LINGER:
        {
            struct linger so_linger;
            RexxObjectPtr obj = context->SendMessage0(val, "l_onoff");

            // RKM:  Using a temp variable eliminates the conditionals here.
            int32_t temp;
            context->Int32(obj, &temp);
            so_linger.l_onoff = temp;

            // RKM:  Something doesn't look right here...both values are setting
            // l_onoff value.
            obj = context->SendMessage0(val, "l_linger");
            context->Int32(obj, &temp);
            so_linger.l_onoff = temp;
            len = sizeof(so_linger);
            retc = setsockopt(socketfd, SOL_SOCKET, option, (SOCKOPTION *)&so_linger, len);
            context->SetObjectVariable("retc", context->Int32(retc));

            setErrno(context, retc == -1);
            return retc;
        }
#ifdef SO_PASSCRED
        case SO_BINDTODEVICE:
        {
            CSTRING strval = context->CString(val);
            len = strlen(strval);
            retc = setsockopt(socketfd, SOL_SOCKET, option, strval, len);
            context->SetObjectVariable("retc", context->Int32(retc));
            setErrno(context, retc == -1);
            return retc;
        }
#endif
#ifdef SO_PEERNAME
        case SO_PEERNAME:    // there is a better way to do this
#endif
#ifdef SO_PEERCRED
        case SO_PEERCRED:    // we do not support credentials
#endif
        default:
            context->SetObjectVariable("retc", context->Int32(-1));
            context->SetObjectVariable("errno", context->Int32(22));
            return -1;
    }
}


// now build the actual entry list
RexxMethodEntry rxsock6_methods[] =
{
    REXX_METHOD(orxSocket6, orxSocket6),
    REXX_METHOD(orxAccept6, orxAccept6),
    REXX_METHOD(orxBind6, orxBind6),
    REXX_METHOD(orxClose6, orxClose6),
    REXX_METHOD(orxConnect6, orxConnect6),
    REXX_METHOD(orxGetAddrinfo6, orxGetAddrinfo6),
    REXX_METHOD(orxGetHostName6, orxGetHostName6),
    REXX_METHOD(orxGetPeerName6, orxGetPeerName6),
    REXX_METHOD(orxGetProtocolByName6, orxGetProtocolByName6),
    REXX_METHOD(orxGetProtocolByNumber6, orxGetProtocolByNumber6),
    REXX_METHOD(orxGetSockName6, orxGetSockName6),
    REXX_METHOD(orxGetSockOpt6, orxGetSockOpt6),
    REXX_METHOD(orxGetStrerror6, orxGetStrerror6),
    REXX_METHOD(orxListen6, orxListen6),
    REXX_METHOD(orxRecv6, orxRecv6),
    REXX_METHOD(orxRecvFrom6, orxRecvFrom6),
#if !defined(WIN32)
    REXX_METHOD(orxSelect6, orxSelect6),
#endif
    REXX_METHOD(orxSend6, orxSend6),
    REXX_METHOD(orxSendTo6, orxSendTo6),
    REXX_METHOD(orxSetSockOpt6, orxSetSockOpt6),
    REXX_LAST_METHOD()
};


RexxPackageEntry rxsock6_package_entry =
{
    STANDARD_PACKAGE_HEADER
    REXX_INTERPRETER_4_0_0,              // anything after 4.0.0 will work
    "RXSOCK6",                           // name of the package
    "4.0",                               // package information
    NULL,                                // no load/unload functions
    NULL,
    NULL,                                // no functions in this package
    rxsock6_methods                      // the exported methods
};

// package loading stub.
OOREXX_GET_PACKAGE(rxsock6);


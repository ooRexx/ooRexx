/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2014-2018 Rexx Language Association. All rights reserved.    */
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
    #ifdef __sun
        #include <alloca.h>
    #endif    #include <sys/socket.h>
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
   #define SOCKOPTION char
#endif

#if !defined(HOST_NAME_MAX)
   #define HOST_NAME_MAX 255
#endif

/*------------------------------------------------------------------
 * rexx includes
 *------------------------------------------------------------------*/
# include "oorexxapi.h"


/**
 * Simple caseless comparison operator.
 *
 * @param op1    The first string
 * @param op2    The second string
 *
 * @return 0 if they are equal, non-zero if unequal.
 */
int caselessCompare(const char *op1, const char *op2)
{
    for (; tolower(*op1) == tolower(*op2); op1++,op2++)
    {
        if (*op1 == 0)
        {
            return 0;
        }
    }

    return(tolower(*op1) - tolower(*op2));
}

#define CheckOpt(n) if (!caselessCompare(#n, pszOptName)) return n;

/**
 * Convert a string option name into the numeric equivalent.
 *
 * @param pszOptName The string option name.
 *
 * @return The numeric equivalent, or -1 for unknown options.
 */
int stringToSockOpt(const char * pszOptName)
{
    CheckOpt(SO_DEBUG)
    CheckOpt(SO_REUSEADDR)
    CheckOpt(SO_KEEPALIVE)
    CheckOpt(SO_DONTROUTE)
    CheckOpt(SO_BROADCAST)
    CheckOpt(SO_USELOOPBACK)
    CheckOpt(SO_LINGER)
    CheckOpt(SO_OOBINLINE)
    CheckOpt(SO_SNDBUF)
    CheckOpt(SO_RCVBUF)
    CheckOpt(SO_SNDLOWAT)
    CheckOpt(SO_RCVLOWAT)
    CheckOpt(SO_SNDTIMEO)
    CheckOpt(SO_RCVTIMEO)
    CheckOpt(SO_ERROR)
    CheckOpt(SO_TYPE)
#ifdef SO_SNDBUFFORCE
    CheckOpt(SO_SNDBUFFORCE)
#endif
#ifdef SO_RCVBUFFORCE
    CheckOpt(SO_RCVBUFFORCE)
#endif
#ifdef SO_NO_CHECK
    CheckOpt(SO_NO_CHECK)
#endif
#ifdef SO_PRIORITY
    CheckOpt(SO_PRIORITY)
#endif
#ifdef SO_BSDCOMPAT
    CheckOpt(SO_BSDCOMPAT)
#endif
#ifdef SO_REUSEPORT
    CheckOpt(SO_REUSEPORT)
#endif
#ifdef SO_SECURITY_AUTHENTICATION
    CheckOpt(SO_SECURITY_AUTHENTICATION)
#endif
#ifdef SO_SECURITY_ENCRYPTION_TRANSPORT
    CheckOpt(SO_SECURITY_ENCRYPTION_TRANSPORT)
#endif
#ifdef SO_SECURITY_ENCRYPTION_NETWORK
    CheckOpt(SO_SECURITY_ENCRYPTION_NETWORK)
#endif
#ifdef SO_ATTACH_FILTER
    CheckOpt(SO_ATTACH_FILTER)
#endif
#ifdef SO_DETACH_FILTER
    CheckOpt(SO_DETACH_FILTER)
#endif
#ifdef SO_TIMESTAMP
    CheckOpt(SO_TIMESTAMP)
#endif
#ifdef SO_PEERSEC
    CheckOpt(SO_PEERSEC)
#endif
#ifdef SO_PASSSEC
    CheckOpt(SO_PASSSEC)
#endif
#ifdef SO_TIMESTAMPNS
    CheckOpt(SO_TIMESTAMPNS)
#endif
#ifdef SO_MARK
    CheckOpt(SO_MARK)
#endif
#ifdef SO_TIMESTAMPING
    CheckOpt(SO_TIMESTAMPING)
#endif
#ifdef SO_PROTOCOL
    CheckOpt(SO_PROTOCOL)
#endif
#ifdef SO_DOMAIN
    CheckOpt(SO_DOMAIN)
#endif
#ifdef SO_RXQ_OVFL
    CheckOpt(SO_RXQ_OVFL)
#endif
#ifdef SO_RXQ_WIFI_STATUS
    CheckOpt(SO_RXQ_WIFI_STATUS)
#endif
#ifdef SO_PEEK_OFF
    CheckOpt(SO_PEEK_OFF)
#endif
#ifdef SO_NOFCS
    CheckOpt(SO_NOFCS)
#endif
#ifdef SO_LOCK_FILTER
    CheckOpt(SO_LOCK_FILTER)
#endif
#ifdef SO_SELECT_ERR_QUEUE
    CheckOpt(SO_SELECT_ERR_QUEUE)
#endif
#ifdef SO_BUSY_POLL
    CheckOpt(SO_BUSY_POLL)
#endif
#ifdef SO_PASSCRED
    CheckOpt(SO_PASSCRED)
#endif

    return -1;
}


/**
 * Convert a string socket family name to its numeric equivalent
 *
 * @return The mapped numeric value or -1 for an unknown name.
 */
int stringToFamily(const char *pszOptName)
{
#ifdef PF_UNSPEC
    CheckOpt(PF_UNSPEC)
#endif
#ifdef PF_LOCAL
    CheckOpt(PF_LOCAL)
#endif
#ifdef PF_UNIX
    CheckOpt(PF_UNIX)
#endif
#ifdef PF_FILE
    CheckOpt(PF_FILE)
#endif
#ifdef PF_INET
    CheckOpt(PF_INET)
#endif
#ifdef PF_IMPLINK
    CheckOpt(PF_IMPLINK)
#endif
#ifdef PF_AX25
    CheckOpt(PF_AX25)
#endif
#ifdef PF_PUP
    CheckOpt(PF_PUP)
#endif
#ifdef PF_CHAOS
    CheckOpt(PF_CHAOS)
#endif
#ifdef PF_APPLETALK
    CheckOpt(PF_APPLETALK)
#endif
#ifdef PF_NETROM
    CheckOpt(PF_NETROM)
#endif
#ifdef PF_IPX
    CheckOpt(PF_IPX)
#endif
#ifdef PF_NS
    CheckOpt(PF_NS)
#endif
#ifdef PF_ISO
    CheckOpt(PF_ISO)
#endif
#ifdef PF_OSI
    CheckOpt(PF_OSI)
#endif
#ifdef PF_BRIDGE
    CheckOpt(PF_BRIDGE)
#endif
#ifdef PF_ECMA
    CheckOpt(PF_ECMA)
#endif
#ifdef PF_ATMPVC
    CheckOpt(PF_ATMPVC)
#endif
#ifdef PF_DATAKIT
    CheckOpt(PF_DATAKIT)
#endif
#ifdef PF_X25
    CheckOpt(PF_X25)
#endif
#ifdef PF_CCITT
    CheckOpt(PF_CCITT)
#endif
#ifdef PF_INET6
    CheckOpt(PF_INET6)
#endif
#ifdef PF_SNA
    CheckOpt(PF_SNA)
#endif
#ifdef PF_ROSE
    CheckOpt(PF_ROSE)
#endif
#ifdef PF_DECnet
    CheckOpt(PF_DECnet)
#endif
#ifdef PF_DLI
    CheckOpt(PF_DLI)
#endif
#ifdef PF_NETBEUI
    CheckOpt(PF_NETBEUI)
#endif
#ifdef PF_SECURITY
    CheckOpt(PF_SECURITY)
#endif
#ifdef PF_KEY
    CheckOpt(PF_KEY)
#endif
#ifdef PF_NETLINK
    CheckOpt(PF_NETLINK)
#endif
#ifdef PF_ROUTE
    CheckOpt(PF_ROUTE)
#endif
#ifdef PF_PACKET
    CheckOpt(PF_PACKET)
#endif
#ifdef PF_ASH
    CheckOpt(PF_ASH)
#endif
#ifdef PF_ECONET
    CheckOpt(PF_ECONET)
#endif
#ifdef PF_ATMSVC
    CheckOpt(PF_ATMSVC)
#endif
#ifdef PF_RDS
    CheckOpt(PF_RDS)
#endif
#ifdef PF_SNA
    CheckOpt(PF_SNA)
#endif
#ifdef PF_IRDA
    CheckOpt(PF_IRDA)
#endif
#ifdef PF_PPPOX
    CheckOpt(PF_PPPOX)
#endif
#ifdef PF_WANPIPE
    CheckOpt(PF_WANPIPE)
#endif
#ifdef PF_LLC
    CheckOpt(PF_LLC)
#endif
#ifdef PF_CAN
    CheckOpt(PF_CAN)
#endif
#ifdef PF_TIPC
    CheckOpt(PF_TIPC)
#endif
#ifdef PF_BLUETOOTH
    CheckOpt(PF_BLUETOOTH)
#endif
#ifdef PF_IUCV
    CheckOpt(PF_IUCV)
#endif
#ifdef PF_RXRPC
    CheckOpt(PF_RXRPC)
#endif
#ifdef PF_ISDN
    CheckOpt(PF_ISDN)
#endif
#ifdef PF_PHONET
    CheckOpt(PF_PHONET)
#endif
#ifdef PF_IEEE802154
    CheckOpt(PF_IEEE802154)
#endif
#ifdef PF_CAIF
    CheckOpt(PF_CAIF)
#endif
#ifdef PF_ALG
    CheckOpt(PF_ALG)
#endif
#ifdef PF_NFC
    CheckOpt(PF_NFC)
#endif
#ifdef PF_VSOCK
    CheckOpt(PF_VSOCK)
#endif
#ifdef PF_KCM
    CheckOpt(PF_KCM)
#endif
#ifdef PF_QIPCRTR
    CheckOpt(PF_QIPCRTR)
#endif
#ifdef PF_SMP
    CheckOpt(PF_SMP)
#endif
    return -1;
}


/**
 * Convert a string socket family name to its numeric equivalent
 *
 * @return The mapped numeric value or -1 for an unknown name.
 */
int stringToAddrType(const char *pszOptName)
{
#ifdef AF_UNSPEC
    CheckOpt(AF_UNSPEC)
#endif
#ifdef AF_LOCAL
    CheckOpt(AF_LOCAL)
#endif
#ifdef AF_UNIX
    CheckOpt(AF_UNIX)
#endif
#ifdef AF_FILE
    CheckOpt(AF_FILE)
#endif
#ifdef AF_INET
    CheckOpt(AF_INET)
#endif
#ifdef AF_IMPLINK
    CheckOpt(AF_IMPLINK)
#endif
#ifdef AF_AX25
    CheckOpt(AF_AX25)
#endif
#ifdef AF_PUP
    CheckOpt(AF_PUP)
#endif
#ifdef AF_CHAOS
    CheckOpt(AF_CHAOS)
#endif
#ifdef AF_APPLETALK
    CheckOpt(AF_APPLETALK)
#endif
#ifdef AF_NETROM
    CheckOpt(AF_NETROM)
#endif
#ifdef AF_IPX
    CheckOpt(AF_IPX)
#endif
#ifdef AF_NS
    CheckOpt(AF_NS)
#endif
#ifdef AF_ISO
    CheckOpt(AF_ISO)
#endif
#ifdef AF_OSI
    CheckOpt(AF_OSI)
#endif
#ifdef AF_BRIDGE
    CheckOpt(AF_BRIDGE)
#endif
#ifdef AF_ECMA
    CheckOpt(AF_ECMA)
#endif
#ifdef AF_ATMPVC
    CheckOpt(AF_ATMPVC)
#endif
#ifdef AF_DATAKIT
    CheckOpt(AF_DATAKIT)
#endif
#ifdef AF_X25
    CheckOpt(AF_X25)
#endif
#ifdef AF_CCITT
    CheckOpt(AF_CCITT)
#endif
#ifdef AF_INET6
    CheckOpt(AF_INET6)
#endif
#ifdef AF_SNA
    CheckOpt(AF_SNA)
#endif
#ifdef AF_ROSE
    CheckOpt(AF_ROSE)
#endif
#ifdef AF_DECnet
    CheckOpt(AF_DECnet)
#endif
#ifdef AF_DLI
    CheckOpt(AF_DLI)
#endif
#ifdef AF_NETBEUI
    CheckOpt(AF_NETBEUI)
#endif
#ifdef AF_SECURITY
    CheckOpt(AF_SECURITY)
#endif
#ifdef AF_KEY
    CheckOpt(AF_KEY)
#endif
#ifdef AF_NETLINK
    CheckOpt(AF_NETLINK)
#endif
#ifdef AF_ROUTE
    CheckOpt(AF_ROUTE)
#endif
#ifdef AF_PACKET
    CheckOpt(AF_PACKET)
#endif
#ifdef AF_ASH
    CheckOpt(AF_ASH)
#endif
#ifdef AF_ECONET
    CheckOpt(AF_ECONET)
#endif
#ifdef AF_ATMSVC
    CheckOpt(AF_ATMSVC)
#endif
#ifdef AF_RDS
    CheckOpt(AF_RDS)
#endif
#ifdef AF_SNA
    CheckOpt(AF_SNA)
#endif
#ifdef AF_IRDA
    CheckOpt(AF_IRDA)
#endif
#ifdef AF_PPPOX
    CheckOpt(AF_PPPOX)
#endif
#ifdef AF_WANPIPE
    CheckOpt(AF_WANPIPE)
#endif
#ifdef AF_LLC
    CheckOpt(AF_LLC)
#endif
#ifdef AF_CAN
    CheckOpt(AF_CAN)
#endif
#ifdef AF_TIPC
    CheckOpt(AF_TIPC)
#endif
#ifdef AF_BLUETOOTH
    CheckOpt(AF_BLUETOOTH)
#endif
#ifdef AF_IUCV
    CheckOpt(AF_IUCV)
#endif
#ifdef AF_RXRPC
    CheckOpt(AF_RXRPC)
#endif
#ifdef AF_ISDN
    CheckOpt(AF_ISDN)
#endif
#ifdef AF_PHONET
    CheckOpt(AF_PHONET)
#endif
#ifdef AF_IEEE802154
    CheckOpt(AF_IEEE802154)
#endif
#ifdef AF_CAIF
    CheckOpt(AF_CAIF)
#endif
#ifdef AF_ALG
    CheckOpt(AF_ALG)
#endif
#ifdef AF_NFC
    CheckOpt(AF_NFC)
#endif
#ifdef AF_VSOCK
    CheckOpt(AF_VSOCK)
#endif
#ifdef AF_KCM
    CheckOpt(AF_KCM)
#endif
#ifdef AF_QIPCRTR
    CheckOpt(AF_QIPCRTR)
#endif
#ifdef AF_SMP
    CheckOpt(AF_SMP)
#endif
    return -1;
}


/**
 * Convert a string socket type name to its numeric equivalent
 *
 * @return The mapped numeric value or -1 for an unknown name.
 */
int stringToSockType(const char *pszOptName)
{
    -- a string value of zero means unspecified
    CheckOpt(0)
#ifdef SOCK_DGRAM
    CheckOpt(SOCK_DGRAM)
#endif
#ifdef SOCK_STREAM
    CheckOpt(SOCK_STREAM)
#endif
#ifdef SOCK_RAW
    CheckOpt(SOCK_RAW)
#endif
#ifdef SOCK_RDM
    CheckOpt(SOCK_RDM)
#endif
#ifdef SOCK_SEQPACKET
    CheckOpt(SOCK_SEQPACKET)
#endif
#ifdef SOCK_DCCP
    CheckOpt(SOCK_DCCP)
#endif
#ifdef SOCK_PACKET
    CheckOpt(SOCK_PACKET)
#endif
#ifdef SOCK_CLOEXEC
    CheckOpt(SOCK_CLOEXEC)
#endif
#ifdef SOCK_NONBLOCK
    CheckOpt(SOCK_NONBLOCK)
#endif

    return -1;
}


/**
 * Convert a string protocol name to its numeric equivalent
 *
 * @return The mapped numeric value or -1 for an unknown name.
 */
int stringToProtocol(const char *pszOptName)
{
    // a non-specified protocol shows up as the string "0" and returns the same
    CheckOpt(0)
#ifdef IPPROTO_IP
    CheckOpt(IPPROTO_IP)
#endif
    CheckOpt(IPPROTO_IP)
#ifdef IPPROTO_HOPOPTS
    CheckOpt(IPPROTO_HOPOPTS)
#endif
#ifdef IPPROTO_ICMP
    CheckOpt(IPPROTO_ICMP)
#endif
#ifdef IPPROTO_IGMP
    CheckOpt(IPPROTO_IGMP)
#endif
#ifdef IPPROTO_IPIP
    CheckOpt(IPPROTO_IPIP)
#endif
#ifdef IPPROTO_TCP
    CheckOpt(IPPROTO_TCP)
#endif
#ifdef IPPROTO_EGP
    CheckOpt(IPPROTO_EGP)
#endif
#ifdef IPPROTO_PUP
    CheckOpt(IPPROTO_PUP)
#endif
#ifdef IPPROTO_UDP
    CheckOpt(IPPROTO_UDP)
#endif
#ifdef IPPROTO_IDP
    CheckOpt(IPPROTO_IDP)
#endif
#ifdef IPPROTO_TP
    CheckOpt(IPPROTO_TP)
#endif
#ifdef IPPROTO_DCCP
    CheckOpt(IPPROTO_DCCP)
#endif
#ifdef IPPROTO_IPV6
    CheckOpt(IPPROTO_IPV6)
#endif
#ifdef IPPROTO_ROUTING
    CheckOpt(IPPROTO_ROUTING)
#endif
#ifdef IPPROTO_FRAGMENT
    CheckOpt(IPPROTO_FRAGMENT)
#endif
#ifdef IPPROTO_RSVP
    CheckOpt(IPPROTO_RSVP)
#endif
#ifdef IPPROTO_GRE
    CheckOpt(IPPROTO_GRE)
#endif
#ifdef IPPROTO_ESP
    CheckOpt(IPPROTO_ESP)
#endif
#ifdef IPPROTO_AH
    CheckOpt(IPPROTO_AH)
#endif
#ifdef IPPROTO_ICMPV6
    CheckOpt(IPPROTO_ICMPV6)
#endif
#ifdef IPPROTO_NONE
    CheckOpt(IPPROTO_NONE)
#endif
#ifdef IPPROTO_DSTOPTS
    CheckOpt(IPPROTO_DSTOPTS)
#endif
#ifdef IPPROTO_MTP
    CheckOpt(IPPROTO_MTP)
#endif
#ifdef IPPROTO_ENCAP
    CheckOpt(IPPROTO_ENCAP)
#endif
#ifdef IPPROTO_PIM
    CheckOpt(IPPROTO_PIM)
#endif
#ifdef IPPROTO_COMP
    CheckOpt(IPPROTO_COMP)
#endif
#ifdef IPPROTO_SCTP
    CheckOpt(IPPROTO_SCTP)
#endif
#ifdef IPPROTO_UDPLITE
    CheckOpt(IPPROTO_UDPLITE)
#endif
#ifdef IPPROTO_MPLS
    CheckOpt(IPPROTO_MPLS)
#endif
#ifdef IPPROTO_RAW
    CheckOpt(IPPROTO_RAW)
#endif

    return -1;
}

#undef CheckOpt
#define CheckOpt(n) case n: return #n;


/**
 * Convert a numeric value back into the STRING name
 *
 * @param option The numeric option value
 *
 * @return The string equivalient or "" for unknown options
 */
const char *sockOptToString(int option)
{
    switch (option)
    {
        CheckOpt(SO_DEBUG)
        CheckOpt(SO_REUSEADDR)
        CheckOpt(SO_KEEPALIVE)
        CheckOpt(SO_DONTROUTE)
        CheckOpt(SO_BROADCAST)
        CheckOpt(SO_USELOOPBACK)
        CheckOpt(SO_LINGER)
        CheckOpt(SO_OOBINLINE)
        CheckOpt(SO_SNDBUF)
        CheckOpt(SO_RCVBUF)
        CheckOpt(SO_SNDLOWAT)
        CheckOpt(SO_RCVLOWAT)
        CheckOpt(SO_SNDTIMEO)
        CheckOpt(SO_RCVTIMEO)
        CheckOpt(SO_ERROR)
        CheckOpt(SO_TYPE)
    #ifdef SO_SNDBUFFORCE
        CheckOpt(SO_SNDBUFFORCE)
    #endif
    #ifdef SO_RCVBUFFORCE
        CheckOpt(SO_RCVBUFFORCE)
    #endif
    #ifdef SO_NO_CHECK
        CheckOpt(SO_NO_CHECK)
    #endif
    #ifdef SO_PRIORITY
        CheckOpt(SO_PRIORITY)
    #endif
    #ifdef SO_BSDCOMPAT
        CheckOpt(SO_BSDCOMPAT)
    #endif
    #ifdef SO_REUSEPORT
        CheckOpt(SO_REUSEPORT)
    #endif
    #ifdef SO_SECURITY_AUTHENTICATION
        CheckOpt(SO_SECURITY_AUTHENTICATION)
    #endif
    #ifdef SO_SECURITY_ENCRYPTION_TRANSPORT
        CheckOpt(SO_SECURITY_ENCRYPTION_TRANSPORT)
    #endif
    #ifdef SO_SECURITY_ENCRYPTION_NETWORK
        CheckOpt(SO_SECURITY_ENCRYPTION_NETWORK)
    #endif
    #ifdef SO_ATTACH_FILTER
        CheckOpt(SO_ATTACH_FILTER)
    #endif
    #ifdef SO_DETACH_FILTER
        CheckOpt(SO_DETACH_FILTER)
    #endif
    #ifdef SO_TIMESTAMP
        CheckOpt(SO_TIMESTAMP)
    #endif
    #ifdef SO_PEERSEC
        CheckOpt(SO_PEERSEC)
    #endif
    #ifdef SO_PASSSEC
        CheckOpt(SO_PASSSEC)
    #endif
    #ifdef SO_TIMESTAMPNS
        CheckOpt(SO_TIMESTAMPNS)
    #endif
    #ifdef SO_MARK
        CheckOpt(SO_MARK)
    #endif
    #ifdef SO_TIMESTAMPING
        CheckOpt(SO_TIMESTAMPING)
    #endif
    #ifdef SO_PROTOCOL
        CheckOpt(SO_PROTOCOL)
    #endif
    #ifdef SO_DOMAIN
        CheckOpt(SO_DOMAIN)
    #endif
    #ifdef SO_RXQ_OVFL
        CheckOpt(SO_RXQ_OVFL)
    #endif
    #ifdef SO_RXQ_WIFI_STATUS
        CheckOpt(SO_RXQ_WIFI_STATUS)
    #endif
    #ifdef SO_PEEK_OFF
        CheckOpt(SO_PEEK_OFF)
    #endif
    #ifdef SO_NOFCS
        CheckOpt(SO_NOFCS)
    #endif
    #ifdef SO_LOCK_FILTER
        CheckOpt(SO_LOCK_FILTER)
    #endif
    #ifdef SO_SELECT_ERR_QUEUE
        CheckOpt(SO_SELECT_ERR_QUEUE)
    #endif
    #ifdef SO_BUSY_POLL
        CheckOpt(SO_BUSY_POLL)
    #endif
    #ifdef SO_PASSCRED
        CheckOpt(SO_PASSCRED)
    #endif
    }

    return "";
}


/**
 * Convert a string socket family name to its numeric equivalent
 *
 * @return The mapped numeric value or -1 for an unknown name.
 */
const char *familyToString(int option)
{
    switch (option)
    {
#ifdef PF_UNSPEC
    CheckOpt(PF_UNSPEC)
#endif
#ifdef PF_LOCAL
    CheckOpt(PF_LOCAL)
#endif
#ifdef PF_UNIX
    CheckOpt(PF_UNIX)
#endif
#ifdef PF_FILE
    CheckOpt(PF_FILE)
#endif
#ifdef PF_INET
    CheckOpt(PF_INET)
#endif
#ifdef PF_IMPLINK
    CheckOpt(PF_IMPLINK)
#endif
#ifdef PF_AX25
    CheckOpt(PF_AX25)
#endif
#ifdef PF_PUP
    CheckOpt(PF_PUP)
#endif
#ifdef PF_CHAOS
    CheckOpt(PF_CHAOS)
#endif
#ifdef PF_APPLETALK
    CheckOpt(PF_APPLETALK)
#endif
#ifdef PF_NETROM
    CheckOpt(PF_NETROM)
#endif
#ifdef PF_IPX
    CheckOpt(PF_IPX)
#endif
#ifdef PF_ISO
    CheckOpt(PF_ISO)
#endif
#ifdef PF_BRIDGE
    CheckOpt(PF_BRIDGE)
#endif
#ifdef PF_ECMA
    CheckOpt(PF_ECMA)
#endif
#ifdef PF_ATMPVC
    CheckOpt(PF_ATMPVC)
#endif
#ifdef PF_DATAKIT
    CheckOpt(PF_DATAKIT)
#endif
#ifdef PF_X25
    CheckOpt(PF_X25)
#endif
#ifdef PF_CCITT
    CheckOpt(PF_CCITT)
#endif
#ifdef PF_INET6
    CheckOpt(PF_INET6)
#endif
#ifdef PF_SNA
    CheckOpt(PF_SNA)
#endif
#ifdef PF_ROSE
    CheckOpt(PF_ROSE)
#endif
#ifdef PF_DECnet
    CheckOpt(PF_DECnet)
#endif
#ifdef PF_DLI
    CheckOpt(PF_DLI)
#endif
#ifdef PF_NETBEUI
    CheckOpt(PF_NETBEUI)
#endif
#ifdef PF_SECURITY
    CheckOpt(PF_SECURITY)
#endif
#ifdef PF_KEY
    CheckOpt(PF_KEY)
#endif
#ifdef PF_NETLINK
    CheckOpt(PF_NETLINK)
#endif
#ifdef PF_ROUTE
    CheckOpt(PF_ROUTE)
#endif
#ifdef PF_PACKET
    CheckOpt(PF_PACKET)
#endif
#ifdef PF_ASH
    CheckOpt(PF_ASH)
#endif
#ifdef PF_ECONET
    CheckOpt(PF_ECONET)
#endif
#ifdef PF_ATMSVC
    CheckOpt(PF_ATMSVC)
#endif
#ifdef PF_RDS
    CheckOpt(PF_RDS)
#endif
#ifdef PF_IRDA
    CheckOpt(PF_IRDA)
#endif
#ifdef PF_PPPOX
    CheckOpt(PF_PPPOX)
#endif
#ifdef PF_WANPIPE
    CheckOpt(PF_WANPIPE)
#endif
#ifdef PF_LLC
    CheckOpt(PF_LLC)
#endif
#ifdef PF_CAN
    CheckOpt(PF_CAN)
#endif
#ifdef PF_TIPC
    CheckOpt(PF_TIPC)
#endif
#ifdef PF_BLUETOOTH
    CheckOpt(PF_BLUETOOTH)
#endif
#ifdef PF_IUCV
    CheckOpt(PF_IUCV)
#endif
#ifdef PF_RXRPC
    CheckOpt(PF_RXRPC)
#endif
#ifdef PF_ISDN
    CheckOpt(PF_ISDN)
#endif
#ifdef PF_PHONET
    CheckOpt(PF_PHONET)
#endif
#ifdef PF_IEEE802154
    CheckOpt(PF_IEEE802154)
#endif
#ifdef PF_CAIF
    CheckOpt(PF_CAIF)
#endif
#ifdef PF_ALG
    CheckOpt(PF_ALG)
#endif
#ifdef PF_NFC
    CheckOpt(PF_NFC)
#endif
#ifdef PF_VSOCK
    CheckOpt(PF_VSOCK)
#endif
#ifdef PF_KCM
    CheckOpt(PF_KCM)
#endif
#ifdef PF_QIPCRTR
    CheckOpt(PF_QIPCRTR)
#endif
#ifdef PF_SMP
    CheckOpt(PF_SMP)
#endif
    }
    return "";
}


/**
 * Convert a string socket family name to its numeric equivalent
 *
 * @return The mapped numeric value or -1 for an unknown name.
 */
const char *addrTypeToString(int option)
{
    switch (option)
    {
#ifdef AF_UNSPEC
    CheckOpt(AF_UNSPEC)
#endif
#ifdef AF_LOCAL
    CheckOpt(AF_LOCAL)
#endif
#ifdef AF_UNIX
    CheckOpt(AF_UNIX)
#endif
#ifdef AF_FILE
    CheckOpt(AF_FILE)
#endif
#ifdef AF_INET
    CheckOpt(AF_INET)
#endif
#ifdef AF_IMPLINK
    CheckOpt(AF_IMPLINK)
#endif
#ifdef AF_AX25
    CheckOpt(AF_AX25)
#endif
#ifdef AF_PUP
    CheckOpt(AF_PUP)
#endif
#ifdef AF_CHAOS
    CheckOpt(AF_CHAOS)
#endif
#ifdef AF_APPLETALK
    CheckOpt(AF_APPLETALK)
#endif
#ifdef AF_NETROM
    CheckOpt(AF_NETROM)
#endif
#ifdef AF_IPX
    CheckOpt(AF_IPX)
#endif
#ifdef AF_ISO
    CheckOpt(AF_ISO)
#endif
#ifdef AF_BRIDGE
    CheckOpt(AF_BRIDGE)
#endif
#ifdef AF_ECMA
    CheckOpt(AF_ECMA)
#endif
#ifdef AF_ATMPVC
    CheckOpt(AF_ATMPVC)
#endif
#ifdef AF_DATAKIT
    CheckOpt(AF_DATAKIT)
#endif
#ifdef AF_X25
    CheckOpt(AF_X25)
#endif
#ifdef AF_CCITT
    CheckOpt(AF_CCITT)
#endif
#ifdef AF_INET6
    CheckOpt(AF_INET6)
#endif
#ifdef AF_SNA
    CheckOpt(AF_SNA)
#endif
#ifdef AF_ROSE
    CheckOpt(AF_ROSE)
#endif
#ifdef AF_DECnet
    CheckOpt(AF_DECnet)
#endif
#ifdef AF_DLI
    CheckOpt(AF_DLI)
#endif
#ifdef AF_NETBEUI
    CheckOpt(AF_NETBEUI)
#endif
#ifdef AF_SECURITY
    CheckOpt(AF_SECURITY)
#endif
#ifdef AF_KEY
    CheckOpt(AF_KEY)
#endif
#ifdef AF_NETLINK
    CheckOpt(AF_NETLINK)
#endif
#ifdef AF_ROUTE
    CheckOpt(AF_ROUTE)
#endif
#ifdef AF_PACKET
    CheckOpt(AF_PACKET)
#endif
#ifdef AF_ASH
    CheckOpt(AF_ASH)
#endif
#ifdef AF_ECONET
    CheckOpt(AF_ECONET)
#endif
#ifdef AF_ATMSVC
    CheckOpt(AF_ATMSVC)
#endif
#ifdef AF_RDS
    CheckOpt(AF_RDS)
#endif
#ifdef AF_IRDA
    CheckOpt(AF_IRDA)
#endif
#ifdef AF_PPPOX
    CheckOpt(AF_PPPOX)
#endif
#ifdef AF_WANPIPE
    CheckOpt(AF_WANPIPE)
#endif
#ifdef AF_LLC
    CheckOpt(AF_LLC)
#endif
#ifdef AF_CAN
    CheckOpt(AF_CAN)
#endif
#ifdef AF_TIPC
    CheckOpt(AF_TIPC)
#endif
#ifdef AF_BLUETOOTH
    CheckOpt(AF_BLUETOOTH)
#endif
#ifdef AF_IUCV
    CheckOpt(AF_IUCV)
#endif
#ifdef AF_RXRPC
    CheckOpt(AF_RXRPC)
#endif
#ifdef AF_ISDN
    CheckOpt(AF_ISDN)
#endif
#ifdef AF_PHONET
    CheckOpt(AF_PHONET)
#endif
#ifdef AF_IEEE802154
    CheckOpt(AF_IEEE802154)
#endif
#ifdef AF_CAIF
    CheckOpt(AF_CAIF)
#endif
#ifdef AF_ALG
    CheckOpt(AF_ALG)
#endif
#ifdef AF_NFC
    CheckOpt(AF_NFC)
#endif
#ifdef AF_VSOCK
    CheckOpt(AF_VSOCK)
#endif
#ifdef AF_KCM
    CheckOpt(AF_KCM)
#endif
#ifdef AF_QIPCRTR
    CheckOpt(AF_QIPCRTR)
#endif
#ifdef AF_SMP
    CheckOpt(AF_SMP)
#endif
    }
    return "";
}


/**
 * Convert a string socket type name to its numeric equivalent
 *
 * @return The mapped numeric value or -1 for an unknown name.
 */
const char *sockTypeToString(int option)
{
    switch (option)
    {
    --zero has special meaning as unspecified
    CheckOpt(0)
#ifdef SOCK_DGRAM
    CheckOpt(SOCK_DGRAM)
#endif
#ifdef SOCK_STREAM
    CheckOpt(SOCK_STREAM)
#endif
#ifdef SOCK_RAW
    CheckOpt(SOCK_RAW)
#endif
#ifdef SOCK_RDM
    CheckOpt(SOCK_RDM)
#endif
#ifdef SOCK_SEQPACKET
    CheckOpt(SOCK_SEQPACKET)
#endif
#ifdef SOCK_DCCP
    CheckOpt(SOCK_DCCP)
#endif
#ifdef SOCK_PACKET
    CheckOpt(SOCK_PACKET)
#endif
#ifdef SOCK_CLOEXEC
    CheckOpt(SOCK_CLOEXEC)
#endif
#ifdef SOCK_NONBLOCK
    CheckOpt(SOCK_NONBLOCK)
#endif
    }

    return "";
}


/**
 * Convert a string protocol name to its numeric equivalent
 *
 * @return The mapped numeric value or -1 for an unknown name.
 */
const char *protocolToString(int option)
{
    switch (option)
    {
#ifdef IPPROTO_IP
    CheckOpt(IPPROTO_IP)
#endif
#ifdef IPPROTO_HOPOPTS
    CheckOpt(IPPROTO_HOPOPTS)
#endif
#ifdef IPPROTO_ICMP
    CheckOpt(IPPROTO_ICMP)
#endif
#ifdef IPPROTO_IGMP
    CheckOpt(IPPROTO_IGMP)
#endif
#ifdef IPPROTO_IPIP
    CheckOpt(IPPROTO_IPIP)
#endif
#ifdef IPPROTO_TCP
    CheckOpt(IPPROTO_TCP)
#endif
#ifdef IPPROTO_EGP
    CheckOpt(IPPROTO_EGP)
#endif
#ifdef IPPROTO_PUP
    CheckOpt(IPPROTO_PUP)
#endif
#ifdef IPPROTO_UDP
    CheckOpt(IPPROTO_UDP)
#endif
#ifdef IPPROTO_IDP
    CheckOpt(IPPROTO_IDP)
#endif
#ifdef IPPROTO_TP
    CheckOpt(IPPROTO_TP)
#endif
#ifdef IPPROTO_DCCP
    CheckOpt(IPPROTO_DCCP)
#endif
#ifdef IPPROTO_IPV6
    CheckOpt(IPPROTO_IPV6)
#endif
#ifdef IPPROTO_ROUTING
    CheckOpt(IPPROTO_ROUTING)
#endif
#ifdef IPPROTO_FRAGMENT
    CheckOpt(IPPROTO_FRAGMENT)
#endif
#ifdef IPPROTO_RSVP
    CheckOpt(IPPROTO_RSVP)
#endif
#ifdef IPPROTO_GRE
    CheckOpt(IPPROTO_GRE)
#endif
#ifdef IPPROTO_ESP
    CheckOpt(IPPROTO_ESP)
#endif
#ifdef IPPROTO_AH
    CheckOpt(IPPROTO_AH)
#endif
#ifdef IPPROTO_ICMPV6
    CheckOpt(IPPROTO_ICMPV6)
#endif
#ifdef IPPROTO_NONE
    CheckOpt(IPPROTO_NONE)
#endif
#ifdef IPPROTO_DSTOPTS
    CheckOpt(IPPROTO_DSTOPTS)
#endif
#ifdef IPPROTO_MTP
    CheckOpt(IPPROTO_MTP)
#endif
#ifdef IPPROTO_ENCAP
    CheckOpt(IPPROTO_ENCAP)
#endif
#ifdef IPPROTO_PIM
    CheckOpt(IPPROTO_PIM)
#endif
#ifdef IPPROTO_COMP
    CheckOpt(IPPROTO_COMP)
#endif
#ifdef IPPROTO_SCTP
    CheckOpt(IPPROTO_SCTP)
#endif
#ifdef IPPROTO_UDPLITE
    CheckOpt(IPPROTO_UDPLITE)
#endif
#ifdef IPPROTO_MPLS
    CheckOpt(IPPROTO_MPLS)
#endif
#ifdef IPPROTO_RAW
    CheckOpt(IPPROTO_RAW)
#endif
    }

    return "";
}


/**
 * Convert a string protocol name to its numeric equivalent
 *
 * @return The mapped numeric value or -1 for an unknown name.
 */
const char *errnoToString(int option)
{
    switch (option)
    {
         CheckOpt(EPERM)
#ifdef ENOENT
         CheckOpt(ENOENT)
#endif
#ifdef ESRCH
         CheckOpt(ESRCH)
#endif
#ifdef EINTR
         CheckOpt(EINTR)
#endif
#ifdef EIO
         CheckOpt(EIO)
#endif
#ifdef ENXIO
         CheckOpt(ENXIO)
#endif
#ifdef E2BIG
         CheckOpt(E2BIG)
#endif
#ifdef ENOEXEC
         CheckOpt(ENOEXEC)
#endif
#ifdef EBADF
         CheckOpt(EBADF)
#endif
#ifdef ECHILD
         CheckOpt(ECHILD)
#endif
#ifdef EAGAIN
         CheckOpt(EAGAIN)
#endif
#ifdef ENOMEM
         CheckOpt(ENOMEM)
#endif
#ifdef EACCES
         CheckOpt(EACCES)
#endif
#ifdef EFAULT
         CheckOpt(EFAULT)
#endif
#ifdef ENOTBLK
         CheckOpt(ENOTBLK)
#endif
#ifdef EBUSY
         CheckOpt(EBUSY)
#endif
#ifdef EEXIST
         CheckOpt(EEXIST)
#endif
#ifdef EXDEV
         CheckOpt(EXDEV)
#endif
#ifdef ENODEV
         CheckOpt(ENODEV)
#endif
#ifdef ENOTDIR
         CheckOpt(ENOTDIR)
#endif
#ifdef EISDIR
         CheckOpt(EISDIR)
#endif
#ifdef EINVAL
         CheckOpt(EINVAL)
#endif
#ifdef ENFILE
         CheckOpt(ENFILE)
#endif
#ifdef EMFILE
         CheckOpt(EMFILE)
#endif
#ifdef ENOTTY
         CheckOpt(ENOTTY)
#endif
#ifdef ETXTBSY
         CheckOpt(ETXTBSY)
#endif
#ifdef EFBIG
         CheckOpt(EFBIG)
#endif
#ifdef ENOSPC
         CheckOpt(ENOSPC)
#endif
#ifdef ESPIPE
         CheckOpt(ESPIPE)
#endif
#ifdef EROFS
         CheckOpt(EROFS)
#endif
#ifdef EMLINK
         CheckOpt(EMLINK)
#endif
#ifdef EPIPE
         CheckOpt(EPIPE)
#endif
#ifdef EDOM
         CheckOpt(EDOM)
#endif
#ifdef ERANGE
         CheckOpt(ERANGE)
#endif
#ifdef EDEADLK
         CheckOpt(EDEADLK)
#endif
#ifdef ENAMETOOLONG
         CheckOpt(ENAMETOOLONG)
#endif
#ifdef ENOLCK
         CheckOpt(ENOLCK)
#endif
#ifdef ENOSYS
         CheckOpt(ENOSYS)
#endif
#ifdef ENOTEMPTY
         CheckOpt(ENOTEMPTY)
#endif
#ifdef ELOOP
         CheckOpt(ELOOP)
#endif
#ifdef ENOMSG
         CheckOpt(ENOMSG)
#endif
#ifdef EIDRM
         CheckOpt(EIDRM)
#endif
#ifdef ECHRNG
         CheckOpt(ECHRNG)
#endif
#ifdef EL2NSYNC
         CheckOpt(EL2NSYNC)
#endif
#ifdef EL3HLT
         CheckOpt(EL3HLT)
#endif
#ifdef EL3RST
         CheckOpt(EL3RST)
#endif
#ifdef ELNRNG
         CheckOpt(ELNRNG)
#endif
#ifdef EUNATCH
         CheckOpt(EUNATCH)
#endif
#ifdef ENOCSI
         CheckOpt(ENOCSI)
#endif
#ifdef EL2HLT
         CheckOpt(EL2HLT)
#endif
#ifdef EBADE
         CheckOpt(EBADE)
#endif
#ifdef EBADR
         CheckOpt(EBADR)
#endif
#ifdef EXFULL
         CheckOpt(EXFULL)
#endif
#ifdef ENOANO
         CheckOpt(ENOANO)
#endif
#ifdef EBADRQC
         CheckOpt(EBADRQC)
#endif
#ifdef EBADSLT
         CheckOpt(EBADSLT)
#endif
#ifdef EBFONT
         CheckOpt(EBFONT)
#endif
#ifdef ENOSTR
         CheckOpt(ENOSTR)
#endif
#ifdef ENODATA
         CheckOpt(ENODATA)
#endif
#ifdef ETIME
         CheckOpt(ETIME)
#endif
#ifdef ENOSR
         CheckOpt(ENOSR)
#endif
#ifdef ENONET
         CheckOpt(ENONET)
#endif
#ifdef ENOPKG
         CheckOpt(ENOPKG)
#endif
#ifdef EREMOTE
         CheckOpt(EREMOTE)
#endif
#ifdef ENOLINK
         CheckOpt(ENOLINK)
#endif
#ifdef EADV
         CheckOpt(EADV)
#endif
#ifdef ESRMNT
         CheckOpt(ESRMNT)
#endif
#ifdef ECOMM
         CheckOpt(ECOMM)
#endif
#ifdef EPROTO
         CheckOpt(EPROTO)
#endif
#ifdef EMULTIHOP
         CheckOpt(EMULTIHOP)
#endif
#ifdef EDOTDOT
         CheckOpt(EDOTDOT)
#endif
#ifdef EBADMSG
         CheckOpt(EBADMSG)
#endif
#ifdef EOVERFLOW
         CheckOpt(EOVERFLOW)
#endif
#ifdef ENOTUNIQ
         CheckOpt(ENOTUNIQ)
#endif
#ifdef EBADFD
         CheckOpt(EBADFD)
#endif
#ifdef EREMCHG
         CheckOpt(EREMCHG)
#endif
#ifdef ELIBACC
         CheckOpt(ELIBACC)
#endif
#ifdef ELIBBAD
         CheckOpt(ELIBBAD)
#endif
#ifdef ELIBSCN
         CheckOpt(ELIBSCN)
#endif
#ifdef ELIBMAX
         CheckOpt(ELIBMAX)
#endif
#ifdef ELIBEXEC
         CheckOpt(ELIBEXEC)
#endif
#ifdef EILSEQ
         CheckOpt(EILSEQ)
#endif
#ifdef ERESTART
         CheckOpt(ERESTART)
#endif
#ifdef ESTRPIPE
         CheckOpt(ESTRPIPE)
#endif
#ifdef EUSERS
         CheckOpt(EUSERS)
#endif
#ifdef ENOTSOCK
         CheckOpt(ENOTSOCK)
#endif
#ifdef EDESTADDRREQ
         CheckOpt(EDESTADDRREQ)
#endif
#ifdef EMSGSIZE
         CheckOpt(EMSGSIZE)
#endif
#ifdef EPROTOTYPE
         CheckOpt(EPROTOTYPE)
#endif
#ifdef ENOPROTOOPT
         CheckOpt(ENOPROTOOPT)
#endif
#ifdef EPROTONOSUPPORT
         CheckOpt(EPROTONOSUPPORT)
#endif
#ifdef ESOCKTNOSUPPORT
         CheckOpt(ESOCKTNOSUPPORT)
#endif
#ifdef EOPNOTSUPP
         CheckOpt(EOPNOTSUPP)
#endif
#ifdef EPFNOSUPPORT
         CheckOpt(EPFNOSUPPORT)
#endif
#ifdef EAFNOSUPPORT
         CheckOpt(EAFNOSUPPORT)
#endif
#ifdef EADDRINUSE
         CheckOpt(EADDRINUSE)
#endif
#ifdef EADDRNOTAVAIL
         CheckOpt(EADDRNOTAVAIL)
#endif
#ifdef ENETDOWN
         CheckOpt(ENETDOWN)
#endif
#ifdef ENETUNREACH
         CheckOpt(ENETUNREACH)
#endif
#ifdef ENETRESET
         CheckOpt(ENETRESET)
#endif
#ifdef ECONNABORTED
         CheckOpt(ECONNABORTED)
#endif
#ifdef ECONNRESET
         CheckOpt(ECONNRESET)
#endif
#ifdef ENOBUFS
         CheckOpt(ENOBUFS)
#endif
#ifdef EISCONN
         CheckOpt(EISCONN)
#endif
#ifdef ENOTCONN
         CheckOpt(ENOTCONN)
#endif
#ifdef ESHUTDOWN
         CheckOpt(ESHUTDOWN)
#endif
#ifdef ETOOMANYREFS
         CheckOpt(ETOOMANYREFS)
#endif
#ifdef ETIMEDOUT
         CheckOpt(ETIMEDOUT)
#endif
#ifdef ECONNREFUSED
         CheckOpt(ECONNREFUSED)
#endif
#ifdef EHOSTDOWN
         CheckOpt(EHOSTDOWN)
#endif
#ifdef EHOSTUNREACH
         CheckOpt(EHOSTUNREACH)
#endif
#ifdef EALREADY
         CheckOpt(EALREADY)
#endif
#ifdef EINPROGRESS
         CheckOpt(EINPROGRESS)
#endif
#ifdef ESTALE
         CheckOpt(ESTALE)
#endif
#ifdef EUCLEAN
         CheckOpt(EUCLEAN)
#endif
#ifdef ENOTNAM
         CheckOpt(ENOTNAM)
#endif
#ifdef ENAVAIL
         CheckOpt(ENAVAIL)
#endif
#ifdef EISNAM
         CheckOpt(EISNAM)
#endif
#ifdef EREMOTEIO
         CheckOpt(EREMOTEIO)
#endif
#ifdef EDQUOT
         CheckOpt(EDQUOT)
#endif
#ifdef ENOMEDIUM
         CheckOpt(ENOMEDIUM)
#endif
#ifdef EMEDIUMTYPE
         CheckOpt(EMEDIUMTYPE)
#endif
#ifdef ECANCELED
         CheckOpt(ECANCELED)
#endif
#ifdef ENOKEY
         CheckOpt(ENOKEY)
#endif
#ifdef EKEYEXPIRED
         CheckOpt(EKEYEXPIRED)
#endif
#ifdef EKEYREVOKED
         CheckOpt(EKEYREVOKED)
#endif
#ifdef EKEYREJECTED
         CheckOpt(EKEYREJECTED)
#endif
#ifdef EOWNERDEAD
         CheckOpt(EOWNERDEAD)
#endif
    }
    return "";
}


/**
 * Common routine for retrieving the socket descriptor from the current
 * instance.
 *
 * @param context The current method context
 *
 * @return The binary version of the socket descriptor.
 */
ORXSOCKET getSocket(RexxMethodContext* context)
{
    uintptr_t temp;
    // get the socket file descriptor
    RexxObjectPtr rxsockfd = context->GetObjectVariable("socketfd");
    // is is possible that we might have had an error before everything
    // was initialized, so return a zero value if not found
    if (rxsockfd == NULL)
    {
       return (ORXSOCKET)0;
    }
    context->Uintptr(rxsockfd, &temp);
    return (ORXSOCKET)temp;
}


/**
 * Retrieve a socket descriptor from another socket object.
 *
 * @param context The current execution context.
 * @param obj     The target object.
 *
 * @return The binary version of the descriptor.
 */
ORXSOCKET getSocket(RexxMethodContext* context, RexxObjectPtr obj)
{
    RexxObjectPtr rxsockfd = context->SendMessage0(obj, "socketfd");
    // is is possible that we might have had an error before everything
    // was initialized, so return a zero value if not found
    if (rxsockfd == NULL)
    {
       return (ORXSOCKET)0;
    }

    uintptr_t temp;
    context->Uintptr(rxsockfd, &temp);
    return (ORXSOCKET)temp;
}


/**
 * convert a socket descriptor into a rexx object.
 *
 * @param context The current execution context.
 * @param socket  The socket value
 *
 * @return A Rexx object for storing the socket object.
 */
RexxObjectPtr socketToObject(RexxMethodContext* context, ORXSOCKET socket)
{
    return context->UintptrToObject(socket);
}


/**
 * retrieve a int 32 value from an object
 *
 * @param context The current method context.
 * @param o       The object we're retrieving it from
 * @param name    The method name that will retrieve the value.
 *
 * @return The value, converted to a int32_t.
 */
int32_t getInt32(RexxMethodContext *context, RexxObjectPtr o, const char *name)
{
    RexxObjectPtr obj = context->SendMessage0(o, name);
    int32_t tmp = 0;
    context->Int32(obj, &tmp);
    return tmp;
}

/**
 * retrieve an unsigned int 32 value from an object
 *
 * @param context The current method context.
 * @param o       The object we're retrieving it from
 * @param name    The method name that will retrieve the value.
 *
 * @return The value, converted to a uint32_t.
 */
uint32_t getUint32(RexxMethodContext *context, RexxObjectPtr o, const char *name)
{
    RexxObjectPtr obj = context->SendMessage0(o, name);
    uint32_t tmp = 0;
    context->ObjectToUnsignedInt32(obj, &tmp);
    return tmp;
}

/**
 * retrieve a string value from an object
 *
 * @param context The current method context.
 * @param o       The object we're working with.
 * @param name    The name of the method used to retrieve the value.
 *
 * @return A const char * to the string value.
 */
const char *getStringValue(RexxMethodContext *context, RexxObjectPtr o, const char *name)
{
    RexxObjectPtr obj = context->SendMessage0(o, name);
    return context->CString(obj);
}

// set 32-bit int value in an object
void setValue(RexxMethodContext *context, RexxObjectPtr o, const char *name, int32_t v)
{
    context->SendMessage1(o, name, context->Int32ToObject(v));
}

// set an unsigned 32-bit int value in an object
void setValue(RexxMethodContext *context, RexxObjectPtr o, const char *name, uint32_t v)
{
    context->SendMessage1(o, name, context->UnsignedInt32ToObject(v));
}

// set a string value into an object
void setValue(RexxMethodContext *context, RexxObjectPtr o, const char *name, const char *v)
{
    context->SendMessage1(o, name, context->CString(v));
}

// set a string value into an object
void setValue(RexxMethodContext *context, RexxObjectPtr o, const char *name, RexxObjectPtr v)
{
    context->SendMessage1(o, name, v);
}


// Common routine for setting the error condition
void setErrno(RexxMethodContext* context, bool hasError)
{
    context->SetObjectVariable("errno", context->Int32(hasError ? sock_errno() : 0));
}

// Common routine for setting the error condition
void setErrno(RexxMethodContext* context, int32_t errNo)
{
    context->SetObjectVariable("errno", context->Int32(errNo));
}


// Common routine for setting the error condition
void setRetc(RexxMethodContext* context, int32_t v)
{
    context->SetObjectVariable("retc", context->Int32(v));
}

void setRetc(RexxMethodContext* context, uintptr_t v)
{
    context->SetObjectVariable("retc", context->UintptrToObject(v));
}

void setRetc(RexxMethodContext* context, RexxObjectPtr v)
{
    context->SetObjectVariable("retc", v);
}


// Common routine
const char *local_inet_ntop(int af, void *src, char *dst, socklen_t size)
{
#ifdef WIN32
     return InetNtop(af, src, dst, size);
#else
     return inet_ntop(af, src, dst, size);
#endif
}


int local_inet_pton(int af, const char *src, void *dst)
{
#ifdef WIN32
    return InetPton(af, src, dst);
#else
    return inet_pton(af, src, dst);
#endif
}


/**
 * Format an inet address into either the IPV4 or IPV6 style.
 *
 * @param addr   The address structure.
 * @param stringAddress
 */
void internetAddressToString(struct sockaddr_in *addr, char *stringAddress)
{
    if (addr->sin_family == AF_INET)
    {
        local_inet_ntop(AF_INET, &addr->sin_addr, stringAddress, INET6_ADDRSTRLEN);
    }
    else
    {
        struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *)addr;
        local_inet_ntop(AF_INET6, &addr6->sin6_addr, stringAddress, INET6_ADDRSTRLEN);
    }
}



/**
 * Format an inet address into either the IPV4 or IPV6 style.
 *
 * @param family  The family style of the address (AF_INET or AF_INET6)
 * @param rawAddr The pointer to the raw address storage
 * @param stringAddress
 *                The location for the returned formatted address.
 */
void internetAddressToString(int family, const char *rawAddr, char *stringAddress)
{
    if (family == AF_INET)
    {
        addr_in addr;
        memset(addr, 0, sizeof(addr));

        addr.sin_family = family;
        addr.sin_addr = (*(uint32_t *)rawAddr);

        local_inet_ntop(AF_INET, &addr->sin_addr, stringAddress, INET6_ADDRSTRLEN);
    }
    else
    {
        addr_in6 addr;
        memset(addr, 0, sizeof(addr));

        addr.sin6_family = family;
        addr.sin6_addr = (*(struct in6_addr *)rawAddr);

        local_inet_ntop(AF_INET6, &addr6->sin6_addr, stringAddress, INET6_ADDRSTRLEN);
    }
}


/**
 * Convert a string ip address into a binary version
 *
 * @param addr   The addr structure used to return the address
 * @param family The type of address (AF_INET or AF_INET6)
 * @param stringAddress
 *               The string version of the name.
 */
void stringToInternetAddress(struct sockaddr_in *addr, uint16_t family, char *stringAddress)
{
    // fill in the family in the struct
    addr->sin_family = family;
    if (family == AF_INET)
    {
        local_inet_pton(AF_INET, stringAddress, &addr->sin_addr.s_addr);
    }
    else
    {
        struct sockaddr_in6 * addr6 = (struct sockaddr_in6 *)&addr;
        local_inet_pton(AF_INET6, stringAddress, &addr6->sin6_addr.s6_addr);
    }
}


/**
 * Create a host entity object and populate it.
 *
 * @param context  The current call context.
 * @param pHostEnt The pHostEnt structure source for the information (can be NULL)
 *
 * @return A HostEntity instance.
 */
RexxObjectPtr createHostEntity(RexxMethodContext *context, struct hostent *pHostEnt)
{
    RexxClassObject classinst = context->FindClass("HostEntity");
    RexxObjectPtr entity = context->SendMessage0(classinst, "new");

    // if we don't have any actual information, return the empty entity
    if (pHostEnt == NULL)
    {
        // add the error information
        setValue(context, entity, "errno=", sock_errno());
        return entity;
    }

    // the family name
    setValue(context, entity, "name=", pHostEnt->h_name);

    // aliases are returned as an array. create a reasonable default size
    RexxArrayObject aliases = context->NewArray(5);
    setValue(context, entity, "aliases=", aliases);
    // now add in the alias values
    for (size_t count=0; pHostEnt->h_aliases[count]; count++)
    {
        context->ArrayAppendString(aliases, pHostEnt->h_aliases[count], strlen(pHostEnt->h_aliases[count]));
    }

    // the address type, which is variable
    setValue(context, entity, "addresstype=", addrTypeToString(pHostEnt->h_addrtype));

    // get the string version of the address. This is just the first
    // address on the list.
    char stringAddress[INET6_ADDRSTRLEN];
    ineternetAddressToString(pHostEnt->h_addrtype, pHostEnt->h_addr, stringAddress);
    setValue(context, entity, "address=", stringAddress);

    // There may be multiple addresses in an array. create a reasonable default size
    RexxArrayObject addresses = context->NewArray(5);
    setValue(context, entity, "addresses=", addresses);
    for (size_t count = 0; pHostEnt->h_addr_list[count]; count++)
    {
        ineternetAddressToString(pHostEnt->h_addrtype, pHostEnt->h_addrlist[count], stringAddress);
        context->ArrayAppendString(addresses, stringAddress, strlen(stringAddress));
    }

    return entity;
}


// a helper class to simplify the code that uses address objects.
class InetAddress
{
public:
    InetAddress(RexxMethodContext *c, RexxObjectPtr o) : inetaddr(o), context(c) {}

    // validate that this is a correct inet address object
    bool validate()
    {
        if (!context->IsOfType(inetaddr, "InetAddress"))
        {
            RexxArrayObject arrobj = context->NewArray(1);
            context->ArrayAppendString(arrobj, "address", strlen("address"));
            context->ArrayAppendString(arrobj, "InetAddress", strlen("InetAddress"));
            context->RaiseException(Rexx_Error_Invalid_argument_noclass, arrobj);
            return false;
        }
        return true;
    }

    void setFamily(uint16_t f)
    {
        context->SendMessage1(inetaddr, "family=", context->UnsignedInt32(f));
    }

    void setPort(uint16_t p)
    {
        context->SendMessage1(inetaddr, "port=", context->UnsignedInt32(p));
    }

    void setAddress(const char *a)
    {
        context->SendMessage1(inetaddr, "address=", context->String(a));
    }

    uint16_t family()
    {
        RexxObjectPtr obj = context->SendMessage0(inetaddr, "family");
        uint32_t tmp;
        context->UnsignedInt32(obj, &tmp);
        return (uint16_t)tmp;
    }

    uint16_t port()
    {
        RexxObjectPtr obj = context->SendMessage0(inetaddr, "port");
        uint32_t tmp;
        context->UnsignedInt32(obj, &tmp);
        return (uint16_t)tmp;
    }

    const char *address()
    {
        RexxObjectPtr obj = context->SendMessage0(inetaddr, "address");
        return context->CString(obj);
    }

    // prep the address structures from the InetAddress object
    void prep(struct sockaddr_storage &addr)
    {
        uint16_t fam = family();
        if (fam == AF_INET)
        {
            struct sockaddr_in *addr4 = (struct sockaddr_in *)&addr;
            addr4->sin_family = fam;
            addr4->sin_port = port();
            local_inet_pton(AF_INET, address(), &addr4->sin_addr.s_addr);
        }
        else
        {
            struct sockaddr_in6 * addr6 = (struct sockaddr_in6 *)&addr;
            addr6->sin6_family = fam;
            addr6->sin6_port = port();
            local_inet_pton(AF_INET6, address(), &addr6->sin6_addr.s6_addr);
        }
    }

    // update the object using newly obtained information.
    void update(struct sockaddr_storage &addr)
    {
        // fill out the caller's inetaddress
        if (addr.ss_family == AF_INET)
        {
            struct sockaddr_in *addr4 = (struct sockaddr_in *)&addr;
            setFamily(addr4->sin_family);
            setPort(addr4->sin_port);
        }
        else
        {
            struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *)&addr;
            setFamily(addr6->sin6_family);
            setPort(addr6->sin6_port);
        }

        char stringAddress[INET6_ADDRSTRLEN];
        internetAddressToString(addr, stringAddress);
        setAddress(stringAddress);
        // clear any error status
        setErrno(0);
    }


protected:
    RexxObjectPtr inetaddr;         // the Rexx object version
    RexxMethodContext *context;     // current call context.
};


/*----------------------------------------------------------------------------*/
/* Method: init                                                               */
/* Description: instance initialization                                       */
/* Arguments:                                                                 */
/*         domain   - socket domain, like PF_INET6                            */
/*         type     - socket type, like SOCK_STREAM                           */
/*         protocol - socket protocol, usually zero                           */
/*----------------------------------------------------------------------------*/

RexxMethod3(RexxObjectPtr,             // Return type
            socket_init,               // Object_method name
            CSTRING, domainStr,        // protocol family
            CSTRING, typeStr,          // socket type
            CSTRING, protocolStr)      // protocol
{
    // convert the string option names to the numeric equivalents
    int domain = stringToFamily(domainStr);
    int type = stringToSockType(typeStr);
    int protocol = stringToProtocol(protocolStr);

#ifdef WIN32
    WORD wVersionRequested;
    WSADATA wsaData;
    wVersionRequested = MAKEWORD( 1, 1 );
    int rc = WSAStartup( wVersionRequested, &wsaData );
#endif
    // perform function and return
    ORXSOCKET socketfd = socket(domain, type, protocol);
    if (socketfd == INVALID_SOCKET)
    {
        setRetc(context, -1);
        setErrno(context, sock_errno());
    }
    else
    {
        setRetc(context, socketfd);
        setErrno(context, (int32_t)0);
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
            socket_accept,             // Object_method name
            OSELF, self,               // The object itself,
            OPTIONAL_RexxObjectPtr, inetaddr) // INetaddr instance
{
    ORXSOCKET socketfd = getSocket(context);

    struct sockaddr_storage myaddr;
    memset(&myaddr, 0, sizeof(struct sockaddr_storage));
    socklen_t len = sizeof(struct sockaddr_storage);

    // perform function and return
    ORXSOCKET newSocket = accept(socketfd, (struct sockaddr *)&myaddr, &len);

    RexxObjectPtr newSocketObject = socketToObject(context, newSocket);
    setRetc(context, newSocketObject);
    setErrno(context, newSocket == INVALID_SOCKET);
    // if this was not a good socket return, return ,nil
    if (retc == INVALID_SOCKET)
    {
        return context->Nil();
    }

    // we've been asked to return information
    if (inetaddr != NULLOBJECT)
    {
        InetAddress addr(context, inetaddr);

        // it must be a valid InetAddress object
        if (!addr.validate())
        {
            return context->String("-1");
        }
        // update the InetAddress object with the current information
        addr.update(myaddr);
    }

    // now create a wrappered socket object. start by copying ourselves so that
    // all of the settings are maintained. This includes the last return and errno values.
    RexxObjectPtr newSocketObject = context->SendMessage0(self, "copy");

    // the new socket needs the new socket descriptor
    setValue(context, newSocketObject, "socketfd=", newSocketObject);
    return newSocketObject;
}


/*----------------------------------------------------------------------------*/
/* Method: bind                                                               */
/* Description: bind a socket to an address.                                  */
/* Arguments:                                                                 */
/*         inetaddr - Inetaddr instance initialized with the address          */
/*----------------------------------------------------------------------------*/

RexxMethod1(int,                       // Return type
            socket_bind,               // Object_method name
            RexxObjectPtr, inetaddr)   // Inetaddr instance
{
    ORXSOCKET socketfd = getSocket(context);

    struct sockaddr_storage myaddr;
    memset(&myaddr, 0, sizeof(struct sockaddr_storage));

    InetAddress addr(context, inetaddr);

    if (!addr.validate())
    {
        return -1;
    }

    // get the specifics from the address object
    addr.prep(myaddr);

    // we need the proper structure length set here
    socklen_t len = myaddr.ss_family == AF_INET ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6);

    int retc = bind(socketfd, (struct sockaddr *)&myaddr, len);
    setRetc(context, retc);
    setErrno(context, retc == -1);
    return retc;
}


/*----------------------------------------------------------------------------*/
/* Method: close                                                              */
/* Description: shutdown and close a socket                                   */
/* Arguments: none                                                            */
/*----------------------------------------------------------------------------*/
RexxMethod0(uintptr_t,                 // Return type
            socket_close)              // Object_method name
{
    ORXSOCKET socketfd = getSocket(context);
    // perform function and return
    shutdown(socketfd, 2);
    ORXSOCKET retc = closesocket(socketfd);

    // clear out the socket descriptor so that uninit won't close again.
    context->SetObjectVariable("socketfd", context->Int32(-1));
    setRetc(context, (uintptr_t)retc);
    setErrno(context, retc == -1);

    return (uintptr_t)retc;
}


/*----------------------------------------------------------------------------*/
/* Method: connect                                                            */
/* Description: connect a socket to a remote address                          */
/* Arguments:                                                                 */
/*         inetaddr - Inetaddr instance initialized with the address          */
/*----------------------------------------------------------------------------*/

RexxMethod1(int,                       // Return type
            socket_connect,            // Object_method name
            RexxObjectPtr, inetaddr)   // Inetaddr instance
{
    ORXSOCKET socketfd = getSocket(context);

    struct sockaddr_storage myaddr;
    memset(&myaddr, 0, sizeof(struct sockaddr_storage));
    socklen_t len = sizeof(myaddr);

    InetAddress addr(context, inetaddr);

    if (!addr.validate())
    {
        return -1;
    }

    // get the specifics from the address object
    addr.prep(myaddr);

    int retc = connect(socketfd, (struct sockaddr *)&myaddr, len);

    setRetc(context, retc);
    setErrno(context, retc == -1);
    return retc;
}


/**
 * Populate an AddrInfo object instance with the information from
 * the system structure.
 *
 * @param context The call context.
 * @param info    The addrinfo structure
 *
 * @return A Rexx object containing the address information.
 */
void buildSockAddr(RexxMethodContext *context, RexxObjectPtr addrObj, struct sockaddr *addr)
{
    setValue(context, obj, "addressFamily=", familyToString(addr->sa_family));
    setValue(context, obj, "addressPort=", ((sockaddr_in *)addr)->sin_port);
    // now format the string address
    char stringAddress[INET6_ADDRSTRLEN];
    internetAddressToString(addr, stringAddress);
    setValue(context, obj, "address=", stringAddress);

    return obj;
}


/**
 * Populate an AddrInfo object instance with the information from
 * the system structure.
 *
 * @param context The call context.
 * @param info    The addrinfo structure
 *
 * @return A Rexx object containing the address information.
 */
RexxObjectPtr buildAddressInfo(RexxMethodContext *context, struct addrinfo *info)
{
    RexxClassObject cobj = context->FindClass("AddrInfo");
    RexxObjectPtr obj = context->SendMessage0(cobj, "new");
    // the rexx object version of the flags is a 4-byte string where the
    // bit values are manipulated using the bitXXX methods. We need to store
    // this as a string also

    setValue(context, obj, "flags=", context->NewString((const char *)&info->ai_flags, sizeof(info->ai_flags));
    setValue(context, obj, "family=", familyToString(info->ai_family));
    setValue(context, obj, "socktype=", sockTypeToString(info->ai_socktype));
    setValue(context, obj, "protocol=", protocolToString(info->ai_protocol));
    if (info->ai_canonname != NULL)
    {
        setValue(context, obj, "ai_canonname=", info->ai_canonname);
    }
    // if we have address information, add that to the object too
    if (info->ai_addr != NULL)
    {
        buildSockAddr(context, obj, info->ai_addr);
    }

    return obj;
}


/**
 * Populate an AddrInfo object instance with the information from
 * the system structure.
 *
 * @param context The call context.
 * @param info    The addrinfo structure
 *
 * @return A Rexx object containing the address information.
 */
RexxObjectPtr buildSockAddr(RexxMethodContext *context, struct sockaddr *info)
{
    RexxClassObject cobj = context->FindClass("AddrInfo");
    RexxObjectPtr obj = context->SendMessage0(cobj, "new");

    // Build an AddressInfo object and populate it with just the socket
    // address portion
    buildSockAddr(context, obj, info);

    return obj;
}


/**
 * Build a complex AddrInfo object that contains references to alternate
 * addresses.
 *
 * @param context The call context
 * @param info    The root addrinfo structure
 *
 * @return The full set of information available with this addrinfo entry.
 */
RexxObjectPtr buildAddressInfoSet(RexxMethodContext *context, struct addrinfo *info)
{
    // first build the root addrinfo item
    RexxObjectPtr baseAddressInfo = buildAddressInfo(context, info);

    // and add an array for holding the secondary addresses
    RexxArrayObject addressArray = context->NewArray(5);
    setValue(context, baseAddressInfo, "addresses=", addressArray);
    // now format all of the secondary addresses, which does not receive the addresses
    // array
    while (info->ai_next != NULL)
    {

        context->ArrayAppend(addressArray, buildAddressInfo(context, info->ai_next));
        info = info->ai_next;
    }

    // and return the fully formatted set
    return baseAddressInfo;
}


/**
 * Populate an addrinfo structure from the equivalent rexx object version.
 *
 * @param context The current execution context
 * @param infoObj The Rexx addrinfo object
 * @param info    The target addrinfo structure
 */
void populateAddrInfo(RexxMethodContext *context, RexxObjectPtr infoObj, struct addrinfo &info)
{
    // the flags are stored in the Rexx object as 4-byte string that is the actual binary
    // representation of the flags. We need to directly convert this string value into
    // an int value.
    int *flagData = (int *)getStringValue(context, infoObj, "flags");
    info.ai_flags = *flagData;
    info.ai_family = stringToFamily(getStringValue(context, infoObj, "family"));
    info.ai_socktype = stringToSockType(getStringValue(context, infoObj, "socktype"));
    info.ai_protocol = stringToProtocol(getStringValue(context, infoObj, "protocol"));
    info.ai_canonname = (char *)getStringValue(context, infoObj, "canonname");
    int family = stringToFamily(getStringValue(context, infoObj, "addressFamily"));

    if (family == AF_INET)
    {
        struct sockaddr_in * myaddr4 = (struct sockaddr_in *)info.ai_addr;
        myaddr4->sin_family = (uint16_t)family;
        myaddr4->sin_port = getInt32Value(context, infoObj, "addressPort");
        local_inet_pton(AF_INET, getStringValue(context, infoObj, "address"), &myaddr4->sin_addr.s_addr);
        info.ai_addrlen = sizeof(struct sockaddr_in);
    }
    else
    {
        struct sockaddr_in6 * myaddr6 = (struct sockaddr_in6 *)info.ai_addr;
        myaddr6->sin6_family = (uint16_t)family;
        myaddr4->sin6_port = getInt32Value(context, infoObj, "addressPort");
        local_inet_pton(AF_INET6, getStringValue(context, infoObj, "sa_addr"), &myaddr6 ->sin6_addr.s6_addr);
        info.ai_addrlen = sizeof(struct sockaddr_in6);
    }
}


/**
 * Create an empty AddrInfo object that is returned for error
 * situations.
 *
 * @param context The call context.
 * @param info    The addrinfo structure
 *
 * @return A Rexx object containing the address information.
 */
RexxObjectPtr buildErrorAddressInfo(RexxMethodContext *context, int errno)
{
    RexxClassObject cobj = context->FindClass("AddrInfo");
    RexxObjectPtr obj = context->SendMessage0(cobj, "new");

    setValue(context, obj, "errno", sock_errno());
    return obj;
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

RexxMethod4(RexxObjectPtr,             // Return type
            socket_getAddrInfo,        // Object_method name
            CSTRING, nodename,         // the host name or ip address
            OPTIONAL_CSTRING, servname,// the service name or number
            OPTIONAL_RexxObjectPtr, hints) // an Inetaddr for the search hints
{
    // this is our hints that we populate from the object version
    struct addrinfo shints;
    struct sockaddr_storage hintAddr;

    // fill out the shints struct
    memset(&shints, 0, sizeof(shints));
    memset(&hintAddr, 0, sizeof(hintAddr));
    shints->ai_addr = &hintAddr;

    // the hints are optional, so we pass NULL if we don't have one provided
    struct addrinfo *hintAddr = NULL;

    // if we have a hints object provided, pluck the information from it
    // and fill in the structure
    if (hints != NULL)
    {
        hintAddr = &shints;
        populateAddrInfo(context, hints, shints);
    }

    //perform function
    struct addrinfo *returnAddressInfo;
    int retc = getaddrinfo(nodename, servname, hintAddr, &returnAddressInfo);

    // if there was any error or nothing returned, return an
    // empty object with error information included
    if (retc != 0 || *returnAddressInfo == NULL)
    {
        return errorAddressInfo(context);
    }

    // convert the returned data to a Rexx Object
    return buildAddressInfoSet(context, returnAddressInfo);
}


/*----------------------------------------------------------------------------*/
/* Method: gai_strerror                                                       */
/* Description: get the error text associated with an error code from         */
/*              getaddrinfo method.                                           */
/* Arguments:                                                                 */
/*         errcode - error code                                               */
/*----------------------------------------------------------------------------*/

RexxMethod1(RexxObjectPtr,             // Return type
            socket_getStringError,     // Object_method name
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
            inetaddress_getHostName)   // Object_method name
{
    char host[HOST_NAME_MAX + 1];
    host[0] = '\0';

    // perform function and return
    gethostname(host, sizeof(host));

    return context->String(host);
}


/*----------------------------------------------------------------------------*/
/* Method: getPeerName                                                        */
/* Description: get the peer name connected to a socket                       */
/* Arguments:                                                                 */
/*         inetaddr - Inetaddr instance                                       */
/*----------------------------------------------------------------------------*/

RexxMethod0(int,                       // Return type
            socket_getPeerName)        // Object_method name
{
    struct sockaddr_storage myaddr;
    socklen_t len = sizeof(myaddr);

    // get the socket file descriptor
    ORXSOCKET socketfd = getSocket(context);

    // perform function and return
    memset(&myaddr, 0, sizeof(struct sockaddr_storage));

    getpeername(socketfd, (struct sockaddr *)&myaddr, &len);

    return buildSockAddr(context, (struct sockaddr *)&myaddr);
}


/*----------------------------------------------------------------------------*/
/* Method: getProtoByName                                                     */
/* Description: get the protocol by its name.                                 */
/* Arguments:                                                                 */
/*         proto - protocol name                                              */
/*----------------------------------------------------------------------------*/

RexxMethod1(int,                       // Return type
            inetAddress_getProtocolByName,  // Object_method name
            CSTRING, protoname)        // Protocol name
{
    // perform function and return
    struct protoent *ent = getprotobyname(protoname);
    if (ent == NULL)
    {
        setRetc(context, -1);
        setErrno(context, 22);
        return -1;
    }
    setRetc(context, 0);
    setErrno(context, (int32_t)0);
    return ent->p_proto;
}


/*----------------------------------------------------------------------------*/
/* Method: getProtoByNumber                                                   */
/* Description: get the protocol by its name.                                 */
/* Arguments:                                                                 */
/*         proto - protocol number                                            */
/*----------------------------------------------------------------------------*/

RexxMethod1(RexxStringObject,          // Return type
            socket_getProtocolByNumber,// Object_method name
            int, proto)                // Protocol number
{
    // perform function and return
    struct protoent *ent = getprotobynumber(proto);
    if (ent == NULL)
    {
        return context->String("-1");
    }
    setRetc(context, 0);
    setErrno(context, (int32_t)0);
    return context->String(ent->p_name);
}


/*----------------------------------------------------------------------------*/
/* Method: getSockName                                                        */
/* Description: get the socket name of the socket.                            */
/* Arguments:                                                                 */
/*         inetaddr - Inetaddr instance                                       */
/*----------------------------------------------------------------------------*/

RexxMethod0(RexxObjectPtr,             // Return type
            socket_getSockName)        // Object_method name
{
    struct sockaddr_storage myaddr;
    socklen_t len = sizeof(myaddr);

    // get the socket file descriptor
    ORXSOCKET socketfd = getSocket(context);

    // perform function and return
    memset(&myaddr, 0, sizeof(struct sockaddr_storage));

    int retc = getsockname(socketfd, (struct sockaddr *)&myaddr, &len);

    // if there was an error return an empty address item with the error code.
    if (retc != 0)
    {
        return buildErrorAddressInfo(context);
    }

    // and build an InetAddress object containing this information
    return buildSockAddr(context, (struct sockaddr *)&myaddr);
}


/*----------------------------------------------------------------------------*/
/* Method: getsockopt                                                         */
/* Description: get a socket option.                                          */
/* Arguments:                                                                 */
/*         option - socket option                                             */
/*----------------------------------------------------------------------------*/

RexxMethod1(RexxObjectPtr,             // Return type
            socket_getSockOpt,         // Object_method name
            CSTRING, optionStr)        // socket option
{

    // get the socket file descriptor
    ORXSOCKET socketfd = getSocket(context);

    // convert from a string to the numeric equivalent
    int option = stringToSockOpt(optionStr);

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
            socklen_t len = (int)sizeof(int);
            int sockval_int = 0;

            int retc = getsockopt(socketfd, SOL_SOCKET, option, (SOCKOPTION *)&sockval_int, &len);
            setRetc(context, retc);
            setErrno(context, retc == -1);

            if (retc == -1)
            {
                return context->Int32(retc);
            }
            return context->Int32(sockval_int);
        }
        case SO_LINGER:
        {
            struct linger so_linger;
            socklen_t len = (int) sizeof(so_linger);
            int retc = getsockopt(socketfd, SOL_SOCKET, option, (SOCKOPTION *)&so_linger, &len);
            setRetc(context, retc);
            setErrno(context, retc == -1);

            if (retc == 0)
            {
                RexxClassObject l_class= context->FindClass("Linger");
                RexxObjectPtr l_obj = context->SendMessage0(l_class, "new");
                setValue(context, l_obj, "l_onoff=", so_linger.l_onoff);
                setValue(context, l_obj, "l_linger=", so_linger.l_linger);
                return l_obj;
            }
            return context->Int32(retc);
        }
#ifdef SO_BINDTODEVICE
        case SO_BINDTODEVICE:
        {
            char sockval_str[512];
            socklen_t len = sizeof(sockval_str);
            int retc = getsockopt(socketfd, SOL_SOCKET, option, (SOCKOPTION *)&sockval_str, &len);
            setRetc(context, retc);
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
            setRetc(context, -1);
            setErrno(context, 22);
            return context->Int32(-1);
    }
}


/*----------------------------------------------------------------------------*/
/* Method: listen                                                             */
/* Description: listen on a socket.                                           */
/* Arguments:                                                                 */
/*         backlog - number of possible waiting clients.                      */
/*----------------------------------------------------------------------------*/

RexxMethod1(uintptr_t,                 // Return type
            socket_listen,             // Object_method name
            int, backlog)              // backlog
{
    // get the socket file descriptor
    ORXSOCKET socketfd = getSocket(context);

    // perform function and return
    ORXSOCKET retc = listen(socketfd, backlog);
    setRetc(context, socketToObject(context, retc));

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
            socket_receive,            // Object_method name
            size_t, len)               // number of bytes to be read
{
    RexxBufferStringObject buffer = context->NewBufferString(len);
    char *cblock = (char *)context->BufferStringData(buffer);

    // get the socket file descriptor
    ORXSOCKET socketfd = getSocket(context);

    // perform function and return
    size_t lenread = recv(socketfd, cblock, (int)len, 0);
    setRetc(context, context->Int64((int64_t)lenread));
    setErrno(context, lenread == -1);

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
            socket_receiveFrom,        // Object_method name
            int, len,                  // number of bytes to be read
            RexxObjectPtr, inetaddr)   // Inetaddr instance
{
    RexxBufferStringObject buffer = context->NewBufferString(len);
    char *cblock = (char *)context->BufferStringData(buffer);

    struct sockaddr_storage myaddr;
    socklen_t slen = sizeof(myaddr);

    // get the socket file descriptor
    ORXSOCKET socketfd = getSocket(context);

    // perform function and return
    memset(&myaddr, 0, sizeof(struct sockaddr_storage));

    InetAddress addr(context, inetaddr);

    if (!addr.validate())
    {
        return (RexxObjectPtr) context->NullString();
    }

    // fill in the address info from the object
    addr.prep(myaddr);

    size_t lenread = recvfrom(socketfd, cblock, len, 0, (struct sockaddr *)&myaddr, &slen);

    setRetc(context, context->Int64((int64_t)lenread));
    setErrno(context, lenread == -1);

    if (lenread == -1)
    {
        return context->Nil();
    }

    // set to the actual read length and return
    context->FinishBufferString(buffer, lenread);
    return buffer;
}


/**
 * Perform setup for a call to the select() function.
 *
 * @param context The current execution context.
 * @param fdarray Any optional array object containing sockets to be used for the select.
 * @param fdset   An fd_set value that the socket descriptors are written to.
 * @param maxsocketfd
 *                Am accumulator used to track the maximum socket number required for the select() call.
 *
 * @return A pointer to the fd_set or NULL if there we no sockets to process.
 */
fd_set *buildFDSet(RexxMethodContext *context, RexxArrayObject fdarray, fd_set &fdset, ORXSOCKET &maxsocketfd)
{
    // the arrays are optional, so return nothing if we're not
    // interested in thie set
    if (fdarray == NULL)
    {
        return NULL;
    }

    // initialize the set
    FD_ZERO(&fdset);
    size_t items = context->ArrayItems(fdarray);
    for (size_t i = 1; i <= items; i++)
    {
        // each item is a socket and we need the socket from that array.
        RexxObjectPtr rxsockfd = context->ArrayAt(fdarray, i);
        ORXSOCKET socketfd = getSocket(context, rxsockfd);
        if (socketfd > maxsocketfd)
        {
            maxsocketfd = socketfd;
        }
        // add the socket to the set
        FD_SET(socketfd, &fdset);
    }

    // we have something, so return a pointer to the set
    // to be used in the select.
    return &fdset;
}


/**
 * Update the select arrays with the results of the select
 *
 * @param context The current execution context.
 * @param fdarray The input array (optional). Non-selected sockets will be removed from this array.
 * @param fdset   The fd_set value after the select() operation
 */
void updateFDSet(RexxMethodContext *context, RexxArrayObject fdarray, fd_set &fdset)
{
    // set the read array
    if (fdarray != NULL)
    {
        size_t items = context->ArrayItems(fdarray);
        for (size_t i = 1; i <= items; i++)
        {
            // each item is a socket and we need the socket from that array.
            RexxObjectPtr rxsockfd = context->ArrayAt(fdarray, i);
            ORXSOCKET socketfd = getSocket(context, rxsockfd);
            if (!FD_ISSET(socketfd, &fdset))
            {
                // remove the item so DO OVER can be used on the remainders
                context->SendMessage1(fdarray, "remove", context->StringSizeToObject(i));
            }
        }
    }
}



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
            socket_select,             // Object_method name
            OPTIONAL_RexxArrayObject, readfds,  // array of read file descriptors
            OPTIONAL_RexxArrayObject, writefds, // array of write file descriptors
            OPTIONAL_RexxArrayObject, excpfds,  // array of read file descriptors
            int, timeout)              // timeout in milliseconds
{
    ORXSOCKET maxsocketfd = 0;

    fd_set read_set;
    fd_set write_set;
    fd_set excp_set;

    fd_set *pread_set = buildFDSet(context, readfds, read_set, maxsocketfd);
    fd_set *pwrite_set = buildFDSet(context, writefds, write_set, maxsocketfd);
    fd_set *pexcp_set = buildFDSet(context, excpfds, excp_set, maxsocketfd);

    struct timeval sel_timeout;
    // get the timeout
    sel_timeout.tv_sec = timeout / 1000;
    sel_timeout.tv_usec = timeout % 1000;
    // perform the select() operation
    int retc = select((int)maxsocketfd + 1, pread_set, pwrite_set, pexcp_set, &sel_timeout);
    setRetc(context, retc);
    setErrno(context, retc == -1);

    // since our result is done by removing the non-selected sockets,
    // we need to update even if nothing is selected.
    if (retc >= 0)
    {
        // now update the argument arrays with the results
        updateFDSet(context, readfds, read_set);
        updateFDSet(context, writefds, write_set);
        updateFDSet(context, excpfds, excp_set);
    }
    // return
    return retc;
}


/*----------------------------------------------------------------------------*/
/* Method: send                                                               */
/* Description: write a block of text to the socket.                          */
/* Arguments:                                                                 */
/*         text - the bytes to be written to the socket                       */
/*----------------------------------------------------------------------------*/

RexxMethod1(int,                       // Return type
            socket_send,               // Object_method name
            RexxStringObject, block)   // bytes to be written
{
    // get the socket file descriptor
    ORXSOCKET socketfd = getSocket(context);

    // perform function and return
    int retc = send(socketfd, context->CString(block), (int)context->StringLength(block), 0);

    setRetc(context, retc);
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
            socket_sendTo,             // Object_method name
            RexxStringObject, data,    // data to be sent
            RexxObjectPtr, inetaddr)   // Inetaddr instance
{
    struct sockaddr_storage myaddr;

    // get the socket file descriptor
    ORXSOCKET socketfd = getSocket(context);

    InetAddress addr(context, inetaddr);

    // perform function and return
    if (!addr.validate())
    {
        return -1;
    }

    // fill in the control blocks with the address info
    addr.prep(myaddr);

    int lenwritten = sendto(socketfd, context->CString(data), (int)context->StringLength(data), 0, (struct sockaddr *)&myaddr, sizeof(myaddr));

    setRetc(context, (int32_t)lenwritten);
    setErrno(context, lenwritten == -1);
    return lenwritten;
}


/*----------------------------------------------------------------------------*/
/* Method: setsockopt                                                         */
/* Description: set a socket option.                                          */
/* Arguments:                                                                 */
/*         option - socket option                                             */
/*         val   - value for the option                                       */
/*----------------------------------------------------------------------------*/

RexxMethod2(int,                       // Return type
            socket_setSockOpt,         // Object_method name
            CSTRING, optionStr,        // socket option
            RexxObjectPtr, val)        // socket option value
{
    // get the socket file descriptor
    ORXSOCKET socketfd = getSocket(context);

    int option = stringToSockOpt(optionStr);

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
            int sockval_int;

            // boolean/int options
            context->Int32(val, &sockval_int);
            int retc = setsockopt(socketfd, SOL_SOCKET, option, (SOCKOPTION *)&sockval_int, sizeof(int));

            setRetc(context, retc);
            setErrno(context, retc == -1);
            return retc;
        }
        case SO_LINGER:
        {
            struct linger so_linger;
            RexxObjectPtr obj = context->SendMessage0(val, "l_onoff");

            so_linger.l_onoff = getInt32(context, obj, "l_onoff");
            so_linger.l_linger = getInt32(context, obj, "l_onoff");

            int retc = setsockopt(socketfd, SOL_SOCKET, option, (SOCKOPTION *)&so_linger, sizeof(so_linger));

            setRetc(context, retc);
            setErrno(context, retc == -1);
            return retc;
        }
#ifdef SO_PASSCRED
        case SO_BINDTODEVICE:
        {
            CSTRING strval = context->CString(val);
            int retc = setsockopt(socketfd, SOL_SOCKET, option, strval, strlen(strval));

            setRetc(context, retc);
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
            setRetc(context, -1);
            setErrno(context, 22);
            return -1;
    }
}

RexxMethod1(CSTRING,                   // Return type
            socket_familyToString,     // Object_method name
            int, option)               // family value
{
    return familyToString(option);
}

RexxMethod1(CSTRING,                   // Return type
            socket_sockOptToString,    // Object_method name
            int, option)               // option value
{
    return sockOptToString(option);
}

RexxMethod1(CSTRING,                   // Return type
            socket_sockTypeToString,   // Object_method name
            int, option)               // option value
{
    return sockTypeToString(option);
}


RexxMethod1(CSTRING,                   // Return type
            socket_protocolToString,   // Object_method name
            int, option)               // option value
{
    return protocolToString(option);
}


RexxMethod1(CSTRING,                   // Return type
            socket_errnoToString,      // Object_method name
            int, option)               // option value
{
    return errnoToString(option);
}


/*------------------------------------------------------------------
 * gethostbyaddr()
 *------------------------------------------------------------------*/
RexxRoutine3(RexxObjectPtr, inetaddress_getHostByAddr, CSTRING, addrArg, RexxObjectPtr, OPTIONAL_CSTRING, domainName)
{

    // set a default domain
    int domain = AF_INET;

    // if we had the optional argument, convert the string constant
    // to the numeric value
    if (domainName != NULL)
    {
        domain = stringToAddressType(domainName);
    }

    struct sockaddr_storage addr;

    stringToInternetAddress(&addr, domain, addrArg);

    struct hostent *pHostEnt = gethostbyaddr((char*)&addr, sizeof(addr), domain);
    // convert this into an object
    return createHostEntity(context, pHostEnt);
}


/*------------------------------------------------------------------
 *  gethostbyname()
 *------------------------------------------------------------------*/
RexxRoutine2(RexxObjectPtr, inetaddress_getHostByName, CSTRING, name, OPTIONAL_STRING, domainName)
{
    // set a default domain
    int domain = AF_INET;

    // if we had the optional argument, convert the string constant
    // to the numeric value
    if (domainName != NULL)
    {
        domain = stringToAddressType(domainName);
    }

    struct hostent *pHostEnt = gethostbyname(name, domain);
    // convert this into an object
    return createHostEntity(context, pHostEnt);
}


/*------------------------------------------------------------------
 *  gethostid()
 *------------------------------------------------------------------*/
RexxRoutine0(RexxObjectPtr, inetaddress_getHost)
{
    char     hostName[128];                  // buffer for host name
    /*
     *   Retrieve my ip address.  Assuming the hosts file in
     *   in %systemroot%/system/drivers/etc/hosts contains my computer name.
     */                                      //get our name
    if (gethostname(pszBuff, sizeof(pszBuff)))
    {
        // not retrievable, return an empty host entity
        return createHostEntity(context, NULL);
    }
    // now get the host information and return the HostEntity
    struct hostent *pHostEnt = gethostbyname(pszBuff);
    // convert this into an object
    return createHostEntity(context, pHostEnt);
}


// now build the actual entry list
RexxMethodEntry rxsock6_methods[] =
{
    REXX_METHOD(socket_init, socket_init),
    REXX_METHOD(socket_accept, socket_accept),
    REXX_METHOD(socket_bind, socket_bind),
    REXX_METHOD(socket_close, socket_close),
    REXX_METHOD(socket_connect, socket_connect),
    REXX_METHOD(socket_getAddrInfo, socket_getAddrInfo),
    REXX_METHOD(socket_getHostName, socket_getHostName),
    REXX_METHOD(socket_getPeerName, socket_getPeerName),
    REXX_METHOD(socket_getProtocolByName, socket_getProtocolByName),
    REXX_METHOD(socket_getProtocolByNumber, socket_getProtocolByNumber),
    REXX_METHOD(socket_getSockName, socket_getSockName),
    REXX_METHOD(socket_getSockOpt, socket_getSockOpt),
    REXX_METHOD(socket_getStringError, socket_getStringError),
    REXX_METHOD(socket_listen, socket_listen),
    REXX_METHOD(socket_receive,socket_receive),
    REXX_METHOD(socket_receiveFrom, socket_receiveFrom),
    REXX_METHOD(socket_select, socket_select),
    REXX_METHOD(socket_send, socket_send),
    REXX_METHOD(socket_sendTo, socket_sendTo),
    REXX_METHOD(socket_setSockOpt, socket_setSockOpt),
    REXX_METHOD(socket_familyToString, socket_familyToString),
    REXX_METHOD(socket_sockOptToString, socket_sockOptToString),
    REXX_METHOD(socket_sockTypeToString, socket_sockTypeToString),
    REXX_METHOD(socket_protocolToString, socket_protocolToString),
    REXX_METHOD(socket_errnoToString, socket_errnoToString),
    REXX_METHOD(inetaddress_getHost, inetaddress_getHost),
    REXX_METHOD(inetaddress_getHostByName, inetaddress_getHostByName),
    REXX_METHOD(inetaddress_getHostByAddr, inetaddress_getHostByAddr),
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


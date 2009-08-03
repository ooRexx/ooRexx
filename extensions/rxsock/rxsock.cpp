/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2009 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* http://www.oorexx.org/license.html                          */
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
/***************************************************************************/
/* REXX sockets function support                                           */
/*                 sockets utility function package                        */
/***************************************************************************/

/*------------------------------------------------------------------
 * program defines
 *------------------------------------------------------------------*/
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#define PROG_DESC "REXX function package for tcp/ip sockets"
#define PROG_COPY "(c) Copyright International Business Machines Corporation 1993, 2004"
#define PROG_COPY1 "(c) Copyright Rexx Language Association 2005-2009"
#define PROG_ALRRa "All Rights Reserved."
#define PROG_ALRRb "This program and the accompanying materials"
#define PROG_ALRRc "are made available under the terms of the Common Public License v1.0"


#ifdef OPSYS_LINUX
  #define SO_USELOOPBACK  0x0040    /* bypass hardware when possible         */
#endif

/*------------------------------------------------------------------
 * standard includes
 *------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ctype.h>
#include <setjmp.h>

/*------------------------------------------------------------------
 * Windows includes
 *------------------------------------------------------------------*/
#if defined(WIN32)
   #include <winsock.h>
#endif

/*------------------------------------------------------------------
 * rexx includes
 *------------------------------------------------------------------*/
#include "oorexxapi.h"

/*------------------------------------------------------------------
 * tcp/ip includes
 *------------------------------------------------------------------*/
#include <sys/types.h>
#include <errno.h>

#if !defined(WIN32)
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#endif

/*------------------------------------------------------------------
 * include for this app
 *------------------------------------------------------------------*/
#include "rxsock.h"

/*-/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\-*/
/*-\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/-*/

/*------------------------------------------------------------------
 * caseless string compare
 *------------------------------------------------------------------*/
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

/*------------------------------------------------------------------
 * strip blanks from a line
 *------------------------------------------------------------------*/
void stripBlanks(char *string)
{
    size_t sLen;
    size_t leading;

    sLen = strlen(string);

    /*---------------------------------------------------------------
     * strip trailing blanks
     *---------------------------------------------------------------*/
    while (sLen && (' ' == string[sLen-1]))
    {
        string[sLen-1] = 0;
    }

    /*---------------------------------------------------------------
     * strip leading blanks
     *---------------------------------------------------------------*/
    leading = strspn(string," ");
    if (leading)
    {
        memmove(string,string+leading,sLen+1);
    }
}


/*-/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\-*/
/*-\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/-*/

/*------------------------------------------------------------------
 * get a rexx stem element as a string value.
 *------------------------------------------------------------------*/
char *getStemElement(RexxCallContext *context, StemManager &stem, const char *name)
{
    // first get the referenced object, returning NULL if it is not set
    RexxObjectPtr obj = stem.getValue(name);
    if (obj == NULLOBJECT)
    {
        return NULL;
    }

    // get this as a copy of the string value, because we may munge this
    return strdup(context->ObjectToStringValue(obj));
}


/*------------------------------------------------------------------
 * convert a stem variable to an array of ints
 *------------------------------------------------------------------*/
void stemToIntArray(RexxCallContext *context, RexxObjectPtr stemSource, int &count, int *&arr)
{
    StemManager stem(context);

    if (!stem.resolveStem(stemSource))
    {
        return;
    }

    // set initial values
    count = 0;
    arr = NULL;

    // get the stem.0 item
    RexxObjectPtr countObj = stem.getValue((size_t)0);

    // try to convert this
    wholenumber_t temp;
    if (!context->WholeNumber(countObj, &temp))
    {
        return;
    }

    /*---------------------------------------------------------------
     * allocate array
     *---------------------------------------------------------------*/
    arr = (int *)malloc(sizeof(int) * temp);
    if (arr == NULL)
    {
        return;
    }

    count = temp;

    /*---------------------------------------------------------------
     * get each value ...
     *---------------------------------------------------------------*/
    for (int i = 0; i < count; i++)
    {
        countObj = stem.getValue(i + 1);

        if (!context->ObjectToWholeNumber(countObj, &temp))
        {
            free(arr);
            arr = NULL;
            return;
        }
        arr[i]  = (int)temp;
    }
    return;
}

/*------------------------------------------------------------------
 * convert an array of ints to a stem variable
 *------------------------------------------------------------------*/
void intArrayToStem(RexxCallContext *context, RexxObjectPtr stemSource, int count, int *arr)
{
    StemManager stem(context);

    if (!stem.resolveStem(stemSource))
    {
        return;
    }

    /*---------------------------------------------------------------
     * set 0'th value
     *---------------------------------------------------------------*/

    stem.setValue((size_t)0, context->WholeNumber(count));

    /*---------------------------------------------------------------
     * set each value
     *---------------------------------------------------------------*/
    for (int i = 0; i < count; i++)
    {
        stem.setValue(i + 1, context->WholeNumber(arr[i]));
    }

    return;
}

/*------------------------------------------------------------------
 * convert a stemmed variable to a sockaddr
 *------------------------------------------------------------------*/
void stemToSockAddr(RexxCallContext *context, StemManager &stem, sockaddr_in *pSockAddr)
{
    char *pszFamily = NULL;
    char *pszPort   = NULL;
    char *pszAddr   = NULL;

    /*---------------------------------------------------------------
     * initialize sockaddr
     *---------------------------------------------------------------*/
    memset(pSockAddr, 0, sizeof(*pSockAddr));

    /*---------------------------------------------------------------
     * get fields
     *---------------------------------------------------------------*/

    pszFamily = getStemElement(context, stem, "FAMILY");
    pszPort   = getStemElement(context, stem, "PORT");
    pszAddr   = getStemElement(context, stem, "ADDR");

    /*---------------------------------------------------------------
     * if any fields invalid, quit
     *---------------------------------------------------------------*/
    if (pszFamily == NULL || pszPort == NULL || pszAddr == NULL)
    {
        goto CleanUp;
    }

    stripBlanks(pszFamily);
    stripBlanks(pszPort);
    stripBlanks(pszAddr);

    /*---------------------------------------------------------------
     * get family
     *---------------------------------------------------------------*/
    if (!caselessCompare(pszFamily,"AF_INET"))
    {
        pSockAddr->sin_family = AF_INET;
    }
    else
    {
        pSockAddr->sin_family = (short) strtol(pszFamily,NULL,10);
    }

    /*---------------------------------------------------------------
     * get port
     *---------------------------------------------------------------*/
    pSockAddr->sin_port = (unsigned short) strtoul(pszPort, NULL, 10);
    pSockAddr->sin_port = htons(pSockAddr->sin_port);

    /*---------------------------------------------------------------
     * get addr
     *---------------------------------------------------------------*/
    if (!caselessCompare(pszAddr,"INADDR_ANY"))
    {
        pSockAddr->sin_addr.s_addr = INADDR_ANY;
    }
    else
    {
        pSockAddr->sin_addr.s_addr = inet_addr(pszAddr);
    }

    /*------------------------------------------------------------------
     * clean up and leave
     *------------------------------------------------------------------*/
    CleanUp:
    if (pszFamily == NULL)
    {
        free(pszFamily);
    }
    if (pszPort == NULL)
    {
        free(pszPort);
    }
    if (pszAddr == NULL)
    {
        free(pszAddr);
    }
}

/*------------------------------------------------------------------
 * convert a sockaddr to a stemmed variable
 *------------------------------------------------------------------*/
void sockAddrToStem(RexxCallContext *context, sockaddr_in *pSockAddr, StemManager &stem )
{
    /*---------------------------------------------------------------
     * set family
     *---------------------------------------------------------------*/
    stem.setValue("FAMILY", context->WholeNumber(pSockAddr->sin_family));

    /*---------------------------------------------------------------
     * set port
     *---------------------------------------------------------------*/
    stem.setValue("PORT", context->UnsignedInt32(htons(pSockAddr->sin_port)));

    /*---------------------------------------------------------------
     * set address
     *---------------------------------------------------------------*/
    stem.setValue("ADDR", context->String(inet_ntoa(pSockAddr->sin_addr)));
}

/*------------------------------------------------------------------
 * convert a hostent to a stemmed variable
 *------------------------------------------------------------------*/
void hostEntToStem(RexxCallContext *context, struct hostent *pHostEnt, StemManager &stem)
{
    char     szBuffer[20];
    int      count;
    in_addr  addr;

    /*---------------------------------------------------------------
     * set family
     *---------------------------------------------------------------*/
    stem.setValue("NAME", context->String(pHostEnt->h_name));

    /*---------------------------------------------------------------
     * set aliases
     *---------------------------------------------------------------*/
    for (count=0; pHostEnt->h_aliases[count]; count++)
    {
        sprintf(szBuffer,"ALIAS.%d",count+1);
        stem.setValue(szBuffer, context->String(pHostEnt->h_aliases[count]));
    }

    stem.setValue("ALIAS.0", context->WholeNumber(count));

    /*---------------------------------------------------------------
     * set addrtype
     *---------------------------------------------------------------*/
    stem.setValue("ADDRTYPE", context->String("AF_INET"));

    /*---------------------------------------------------------------
     * set addr
     *---------------------------------------------------------------*/
    addr.s_addr = (*(uint32_t *)pHostEnt->h_addr);
    stem.setValue("ADDR", context->String(inet_ntoa(addr)));

    /*---------------------------------------------------------------
     *  the stem variable variablename.addr.0  contains count of available
     *  addresses and variablename.addr.n is each address.
     *
     *  the original form is also returned to preserve compatability
     *  with old applications.
     *---------------------------------------------------------------
     *---------------------------------------------------------------
     * set addresses (note there can be many)
     *---------------------------------------------------------------*/
    for (count=0; pHostEnt->h_addr_list[count]; count++)
    {
        sprintf(szBuffer, "ADDR.%d", count+1);
        addr.s_addr = (*(uint32_t *)pHostEnt->h_addr_list[count]);

        stem.setValue(szBuffer, context->String(inet_ntoa(addr)));
    }

    stem.setValue("ADDR.0", context->WholeNumber(count));
}


/*------------------------------------------------------------------
 * convert a string sock option to an integer
 *------------------------------------------------------------------*/
int stringToSockOpt(const char * pszOptName)
{
    if (!pszOptName) return 0;

    if (!caselessCompare("SO_DEBUG"       ,pszOptName)) return SO_DEBUG;
    else if (!caselessCompare("SO_REUSEADDR"   ,pszOptName)) return SO_REUSEADDR;
    else if (!caselessCompare("SO_KEEPALIVE"   ,pszOptName)) return SO_KEEPALIVE;
    else if (!caselessCompare("SO_DONTROUTE"   ,pszOptName)) return SO_DONTROUTE;
    else if (!caselessCompare("SO_BROADCAST"   ,pszOptName)) return SO_BROADCAST;
    else if (!caselessCompare("SO_USELOOPBACK" ,pszOptName)) return SO_USELOOPBACK;
    else if (!caselessCompare("SO_LINGER"      ,pszOptName)) return SO_LINGER;
    else if (!caselessCompare("SO_OOBINLINE"   ,pszOptName)) return SO_OOBINLINE;
    else if (!caselessCompare("SO_SNDBUF"      ,pszOptName)) return SO_SNDBUF;
    else if (!caselessCompare("SO_RCVBUF"      ,pszOptName)) return SO_RCVBUF;
    else if (!caselessCompare("SO_SNDLOWAT"    ,pszOptName)) return SO_SNDLOWAT;
    else if (!caselessCompare("SO_RCVLOWAT"    ,pszOptName)) return SO_RCVLOWAT;
    else if (!caselessCompare("SO_SNDTIMEO"    ,pszOptName)) return SO_SNDTIMEO;
    else if (!caselessCompare("SO_RCVTIMEO"    ,pszOptName)) return SO_RCVTIMEO;
    else if (!caselessCompare("SO_ERROR"       ,pszOptName)) return SO_ERROR;
    else if (!caselessCompare("SO_TYPE"        ,pszOptName)) return SO_TYPE;

    return 0;
}


/*-/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\-*/
/*-\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/-*/

/*------------------------------------------------------------------
 * set errno
 *------------------------------------------------------------------*/
void setErrno(RexxCallContext *context)
{
    char szBuff[20];
    const char *pszErrno = szBuff;
    int   theErrno;

#if defined(WIN32)
    theErrno = WSAGetLastError();
#elif defined(OPSYS_AIX) || defined(OPSYS_LINUX)
    theErrno = errno;
#endif

    switch (theErrno)
    {
#ifdef WIN32
        case WSAEINTR              : pszErrno = "EINTR";                break;
        case WSAEBADF              : pszErrno = "EBADF";                break;
        case WSAEACCES             : pszErrno = "EACCES";               break;
        case WSAEFAULT             : pszErrno = "EFAULT";               break;
        case WSAEINVAL             : pszErrno = "EINVAL";               break;
        case WSAEMFILE             : pszErrno = "EMFILE";               break;
        case WSAEWOULDBLOCK     : pszErrno = "EWOULDBLOCK";          break;
        case WSAEINPROGRESS     : pszErrno = "EINPROGRESS";          break;
        case WSAEALREADY        : pszErrno = "EALREADY";             break;
        case WSAENOTSOCK        : pszErrno = "ENOTSOCK";             break;
        case WSAEDESTADDRREQ    : pszErrno = "EDESTADDRREQ";         break;
        case WSAEMSGSIZE        : pszErrno = "EMSGSIZE";             break;
        case WSAEPROTOTYPE      : pszErrno = "EPROTOTYPE";           break;
        case WSAENOPROTOOPT     : pszErrno = "ENOPROTOOPT";          break;
        case WSAEPROTONOSUPPORT : pszErrno = "EPROTONOSUPPORT";      break;
        case WSAESOCKTNOSUPPORT : pszErrno = "ESOCKTNOSUPPORT";      break;
        case WSAEOPNOTSUPP      : pszErrno = "EOPNOTSUPP";           break;
        case WSAEPFNOSUPPORT    : pszErrno = "EPFNOSUPPORT";         break;
        case WSAEAFNOSUPPORT    : pszErrno = "EAFNOSUPPORT";         break;
        case WSAEADDRINUSE      : pszErrno = "EADDRINUSE";           break;
        case WSAEADDRNOTAVAIL   : pszErrno = "EADDRNOTAVAIL";        break;
        case WSAENETDOWN        : pszErrno = "ENETDOWN";             break;
        case WSAENETUNREACH     : pszErrno = "ENETUNREACH";          break;
        case WSAENETRESET       : pszErrno = "ENETRESET";            break;
        case WSAECONNABORTED    : pszErrno = "ECONNABORTED";         break;
        case WSAECONNRESET      : pszErrno = "ECONNRESET";           break;
        case WSAENOBUFS         : pszErrno = "ENOBUFS";              break;
        case WSAEISCONN         : pszErrno = "EISCONN";              break;
        case WSAENOTCONN        : pszErrno = "ENOTCONN";             break;
        case WSAESHUTDOWN       : pszErrno = "ESHUTDOWN";            break;
        case WSAETOOMANYREFS    : pszErrno = "ETOOMANYREFS";         break;
        case WSAETIMEDOUT       : pszErrno = "ETIMEDOUT";            break;
        case WSAECONNREFUSED    : pszErrno = "ECONNREFUSED";         break;
        case WSAELOOP           : pszErrno = "ELOOP";                break;
        case WSAENAMETOOLONG    : pszErrno = "ENAMETOOLONG";         break;
        case WSAEHOSTDOWN       : pszErrno = "EHOSTDOWN";            break;
        case WSAEHOSTUNREACH    : pszErrno = "EHOSTUNREACH";         break;
        case WSASYSNOTREADY               : pszErrno = "WSASYSNOTREADY";       break;
        case WSAVERNOTSUPPORTED   : pszErrno = "WSASAVERNOTSUPPORTED"; break;
        case WSANOTINITIALISED    : pszErrno = "WSANOTINITIALISED";    break;
        case WSAHOST_NOT_FOUND  : pszErrno = "HOST_NOT_FOUND";       break;
        case WSATRY_AGAIN       : pszErrno = "TRY_AGAIN";           break;
        case WSANO_RECOVERY          : pszErrno = "NO_RECOVERY";          break;
        case WSANO_DATA         : pszErrno = "NO_DATA";              break;
#else

        case EWOULDBLOCK     : pszErrno = "EWOULDBLOCK";          break;
        case EINPROGRESS     : pszErrno = "EINPROGRESS";          break;
        case EALREADY        : pszErrno = "EALREADY";             break;
        case ENOTSOCK        : pszErrno = "ENOTSOCK";             break;
        case EDESTADDRREQ    : pszErrno = "EDESTADDRREQ";         break;
        case EMSGSIZE        : pszErrno = "EMSGSIZE";             break;
        case EPROTOTYPE      : pszErrno = "EPROTOTYPE";           break;
        case ENOPROTOOPT     : pszErrno = "ENOPROTOOPT";          break;
        case EPROTONOSUPPORT : pszErrno = "EPROTONOSUPPORT";      break;
        case ESOCKTNOSUPPORT : pszErrno = "ESOCKTNOSUPPORT";      break;
        case EOPNOTSUPP      : pszErrno = "EOPNOTSUPP";           break;
        case EPFNOSUPPORT    : pszErrno = "EPFNOSUPPORT";         break;
        case EAFNOSUPPORT    : pszErrno = "EAFNOSUPPORT";         break;
        case EADDRINUSE      : pszErrno = "EADDRINUSE";           break;
        case EADDRNOTAVAIL   : pszErrno = "EADDRNOTAVAIL";        break;
        case ENETDOWN        : pszErrno = "ENETDOWN";             break;
        case ENETUNREACH     : pszErrno = "ENETUNREACH";          break;
        case ENETRESET       : pszErrno = "ENETRESET";            break;
        case ECONNABORTED    : pszErrno = "ECONNABORTED";         break;
        case ECONNRESET      : pszErrno = "ECONNRESET";           break;
        case ENOBUFS         : pszErrno = "ENOBUFS";              break;
        case EISCONN         : pszErrno = "EISCONN";              break;
        case ENOTCONN        : pszErrno = "ENOTCONN";             break;
        case ESHUTDOWN       : pszErrno = "ESHUTDOWN";            break;
        case ETOOMANYREFS    : pszErrno = "ETOOMANYREFS";         break;
        case ETIMEDOUT       : pszErrno = "ETIMEDOUT";            break;
        case ECONNREFUSED    : pszErrno = "ECONNREFUSED";         break;
        case ELOOP           : pszErrno = "ELOOP";                break;
        case ENAMETOOLONG    : pszErrno = "ENAMETOOLONG";         break;
        case EHOSTDOWN       : pszErrno = "EHOSTDOWN";            break;
        case EHOSTUNREACH    : pszErrno = "EHOSTUNREACH";         break;
        case ENOTEMPTY       : pszErrno = "ENOTEMPTY";            break;
#endif
        default:
            sprintf(szBuff,"%d",theErrno);
    }

    context->SetContextVariable("errno", context->String(pszErrno));
}


/*------------------------------------------------------------------
 * set h_errno
 *------------------------------------------------------------------*/
void setH_Errno(RexxCallContext *context)
{
    char szBuff[20];
    const char *pszErrno = szBuff;
    int   theErrno;

    theErrno = 1541;

    switch (theErrno)
    {
        case HOST_NOT_FOUND  : pszErrno = "HOST_NOT_FOUND";       break;
        case TRY_AGAIN       : pszErrno = "TRY_AGAIN";            break;
        case NO_RECOVERY     : pszErrno = "NO_RECOVERY";          break;
        case NO_ADDRESS      : pszErrno = "NO_ADDRESS";           break;

        default:
            sprintf(szBuff,"%d",theErrno);
    }

    context->SetContextVariable("h_errno", context->String(pszErrno));
}


/*------------------------------------------------------------------
 * set the error variables at function cleanup.
 *------------------------------------------------------------------*/
void cleanup(RexxCallContext *context)
{
    setErrno(context);
    setH_Errno(context);
}

/*-/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\-*/
/*-\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/-*/

/*-/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\-*/
/*-\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/-*/

/*------------------------------------------------------------------
 *
 *------------------------------------------------------------------*/
RexxRoutine0(RexxStringObject, SockVersion)
{
    char buffer[256];

    sprintf(buffer, "%d.%d.%d", ORX_VER, ORX_REL, ORX_MOD);
    return context->String(buffer);
}

/*------------------------------------------------------------------
 * load the function package
 *------------------------------------------------------------------*/
RexxRoutine1(CSTRING,  SockLoadFuncs, OPTIONAL_CSTRING, version)
{
    // The rest of this is a NOP now.
    return "";
}

/*-/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\-*/
/*-\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/-*/

/*------------------------------------------------------------------
 * drop the function package
 *------------------------------------------------------------------*/
RexxRoutine0(RexxStringObject, SockDropFuncs)
{
    // No dropping when used as a package manager.
    return context->NullString();
}

/*-/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\-*/
/*-\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/-*/

/*------------------------------------------------------------------
 * cause a trap to unload the DLL
 *------------------------------------------------------------------*/
RexxRoutine0(int, SockDie)
{
    return 0;
}

/*------------------------------------------------------------------
 * declare external functions
 *------------------------------------------------------------------*/

REXX_TYPED_ROUTINE_PROTOTYPE(SockDropFuncs);
REXX_TYPED_ROUTINE_PROTOTYPE(SockVersion);
REXX_TYPED_ROUTINE_PROTOTYPE(SockDie);
REXX_TYPED_ROUTINE_PROTOTYPE(SockException);
REXX_TYPED_ROUTINE_PROTOTYPE(SockAccept);
REXX_TYPED_ROUTINE_PROTOTYPE(SockBind);
REXX_TYPED_ROUTINE_PROTOTYPE(SockClose);
REXX_TYPED_ROUTINE_PROTOTYPE(SockConnect);
REXX_TYPED_ROUTINE_PROTOTYPE(SockGetHostByAddr);
REXX_TYPED_ROUTINE_PROTOTYPE(SockGetHostByName);
REXX_TYPED_ROUTINE_PROTOTYPE(SockGetHostId);
REXX_TYPED_ROUTINE_PROTOTYPE(SockGetPeerName);
REXX_TYPED_ROUTINE_PROTOTYPE(SockGetSockName);
REXX_TYPED_ROUTINE_PROTOTYPE(SockGetSockOpt);
REXX_TYPED_ROUTINE_PROTOTYPE(SockInit);
REXX_TYPED_ROUTINE_PROTOTYPE(SockIoctl);
REXX_TYPED_ROUTINE_PROTOTYPE(SockListen);
REXX_TYPED_ROUTINE_PROTOTYPE(SockPSock_Errno);
REXX_TYPED_ROUTINE_PROTOTYPE(SockRecv);
REXX_TYPED_ROUTINE_PROTOTYPE(SockRecvFrom);
REXX_TYPED_ROUTINE_PROTOTYPE(SockSelect);
REXX_TYPED_ROUTINE_PROTOTYPE(SockSend);
REXX_TYPED_ROUTINE_PROTOTYPE(SockSendTo);
REXX_TYPED_ROUTINE_PROTOTYPE(SockSetSockOpt);
REXX_TYPED_ROUTINE_PROTOTYPE(SockShutDown);
REXX_TYPED_ROUTINE_PROTOTYPE(SockSock_Errno);
REXX_TYPED_ROUTINE_PROTOTYPE(SockSocket);
REXX_TYPED_ROUTINE_PROTOTYPE(SockSoClose);


// now build the actual entry list
RexxRoutineEntry rxsock_functions[] =
{
    REXX_TYPED_ROUTINE( SockLoadFuncs,      SockLoadFuncs),
    REXX_TYPED_ROUTINE( SockDropFuncs,      SockDropFuncs),
    REXX_TYPED_ROUTINE( SockAccept,         SockAccept),
    REXX_TYPED_ROUTINE( SockBind,           SockBind),
    REXX_TYPED_ROUTINE( SockClose,          SockClose),
    REXX_TYPED_ROUTINE( SockConnect,        SockConnect),
    REXX_TYPED_ROUTINE( SockGetHostByAddr,  SockGetHostByAddr),
    REXX_TYPED_ROUTINE( SockGetHostByName,  SockGetHostByName),
    REXX_TYPED_ROUTINE( SockGetHostId,      SockGetHostId),
    REXX_TYPED_ROUTINE( SockGetPeerName,    SockGetPeerName),
    REXX_TYPED_ROUTINE( SockGetSockName,    SockGetSockName),
    REXX_TYPED_ROUTINE( SockGetSockOpt,     SockGetSockOpt),
    REXX_TYPED_ROUTINE( SockInit,           SockInit),
    REXX_TYPED_ROUTINE( SockIoctl,          SockIoctl),
    REXX_TYPED_ROUTINE( SockListen,         SockListen),
    REXX_TYPED_ROUTINE( SockPSock_Errno,    SockPSock_Errno),
    REXX_TYPED_ROUTINE( SockRecv,           SockRecv),
    REXX_TYPED_ROUTINE( SockRecvFrom,       SockRecvFrom),
    REXX_TYPED_ROUTINE( SockSelect,         SockSelect),
    REXX_TYPED_ROUTINE( SockSend,           SockSend),
    REXX_TYPED_ROUTINE( SockSendTo,         SockSendTo),
    REXX_TYPED_ROUTINE( SockSetSockOpt,     SockSetSockOpt),
    REXX_TYPED_ROUTINE( SockShutDown,       SockShutDown),
    REXX_TYPED_ROUTINE( SockSock_Errno,     SockSock_Errno),
    REXX_TYPED_ROUTINE( SockSocket,         SockSocket),
    REXX_TYPED_ROUTINE( SockSoClose,        SockSoClose),
    REXX_TYPED_ROUTINE( SockVersion,        SockVersion),
    REXX_LAST_ROUTINE()
};

RexxPackageEntry rxsock_package_entry =
{
    STANDARD_PACKAGE_HEADER
    REXX_INTERPRETER_4_0_0,              // anything after 4.0.0 will work
    "RXSOCK",                            // name of the package
    "4.0",                               // package information
    NULL,                                // no load/unload functions
    NULL,
    rxsock_functions,                    // the exported functions
    NULL                                 // no methods in this package
};

// package loading stub.
OOREXX_GET_PACKAGE(rxsock);

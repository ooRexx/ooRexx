/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2006 Rexx Language Association. All rights reserved.    */
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
/* REXX sockets function support                               rxsock.c    */
/*                 sockets utility function package                        */
/***************************************************************************/

/*------------------------------------------------------------------
 * program defines
 *------------------------------------------------------------------*/
#if defined(OPSYS_AIX) || defined(OPSYS_LINUX)
#define PROG_NAME "rxsock"
#else
#define PROG_NAME "RxSock"
#endif

#define PROG_DESC "REXX function package for tcp/ip sockets"
#define PROG_COPY "(c) Copyright International Business Machines Corporation 1993, 2004"
#define PROG_COPY1 "(c) Copyright Rexx Language Association 2005-2006"
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
#include "rexx.h"

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

/*------------------------------------------------------------------
 * function table
 *------------------------------------------------------------------*/
#define TABLEENTRY(fun) { #fun , fun},

typedef struct
{
    const char *                   pszName;
    RexxFunctionHandler  *pRxFunction;
} RxSockFuncTableEntry;

RxSockFuncTableEntry RxSockFuncTable[] =
{
    TABLEENTRY( SockDropFuncs             )
    TABLEENTRY( SockAccept                )
    TABLEENTRY( SockBind                  )
    TABLEENTRY( SockClose                 )
    TABLEENTRY( SockConnect               )
    TABLEENTRY( SockGetHostByAddr         )
    TABLEENTRY( SockGetHostByName         )
    TABLEENTRY( SockGetHostId             )
    TABLEENTRY( SockGetPeerName           )
    TABLEENTRY( SockGetSockName           )
    TABLEENTRY( SockGetSockOpt            )
    TABLEENTRY( SockInit                  )
    TABLEENTRY( SockIoctl                 )
    TABLEENTRY( SockListen                )
    TABLEENTRY( SockPSock_Errno           )
    TABLEENTRY( SockRecv                  )
    TABLEENTRY( SockRecvFrom              )
    TABLEENTRY( SockSelect                )
    TABLEENTRY( SockSend                  )
    TABLEENTRY( SockSendTo                )
    TABLEENTRY( SockSetSockOpt            )
    TABLEENTRY( SockShutDown              )
    TABLEENTRY( SockSock_Errno            )
    TABLEENTRY( SockSocket                )
    TABLEENTRY( SockSoClose               )
    TABLEENTRY( SockVersion               )
};

#define RxSockFuncTableSize \
   (sizeof RxSockFuncTable / sizeof RxSockFuncTable[0] )

/*-/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\-*/
/*-\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/-*/

#if defined(OPSYS_AIX) || defined(OPSYS_LINUX)
/*------------------------------------------------------------------
 * stricmp for aix
 *------------------------------------------------------------------*/
int stricmp(const char *op1, const char *op2)
{
    for (; tolower(*op1) == tolower(*op2); op1++,op2++)
        if (*op1 == 0)
            return 0;

    return(tolower(*op1) - tolower(*op2));
}
#endif

/*------------------------------------------------------------------
 * strip blanks from a line
 *------------------------------------------------------------------*/
void StripBlanks(char *string)
{
    size_t sLen;
    size_t leading;

    sLen = strlen(string);

    /*---------------------------------------------------------------
     * strip trailing blanks
     *---------------------------------------------------------------*/
    while (sLen && (' ' == string[sLen-1]))
        string[sLen-1] = 0;

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
 * set a rexx variable
 *------------------------------------------------------------------*/
void RxVarSet(const char * pszStem, const char * pszTail, const char * pszValue )
{
    SHVBLOCK shv;
    char *pszVariable;

    if (!pszStem)
        return;

    /*---------------------------------------------------------------
     * get variable name
     *---------------------------------------------------------------*/
    pszVariable = (char *)malloc(1+strlen(pszStem)+strlen(pszTail));
    if (!pszVariable)
        return;

    strcpy(pszVariable,pszStem);
    strcat(pszVariable,pszTail);

    /*---------------------------------------------------------------
     * set shv values
     *---------------------------------------------------------------*/
    shv.shvcode            = RXSHV_SYSET;
    shv.shvnext            = NULL;
    shv.shvname.strptr     = pszVariable;
    shv.shvname.strlength  = strlen(pszVariable);
    shv.shvvalue.strptr    = const_cast<char *>(pszValue);
    shv.shvvalue.strlength = strlen(pszValue);

    RexxVariablePool(&shv);

    /*---------------------------------------------------------------
     * free temp var, if we used it
     *---------------------------------------------------------------*/
    free(pszVariable);
}

/*------------------------------------------------------------------
 * set a rexx variable
 *------------------------------------------------------------------*/
void RxVarSet(const char * pszVariable, const char * pszValue)
{
    SHVBLOCK shv;

    if (!pszVariable)
        return;

    /*---------------------------------------------------------------
     * set shv values
     *---------------------------------------------------------------*/
    shv.shvcode            = RXSHV_SYSET;
    shv.shvnext            = NULL;
    shv.shvname.strptr     = pszVariable;
    shv.shvname.strlength  = strlen(pszVariable);
    shv.shvvalue.strptr    = const_cast<char *>(pszValue);
    shv.shvvalue.strlength = strlen(pszValue);

    RexxVariablePool(&shv);
}

/*------------------------------------------------------------------
 * get a rexx variable - return value must be freed by caller
 *------------------------------------------------------------------*/
char * RxVarGet(const char * pszVariable)
{
    SHVBLOCK shv;
    char *      pszValue;

    /*---------------------------------------------------------------
     * set shv values
     *---------------------------------------------------------------*/
    shv.shvcode            = RXSHV_SYFET;
    shv.shvnext            = NULL;
    shv.shvname.strptr     = pszVariable;
    shv.shvname.strlength  = strlen(pszVariable);
    shv.shvvalue.strptr    = NULL;

    RexxVariablePool(&shv);

    /*---------------------------------------------------------------
     * check for errors
     *---------------------------------------------------------------*/
    if (!shv.shvvalue.strptr)
        return NULL;

    /*---------------------------------------------------------------
     * make copy of value, + 1 for hex 0 (not added by rexx)
     *---------------------------------------------------------------*/
    pszValue = (char *)malloc(shv.shvvalue.strlength+1);
    if (!pszValue)
        return NULL;

    /*---------------------------------------------------------------
     * copy value into new buffer, free old buffer, return new buffer
     *---------------------------------------------------------------*/
    memcpy(pszValue,shv.shvvalue.strptr,shv.shvvalue.strlength);
    pszValue[shv.shvvalue.strlength] = 0;

    RexxFreeMemory(shv.shvvalue.strptr);
    return pszValue;
}

/*------------------------------------------------------------------
 * get a rexx variable - return value must be freed by caller
 *------------------------------------------------------------------*/
char * RxVarGet(const char * pszStem, const char * pszTail)
{
    SHVBLOCK shv;
    char *pszVariable;
    char *pszValue;

    if (!pszStem)
        return NULL;

    pszVariable = (char *)malloc(1+strlen(pszStem)+strlen(pszTail));
    if (!pszVariable)
        return NULL;

    strcpy(pszVariable,pszStem);
    strcat(pszVariable,pszTail);

    /*---------------------------------------------------------------
     * set shv values
     *---------------------------------------------------------------*/
    shv.shvcode            = RXSHV_SYFET;
    shv.shvnext            = NULL;
    shv.shvname.strptr     = pszVariable;
    shv.shvname.strlength  = strlen(pszVariable);
    shv.shvvalue.strptr    = NULL;

    RexxVariablePool(&shv);

    /*---------------------------------------------------------------
     * free temp var, if we used it
     *---------------------------------------------------------------*/
    free(pszVariable);

    /*---------------------------------------------------------------
     * check for errors
     *---------------------------------------------------------------*/
    if (!shv.shvvalue.strptr)
        return NULL;

    /*---------------------------------------------------------------
     * make copy of value, + 1 for hex 0 (not added by rexx)
     *---------------------------------------------------------------*/
    pszValue = (char *)malloc(shv.shvvalue.strlength+1);
    if (!pszValue)
        return NULL;

    /*---------------------------------------------------------------
     * copy value into new buffer, free old buffer, return new buffer
     *---------------------------------------------------------------*/
    memcpy(pszValue,shv.shvvalue.strptr,shv.shvvalue.strlength);
    pszValue[shv.shvvalue.strlength] = 0;

    RexxFreeMemory(shv.shvvalue.strptr);
    return pszValue;
}

/*-/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\-*/
/*-\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/-*/

/*------------------------------------------------------------------
 * convert a rexx string to a size_t
 *------------------------------------------------------------------*/
size_t rxs2size_t(PCONSTRXSTRING  pRxStr, int *rc)
{
    size_t n;
    char *   dummy;

    /*---------------------------------------------------------------
     * check for errors
     *---------------------------------------------------------------*/
    if (!pRxStr)
        return 0;

    if (!pRxStr->strlength)
        return 0;

    /*---------------------------------------------------------------
     * convert
     *---------------------------------------------------------------*/
    n   = (size_t) strtoul(pRxStr->strptr,&dummy,10);
    *rc = (0 == *dummy);

    return n;
}

/*------------------------------------------------------------------
 * convert a rexx string to an int
 *------------------------------------------------------------------*/
int rxs2int(PCONSTRXSTRING  pRxStr, int *rc)
{
    int   n;

    /*---------------------------------------------------------------
     * check for errors
     *---------------------------------------------------------------*/
    if (!pRxStr)
        return 0;

    if (!pRxStr->strlength)
        return 0;

    /*---------------------------------------------------------------
     * convert
     *---------------------------------------------------------------*/
    n   = atoi(pRxStr->strptr);
    return n;
}

/*------------------------------------------------------------------
 * convert an int to a rexx string (already allocated)
 *------------------------------------------------------------------*/
void int2rxs(int i, PRXSTRING  pRxStr)
{
    sprintf(pRxStr->strptr,"%d",i);
    pRxStr->strlength = strlen(pRxStr->strptr);
}

/*------------------------------------------------------------------
 * convert a stem variable to an array of ints
 *------------------------------------------------------------------*/
void rxstem2intarray(PCONSTRXSTRING   pRxStr, int *count, int **arr)
{
    char *   countStr;
    char *   dummy;
    char  numBuff[10];
    char *numString;
    int   i;

    /*---------------------------------------------------------------
     * get stem.0 value
     *---------------------------------------------------------------*/
    if (!pRxStr || !pRxStr->strptr)
    {
        *count = 0;
        *arr   = NULL;
        return;
    }

    countStr = RxVarGet(pRxStr->strptr,"0");

    /*---------------------------------------------------------------
     * convert to an integer
     *---------------------------------------------------------------*/
    *count  = (int) strtoul(countStr,&dummy,10);
    if (0 != *dummy)
    {
        *count = 0;
        *arr   = NULL;
        return;
    }

    free(countStr);

    /*---------------------------------------------------------------
     * allocate array
     *---------------------------------------------------------------*/
    *arr = (int *)malloc(1 + sizeof(int) * *count);
    if (!*arr)
    {
        *count = 0;
        *arr   = NULL;
        return;
    }

    /*---------------------------------------------------------------
     * get each value ...
     *---------------------------------------------------------------*/
    for (i=0; i<*count; i++)
    {
        sprintf(numBuff,"%d",i+1);
        numString = RxVarGet(pRxStr->strptr,numBuff);
        (*arr)[i]  = (int) strtoul(numString,&dummy,10);
        free(numString);
    }

    return;
}

/*------------------------------------------------------------------
 * convert an array of ints to a stem variable
 *------------------------------------------------------------------*/
void intarray2rxstem(PCONSTRXSTRING   pRxStr, int count, int *arr)
{
    int   i;
    char  numBuff1[10];
    char  numBuff2[10];

    /*---------------------------------------------------------------
     * sanity check
     *---------------------------------------------------------------*/
    if (!pRxStr || !pRxStr->strptr)
    {
        return;
    }

    /*---------------------------------------------------------------
     * set 0'th value
     *---------------------------------------------------------------*/
    sprintf(numBuff1,"%d",count);
    RxVarSet(pRxStr->strptr,"0",numBuff1);

    /*---------------------------------------------------------------
     * set each value
     *---------------------------------------------------------------*/
    for (i=0; i<count; i++)
    {
        sprintf(numBuff1,"%d",i+1);
        sprintf(numBuff2,"%d",arr[i]);
        RxVarSet(pRxStr->strptr,numBuff1,numBuff2);
    }

    return;
}

/*------------------------------------------------------------------
 * convert a stemmed variable to a sockaddr
 *------------------------------------------------------------------*/
void stem2sockaddr(const char * pszStem, sockaddr_in *pSockAddr)
{
    char * pszFamily = NULL;
    char * pszPort   = NULL;
    char * pszAddr   = NULL;

    if (!pSockAddr || !pszStem)
        return;

    /*---------------------------------------------------------------
     * initialize sockaddr
     *---------------------------------------------------------------*/
    memset(pSockAddr,0,sizeof(*pSockAddr));

    /*---------------------------------------------------------------
     * get fields
     *---------------------------------------------------------------*/

    pszFamily = RxVarGet(pszStem,"family");
    pszPort   = RxVarGet(pszStem,"port");
    pszAddr   = RxVarGet(pszStem,"addr");

    StripBlanks(pszFamily);
    StripBlanks(pszPort);
    StripBlanks(pszAddr);

    /*---------------------------------------------------------------
     * if any fields invalid, quit
     *---------------------------------------------------------------*/
    if (!pszFamily || !pszPort || !pszAddr)
        goto CleanUp;

    /*---------------------------------------------------------------
     * get family
     *---------------------------------------------------------------*/
    if (!stricmp(pszFamily,"AF_INET"))
        pSockAddr->sin_family = AF_INET;
    else
        pSockAddr->sin_family = (short)strtol(pszFamily,NULL,10);

    /*---------------------------------------------------------------
     * get port
     *---------------------------------------------------------------*/
    pSockAddr->sin_port = (unsigned short)strtoul(pszPort,NULL,10);
    pSockAddr->sin_port = htons(pSockAddr->sin_port);

    /*---------------------------------------------------------------
     * get addr
     *---------------------------------------------------------------*/
    if (!stricmp(pszAddr,"INADDR_ANY"))
        pSockAddr->sin_addr.s_addr = INADDR_ANY;
    else
        pSockAddr->sin_addr.s_addr = inet_addr(pszAddr);

    /*------------------------------------------------------------------
     * clean up and leave
     *------------------------------------------------------------------*/
    CleanUp:
    if (pszFamily) free(pszFamily);
    if (pszPort)   free(pszPort);
    if (pszAddr)   free(pszAddr);
}

/*------------------------------------------------------------------
 * convert a sockaddr to a stemmed variable
 *------------------------------------------------------------------*/
void sockaddr2stem(sockaddr_in *pSockAddr, const char * pszStem)
{
    char szBuffer[20];

    if (!pSockAddr || !pszStem)
        return;

    /*---------------------------------------------------------------
     * set family
     *---------------------------------------------------------------*/
    sprintf(szBuffer,"%hd", pSockAddr->sin_family);
    RxVarSet(pszStem,"family",szBuffer);

    /*---------------------------------------------------------------
     * set port
     *---------------------------------------------------------------*/
    sprintf(szBuffer,"%hu",htons(pSockAddr->sin_port));
    RxVarSet(pszStem,"port",szBuffer);

    /*---------------------------------------------------------------
     * set address
     *---------------------------------------------------------------*/
    RxVarSet(pszStem,"addr",inet_ntoa(pSockAddr->sin_addr));
}

/*------------------------------------------------------------------
 * convert a hostent to a stemmed variable
 *------------------------------------------------------------------*/
void hostent2stem(struct hostent *pHostEnt, const char *pszStem)
{
    char    szBuffer[20];
    int      count;
    in_addr  addr;

    if (!pHostEnt || !pszStem)
        return;

    /*---------------------------------------------------------------
     * set family
     *---------------------------------------------------------------*/
    RxVarSet(pszStem,"name",pHostEnt->h_name);

    /*---------------------------------------------------------------
     * set aliases
     *---------------------------------------------------------------*/
    for (count=0; pHostEnt->h_aliases[count]; count++)
    {
        sprintf(szBuffer,"alias.%d",count+1);
        RxVarSet(pszStem,szBuffer,pHostEnt->h_aliases[count]);
    }

    sprintf(szBuffer,"%d",count);
    RxVarSet(pszStem,"alias.0",szBuffer);

    /*---------------------------------------------------------------
     * set addrtype
     *---------------------------------------------------------------*/
    RxVarSet(pszStem,"addrtype","AF_INET");

    /*---------------------------------------------------------------
     * set addr
     *---------------------------------------------------------------*/
    addr.s_addr = (u_long)(*(size_t *)pHostEnt->h_addr);
    RxVarSet(pszStem,"addr",inet_ntoa(addr));
    RxVarSet(pszStem,"addr",inet_ntoa(addr));

    /*---------------------------------------------------------------
     * this is an  extension to the os/2 version.   Dale Posey.
     *
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
        sprintf(szBuffer,"addr.%d",count+1);
        addr.s_addr = (u_long)(*(size_t *)pHostEnt->h_addr_list[count]);

        RxVarSet(pszStem,szBuffer, inet_ntoa(addr));
    }

    sprintf(szBuffer,"%d",count);
    RxVarSet(pszStem,"addr.0",szBuffer);
}

/*------------------------------------------------------------------
 * convert a string sock option to an integer
 *------------------------------------------------------------------*/
int rxs2SockOpt(const char * pszOptName)
{
    if (!pszOptName) return 0;

    if (!stricmp("SO_DEBUG"       ,pszOptName)) return SO_DEBUG;
    else if (!stricmp("SO_REUSEADDR"   ,pszOptName)) return SO_REUSEADDR;
    else if (!stricmp("SO_KEEPALIVE"   ,pszOptName)) return SO_KEEPALIVE;
    else if (!stricmp("SO_DONTROUTE"   ,pszOptName)) return SO_DONTROUTE;
    else if (!stricmp("SO_BROADCAST"   ,pszOptName)) return SO_BROADCAST;
    else if (!stricmp("SO_USELOOPBACK" ,pszOptName)) return SO_USELOOPBACK;
    else if (!stricmp("SO_LINGER"      ,pszOptName)) return SO_LINGER;
    else if (!stricmp("SO_OOBINLINE"   ,pszOptName)) return SO_OOBINLINE;
    else if (!stricmp("SO_SNDBUF"      ,pszOptName)) return SO_SNDBUF;
    else if (!stricmp("SO_RCVBUF"      ,pszOptName)) return SO_RCVBUF;
    else if (!stricmp("SO_SNDLOWAT"    ,pszOptName)) return SO_SNDLOWAT;
    else if (!stricmp("SO_RCVLOWAT"    ,pszOptName)) return SO_RCVLOWAT;
    else if (!stricmp("SO_SNDTIMEO"    ,pszOptName)) return SO_SNDTIMEO;
    else if (!stricmp("SO_RCVTIMEO"    ,pszOptName)) return SO_RCVTIMEO;
    else if (!stricmp("SO_ERROR"       ,pszOptName)) return SO_ERROR;
    else if (!stricmp("SO_TYPE"        ,pszOptName)) return SO_TYPE;

    return 0;
}


/*-/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\-*/
/*-\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/-*/

/*------------------------------------------------------------------
 * set errno
 *------------------------------------------------------------------*/
void SetErrno(void)
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

    RxVarSet("errno", pszErrno);
}


/*------------------------------------------------------------------
 * set h_errno
 *------------------------------------------------------------------*/
void SetH_Errno(void)
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

    RxVarSet("h_errno",pszErrno);
}

/*-/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\-*/
/*-\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/-*/

static int Initialized = 0;

/*------------------------------------------------------------------
 * Rexx external function gateway
 *------------------------------------------------------------------*/
APIRET REXXENTRY SockFunctionGateWay(const char *name, size_t argc, PCONSTRXSTRING argv, const char *qName, PRXSTRING  retStr)
{
    size_t                       i;
    APIRET                       ulRc;
    RexxFunctionHandler         *pRxFunc;
#ifdef WIN32
    WORD wVersionRequested;
    WSADATA wsaData;
#endif


    /*---------------------------------------------------------------
     * initialize return value to empty string
     *---------------------------------------------------------------*/
    retStr->strlength = 0;

    /*---------------------------------------------------------------
     * call sock_init(), if we need to
     *---------------------------------------------------------------*/
    if (!Initialized)
    {
        Initialized = 1;

#if defined(WIN32)
        wVersionRequested = MAKEWORD( 1, 1 );
        WSAStartup( wVersionRequested, &wsaData );
#endif
    }

    /*---------------------------------------------------------------
     * get function
     *---------------------------------------------------------------*/
    for (pRxFunc=NULL, i=0; !pRxFunc && i<RxSockFuncTableSize; i++)
        if (!stricmp(name,RxSockFuncTable[i].pszName))
            pRxFunc = RxSockFuncTable[i].pRxFunction;

        /*---------------------------------------------------------------
         * if not found, return syntax error
         *---------------------------------------------------------------*/
    if (!pRxFunc)
    {
        ulRc = 40;
        goto cleanUp;
    }

    /*---------------------------------------------------------------
     * call function
     *---------------------------------------------------------------*/
    ulRc = pRxFunc(name,argc,argv,qName,retStr);

    /*---------------------------------------------------------------
     * set errno and h_errno
     *---------------------------------------------------------------*/
    cleanUp:
    SetErrno();
    SetH_Errno();

    return ulRc;
}

/*-/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\-*/
/*-\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/-*/

/*------------------------------------------------------------------
 *
 *------------------------------------------------------------------*/
APIRET REXXENTRY SockVersion(const char *name, size_t argc, PCONSTRXSTRING argv, const char *qName, PRXSTRING  retStr)
{
    sprintf(retStr->strptr, "%d.%d.%d", ORX_VER, ORX_REL, ORX_MOD);
    retStr->strlength = strlen(retStr->strptr);
    return 0;
}

/*-/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\-*/
/*-\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/-*/
#if defined(OPSYS_AIX) || defined(OPSYS_LINUX)
APIRET REXXENTRY SOCKLOADFUNCS(const char *name, size_t argc, PCONSTRXSTRING argv, const char *qName, PRXSTRING  retStr)
{
    return(SockLoadFuncs( name, argc, argv, qName, retStr ));
}
    #endif

/*------------------------------------------------------------------
 * load the function package
 *------------------------------------------------------------------*/
APIRET REXXENTRY SockLoadFuncs(const char *name, size_t argc, PCONSTRXSTRING argv, const char *qName, PRXSTRING  retStr)
{
    size_t i;

    /*---------------------------------------------------------------
     * initialize return value to empty string
     *---------------------------------------------------------------*/
    retStr->strlength = 0;

    if (!argc)
    {
        printf("%s %d.%d.%d - %s\n",
               PROG_NAME, ORX_VER, ORX_REL, ORX_MOD, PROG_DESC);
/*      printf("  by %s  (%s)\n\n",PROG_AUTH,PROG_ADDR); */
        printf("%s\n",PROG_COPY);
        printf("%s\n",PROG_COPY1);
        printf("%s\n",PROG_ALRRa);
        printf("%s\n",PROG_ALRRb);
        printf("%s\n",PROG_ALRRc);
        printf("\n");
    }

    for (i=0; i<RxSockFuncTableSize; i++)
        RexxRegisterFunctionDll(RxSockFuncTable[i].pszName, PROG_NAME, "SockFunctionGateWay");
    return 0;
}

/*-/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\-*/
/*-\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/-*/

/*------------------------------------------------------------------
 * drop the function package
 *------------------------------------------------------------------*/
APIRET REXXENTRY SockDropFuncs(const char *name, size_t argc, PCONSTRXSTRING argv, const char *qName, PRXSTRING  retStr)
{
    size_t i;

    /*---------------------------------------------------------------
     * initialize return value to empty string
     *---------------------------------------------------------------*/
    retStr->strlength = 0;

    RexxDeregisterFunction("SockLoadFuncs");

    for (i=0; i<RxSockFuncTableSize; i++)
    {
        RexxDeregisterFunction(RxSockFuncTable[i].pszName);
    }

#ifdef WIN32
    WSACleanup();                       // deregister from Windows Sockets
#endif
    return 0;
}

/*-/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\-*/
/*-\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/-*/

/*------------------------------------------------------------------
 * cause a trap to unload the DLL
 *------------------------------------------------------------------*/
APIRET REXXENTRY SockDie(const char *name, size_t argc, PCONSTRXSTRING argv, const char *qName, PRXSTRING  retStr)
{
    int *p;

    p  = NULL;
    *p = 1;

    return 0;
}




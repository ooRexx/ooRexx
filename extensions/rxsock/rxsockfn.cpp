/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2024 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* https://www.oorexx.org/license.html                                        */
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
/* REXX sockets function support                               rxsockfn.c  */
/*                 sockets utility function package                        */
/***************************************************************************/
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ctype.h>
#include <setjmp.h>

/*------------------------------------------------------------------
 * rexx includes
 *------------------------------------------------------------------*/
# include "oorexxapi.h"
/*------------------------------------------------------------------
 * tcp/ip includes
 *------------------------------------------------------------------*/
#include <sys/types.h>
#include <errno.h>

#ifndef WIN32
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
# ifdef __APPLE__
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
#if defined( HAVE_IFADDRS_H )
#include <ifaddrs.h>
#endif
#endif

#define psock_errno(s) fprintf(stderr, "RxSOCK Error: %s\n", s)

#if defined(WIN32)                     // define errno equivalents for windows
   #define sock_errno() WSAGetLastError()
#else
   #define sock_errno() errno
#endif

/*------------------------------------------------------------------
 * include for this app
 *------------------------------------------------------------------*/
#include "rxsock.h"

/*------------------------------------------------------------------
 * sock_errno()
 *------------------------------------------------------------------*/
RexxRoutine0(int, SockSock_Errno)
{
    return sock_errno();
}


/*-/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\-*/
/*-\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/-*/

/*------------------------------------------------------------------
 * psock_errno()
 *------------------------------------------------------------------*/
RexxRoutine1(CSTRING, SockPSock_Errno, OPTIONAL_CSTRING, type)
{
    if (type == NULL)
    {
        type = "";
    }
    psock_errno(type);
    return "";
}


/*-/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\-*/
/*-\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/-*/

/*------------------------------------------------------------------------------
 * accept()
 *
 * @remarks  The sockAddrToStem() function calls both htons() and inet_ntoa().
 *           On Windows, one or both, of those functions sets errno back to 0.
 *           This prevents the Rexx programmer from ever seeing the errno if
 *           accept fails.  Because of this, we call cleanup() immediately after
 *           the accept call in the belief that the Rexx programmer is more
 *           interested in the result of accept().
* ----------------------------------------------------------------------------*/
RexxRoutine2(int, SockAccept, int, sock, OPTIONAL_RexxObjectPtr, stemSource)
{
    sockaddr_in  addr;
    socklen_t    nameLen;

    nameLen = sizeof(addr);
    // (int) cast avoids C4244 on Windows 64-bit
    int rc = (int)accept(sock, (struct sockaddr *)&addr, &nameLen);

    // set the errno variables
    setErrno(context, rc >= 0);

    /*---------------------------------------------------------------
     * set addr, if asked for
     *---------------------------------------------------------------*/
    if (stemSource != NULLOBJECT)
    {
        StemManager stem(context);

        if (!stem.resolveStem(stemSource))
        {
            return 0;
        }
        sockAddrToStem(context, &addr, stem);
    }

    return rc;
}


/*-/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\-*/
/*-\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/-*/

/*------------------------------------------------------------------
 * bind()
 *------------------------------------------------------------------*/
RexxRoutine2(int, SockBind, int, sock, RexxObjectPtr, stemSource)
{
    StemManager stem(context);

    if (!stem.resolveStem(stemSource))
    {
        return 0;
    }

    sockaddr_in  addr;

    /*---------------------------------------------------------------
     * get addr
     *---------------------------------------------------------------*/
    stemToSockAddr(context, stem, &addr);

    /*---------------------------------------------------------------
     * call function
     *---------------------------------------------------------------*/
    int rc = bind(sock, (struct sockaddr *)&addr, sizeof(addr));
    // make sure the errno variables are set
    setErrno(context, rc >= 0);
    return rc;
}


/*-/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\-*/
/*-\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/-*/

/*------------------------------------------------------------------
 * close()
 *------------------------------------------------------------------*/
RexxRoutine1(int, SockClose, int, sock)
{
    /*---------------------------------------------------------------
     * call function
     *---------------------------------------------------------------*/
#if defined(WIN32)
    int rc = closesocket(sock);
#else
    int rc = close(sock);
#endif
    // set the errno information
    setErrno(context, rc >= 0);

    return rc;
}

/*-/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\-*/
/*-\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/-*/

/*------------------------------------------------------------------
 * connect()
 *------------------------------------------------------------------*/
RexxRoutine2(int, SockConnect, int, sock, RexxObjectPtr, stemSource)
{
    StemManager stem(context);

    if (!stem.resolveStem(stemSource))
    {
        return 0;
    }

    sockaddr_in  addr;

    /*---------------------------------------------------------------
     * get addr
     *---------------------------------------------------------------*/
    stemToSockAddr(context, stem, &addr);

    /*---------------------------------------------------------------
     * call function
     *---------------------------------------------------------------*/
    int rc = connect(sock,(struct sockaddr *)&addr, sizeof(addr));
    // set the errno information
    setErrno(context, rc >= 0);

    return rc;
}

/*-/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\-*/
/*-\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/-*/

/*------------------------------------------------------------------
 * gethostbyaddr()
 *------------------------------------------------------------------*/
RexxRoutine3(int, SockGetHostByAddr, CSTRING, addrArg, RexxObjectPtr, stemSource, OPTIONAL_int, domain)
{
    struct hostent *pHostEnt;
    in_addr addr;

    StemManager stem(context);

    if (!stem.resolveStem(stemSource))
    {
        return 0;
    }

    addr.s_addr = inet_addr(addrArg);

    if (argumentOmitted(3))
    {
        domain = AF_INET;
    }

    /*---------------------------------------------------------------
     * call function
     *---------------------------------------------------------------*/
    pHostEnt = gethostbyaddr((char*)&addr, sizeof(addr), domain);
    // set the errno information
    setErrno(context, pHostEnt != NULL);

    if (!pHostEnt)
    {
        return 0;
    }
    else
    {
        hostEntToStem(context, pHostEnt, stem);
        return 1;
    }
}

/*-/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\-*/
/*-\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/-*/

/*------------------------------------------------------------------
 *  gethostbyname()
 *------------------------------------------------------------------*/
RexxRoutine2(int, SockGetHostByName, CSTRING, name, RexxObjectPtr, stemSource)
{
    StemManager stem(context);

    if (!stem.resolveStem(stemSource))
    {
        return 0;
    }
    struct hostent *pHostEnt;

    /*---------------------------------------------------------------
     * call function
     *---------------------------------------------------------------*/
    pHostEnt = gethostbyname(name);
    // set the errno information
    setErrno(context, pHostEnt != NULL);

    if (!pHostEnt)
    {
        return 0;
    }
    else
    {
        hostEntToStem(context, pHostEnt, stem);
        return 1;
    }
}

/*-/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\-*/
/*-\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/-*/

/*------------------------------------------------------------------
 *  gethostid()
 *------------------------------------------------------------------*/
RexxRoutine0(RexxStringObject, SockGetHostId)
{
    in_addr ia;
#ifdef WIN32
    char     pszBuff[256];                   // hostnames should be 255 chars or less
    PHOSTENT pHostEnt;                       // ptr to hostent structure

    // get our local hostname
    if (gethostname(pszBuff, sizeof(pszBuff)))
    {
        // set the errno information
        setErrno(context, false);
        return context->String("0.0.0.0");
    }
    pszBuff[255] = '\0';                     // belt and braces
    pHostEnt = gethostbyname(pszBuff);       // get our ip address
    if (!pHostEnt)
    {
        // set the errno information
        setErrno(context, false);
        return context->String("0.0.0.0");
    }
    ia.s_addr = (*(uint32_t *)pHostEnt->h_addr);// in network byte order already
    return context->String(inet_ntoa(ia));
#elif defined(HAVE_GETIFADDRS)
    struct ifaddrs* tifaddrs;
    if (getifaddrs(&tifaddrs)) {
      setErrno(context, false);
      return context->String("0.0.0.0");
    }

    for(struct ifaddrs* tia = tifaddrs; tia->ifa_next != NULL; tia = tia->ifa_next) {
      if (tia->ifa_addr != NULL && tia->ifa_addr->sa_family == AF_INET) {
        struct sockaddr_in* si = (struct sockaddr_in*)tia->ifa_addr;
        if(memcmp("127", inet_ntoa(si->sin_addr), 3)) { /* filter out loopbacks */
          ia.s_addr = si->sin_addr.s_addr;
          break;
        }
      }
    }
    freeifaddrs(tifaddrs);
    return context->String(inet_ntoa(ia));
#elif !defined(WIN32)                                 // temporary measure to cater for all *NIXes
#define h_addr h_addr_list[0]                 // NOTE the #else never active -> REVISE

    char   pszBuff[256];                     // hostnames should be 255 chars or less
    struct hostent *pHostEnt;                // ptr to hostent structure

    // get our local hostname
    if (gethostname(pszBuff, sizeof(pszBuff)))
    {
        // set the errno information
        setErrno(context, false);
        return context->String("0.0.0.0");
    }
    pszBuff[255] = '\0';                     // belt and braces
    pHostEnt = gethostbyname(pszBuff);       // get our ip address
    // set the errno information
    if (!pHostEnt)
    {
        setErrno(context, false);
        return context->String("0.0.0.0");
    }
    ia.s_addr = (*(uint32_t *)pHostEnt->h_addr);// in network byte order already
    return context->String(inet_ntoa(ia));
#else
    ia.s_addr = htonl(gethostid());
    // set the errno information
    setErrno(context, true);
    return context->String(inet_ntoa(ia));
#endif
}

/*-/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\-*/
/*-\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/-*/

/*------------------------------------------------------------------
 *  gethostname()
 *------------------------------------------------------------------*/
RexxRoutine0(RexxStringObject, SockGetHostName)
{
    char pszBuff[256];             // host names should be 255 chars or less
    *pszBuff = '\0';

    int rc = gethostname(pszBuff, sizeof(pszBuff));
    pszBuff[255] = '\0';           // belt and braces

    // set the errno information
    setErrno(context, rc >= 0);

    return context->String(pszBuff);
}

/*-/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\-*/
/*-\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/-*/

/*------------------------------------------------------------------
 * getpeername()
 *------------------------------------------------------------------*/
RexxRoutine2(int, SockGetPeerName, int, sock, RexxObjectPtr, stemSource)
{
    StemManager stem(context);

    if (!stem.resolveStem(stemSource))
    {
        return 0;
    }
    sockaddr_in  addr;
    socklen_t    nameLen;

    /*---------------------------------------------------------------
     * call function
     *---------------------------------------------------------------*/
    nameLen = sizeof(addr);
    int rc = getpeername(sock,(struct sockaddr *)&addr,&nameLen);

    // set the errno information
    setErrno(context, rc >= 0);

    /*---------------------------------------------------------------
     * write address to stem
     *---------------------------------------------------------------*/
    sockAddrToStem(context, &addr, stem);

    /*---------------------------------------------------------------
     * set return code
     *---------------------------------------------------------------*/
    return rc;
}

/*-/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\-*/
/*-\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/-*/

/*------------------------------------------------------------------
 *  getsockname()
 *------------------------------------------------------------------*/
RexxRoutine2(int, SockGetSockName, int, sock, RexxObjectPtr, stemSource)
{
    StemManager stem(context);

    if (!stem.resolveStem(stemSource))
    {
        return 0;
    }
    sockaddr_in  addr;
    socklen_t    nameLen;

    /*---------------------------------------------------------------
     * call function
     *---------------------------------------------------------------*/
    nameLen = sizeof(addr);
    int rc = getsockname(sock,(struct sockaddr *)&addr,&nameLen);
    // set the errno information
    setErrno(context, rc >= 0);

    /*---------------------------------------------------------------
     * write address to stem
     *---------------------------------------------------------------*/
    sockAddrToStem(context, &addr, stem);

    /*---------------------------------------------------------------
     * set return code
     *---------------------------------------------------------------*/
    return rc;
}

/*-/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\-*/
/*-\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/-*/

/*------------------------------------------------------------------
 *  getsockopt()
 *------------------------------------------------------------------*/
RexxRoutine4(int, SockGetSockOpt, int, sock, CSTRING, level, CSTRING, option, CSTRING, var)
{
    struct linger  lingStruct;
    socklen_t      len;
    void          *ptr;
    char           buffer[30];
#ifndef WIN32
    struct timeval tv;
#endif


    if (caselessCompare("SOL_SOCKET", level) != 0)
    {
        context->InvalidRoutine();
        return 0;
    }

    /*---------------------------------------------------------------
     * get option name
     *---------------------------------------------------------------*/
    int opt = stringToSockOpt(option);

    /*---------------------------------------------------------------
     * set up buffer
     *---------------------------------------------------------------*/
    int intVal = 0;

    switch (opt)
    {
        case SO_LINGER:
            ptr = &lingStruct;
            len = sizeof(lingStruct);
            break;

#ifndef WIN32
        // on Windows SO_RCVTIMEO and SO_SNDTIMEO expect a milliseconds
        // DWORD argument, whereas on Unix a struct timeval is expected
        case SO_RCVTIMEO:
        case SO_SNDTIMEO:
            ptr = &tv;
            len = sizeof(tv);
            break;
#endif

        default:
            ptr = &intVal;
            len = sizeof(int);
    }

    /*---------------------------------------------------------------
     * make call
     *---------------------------------------------------------------*/
    int rc = getsockopt(sock,SOL_SOCKET,opt,(char *)ptr,&len);

    // set the errno information
    setErrno(context, rc >= 0);

    /*---------------------------------------------------------------
     * set return value
     *---------------------------------------------------------------*/
    switch (opt)
    {
        case SO_LINGER:
            snprintf(buffer, sizeof(buffer), "%d %d", lingStruct.l_onoff, lingStruct.l_linger);
            break;

        case SO_TYPE:
            switch (intVal)
            {
                case SOCK_STREAM: strcpy(buffer,"STREAM"); break;
                case SOCK_DGRAM:  strcpy(buffer,"DGRAM");  break;
                case SOCK_RAW:    strcpy(buffer,"RAW");    break;
                default:          strcpy(buffer,"UNKNOWN");
            }
            break;

#ifndef WIN32
        // on Windows SO_RCVTIMEO and SO_SNDTIMEO expect a milliseconds
        // DWORD argument, whereas on Unix a struct timeval is expected
        case SO_RCVTIMEO:
        case SO_SNDTIMEO:
            snprintf(buffer, sizeof(buffer), "%d", (int)(tv.tv_sec * 1000 + tv.tv_usec / 1000));
            break;
#endif

        default:
            snprintf(buffer, sizeof(buffer), "%d", intVal);
    }

    // set the variable
    context->SetContextVariable(var, context->String(buffer));
    return rc;
}

/*-/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\-*/
/*-\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/-*/

/*------------------------------------------------------------------
 *  ioctl()
 *------------------------------------------------------------------*/
RexxRoutine3(int, SockIoctl, int, sock, CSTRING, command, RexxObjectPtr, var)
{
    int        cmd = 0;
    void      *data;
    int        dataBuff;
    int        len;
    int        rc;

    if (!caselessCompare(command, "FIONBIO"))
    {
        cmd = FIONBIO;
        int32_t temp;

        if (!context->Int32(var, &temp))
        {
            context->InvalidRoutine();
            return 0;
        }
        dataBuff = (int)temp;
        data     = &dataBuff;
        len      = sizeof(int);
    }
    else if (!caselessCompare(command, "FIONREAD"))
    {
        cmd  = FIONREAD;
        data = &dataBuff;
        len  = sizeof(dataBuff);
    }
    else
    {
        return -1;
    }

    /*---------------------------------------------------------------
     * make call
     *---------------------------------------------------------------*/
#ifdef WIN32
    rc = ioctlsocket(sock,cmd,(u_long *)data);
#else
    rc = ioctl(sock,cmd,data,len);
#endif

    // set the errno information
    setErrno(context, rc >= 0);

    /*---------------------------------------------------------------
     * set output for FIONREAD
     *---------------------------------------------------------------*/
    if (cmd == FIONREAD)
    {
        context->SetContextVariable(context->ObjectToStringValue(var), context->Int32(dataBuff));
    }

    return rc;
}

/*-/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\-*/
/*-\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/-*/

/*------------------------------------------------------------------
 *  listen()
 *------------------------------------------------------------------*/
RexxRoutine2(int, SockListen, int, sock, int, backlog)
{
    /*---------------------------------------------------------------
     * call function
     *---------------------------------------------------------------*/
    int rc = listen(sock, backlog);

    // set the errno information
    setErrno(context, rc >= 0);
    return rc;
}

/*-/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\-*/
/*-\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/-*/

/*------------------------------------------------------------------
 *  recv()
 *------------------------------------------------------------------*/
RexxRoutine4(int, SockRecv, int, sock, CSTRING, var, int, dataLen, OPTIONAL_CSTRING, flagVal)
{
    int       flags;
    long      rc;
    char     *pBuffer;

    /*---------------------------------------------------------------
     * get flags
     *---------------------------------------------------------------*/
    flags = 0;
    if (flagVal != NULL)
    {
        char *flagStr = strdup(flagVal);
        if (flagStr == NULL)
        {
            context->InvalidRoutine();
            return 0;
        }
        const char *pszWord = strtok(flagStr, " ");
        while (pszWord)
        {
            if (!caselessCompare(pszWord,"MSG_OOB"))  flags |= MSG_OOB;
            else if (!caselessCompare(pszWord,"MSG_PEEK")) flags |= MSG_PEEK;
            pszWord = strtok(NULL," ");
        }
        free(flagStr);
    }

    /*---------------------------------------------------------------
     * allocate memory for data
     *---------------------------------------------------------------*/
    pBuffer = (char *)malloc(dataLen);
    if (!pBuffer)
    {
        context->InvalidRoutine();
        return 0;
    }

    /*---------------------------------------------------------------
     * call function
     *---------------------------------------------------------------*/
    rc = recv(sock, pBuffer, dataLen, flags);

    // set the errno information
    setErrno(context, rc >= 0);

    if (-1 == rc)
    {
        dataLen = 0;
    }
    else
    {
        dataLen = rc;
    }

    context->SetContextVariable(var, context->String(pBuffer, dataLen));

    free(pBuffer);

    /*---------------------------------------------------------------
     * set return code
     *---------------------------------------------------------------*/
    return rc;
}


/*-/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\-*/
/*-\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/-*/

/*------------------------------------------------------------------
 *  recvfrom()
 *------------------------------------------------------------------*/
RexxRoutine5(int, SockRecvFrom, int, sock, CSTRING, var, int, dataLen, RexxObjectPtr, flagArg, OPTIONAL_RexxObjectPtr, stemSource)
{
    StemManager stem(context);
    sockaddr_in addr;
    socklen_t   addr_size;


    /*---------------------------------------------------------------
     * get flags
     *---------------------------------------------------------------*/
    int flags = 0;
    // if we have a 5th argument, then the 4th argument is a flag value
    if (stemSource != NULL)
    {
        if (!stem.resolveStem(stemSource))
        {
            return 0;
        }

        char *flagStr = strdup(context->ObjectToStringValue(flagArg));

        const char *pszWord = strtok(flagStr, " ");
        while (pszWord)
        {
            if (!caselessCompare(pszWord,"MSG_OOB"))
            {
                flags |= MSG_OOB;
            }
            else if (!caselessCompare(pszWord,"MSG_PEEK"))
            {
                flags |= MSG_PEEK;
            }
            pszWord = strtok(NULL," ");
        }
        free(flagStr);
    }
    else
    {
        // the 4th argument is a stem variable
        if (!stem.resolveStem(flagArg))
        {
            return 0;
        }
    }

    stemToSockAddr(context, stem, &addr);
    addr_size=sizeof(addr);

    /*---------------------------------------------------------------
     * allocate memory for data
     *---------------------------------------------------------------*/
    char *pBuffer = (char *)malloc(dataLen);
    if (!pBuffer)
    {
        context->InvalidRoutine();
        return 0;
    }

    /*---------------------------------------------------------------
     * call function
     *---------------------------------------------------------------*/
    int rc = recvfrom(sock,pBuffer,dataLen,flags,(struct sockaddr *)&addr,&addr_size);

    // set the errno information
    setErrno(context, rc >= 0);

    if (-1 == rc)
    {
        dataLen = 0;
    }
    else
    {
        dataLen = rc;
    }

    sockAddrToStem(context, &addr, stem);

    context->SetContextVariable(var, context->String(pBuffer, dataLen));

    free(pBuffer);

    /*---------------------------------------------------------------
     * set return code
     *---------------------------------------------------------------*/
    return rc;
}



/*-/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\-*/
/*-\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/-*/
/*------------------------------------------------------------------
 *  select()
 *------------------------------------------------------------------*/
RexxRoutine4(int, SockSelect, OPTIONAL_RexxObjectPtr, array1, OPTIONAL_RexxObjectPtr, array2, OPTIONAL_RexxObjectPtr, array3, OPTIONAL_float, timeout)
{
    struct timeval  timeOutS;
    struct timeval *timeOutP;
    int             rCount = 0;
    int             wCount = 0;
    int             eCount = 0;
    int            *rArray = NULL;
    int            *wArray = NULL;
    int            *eArray = NULL;
    int             i;
    int             j;
    int             rc;
    fd_set   rSetS, *rSet = &rSetS;
    fd_set   wSetS, *wSet = &wSetS;
    fd_set   eSetS, *eSet = &eSetS;
    int             max;

    /*---------------------------------------------------------------
     * get timeout value
     *---------------------------------------------------------------*/
    if (argumentOmitted(4))
    {
        timeOutP = NULL;
    }
    else
    {
        if (timeout < 0)
        {
            timeout = 0;
        }

        timeOutS.tv_sec  = (int)timeout;
        timeOutS.tv_usec = (int)((timeout - timeOutS.tv_sec) * 1000000);
        timeOutP = &timeOutS;
    }

    /*---------------------------------------------------------------
     * get arrays of sockets
     *---------------------------------------------------------------*/
    stemToIntArray(context, array1, rCount, rArray);
    stemToIntArray(context, array2, wCount, wArray);
    stemToIntArray(context, array3, eCount, eArray);

/*------------------------------------------------------------------
 * unix-specific stuff
 *------------------------------------------------------------------*/
    /*---------------------------------------------------------------
     * fill in fd_set's
     *---------------------------------------------------------------*/
    FD_ZERO(rSet);
    FD_ZERO(wSet);
    FD_ZERO(eSet);

    for (i=0; i<rCount; i++)
    {
        FD_SET(rArray[i],rSet);
    }
    for (i=0; i<wCount; i++)
    {
        FD_SET(wArray[i],wSet);
    }
    for (i=0; i<eCount; i++)
    {
        FD_SET(eArray[i],eSet);
    }

    /*---------------------------------------------------------------
     * get max number
     *---------------------------------------------------------------*/
    max = 0;
    for (i=0; i<rCount; i++)
    {
        if (rArray[i] > max)
        {
            max = rArray[i];
        }
    }

    for (i=0; i<wCount; i++)
    {
        if (wArray[i] > max)
        {
            max = wArray[i];
        }
    }

    for (i=0; i<eCount; i++)
    {
        if (eArray[i] > max)
        {
            max = eArray[i];
        }
    }

        /*---------------------------------------------------------------
         * make the call
         *---------------------------------------------------------------*/
    rc = select(max+1,rSet,wSet,eSet,timeOutP);

    // set the errno information
    setErrno(context, rc >= 0);

    /*---------------------------------------------------------------
     * fix up the socket arrays
     *---------------------------------------------------------------*/
    if (rc != 0)
    {
        j = 0;
        for (i=0; i<rCount; i++)
        {
            if (FD_ISSET(rArray[i],rSet))
            {
                rArray[j] = rArray[i];
                j++;
            }
        }
        rCount = j;

        j = 0;
        for (i=0; i<wCount; i++)
        {
            if (FD_ISSET(wArray[i],wSet))
            {
                wArray[j] = wArray[i];
                j++;
            }
        }
        wCount = j;

        j = 0;
        for (i=0; i<eCount; i++)
        {
            if (FD_ISSET(eArray[i],eSet))
            {
                eArray[j] = eArray[i];
                j++;
            }
        }
        eCount = j;
    }


    /*---------------------------------------------------------------
     * reset the stem variables
     *---------------------------------------------------------------*/
    if (rArray)
    {
        intArrayToStem(context, array1, rCount, rArray);
    }
    if (wArray)
    {
        intArrayToStem(context, array2, wCount, wArray);
    }
    if (eArray)
    {
        intArrayToStem(context, array3, eCount, eArray);
    }

    /*---------------------------------------------------------------
     * free arrays
     *---------------------------------------------------------------*/
    if (rArray)
    {
        free(rArray);
    }
    if (wArray)
    {
        free(wArray);
    }
    if (eArray)
    {
        free(eArray);
    }

    /*---------------------------------------------------------------
     * set return code
     *---------------------------------------------------------------*/
    return rc;
}

/*-/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\-*/
/*-\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/-*/

/*------------------------------------------------------------------
 * send()
 *------------------------------------------------------------------*/
RexxRoutine3(int, SockSend, int, sock, RexxStringObject, dataObj, OPTIONAL_CSTRING, flagArg)
{
    /*---------------------------------------------------------------
     * get data length
     *---------------------------------------------------------------*/
    size_t dataLen = context->StringLength(dataObj);
    const char *data    = context->StringData(dataObj);

    /*---------------------------------------------------------------
     * get flags
     *---------------------------------------------------------------*/
    int flags = 0;
    if (flagArg != NULL)
    {
        char *flagStr = strdup(flagArg);
        if (flagStr == NULL)
        {
            context->InvalidRoutine();
            return 0;
        }

        const char *pszWord = strtok(flagStr, " ");
        while (pszWord)
        {
            if (!caselessCompare(pszWord,"MSG_OOB"))
            {
                flags |= MSG_OOB;
            }
            else if (!caselessCompare(pszWord,"MSG_DONTROUTE"))
            {
                flags |= MSG_DONTROUTE;
            }

            pszWord = strtok(NULL," ");
        }
        free(flagStr);
    }

    /*---------------------------------------------------------------
     * call function
     *---------------------------------------------------------------*/
    // (int) cast avoids C4267 on Windows 64-bit, but potential issue
    int rc = send(sock, data, (int)dataLen, flags);

    // set the errno information
    setErrno(context, rc >= 0);

    /*---------------------------------------------------------------
     * set return code
     *---------------------------------------------------------------*/
    return rc;
}

/*-/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\-*/
/*-\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/-*/

/*------------------------------------------------------------------
 * sendto()
 *------------------------------------------------------------------*/
RexxRoutine4(int, SockSendTo, int, sock, RexxStringObject, dataObj, RexxObjectPtr, flagsOrStem, OPTIONAL_RexxObjectPtr, stemSource)
{
    StemManager stem(context);

    sockaddr_in addr;

    /*---------------------------------------------------------------
     * get data length
     *---------------------------------------------------------------*/
    size_t dataLen = context->StringLength(dataObj);
    const char *data    = context->StringData(dataObj);

    /*---------------------------------------------------------------
     * get flags
     *---------------------------------------------------------------*/
    int flags = 0;
    if (stemSource != NULLOBJECT)
    {
        if (!stem.resolveStem(stemSource))
        {
            return 0;
        }

        char *flagStr = strdup(context->ObjectToStringValue(flagsOrStem));
        if (flagStr == NULL)
        {
            context->InvalidRoutine();
            return 0;
        }

        const char *pszWord = strtok(flagStr, " ");
        while (pszWord)
        {
            if (!caselessCompare(pszWord,"MSG_DONTROUTE"))
            {
                flags |= MSG_DONTROUTE;
            }
            pszWord = strtok(NULL," ");
        }

    }
    else
    {
        if (!stem.resolveStem(flagsOrStem))
        {
            return 0;
        }
    }

    stemToSockAddr(context, stem, &addr);

    /*---------------------------------------------------------------
     * call function
     *---------------------------------------------------------------*/
    // (int) cast avoids C4267 on Windows 64-bit, but potential issue
    int rc = sendto(sock, data, (int)dataLen, flags, (struct sockaddr *)&addr, sizeof(addr));

    // set the errno information
    setErrno(context, rc >= 0);

    /*---------------------------------------------------------------
     * set return code
     *---------------------------------------------------------------*/
    return rc;
}

/*-/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\-*/
/*-\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/-*/
/*------------------------------------------------------------------
 * setsockopt()
 *------------------------------------------------------------------*/
RexxRoutine4(int, SockSetSockOpt, int, sock, CSTRING, target, CSTRING, option, CSTRING, arg)
{
    struct linger  lingStruct;
    int            intVal = 0;
    int            intVal2 = 0;
    socklen_t      lenVal;
    int            len;
    void          *ptr;
#ifndef WIN32
    struct timeval tv;
#endif


    if (caselessCompare("SOL_SOCKET", target))
    {
        context->InvalidRoutine();
        return 0;
    }

    /*---------------------------------------------------------------
     * get option name
     *---------------------------------------------------------------*/
    int opt = stringToSockOpt(option);

    /*---------------------------------------------------------------
     * get option value
     *---------------------------------------------------------------*/
    switch (opt)
    {
        default:
            ptr = &intVal;
            len = sizeof(int);
            sscanf(arg, "%d", &intVal);
            break;

        case SO_LINGER:
            ptr = &lingStruct;
            len = sizeof(lingStruct);

            sscanf(arg,"%d %d", &intVal,&intVal2);
            lingStruct.l_onoff  = (u_short)intVal;
            lingStruct.l_linger = (u_short)intVal2;

            break;

        case SO_RCVBUF:
        case SO_SNDBUF:
            ptr = &lenVal;
            len = sizeof(lenVal);

            sscanf(arg, "%d", &lenVal);
            break;

#ifndef WIN32
        // on Windows SO_RCVTIMEO and SO_SNDTIMEO expect a milliseconds
        // DWORD argument, whereas on Unix a struct timeval is expected
        case SO_RCVTIMEO:
        case SO_SNDTIMEO:
            ptr = &tv;
            len = sizeof(tv);
            sscanf(arg, "%d", &intVal);
            tv.tv_sec = intVal / 1000;
            tv.tv_usec = (intVal - tv.tv_sec * 1000) * 1000;
            break;
#endif

        case SO_ERROR:
        case SO_TYPE:
            return -1;
    }

    /*---------------------------------------------------------------
     * make call
     *---------------------------------------------------------------*/
    int rc = setsockopt(sock,SOL_SOCKET,opt,(const char *)ptr,len);

    // set the errno information
    setErrno(context, rc >= 0);

    /*---------------------------------------------------------------
     * set return code
     *---------------------------------------------------------------*/
    return rc;
}

/*-/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\-*/
/*-\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/-*/

/*------------------------------------------------------------------
 * shutdown()
 *------------------------------------------------------------------*/
RexxRoutine2(int, SockShutDown, int, sock, int, how)
{
    /*---------------------------------------------------------------
     * call function
     *---------------------------------------------------------------*/
    int rc = shutdown(sock, how);

    // set the errno information
    setErrno(context, rc >= 0);

    /*---------------------------------------------------------------
     * set return code
     *---------------------------------------------------------------*/
    return rc;
}

/*-/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\-*/
/*-\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/-*/

/*------------------------------------------------------------------
 *  sock_init()
 *------------------------------------------------------------------*/
RexxRoutine0(int, SockInit)
{
#ifdef WIN32
    WORD wVersionRequested;
    WSADATA wsaData;
    wVersionRequested = MAKEWORD( 1, 1 );
    int rc = WSAStartup( wVersionRequested, &wsaData );
#else
    int rc = 0;
#endif
    // set the errno information
    setErrno(context, rc == 0);

    /*---------------------------------------------------------------
     * set return code
     *---------------------------------------------------------------*/
    return rc;
}

/*-/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\-*/
/*-\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/-*/

/*------------------------------------------------------------------
 * socket()
 *------------------------------------------------------------------*/
RexxRoutine3(int, SockSocket, CSTRING, domainArg, CSTRING, typeArg, CSTRING, protocolArg)
{
    int domain;
    int type;
    int protocol;
    char *pszDomain;
    char *pszType;
    char *pszProtocol;

    /*---------------------------------------------------------------
     * get parms
     *---------------------------------------------------------------*/
    pszDomain   = strdup(domainArg);
    pszType     = strdup(typeArg);
    pszProtocol = strdup(protocolArg);

    stripBlanks(pszDomain);
    stripBlanks(pszType);
    stripBlanks(pszProtocol);

    if (!caselessCompare(pszDomain,"AF_INET"))
    {
        domain = AF_INET;
    }
    else
    {
        context->InvalidRoutine();
        return 0;
    }

    if (!caselessCompare(pszType,"SOCK_STREAM")) type = SOCK_STREAM;
    else if (!caselessCompare(pszType,"SOCK_DGRAM" )) type = SOCK_DGRAM;
    else if (!caselessCompare(pszType,"SOCK_RAW"   )) type = SOCK_RAW;
    else
    {
        context->InvalidRoutine();
        return 0;
    }

    if (!caselessCompare(pszProtocol,"IPPROTO_UDP"))
        protocol = IPPROTO_UDP;
    else if (!caselessCompare(pszProtocol,"IPPROTO_TCP"))
        protocol = IPPROTO_TCP;
    else if (!caselessCompare(pszProtocol,"0"          ))
        protocol = 0;
    else
    {
        context->InvalidRoutine();
        return 0;
    }

    /*---------------------------------------------------------------
     * call function
     *---------------------------------------------------------------*/
    // (int) cast avoids C4244 on Windows 64-bit
    int rc = (int)socket(domain, type, protocol);
    // set the errno information
    setErrno(context, rc >= 0);

    /*---------------------------------------------------------------
     * set return code
     *---------------------------------------------------------------*/
    return rc;
}

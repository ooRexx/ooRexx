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

#if !defined(WIN32)
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
#endif

#if defined(WIN32)                     // define errno equivalents for windows
   #define sock_errno() WSAGetLastError()
   #define psock_errno(s) fprintf(stderr, "RxSOCK Error: %s\n", s)
#endif

#if defined(OPSYS_AIX) || defined(OPSYS_LINUX)
   #define sock_errno() errno
   #define psock_errno(s) printf("\nSocket error %s\n",s)
#endif

/*------------------------------------------------------------------
 * include for this app
 *------------------------------------------------------------------*/
#include "rxsock.h"

/*-/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\-*/
//#if !defined(WIN32)
//extern int h_errno;               // where is this used?
//#endif
/*-\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/-*/

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

/*------------------------------------------------------------------
 * accept()
 *------------------------------------------------------------------*/
RexxRoutine2(int, SockAccept, int, sock, OPTIONAL_RexxStemObject, stem)
{
    sockaddr_in  addr;
    socklen_t    nameLen;

    nameLen = sizeof(addr);
    int rc = accept(sock, (struct sockaddr *)&addr, &nameLen);

    /*---------------------------------------------------------------
     * set addr, if asked for
     *---------------------------------------------------------------*/
    if (stem != NULLOBJECT)
    {
        sockAddrToStem(context, &addr, stem);
    }

    // set the errno variables
    cleanup(context);
    return rc;
}


/*-/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\-*/
/*-\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/-*/

/*------------------------------------------------------------------
 * bind()
 *------------------------------------------------------------------*/
RexxRoutine2(int, SockBind, int, sock, RexxStemObject, stem)
{
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
    cleanup(context);
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
    cleanup(context);

    return rc;
}

/*-/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\-*/
/*-\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/-*/

/*------------------------------------------------------------------
 * connect()
 *------------------------------------------------------------------*/
RexxRoutine2(int, SockConnect, int, sock, RexxStemObject, stem)
{
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
    cleanup(context);

    return rc;
}

/*-/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\-*/
/*-\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/-*/

/*------------------------------------------------------------------
 * gethostbyaddr()
 *------------------------------------------------------------------*/
RexxRoutine3(int, SockGetHostByAddr, CSTRING, addrArg, RexxStemObject, stem, OPTIONAL_int, domain)
{
    struct hostent *pHostEnt;
    unsigned int addr = inet_addr(addrArg);

    if (argumentOmitted(3))
    {
        domain = AF_INET;
    }

    /*---------------------------------------------------------------
     * call function
     *---------------------------------------------------------------*/
    pHostEnt = gethostbyaddr((char*)&addr, sizeof(addr), domain);
    // set the errno information
    cleanup(context);

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
RexxRoutine2(int, SockGetHostByName, CSTRING, name, RexxStemObject, stem)
{
    struct hostent *pHostEnt;

    /*---------------------------------------------------------------
     * call function
     *---------------------------------------------------------------*/
    pHostEnt = gethostbyname(name);
    // set the errno information
    cleanup(context);

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
    char     pszBuff[64];                    // buffer for ip address
    PHOSTENT pHostEnt;                       // ptr to hostent structure
    /*
     *   Retrieve my ip address.  Assuming the hosts file in
     *   in %systemroot%/system/drivers/etc/hosts contains my computer name.
     */                                      //get our name
    if (gethostname(pszBuff, sizeof(pszBuff)))
    {
        // set the errno information
        cleanup(context);
        return context->String("0.0.0.0");
    }
    pHostEnt = gethostbyname(pszBuff);       // get our ip address
    if (!pHostEnt)
    {
        // set the errno information
        cleanup(context);
        return context->String("0.0.0.0");
    }
    ia.s_addr = (*(uint32_t *)pHostEnt->h_addr);// in network byte order already
    return context->String(inet_ntoa(ia));
#else
#if defined(OPSYS_AIX) || defined(OPSYS_LINUX)
#define h_addr h_addr_list[0]

    char     pszBuff[64];                    /* buffer for ip address*/
    struct hostent * pHostEnt;               /* ptr to hostent structure*/

    /*get our name*/
    if (gethostname(pszBuff, sizeof(pszBuff)))
    {
        // set the errno information
        cleanup(context);
        return context->String("0.0.0.0");
    }
    pHostEnt = gethostbyname(pszBuff);     /* get our ip address */
    // set the errno information
    cleanup(context);
    if (!pHostEnt)
    {
        return context->String("0.0.0.0");
    }
    ia.s_addr = (*(uint32_t *)pHostEnt->h_addr);// in network byte order already
    return context->String(inet_ntoa(ia));
#else
    ia.s_addr = htonl(gethostid());
    // set the errno information
    cleanup(context);
    return context->String(inet_ntoa(ia));
#endif
#endif
}

/*-/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\-*/
/*-\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/-*/

/*------------------------------------------------------------------
 * getpeername()
 *------------------------------------------------------------------*/
RexxRoutine2(int, SockGetPeerName, int, sock, RexxStemObject, stem)
{
    sockaddr_in  addr;
    int          rc;
    socklen_t    nameLen;

    /*---------------------------------------------------------------
     * call function
     *---------------------------------------------------------------*/
    nameLen = sizeof(addr);
    rc = getpeername(sock,(struct sockaddr *)&addr,&nameLen);

    // set the errno information
    cleanup(context);

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
RexxRoutine2(int, SockGetSockName, int, sock, RexxStemObject, stem)
{
    sockaddr_in  addr;
    int          rc;
    socklen_t    nameLen;

    /*---------------------------------------------------------------
     * call function
     *---------------------------------------------------------------*/
    nameLen = sizeof(addr);
    rc = getsockname(sock,(struct sockaddr *)&addr,&nameLen);
    // set the errno information
    cleanup(context);

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

        default:
            ptr = &intVal;
            len = sizeof(int);
    }

    /*---------------------------------------------------------------
     * make call
     *---------------------------------------------------------------*/
    int rc = getsockopt(sock,SOL_SOCKET,opt,(char *)ptr,&len);

    // set the errno information
    cleanup(context);

    /*---------------------------------------------------------------
     * set return value
     *---------------------------------------------------------------*/
    switch (opt)
    {
        case SO_LINGER:
            sprintf(buffer,"%d %d", lingStruct.l_onoff, lingStruct.l_linger);
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

        default:
            sprintf(buffer,"%d", intVal);
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
    cleanup(context);

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
    cleanup(context);
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
    cleanup(context);

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
RexxRoutine5(int, SockRecvFrom, int, sock, CSTRING, var, int, dataLen, RexxObjectPtr, flagArg, OPTIONAL_RexxStemObject, stem)
{
    int       rc;
    sockaddr_in addr;
    socklen_t   addr_size;


    /*---------------------------------------------------------------
     * get flags
     *---------------------------------------------------------------*/
    int flags = 0;
    // if we have a 5th argument, then the 4th argument is a flag value
    if (stem != NULL)
    {
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
        stem = context->ResolveStemVariable(flagArg);
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
    rc = recvfrom(sock,pBuffer,dataLen,flags,(struct sockaddr *)&addr,&addr_size);

    // set the errno information
    cleanup(context);

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
RexxRoutine4(int, SockSelect, OPTIONAL_RexxStemObject, array1, OPTIONAL_RexxStemObject, array2, OPTIONAL_RexxStemObject, array3, OPTIONAL_int, timeout)
{
    struct timeval  timeOutS;
    struct timeval *timeOutP;
    int             rCount;
    int             wCount;
    int             eCount;
    int            *rArray;
    int            *wArray;
    int            *eArray;
    int             i;
    int             j;
    int             rc;
#if defined(OPSYS_LINUX)
    fd_set   rSetS, *rSet = &rSetS;
    fd_set   wSetS, *wSet = &wSetS;
    fd_set   eSetS, *eSet = &eSetS;
#else
    struct fd_set   rSetS, *rSet = &rSetS;
    struct fd_set   wSetS, *wSet = &wSetS;
    struct fd_set   eSetS, *eSet = &eSetS;
#endif
    int             max;

    /*---------------------------------------------------------------
     * get timeout value
     *---------------------------------------------------------------*/
    if (argumentOmitted(3))
    {
        timeOutP = NULL;
    }
    else
    {
        if (timeout < 0)
        {
            timeout = 0;
        }

        timeOutS.tv_sec  = timeout;
#if defined(OPSYS_LINUX)
        timeOutS.tv_nsec = 0;
#else
        timeOutS.tv_usec = 0;
#endif
        timeOutP = &timeOutS;
    }

    /*---------------------------------------------------------------
     * get arrays of sockets
     *---------------------------------------------------------------*/
    stemToIntArray(context, array1, rCount, rArray);
    stemToIntArray(context, array2, wCount, wArray);
    stemToIntArray(context, array2, eCount, eArray);

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
    cleanup(context);

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
        intArrayToStem(context, array1,rCount,rArray);
    }
    if (wArray)
    {
        intArrayToStem(context, array2,wCount,wArray);
    }
    if (eArray)
    {
        intArrayToStem(context, array3,eCount,eArray);
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
    int rc = send(sock,data,dataLen,flags);

    // set the errno information
    cleanup(context);

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
RexxRoutine4(int, SockSendTo, int, sock, RexxStringObject, dataObj, RexxObjectPtr, flagsOrStem, OPTIONAL_RexxStemObject, stem)
{
    sockaddr_in addr;

    /*---------------------------------------------------------------
     * get data length
     *---------------------------------------------------------------*/
    int dataLen = context->StringLength(dataObj);
    const char *data    = context->StringData(dataObj);

    /*---------------------------------------------------------------
     * get flags
     *---------------------------------------------------------------*/
    int flags = 0;
    if (stem != NULLOBJECT)
    {
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
        stem = context->ResolveStemVariable(flagsOrStem);
    }

    stemToSockAddr(context, stem, &addr);

    /*---------------------------------------------------------------
     * call function
     *---------------------------------------------------------------*/
    int rc = sendto(sock,data,dataLen,flags,(struct sockaddr *)&addr,sizeof(addr));

    // set the errno information
    cleanup(context);

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
    int            intVal;
    int            intVal2;
    int            len;
    void          *ptr;


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
            ptr = &intVal;
            len = sizeof(intVal);

            sscanf(arg, "%d", &intVal);
            break;

        case SO_ERROR:
        case SO_TYPE:
            return -1;
    }

    /*---------------------------------------------------------------
     * make call
     *---------------------------------------------------------------*/
    int rc = setsockopt(sock,SOL_SOCKET,opt,(const char *)ptr,len);

    // set the errno information
    cleanup(context);

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
    cleanup(context);

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
    cleanup(context);

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
    int rc = socket(domain,type,protocol);
    // set the errno information
    cleanup(context);

    /*---------------------------------------------------------------
     * set return code
     *---------------------------------------------------------------*/
    return rc;
}

/*-/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\-*/
/*-\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/-*/

/*------------------------------------------------------------------
 * soclose()
 *------------------------------------------------------------------*/
RexxRoutine1(int, SockSoClose, int, sock)
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
    cleanup(context);
    return rc;
}


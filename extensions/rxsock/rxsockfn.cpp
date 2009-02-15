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
size_t RexxEntry SockSock_Errno(const char *name, size_t argc, PCONSTRXSTRING argv, const char *qName, PRXSTRING  retStr)
{
    retStr->strlength = 0;
    int2rxs(sock_errno(),retStr);
    return 0;
}

/*-/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\-*/
/*-\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/-*/

/*------------------------------------------------------------------
 * psock_errno()
 *------------------------------------------------------------------*/
size_t RexxEntry SockPSock_Errno(const char *name, size_t argc, PCONSTRXSTRING argv, const char *qName, PRXSTRING  retStr)
{

    retStr->strlength = 0;
    if (argc == 1)
        psock_errno(argv[0].strptr);
    else
        psock_errno("");
    return 0;
}

/*-/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\-*/
/*-\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/-*/

/*------------------------------------------------------------------
 * accept()
 *------------------------------------------------------------------*/
size_t RexxEntry SockAccept(const char *name, size_t argc, PCONSTRXSTRING argv, const char *qName, PRXSTRING  retStr)
{
    sockaddr_in  addr;
    int          sock;
    int          rc;
    socklen_t    nameLen;

    /*---------------------------------------------------------------
     * initialize return value, check parms
     *---------------------------------------------------------------*/
    retStr->strlength = 0;

    if ((argc < 1) || (argc > 2))
        return 40;

    /* check for omitted arguments that might cause a trap*/
    if (!argv[0].strptr || ((argc == 2) && !argv[1].strptr))
        return 40;

    /*---------------------------------------------------------------
     * get sock
     *---------------------------------------------------------------*/
    sock = rxs2int(&(argv[0]),&rc);
    if (!rc)
        return 40;

    /*---------------------------------------------------------------
     * call function
     *---------------------------------------------------------------*/
    nameLen = sizeof(addr);
    rc = accept(sock,(struct sockaddr *)&addr,&nameLen);

    /*---------------------------------------------------------------
     * set addr, if asked for
     *---------------------------------------------------------------*/
    if (2 == argc)
        sockaddr2stem(&addr,argv[1].strptr);

    /*---------------------------------------------------------------
     * set return code
     *---------------------------------------------------------------*/
    int2rxs(rc,retStr);

    return 0;
}

/*-/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\-*/
/*-\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/-*/

/*------------------------------------------------------------------
 * bind()
 *------------------------------------------------------------------*/
size_t RexxEntry SockBind(const char *name, size_t argc, PCONSTRXSTRING argv, const char *qName, PRXSTRING  retStr)
{
    sockaddr_in  addr;
    int          sock;
    int          rc;

    /*---------------------------------------------------------------
     * initialize return value, check parms
     *---------------------------------------------------------------*/
    retStr->strlength = 0;

    if (argc != 2)
        return 40;

    /* check for omitted arguments that might cause a trap*/
    if (!argv[0].strptr || !argv[1].strptr)
        return 40;

    /*---------------------------------------------------------------
     * get sock
     *---------------------------------------------------------------*/
    sock = rxs2int(&(argv[0]),&rc);
    if (!rc)
        return 40;

    /*---------------------------------------------------------------
     * get addr
     *---------------------------------------------------------------*/
    stem2sockaddr(argv[1].strptr,&addr);

    /*---------------------------------------------------------------
     * call function
     *---------------------------------------------------------------*/
    rc = bind(sock,(struct sockaddr *)&addr,sizeof(addr));

    /*---------------------------------------------------------------
     * set return code
     *---------------------------------------------------------------*/
    int2rxs(rc,retStr);

    return 0;
}

/*-/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\-*/
/*-\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/-*/

/*------------------------------------------------------------------
 * close()
 *------------------------------------------------------------------*/
size_t RexxEntry SockClose(const char *name, size_t argc, PCONSTRXSTRING argv, const char *qName, PRXSTRING  retStr)
{
    return SockSoClose(name,argc,argv,qName,retStr);
}

/*-/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\-*/
/*-\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/-*/

/*------------------------------------------------------------------
 * connect()
 *------------------------------------------------------------------*/
size_t RexxEntry SockConnect(const char *name, size_t argc, PCONSTRXSTRING argv, const char *qName, PRXSTRING  retStr)
{
    sockaddr_in  addr;
    int          sock;
    int          rc;

    /*---------------------------------------------------------------
     * initialize return value, check parms
     *---------------------------------------------------------------*/
    retStr->strlength = 0;

    if (argc != 2)
        return 40;

    /* check for omitted arguments that might cause a trap*/
    if (!argv[0].strptr || !argv[1].strptr)
        return 40;

    /*---------------------------------------------------------------
     * get sock
     *---------------------------------------------------------------*/
    sock = rxs2int(&(argv[0]),&rc);
    if (!rc)
        return 40;

    /*---------------------------------------------------------------
     * get addr
     *---------------------------------------------------------------*/
    stem2sockaddr(argv[1].strptr,&addr);

    /*---------------------------------------------------------------
     * call function
     *---------------------------------------------------------------*/
    rc = connect(sock,(struct sockaddr *)&addr,sizeof(addr));

    /*---------------------------------------------------------------
     * set return code
     *---------------------------------------------------------------*/
    int2rxs(rc,retStr);

    return 0;
}

/*-/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\-*/
/*-\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/-*/

/*------------------------------------------------------------------
 * gethostbyaddr()
 *------------------------------------------------------------------*/
size_t RexxEntry SockGetHostByAddr(const char *name, size_t argc, PCONSTRXSTRING argv, const char *qName, PRXSTRING  retStr)
{

    struct hostent *pHostEnt;
    int             domain;
    long            addr;
    int             rc;
    const char *    pszStem;

    /*---------------------------------------------------------------
     * initialize return value to empty string
     *---------------------------------------------------------------*/
    retStr->strlength = 0;

    /*---------------------------------------------------------------
     * get parms
     *---------------------------------------------------------------*/
    if ((argc < 2) | (argc > 3))
        return 40;

    /* check for omitted arguments that might cause a trap*/
    if (!argv[0].strptr || !argv[1].strptr ||
        ((argc == 3) && !argv[2].strptr))
        return 40;

    addr = inet_addr(argv[0].strptr);

    pszStem = argv[1].strptr;

    if (2 == argc)
        domain = AF_INET;
    else
        domain = rxs2int(&(argv[2]),&rc);

    /*---------------------------------------------------------------
     * call function
     *---------------------------------------------------------------*/
    pHostEnt = gethostbyaddr((char*)&addr,sizeof(addr),domain);

    if (!pHostEnt)
        int2rxs(0,retStr);

    else
    {
        hostent2stem(pHostEnt,pszStem);
        int2rxs(1,retStr);
    }

    return 0;
}

/*-/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\-*/
/*-\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/-*/

/*------------------------------------------------------------------
 *  gethostbyname()
 *------------------------------------------------------------------*/
size_t RexxEntry SockGetHostByName(const char *name, size_t argc, PCONSTRXSTRING argv, const char *qName, PRXSTRING  retStr)
{
    struct hostent *pHostEnt;
    const char *    pszName;
    const char *    pszStem;

    /*---------------------------------------------------------------
     * initialize return value to empty string
     *---------------------------------------------------------------*/
    retStr->strlength = 0;

    /*---------------------------------------------------------------
     * get parms
     *---------------------------------------------------------------*/
    if (argc != 2)
        return 40;

    pszName = argv[0].strptr;
    pszStem = argv[1].strptr;

    /* check for omitted arguments that might cause a trap*/
    if (!pszName || !pszStem || !argv[0].strlength || !argv[1].strlength)
        return 40;

    /*---------------------------------------------------------------
     * call function
     *---------------------------------------------------------------*/
    pHostEnt = gethostbyname(pszName);

    if (!pHostEnt)
        int2rxs(0,retStr);

    else
    {
        hostent2stem(pHostEnt,pszStem);
        int2rxs(1,retStr);
    }

    return 0;
}

/*-/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\-*/
/*-\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/-*/

/*------------------------------------------------------------------
 *  gethostid()
 *------------------------------------------------------------------*/
size_t RexxEntry SockGetHostId(const char *name, size_t argc, PCONSTRXSTRING argv, const char *qName, PRXSTRING  retStr)
{
    in_addr ia;
    char    *addr;

#ifdef WIN32
    char     pszBuff[64];                    // buffer for ip address
    PHOSTENT pHostEnt;                       // ptr to hostent structure
    /*
     *   Retrieve my ip address.  Assuming the hosts file in
     *   in %systemroot%/system/drivers/etc/hosts contains my computer name.
     */                                      //get our name
    if (gethostname(pszBuff, sizeof(pszBuff)))
    {
        strcpy(retStr->strptr,"0.0.0.0");
        retStr->strlength = strlen(retStr->strptr);
        return 0;
    }
    pHostEnt = gethostbyname(pszBuff);       // get our ip address
    if (!pHostEnt)
    {
        strcpy(retStr->strptr,"0.0.0.0");
        retStr->strlength = strlen(retStr->strptr);
        return 0;
    }
    ia.s_addr = (*(unsigned long *)pHostEnt->h_addr);// in network byte order already
    addr = inet_ntoa(ia);
#else
#if defined(OPSYS_AIX) || defined(OPSYS_LINUX)
#define h_addr h_addr_list[0]

    char     pszBuff[64];                    /* buffer for ip address*/
    struct hostent * pHostEnt;               /* ptr to hostent structure*/

    /*get our name*/
    if (gethostname(pszBuff, sizeof(pszBuff)))
    {
        strcpy(retStr->strptr,"0.0.0.0");
        retStr->strlength = strlen(retStr->strptr);
        return 0;
    }
    pHostEnt = gethostbyname(pszBuff);     /* get our ip address */
    if (!pHostEnt)
    {
        strcpy(retStr->strptr,"0.0.0.0");
        retStr->strlength = strlen(retStr->strptr);
        return 0;
    }
    ia.s_addr = (*(unsigned long *)pHostEnt->h_addr);// in network byte order already
    addr = inet_ntoa(ia);
#else
    ia.s_addr = htonl(gethostid());
    addr = inet_ntoa(ia);
#endif
#endif

    sprintf(retStr->strptr,"%s",addr);
    retStr->strlength = strlen(retStr->strptr);

    return 0;
}

/*-/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\-*/
/*-\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/-*/

/*------------------------------------------------------------------
 * getpeername()
 *------------------------------------------------------------------*/
size_t RexxEntry SockGetPeerName(const char *name, size_t argc, PCONSTRXSTRING argv, const char *qName, PRXSTRING  retStr)
{
    sockaddr_in  addr;
    int          sock;
    int          rc;
    socklen_t    nameLen;

    /*---------------------------------------------------------------
     * initialize return value, check parms
     *---------------------------------------------------------------*/
    retStr->strlength = 0;

    if (argc != 2)
        return 40;

    /* check for omitted arguments that might cause a trap*/
    if (!argv[0].strptr || !argv[1].strptr || !argv[1].strlength)
        return 40;

    /*---------------------------------------------------------------
     * get sock
     *---------------------------------------------------------------*/
    sock = rxs2int(&(argv[0]),&rc);
    if (!rc)
        return 40;

    /*---------------------------------------------------------------
     * call function
     *---------------------------------------------------------------*/
    nameLen = sizeof(addr);
    rc = getpeername(sock,(struct sockaddr *)&addr,&nameLen);

    /*---------------------------------------------------------------
     * write address to stem
     *---------------------------------------------------------------*/
    sockaddr2stem(&addr,argv[1].strptr);

    /*---------------------------------------------------------------
     * set return code
     *---------------------------------------------------------------*/
    int2rxs(rc,retStr);

    return 0;
}

/*-/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\-*/
/*-\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/-*/

/*------------------------------------------------------------------
 *  getsockname()
 *------------------------------------------------------------------*/
size_t RexxEntry SockGetSockName(const char *name, size_t argc, PCONSTRXSTRING argv, const char *qName, PRXSTRING  retStr)
{
    sockaddr_in  addr;
    int          sock;
    int          rc;
    socklen_t    nameLen;

    /*---------------------------------------------------------------
     * initialize return value, check parms
     *---------------------------------------------------------------*/
    retStr->strlength = 0;

    if (argc != 2)
        return 40;

    /* check for omitted arguments that might cause a trap*/
    if (!argv[0].strptr || !argv[1].strptr || !argv[1].strlength)
        return 40;

    /*---------------------------------------------------------------
     * get sock
     *---------------------------------------------------------------*/
    sock = rxs2int(&(argv[0]),&rc);
    if (!rc)
        return 40;

    /*---------------------------------------------------------------
     * call function
     *---------------------------------------------------------------*/
    nameLen = sizeof(addr);
    rc = getsockname(sock,(struct sockaddr *)&addr,&nameLen);

    /*---------------------------------------------------------------
     * write address to stem
     *---------------------------------------------------------------*/
    sockaddr2stem(&addr,argv[1].strptr);

    /*---------------------------------------------------------------
     * set return code
     *---------------------------------------------------------------*/
    int2rxs(rc,retStr);

    return 0;
}

/*-/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\-*/
/*-\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/-*/

/*------------------------------------------------------------------
 *  getsockopt()
 *------------------------------------------------------------------*/
size_t RexxEntry SockGetSockOpt(const char *name, size_t argc, PCONSTRXSTRING argv, const char *qName, PRXSTRING  retStr)
{
    int            sock;
    int            rc;
    int            opt;
    struct linger  lingStruct;
    int            intVal;
    long           longVal;
    socklen_t      len;
    char          *ptr;
    CONSTRXSTRING  rxVar;
    char           pBuffer[30];
    SHVBLOCK       shv;

    /*---------------------------------------------------------------
     * initialize return value, check parms
     *---------------------------------------------------------------*/
    retStr->strlength = 0;

    if (argc != 4)
        return 40;

    /* check for omitted arguments that might cause a trap*/
    if (!argv[0].strptr || !argv[2].strptr || !argv[1].strlength)
        return 40;

    /*---------------------------------------------------------------
     * get sock
     *---------------------------------------------------------------*/
    sock = rxs2int(&(argv[0]),&rc);
    if (!rc)
        return 40;

    /*---------------------------------------------------------------
     * check level
     *---------------------------------------------------------------*/
    if (!argv[1].strptr)
        return 40;

    if (stricmp("SOL_SOCKET",argv[1].strptr))
        return 40;

    /*---------------------------------------------------------------
     * get option name
     *---------------------------------------------------------------*/
    opt = rxs2SockOpt(argv[2].strptr);

    /*---------------------------------------------------------------
     * get variable name
     *---------------------------------------------------------------*/
    rxVar = argv[3];
    if (!rxVar.strptr || !rxVar.strlength)
        return 40;

    /*---------------------------------------------------------------
     * set up buffer
     *---------------------------------------------------------------*/
    longVal = intVal = 0; /* to eliminate compiler warning */

    switch (opt)
    {
        case SO_LINGER:
            ptr = (char *)&lingStruct;
            len = sizeof(lingStruct);
            break;

        case SO_RCVBUF:
        case SO_SNDBUF:
            ptr = (char *)&longVal;
            len = sizeof(long);
            break;

        default:
            ptr = (char *)&intVal;
            len = sizeof(int);
    }

    /*---------------------------------------------------------------
     * make call
     *---------------------------------------------------------------*/
    rc = getsockopt(sock,SOL_SOCKET,opt,ptr,&len);

    /*---------------------------------------------------------------
     * set return value
     *---------------------------------------------------------------*/
    switch (opt)
    {
        case SO_LINGER:
            sprintf(pBuffer,"%ld %ld",
                    (long) lingStruct.l_onoff, (long) lingStruct.l_linger);
            break;

        case SO_TYPE:
            switch (intVal)
            {
                case SOCK_STREAM: strcpy(pBuffer,"STREAM"); break;
                case SOCK_DGRAM:  strcpy(pBuffer,"DGRAM");  break;
                case SOCK_RAW:    strcpy(pBuffer,"RAW");    break;
                default:          strcpy(pBuffer,"UNKNOWN");
            }
            break;

        case SO_RCVBUF:
        case SO_SNDBUF:
            sprintf(pBuffer,"%ld",(long) longVal);
            break;

        default:
            sprintf(pBuffer,"%ld",(long) intVal);
    }

    /*---------------------------------------------------------------
     * set variable
     *---------------------------------------------------------------*/
    shv.shvcode            = RXSHV_SYSET;
    shv.shvnext            = NULL;
    shv.shvname            = rxVar;
    shv.shvvalue.strptr    = pBuffer;
    shv.shvvalue.strlength = strlen(pBuffer);

    RexxVariablePool(&shv);

    /*---------------------------------------------------------------
     * set return code
     *---------------------------------------------------------------*/
    int2rxs(rc,retStr);

    return 0;
}

/*-/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\-*/
/*-\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/-*/

/*------------------------------------------------------------------
 *  ioctl()
 *------------------------------------------------------------------*/
size_t RexxEntry SockIoctl(const char *name, size_t argc, PCONSTRXSTRING argv, const char *qName, PRXSTRING  retStr)
{
    int        sock;
    int        cmd;
    void      *data;
    int        dataBuff;
    int        len;
    int        rc;
    SHVBLOCK   shv;
    char       pBuffer[20];

    /*---------------------------------------------------------------
     * initialize return value, check parms
     *---------------------------------------------------------------*/
    retStr->strlength = 0;

    if (argc != 3)
        return 40;

    /* check for omitted arguments that might cause a trap*/
    if (!argv[0].strptr)
        return 40;

    /*---------------------------------------------------------------
     * get sock
     *---------------------------------------------------------------*/
    sock = rxs2int(&(argv[0]),&rc);
    if (!rc)
        return 40;

    /*---------------------------------------------------------------
     * get command and data
     *---------------------------------------------------------------*/
    if (!argv[1].strptr || !argv[1].strlength)
        return 40;

    if (!argv[2].strptr || !argv[2].strlength)
        return 40;

    cmd = 0; /* to eliminate compiler warning */

    if (!stricmp(argv[1].strptr,"FIONBIO"))
    {
        cmd      = FIONBIO;
        dataBuff = rxs2int(&(argv[2]),&rc);
        data     = &dataBuff;
        len      = sizeof(int);
    }

    else if (!stricmp(argv[1].strptr,"FIONREAD"))
    {
        cmd  = FIONREAD;
        data = &dataBuff;
        len  = sizeof(dataBuff);
    }

    else
    {
        strcpy(retStr->strptr,"-1");
        retStr->strlength = strlen(retStr->strptr);
        return 0;
    }

    /*---------------------------------------------------------------
     * make call
     *---------------------------------------------------------------*/
#ifdef WIN32
    rc = ioctlsocket(sock,cmd,(u_long *)data);
#else
    rc = ioctl(sock,cmd,data,len);
#endif

    /*---------------------------------------------------------------
     * set output for FIONREAD
     *---------------------------------------------------------------*/
    if (cmd == FIONREAD)
    {
        sprintf(pBuffer,"%ld",(long) dataBuff);

        shv.shvcode            = RXSHV_SYSET;
        shv.shvnext            = NULL;
        shv.shvname.strptr     = argv[2].strptr;
        shv.shvname.strlength  = argv[2].strlength;
        shv.shvvalue.strptr    = pBuffer;
        shv.shvvalue.strlength = strlen(pBuffer);

        RexxVariablePool(&shv);
    }

    /*---------------------------------------------------------------
     * set return code
     *---------------------------------------------------------------*/
    int2rxs(rc,retStr);

    return 0;
}

/*-/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\-*/
/*-\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/-*/

/*------------------------------------------------------------------
 *  listen()
 *------------------------------------------------------------------*/
size_t RexxEntry SockListen(const char *name, size_t argc, PCONSTRXSTRING argv, const char *qName, PRXSTRING  retStr)
{
    int  sock;
    int  rc;
    int  backlog;

    /*---------------------------------------------------------------
     * initialize return value, check parms
     *---------------------------------------------------------------*/
    retStr->strlength = 0;

    if (argc != 2)
        return 40;

    /* check for omitted arguments that might cause a trap*/
    if (!argv[0].strptr || !argv[1].strptr)
        return 40;

    /*---------------------------------------------------------------
     * get sock
     *---------------------------------------------------------------*/
    sock = rxs2int(&(argv[0]),&rc);
    if (!rc)
        return 40;

    /*---------------------------------------------------------------
     * get addr
     *---------------------------------------------------------------*/
    backlog = rxs2int(&(argv[1]),&rc);
    if (!rc)
        return 40;

    /*---------------------------------------------------------------
     * call function
     *---------------------------------------------------------------*/
    rc = listen(sock,backlog);

    /*---------------------------------------------------------------
     * set return code
     *---------------------------------------------------------------*/
    int2rxs(rc,retStr);

    return 0;
}

/*-/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\-*/
/*-\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/-*/

/*------------------------------------------------------------------
 *  recv()
 *------------------------------------------------------------------*/
size_t RexxEntry SockRecv(const char *name, size_t argc, PCONSTRXSTRING argv, const char *qName, PRXSTRING  retStr)
{
    int       sock;
    int       dataLen;
    int       flags;
    CONSTRXSTRING  rxVar;
    int       rc;
    char *    pBuffer;
    SHVBLOCK  shv;
    int       chk;

    /*---------------------------------------------------------------
     * initialize return value to empty string
     *---------------------------------------------------------------*/
    retStr->strlength = 0;

    if ((argc < 3) || (argc > 4))
        return 40;

    /* check for omitted arguments that might cause a trap*/
    if (!argv[0].strptr || !argv[1].strptr || !argv[2].strptr ||
        ((argc == 4) && (!argv[3].strptr || !argv[3].strlength)))
        return 40;

    /*---------------------------------------------------------------
     * get sock
     *---------------------------------------------------------------*/
    sock = rxs2int(&(argv[0]),&chk);
    if (!chk)
        return 40;

    /*---------------------------------------------------------------
     * get variable name
     *---------------------------------------------------------------*/
    rxVar = argv[1];

    /*---------------------------------------------------------------
     * get data length
     *---------------------------------------------------------------*/
    dataLen = rxs2int(&(argv[2]),&chk);
    if (!chk)
        return 40;

    /*---------------------------------------------------------------
     * get flags
     *---------------------------------------------------------------*/
    flags = 0;
    if (4 == argc)
    {
        const char *pszWord;

        // strtok modifies the tokenized string.  That's against the rules of
        // usage here, so we need to make a copy first.
        char *flagStr = strdup(argv[3].strptr);

        pszWord = strtok(flagStr," ");
        while (pszWord)
        {
            if (!stricmp(pszWord,"MSG_OOB"))  flags |= MSG_OOB;
            else if (!stricmp(pszWord,"MSG_PEEK")) flags |= MSG_PEEK;
            pszWord = strtok(NULL," ");
        }
        free(flagStr);
    }

    /*---------------------------------------------------------------
     * allocate memory for data
     *---------------------------------------------------------------*/
    pBuffer = (char *)malloc(dataLen);
    if (!pBuffer)
        return 5;

    /*---------------------------------------------------------------
     * call function
     *---------------------------------------------------------------*/
    rc = recv(sock,pBuffer,dataLen,flags);

    if (-1 == rc)
        dataLen = 0;
    else
        dataLen = rc;
    /*---------------------------------------------------------------
     * set variable
     *---------------------------------------------------------------*/
    shv.shvcode            = RXSHV_SYSET;
    shv.shvnext            = NULL;
    shv.shvname            = rxVar;
    shv.shvvalue.strptr    = pBuffer;
    shv.shvvalue.strlength = dataLen;

    RexxVariablePool(&shv);

    free(pBuffer);

    /*---------------------------------------------------------------
     * set return code
     *---------------------------------------------------------------*/
    int2rxs(rc,retStr);

    return 0;
}

/*-/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\-*/
/*-\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/-*/

/*------------------------------------------------------------------
 *  recvfrom()
 *------------------------------------------------------------------*/
size_t RexxEntry SockRecvFrom(const char *name, size_t argc, PCONSTRXSTRING argv, const char *qName, PRXSTRING  retStr)
{
    int       sock;
    int       dataLen;
    int       flags;
    CONSTRXSTRING  rxVar;
    int       rc;
    char     *pBuffer;
    const char *pStem;
    SHVBLOCK  shv;
    int       chk;
    sockaddr_in addr;
    socklen_t   addr_size;

    /*---------------------------------------------------------------
     * initialize return value to empty string
     *---------------------------------------------------------------*/
    retStr->strlength = 0;

    if ((argc < 4) || (argc > 5))
        return 40;

    /* check for omitted arguments that might cause a trap*/
    if (!argv[0].strptr || !argv[1].strptr || !argv[2].strptr ||
        !argv[3].strptr || !argv[3].strlength ||
        ((argc == 5) && (!argv[4].strptr || !argv[4].strlength)))
        return 40;

    /*---------------------------------------------------------------
     * get sock
     *---------------------------------------------------------------*/
    sock = rxs2int(&(argv[0]),&chk);
    if (!chk)
        return 40;

    /*---------------------------------------------------------------
     * get variable name
     *---------------------------------------------------------------*/
    rxVar = argv[1];

    /*---------------------------------------------------------------
     * get data length
     *---------------------------------------------------------------*/
    dataLen = rxs2int(&(argv[2]),&chk);
    if (!chk)
        return 40;

    /*---------------------------------------------------------------
     * get flags
     *---------------------------------------------------------------*/
    flags = 0;
    if (5 == argc)
    {
        const char *pszWord;

        // strtok modifies the tokenized string.  That's against the rules of
        // usage here, so we need to make a copy first.
        char *flagStr = strdup(argv[3].strptr);

        pszWord = strtok(flagStr," ");
        while (pszWord)
        {
            if (!stricmp(pszWord,"MSG_OOB"))  flags |= MSG_OOB;
            else if (!stricmp(pszWord,"MSG_PEEK")) flags |= MSG_PEEK;

            pszWord = strtok(NULL," ");
        }
        free(flagStr);
    }
    /*---------------------------------------------------------------
     * get address
     *---------------------------------------------------------------*/

    if (argc == 5)
        pStem=argv[4].strptr;
    else
        pStem=argv[3].strptr;
    stem2sockaddr(pStem,&addr);
    addr_size=sizeof(addr);

    /*---------------------------------------------------------------
     * allocate memory for data
     *---------------------------------------------------------------*/
    pBuffer = (char *)malloc(dataLen);
    if (!pBuffer)
        return 5;

    /*---------------------------------------------------------------
     * call function
     *---------------------------------------------------------------*/
    rc = recvfrom(sock,pBuffer,dataLen,flags,(struct sockaddr *)&addr,&addr_size);

    if (-1 == rc)
        dataLen = 0;
    else
        dataLen = rc;


    sockaddr2stem(&addr,pStem);

    /*---------------------------------------------------------------
     * set variable
     *---------------------------------------------------------------*/
    shv.shvcode            = RXSHV_SYSET;
    shv.shvnext            = NULL;
    shv.shvname            = rxVar;
    shv.shvvalue.strptr    = pBuffer;
    shv.shvvalue.strlength = dataLen;

    RexxVariablePool(&shv);

    free(pBuffer);

    /*---------------------------------------------------------------
     * set return code
     *---------------------------------------------------------------*/
    int2rxs(rc,retStr);

    return 0;
}

/*-/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\-*/
/*-\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/-*/
/*------------------------------------------------------------------
 *  select()
 *------------------------------------------------------------------*/
size_t RexxEntry SockSelect(const char *name, size_t argc, PCONSTRXSTRING argv, const char *qName, PRXSTRING  retStr)
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
     * initialize return value to empty string
     *---------------------------------------------------------------*/
    retStr->strlength = 0;

    if ((argc < 3) || (argc > 4))
        return 40;

    /*---------------------------------------------------------------
     * get timeout value
     *---------------------------------------------------------------*/
    if ((argc == 3) || !argv[3].strptr || !argv[3].strlength)
        timeOutP = NULL;

    else
    {
        long to;

        to = strtol(argv[3].strptr,NULL,10);

        if (to < 0)
            to = 0;

        timeOutS.tv_sec  = to;
        timeOutS.tv_usec = 0;
        timeOutP = &timeOutS;
    }

    /*---------------------------------------------------------------
     * get arrays of sockets
     *---------------------------------------------------------------*/
    if (argv[0].strptr && argv[0].strlength)
        rxstem2intarray(&(argv[0]),&rCount,&rArray);
    else
    {
        rCount = 0;
        rArray = NULL;
    }

    if (argv[1].strptr && argv[1].strlength)
        rxstem2intarray(&(argv[1]),&wCount,&wArray);
    else
    {
        wCount = 0;
        wArray = NULL;
    }

    if (argv[2].strptr && argv[2].strlength)
        rxstem2intarray(&(argv[2]),&eCount,&eArray);
    else
    {
        eCount = 0;
        eArray = NULL;
    }

/*------------------------------------------------------------------
 * unix-specific stuff
 *------------------------------------------------------------------*/
    /*---------------------------------------------------------------
     * fill in fd_set's
     *---------------------------------------------------------------*/
    FD_ZERO(rSet);
    FD_ZERO(wSet);
    FD_ZERO(eSet);

    for (i=0; i<rCount; i++) FD_SET(rArray[i],rSet);
    for (i=0; i<wCount; i++) FD_SET(wArray[i],wSet);
    for (i=0; i<eCount; i++) FD_SET(eArray[i],eSet);

    /*---------------------------------------------------------------
     * get max number
     *---------------------------------------------------------------*/
    max = 0;
    for (i=0; i<rCount; i++) if (rArray[i] > max) max = rArray[i];
    for (i=0; i<wCount; i++) if (wArray[i] > max) max = wArray[i];
    for (i=0; i<eCount; i++) if (eArray[i] > max) max = eArray[i];

        /*---------------------------------------------------------------
         * make the call
         *---------------------------------------------------------------*/
    rc = select(max+1,rSet,wSet,eSet,timeOutP);

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
    if (rArray) intarray2rxstem(&(argv[0]),rCount,rArray);
    if (wArray) intarray2rxstem(&(argv[1]),wCount,wArray);
    if (eArray) intarray2rxstem(&(argv[2]),eCount,eArray);

    /*---------------------------------------------------------------
     * free arrays
     *---------------------------------------------------------------*/
    if (rArray) free(rArray);
    if (wArray) free(wArray);
    if (eArray) free(eArray);

    /*---------------------------------------------------------------
     * set return code
     *---------------------------------------------------------------*/
    int2rxs(rc,retStr);

    return 0;
}

/*-/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\-*/
/*-\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/-*/

/*------------------------------------------------------------------
 * send()
 *------------------------------------------------------------------*/
size_t RexxEntry SockSend(const char *name, size_t argc, PCONSTRXSTRING argv, const char *qName, PRXSTRING  retStr)
{
    int      sock;
    size_t   dataLen;
    const char *data;
    int      flags;
    RexxReturnCode   rc;
    int      chk;

    /*---------------------------------------------------------------
     * initialize return value to empty string
     *---------------------------------------------------------------*/
    retStr->strlength = 0;

    if ((argc < 2) || (argc > 3))
        return 40;

    /* check for omitted arguments that might cause a trap*/
    if (!argv[0].strptr ||
        ((argc == 3) && (!argv[2].strptr || !argv[2].strlength)))
        return 40;

    /*---------------------------------------------------------------
     * get sock
     *---------------------------------------------------------------*/
    sock = rxs2int(&(argv[0]),&chk);
    if (!chk)
        return 40;

    /*---------------------------------------------------------------
     * get data length
     *---------------------------------------------------------------*/
    dataLen = argv[1].strlength;
    data    = argv[1].strptr;
    if (!data || !dataLen)
        return 40;

    /*---------------------------------------------------------------
     * get flags
     *---------------------------------------------------------------*/
    flags = 0;
    if (3 == argc)
    {
        const char *pszWord;
        // strtok modifies the tokenized string.  That's against the rules of
        // usage here, so we need to make a copy first.
        char *flagStr = strdup(argv[2].strptr);

        pszWord = strtok(flagStr," ");
        while (pszWord)
        {
            if (!stricmp(pszWord,"MSG_OOB"))       flags |= MSG_OOB;
            else if (!stricmp(pszWord,"MSG_DONTROUTE")) flags |= MSG_DONTROUTE;

            pszWord = strtok(NULL," ");
        }
        free(flagStr);
    }

    /*---------------------------------------------------------------
     * call function
     *---------------------------------------------------------------*/
    rc = send(sock,data,dataLen,flags);

    /*---------------------------------------------------------------
     * set return code
     *---------------------------------------------------------------*/
    int2rxs(rc,retStr);

    return 0;
}

/*-/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\-*/
/*-\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/-*/

/*------------------------------------------------------------------
 * sendto()
 *------------------------------------------------------------------*/
size_t RexxEntry SockSendTo(const char *name, size_t argc, PCONSTRXSTRING argv, const char *qName, PRXSTRING  retStr)
{
    int      sock;
    size_t   dataLen;
    const char *data;
    int      flags;
    RexxReturnCode   rc;
    int      chk;
    sockaddr_in addr;
    const char *pStem;

    /*---------------------------------------------------------------
     * initialize return value to empty string
     *---------------------------------------------------------------*/
    retStr->strlength = 0;

    if ((argc < 3) || (argc > 4))
        return 40;

    /* check for omitted arguments that might cause a trap*/
    if (!argv[0].strptr || !argv[2].strptr || !argv[2].strlength ||
        ((argc == 4) && (!argv[3].strptr || !argv[3].strlength)))
        return 40;

    /*---------------------------------------------------------------
     * get sock
     *---------------------------------------------------------------*/
    sock = rxs2int(&(argv[0]),&chk);
    if (!chk)
        return 40;

    /*---------------------------------------------------------------
     * get data length
     *---------------------------------------------------------------*/
    dataLen = argv[1].strlength;
    data    = argv[1].strptr;
    if (!data || !dataLen)
        return 40;

    /*---------------------------------------------------------------
     * get flags
     *---------------------------------------------------------------*/
    flags = 0;
    if (4 == argc)
    {
        const char *pszWord;
        // strtok modifies the tokenized string.  That's against the rules of
        // usage here, so we need to make a copy first.
        char *flagStr = strdup(argv[2].strptr);

        pszWord = strtok(flagStr," ");

        while (pszWord)
        {
            if (!stricmp(pszWord,"MSG_DONTROUTE"))
                flags |= MSG_DONTROUTE;
            pszWord = strtok(NULL," ");
        }
        free(flagStr);
    }

    /*---------------------------------------------------------------
     * get address
     *---------------------------------------------------------------*/

    if (argc == 4)
        pStem=argv[3].strptr;
    else
        pStem=argv[2].strptr;
    stem2sockaddr(pStem,&addr);

    /*---------------------------------------------------------------
     * call function
     *---------------------------------------------------------------*/
    rc = sendto(sock,data,dataLen,flags,(struct sockaddr *)&addr,sizeof(addr));

    /*---------------------------------------------------------------
     * set return code
     *---------------------------------------------------------------*/
    int2rxs(rc,retStr);

    return 0;
}

/*-/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\-*/
/*-\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/-*/
/*------------------------------------------------------------------
 * setsockopt()
 *------------------------------------------------------------------*/
size_t RexxEntry SockSetSockOpt(const char *name, size_t argc, PCONSTRXSTRING argv, const char *qName, PRXSTRING  retStr)
{
    int            sock;
    int            rc;
    int            opt;
    struct linger  lingStruct;
    int            intVal;
    long           longVal;
    long           longVal1;
    long           longVal2;
    int            len;
    char          *ptr;

    /*---------------------------------------------------------------
     * initialize return value, check parms
     *---------------------------------------------------------------*/
    retStr->strlength = 0;

    if (argc != 4)
        return 40;

    /* check for omitted arguments that might cause a trap*/
    if (!argv[0].strptr || !argv[2].strptr ||
        !argv[1].strlength || !argv[2].strlength)
        return 40;

    /*---------------------------------------------------------------
     * get sock
     *---------------------------------------------------------------*/
    sock = rxs2int(&(argv[0]),&rc);
    if (!rc)
        return 40;

    /*---------------------------------------------------------------
     * check level
     *---------------------------------------------------------------*/
    if (!argv[1].strptr)
        return 40;

    if (stricmp("SOL_SOCKET",argv[1].strptr))
        return 40;

    /*---------------------------------------------------------------
     * get option name
     *---------------------------------------------------------------*/
    opt = rxs2SockOpt(argv[2].strptr);

    /*---------------------------------------------------------------
     * check value for a valid string
     *---------------------------------------------------------------*/
    if (!argv[3].strptr || !argv[3].strlength)
        return 40;

    /*---------------------------------------------------------------
     * get option value
     *---------------------------------------------------------------*/
    switch (opt)
    {
        default:
            ptr = (char *)&intVal;
            len = sizeof(int);

            intVal = (int) rxs2int(&(argv[3]),&rc);
            break;

        case SO_LINGER:
            ptr = (char *)&lingStruct;
            len = sizeof(lingStruct);

            sscanf(argv[3].strptr,"%ld %ld",&longVal1,&longVal2);
            lingStruct.l_onoff  = (u_short)longVal1;
            lingStruct.l_linger = (u_short)longVal2;

            break;

        case SO_RCVBUF:
        case SO_SNDBUF:
            ptr = (char *)&longVal;
            len = sizeof(long);

            longVal = rxs2int(&(argv[3]),&rc);
            break;

        case SO_ERROR:
        case SO_TYPE:
            strcpy(retStr->strptr,"-1");
            retStr->strlength = strlen(retStr->strptr);
            return 0;
    }

    /*---------------------------------------------------------------
     * make call
     *---------------------------------------------------------------*/
    rc = setsockopt(sock,SOL_SOCKET,opt,ptr,len);

    /*---------------------------------------------------------------
     * set return code
     *---------------------------------------------------------------*/
    int2rxs(rc,retStr);

    return 0;
}

/*-/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\-*/
/*-\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/-*/

/*------------------------------------------------------------------
 * shutdown()
 *------------------------------------------------------------------*/
size_t RexxEntry SockShutDown(const char *name, size_t argc, PCONSTRXSTRING argv, const char *qName, PRXSTRING  retStr)
{
    int sock;
    int how;
    int rc;

    /*---------------------------------------------------------------
     * initialize return value to empty string
     *---------------------------------------------------------------*/
    retStr->strlength = 0;

    if (argc != 2)
        return 40;

    /* check for omitted arguments that might cause a trap*/
    if (!argv[0].strptr || !argv[1].strptr)
        return 40;

    /*---------------------------------------------------------------
     * get sock
     *---------------------------------------------------------------*/
    sock = rxs2int(&(argv[0]),&rc);
    if (!rc)
        return 40;

    /*---------------------------------------------------------------
     * get how
     *---------------------------------------------------------------*/
    how = rxs2int(&(argv[1]),&rc);
    if (!rc)
        return 40;

    /*---------------------------------------------------------------
     * call function
     *---------------------------------------------------------------*/
    rc = shutdown(sock,how);

    /*---------------------------------------------------------------
     * set return code
     *---------------------------------------------------------------*/
    int2rxs(rc,retStr);

    return 0;
}

/*-/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\-*/
/*-\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/-*/

/*------------------------------------------------------------------
 *  sock_init()
 *------------------------------------------------------------------*/
size_t RexxEntry SockInit(const char *name, size_t argc, PCONSTRXSTRING argv, const char *qName, PRXSTRING  retStr)
{
    int rc;
#ifdef WIN32
    WORD wVersionRequested;
    WSADATA wsaData;
#endif
    /*---------------------------------------------------------------
     * initialize return value to empty string
     *---------------------------------------------------------------*/
    retStr->strlength = 0;

    if (argc)
        return 40;

#if defined(WIN32)
    wVersionRequested = MAKEWORD( 1, 1 );
    rc = WSAStartup( wVersionRequested, &wsaData );
#else
    rc = 0;
#endif

    int2rxs(rc,retStr);

    return 0;
}

/*-/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\-*/
/*-\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/-*/

/*------------------------------------------------------------------
 * socket()
 *------------------------------------------------------------------*/
size_t RexxEntry SockSocket(const char *name, size_t argc, PCONSTRXSTRING argv, const char *qName, PRXSTRING  retStr)
{
    int domain;
    int type;
    int protocol;
    const char *pszDomain;
    const char *pszType;
    const char *pszProtocol;
    int sock;

    /*---------------------------------------------------------------
     * initialize return value to empty string
     *---------------------------------------------------------------*/
    retStr->strlength = 0;

    if (argc != 3)
        return 40;

    /*---------------------------------------------------------------
     * get parms
     *---------------------------------------------------------------*/
    pszDomain   = argv[0].strptr;
    pszType     = argv[1].strptr;
    pszProtocol = argv[2].strptr;

    if (!pszDomain || !pszType || !pszProtocol)
        return 40;

    /* check for omitted arguments that might cause a trap*/
    if (!argv[0].strlength || !argv[1].strlength || !argv[2].strlength)
        return 40;

    if (!stricmp(pszDomain,"AF_INET")) domain = AF_INET;
    else
        return 40;

    if (!stricmp(pszType,"SOCK_STREAM")) type = SOCK_STREAM;
    else if (!stricmp(pszType,"SOCK_DGRAM" )) type = SOCK_DGRAM;
    else if (!stricmp(pszType,"SOCK_RAW"   )) type = SOCK_RAW;
    else
        return 40;

    if (!stricmp(pszProtocol,"IPPROTO_UDP"))
        protocol = IPPROTO_UDP;
    else if (!stricmp(pszProtocol,"IPPROTO_TCP"))
        protocol = IPPROTO_TCP;
/*   else if (!stricmp(pszProtocol,"IPPROTO_ICMP"))
    protocol = IPPROTO_ICMP;
   else if (!stricmp(pszProtocol,"IPPROTO_RAW"))
    protocol = IPPROTO_RAW; */  /* Not supported !! */
    else if (!stricmp(pszProtocol,"0"          ))
        protocol = 0;
    else
        return 40;

    /*---------------------------------------------------------------
     * call function
     *---------------------------------------------------------------*/
    sock = socket(domain,type,protocol);

    /*---------------------------------------------------------------
     * set return code
     *---------------------------------------------------------------*/
    int2rxs(sock,retStr);

    return 0;
}

/*-/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\-*/
/*-\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/-*/

/*------------------------------------------------------------------
 * soclose()
 *------------------------------------------------------------------*/
size_t RexxEntry SockSoClose(const char *name, size_t argc, PCONSTRXSTRING argv, const char *qName, PRXSTRING  retStr)
{
    int sock;
    int rc;

    /*---------------------------------------------------------------
     * initialize return value to empty string
     *---------------------------------------------------------------*/
    retStr->strlength = 0;

    if (argc != 1)
        return 40;

    /* check for omitted arguments that might cause a trap*/
    if (!argv[0].strptr)
        return 40;

    /*---------------------------------------------------------------
     * get sock
     *---------------------------------------------------------------*/
    sock = rxs2int(&(argv[0]),&rc);
    if (!rc)
        return 40;

    /*---------------------------------------------------------------
     * call function
     *---------------------------------------------------------------*/
#if defined(WIN32)
    rc = closesocket(sock);
#else
    rc = close(sock);
#endif

    /*---------------------------------------------------------------
     * set return code
     *---------------------------------------------------------------*/
    int2rxs(rc,retStr);

    return 0;
}


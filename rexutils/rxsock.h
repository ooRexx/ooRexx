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
/* REXX sockets function support                               rxsock.h    */
/*                 sockets utility function package                        */
/***************************************************************************/

/*------------------------------------------------------------------
 * typedef for struct
 *------------------------------------------------------------------*/
typedef struct sockaddr_in sockaddr_in;
typedef struct in_addr     in_addr;

#if defined(WIN32)
typedef int socklen_t;
#endif

/*------------------------------------------------------------------
 * declare external functions
 *------------------------------------------------------------------*/
RexxFunctionHandler SockFunctionGateWay            ;

#if defined(WIN32) || defined(OPSYS_AIX) || defined(OPSYS_LINUX)
RexxFunctionHandler SockLoadFuncs                  ;
#endif

RexxFunctionHandler SockDropFuncs                  ;
RexxFunctionHandler SockVersion                    ;
RexxFunctionHandler SockDie                        ;
RexxFunctionHandler SockException                  ;
RexxFunctionHandler SockAccept                     ;
RexxFunctionHandler SockBind                       ;
RexxFunctionHandler SockClose                      ;
RexxFunctionHandler SockConnect                    ;
RexxFunctionHandler SockGetHostByAddr              ;
RexxFunctionHandler SockGetHostByName              ;
RexxFunctionHandler SockGetHostId                  ;
RexxFunctionHandler SockGetPeerName                ;
RexxFunctionHandler SockGetSockName                ;
RexxFunctionHandler SockGetSockOpt                 ;
RexxFunctionHandler SockInit                       ;
RexxFunctionHandler SockIoctl                      ;
RexxFunctionHandler SockListen                     ;
RexxFunctionHandler SockPSock_Errno                ;
RexxFunctionHandler SockRecv                       ;
RexxFunctionHandler SockRecvFrom                   ;
RexxFunctionHandler SockSelect                     ;
RexxFunctionHandler SockSend                       ;
RexxFunctionHandler SockSendTo                     ;
RexxFunctionHandler SockSetSockOpt                 ;
RexxFunctionHandler SockShutDown                   ;
RexxFunctionHandler SockSock_Errno                 ;
RexxFunctionHandler SockSocket                     ;
RexxFunctionHandler SockSoClose                    ;

/*------------------------------------------------------------------
 * strip blanks from a line
 *------------------------------------------------------------------*/
void StripBlanks(
   char *string
   );

/*------------------------------------------------------------------
 * set a rexx variable
 *------------------------------------------------------------------*/
void RxVarSet(
   PSZ pszStem,
   PSZ pszTail,
   PSZ pszValue
   );

/*------------------------------------------------------------------
 * get a rexx variable - return value must be freed by caller
 *------------------------------------------------------------------*/
PSZ RxVarGet(
   PSZ pszStem,
   PSZ pszTail
   );

/*------------------------------------------------------------------
 * convert a rexx string to a ULONG
 *------------------------------------------------------------------*/
ULONG rxs2ulong(
   PRXSTRING  pRxStr,
   int       *rc
   );

/*------------------------------------------------------------------
 * convert a rexx string to a LONG
 *------------------------------------------------------------------*/
LONG rxs2long(
   PRXSTRING  pRxStr,
   int       *rc
   );

/*------------------------------------------------------------------
 * convert an int to a rexx string (already allocated)
 *------------------------------------------------------------------*/
void int2rxs(
   int        i,
   PRXSTRING  pRxStr
   );

/*------------------------------------------------------------------
 * convert a stem variable to an array of ints
 *------------------------------------------------------------------*/
void rxstem2intarray(
   PRXSTRING   pRxStr,
   int        *count,
   int       **arr
   );

/*------------------------------------------------------------------
 * convert an array of ints to a stem variable
 *------------------------------------------------------------------*/
void intarray2rxstem(
   PRXSTRING   pRxStr,
   int         count,
   int        *arr
   );

/*------------------------------------------------------------------
 * convert a stemmed variable to a sockaddr
 *------------------------------------------------------------------*/
void stem2sockaddr(
   PSZ          pszStem,
   sockaddr_in *pSockAddr
   );

/*------------------------------------------------------------------
 * convert a sockaddr to a stemmed variable
 *------------------------------------------------------------------*/
void sockaddr2stem(
   sockaddr_in *pSockAddr,
   PSZ          pszStem
   );

/*------------------------------------------------------------------
 * convert a hostent to a stemmed variable
 *------------------------------------------------------------------*/
void hostent2stem(
   struct hostent *pHostEnt,
   PSZ             pszStem
   );

/*------------------------------------------------------------------
 * convert a string sock option to an integer
 *------------------------------------------------------------------*/
int rxs2SockOpt(
   PSZ pszOptName
   );

/*------------------------------------------------------------------
 * set errno
 *------------------------------------------------------------------*/
void SetErrno(void);

/*------------------------------------------------------------------
 * set h_errno
 *------------------------------------------------------------------*/
void SetH_Errno(void);

#if defined(OPSYS_AIX) || defined(OPSYS_LINUX)
/*------------------------------------------------------------------
 * string compare ignore upper and lower case
 *------------------------------------------------------------------*/
int stricmp( char *op1, char *op2 );
#endif

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
/*********************************************************************/
/* REXX for UNIX                                            APIDefinitions.h */
/*********************************************************************/

#ifndef AIXAPI_H_INCLUDED
#define AIXAPI_H_INCLUDED
/***    Structure of Rexx API registration block (APIBLOCK) */

#define USERLENGTH  8                  /* Length of saved user data  */
#define MAXNAME     128
#define REGNOOFTYPES 3

typedef struct apireg_node {
    ULONG  next;                       /* number of the next block     */
    CHAR   apiname[MAXNAME];           /* routine name                 */
    CHAR   apidll_name[MAXNAME];       /* module name                  */
    CHAR   apidll_proc[MAXNAME];       /* procedure name               */
    UCHAR  apiuser[USERLENGTH];        /* user area                    */
    PFN    apiaddr;                    /* routine address              */
    LONG   apitype;                    /* 16 or 32-bit flag            */
    PVOID  apimod_handle;              /* dynalink module handle       */
    ULONG  apidrop_auth;               /* Permission to drop           */
    PID    apipid;                     /* Pid of Registrant            */
    PID    apiownpid;                  /* Pid of owner                 */
    LONG   apiFunRegFlag;              /* Main reg set to >0<          */
    } APIBLOCK;

typedef APIBLOCK *PAPIBLOCK;

#define APISIZE      sizeof(APIBLOCK)  /* for size of API Block      */
#define NAMESIZE   0x00ff              /* size of a function name    */
#define SZSTR(x)     (x?strlen(x)+1:0) /* size of allocated string   */
#define MAXARGS               20     /* max # args to func. or proc. */
#define YES                 1
#define NO                  0




#define NAMESIZE   0x00ff              /* size of a function name    */

typedef struct _MACRO {                /****** MACRO structure *******/
      ULONG          next;             /* pointer to next function   */
      CHAR           name[NAMESIZE];   /* function name              */
      RXSTRING       temp_buf;         /* temp  buffer               */
      ULONG          image;            /* pcode+literals image       */
      ULONG          i_size;           /* size of image              */
      ULONG          srch_pos;         /* search order position      */
      } MACRO;                         /******************************/
                                       /******************************/
typedef MACRO *PMACRO;                 /* pointer to MACRO structure */
                                       /******************************/
#define MACROSIZE    sizeof(MACRO)     /* size of MACRO structure    */
#define PMNULL       ((PMACRO *)0)     /* null pointer to PMACRO     */
#define SZSTR(x)     (x?strlen(x)+1:0) /* size of allocated string   */


/* Semaphore control structure. Used for the rexxutil semaphores     */
typedef struct _SEMCONT {
      CHAR    name[MAXNAME];           /* semaphore name             */
      INT     usecount;                /* semaphore usecount         */
      INT     type;                    /* semaphore type: 0=Event    */
                                       /*                 1=Mutex    */
      BOOL    waitandreset;            /* eventsem second new arg    */
} SEMCONT;


#define EVENT 0                        /* event semaphore            */
#define MUTEX 1                        /* mutex semaphore            */

/* ----------------------------------------------------------------- */
/* Prototype use in okstart and rexxapi                              */
LONG RxAPIHOMEset(void);

#endif

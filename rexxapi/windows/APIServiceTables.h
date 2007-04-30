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

#ifndef WINAPI_H_INCLUDED
#define WINAPI_H_INCLUDED
#define USERLENGTH  8                  /* Length of saved user data  */

#include "rexx.h"
#define SZSTR(x)     (x?strlen(x)+1:0) /* size of allocated string   */

/***    Structure of Rexx API registration block (APIBLOCK) */

#define MACRO_ADD 0
#define MACRO_DROP 1
#define MACRO_CLEAR 2
#define MACRO_QUERY 3
#define MACRO_REORDER 4
#define MACRO_EXECUTE 5
#define MACRO_LIST 6


typedef struct apireg_node {
    struct apireg_node *next;          /* pointer to the next block  */
    PSZ    apiname;                    /* routine name               */
    PSZ    apidll_name;                /* module name                */
    PSZ    apidll_proc;                /* procedure name             */
    UCHAR  apiuser[USERLENGTH];        /* user area                  */
    PFN    apiaddr;                    /* routine address            */
    LONG   apitype;                    /* 16 or 32-bit flag          */
    ULONG  apimod_handle;              /* dynalink module handle     */
    ULONG  apidrop_auth;               /* Permission to drop         */
    PID    apipid;                     /* Pid of Registrant          */
    ULONG  apisid;                     /* Session ID.                */
    ULONG  apisize;                    /* actual total size of block */
    PID   *pUserPIDs;                  /* processes using this block */
    ULONG  uPIDBlockSize;              /* size of array of PIDs      */
    } APIBLOCK;

typedef APIBLOCK *PAPIBLOCK;
typedef HANDLE    HAPIBLOCK;           /* Handle of APIBLOCK memory  */

#define APISIZE      sizeof(APIBLOCK)  /* for size of API Block      */

#define NAMESIZE   0x00ff              /* size of a function name    */

typedef struct _MACRO {                /****** MACRO structure *******/
      struct _MACRO *next;             /* pointer to next function   */
      CHAR           name[NAMESIZE];   /* function name              */
      RXSTRING       image;            /* pcode+literals image       */
      ULONG          i_size;           /* size of image              */
      ULONG          srch_pos;         /* search order position      */
      } MACRO;                         /******************************/
                                       /******************************/
typedef MACRO *PMACRO;                 /* pointer to MACRO structure */
                                       /******************************/
#define MACROIMAGE_ABS(cb)    (((LONG)((cb)->image.strptr))? \
                              (PSZ)((PSZ)(cb) + (LONG)((cb)->image.strptr)):\
                              NULL)
#define MACROIMAGE_REL(cb)    (((LONG)((cb)->image.strptr))? \
                                                          (LONG)((cb)->image.strptr)) - (LONG)((PSZ)(cb)):NULL)


#define MACROSIZE    sizeof(MACRO)     /* size of MACRO structure    */
#define PMNULL       ((PMACRO *)0)     /* null pointer to PMACRO     */
#define SZSTR(x)     (x?strlen(x)+1:0) /* size of allocated string   */

#define APIBLOCKNAME(cb)    (((LONG)((cb)->apiname))? \
                            (PSZ)((PSZ)(cb) + (LONG)((cb)->apiname)):\
                            NULL)
#define APIBLOCKDLLNAME(cb) (((LONG)((cb)->apidll_name))? \
                            (PSZ)((PSZ)cb + (LONG)((cb)->apidll_name)): \
                            NULL)
#define APIBLOCKDLLPROC(cb) (((LONG)((cb)->apidll_proc))? \
                            (PSZ)((PSZ)(cb) + (LONG)((cb)->apidll_proc)): \
                            NULL)
#endif

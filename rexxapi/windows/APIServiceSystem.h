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



#ifndef ___RXSYS_H_INCLUDED
#define ___RXSYS_H_INCLUDED


/*-------------------------------------------------------------------*/
/* System Configuration Constants                                    */
/*-------------------------------------------------------------------*/
#ifdef M_I86LM
#define NIL                 (0L)
#else
#define NIL                 0
#endif

/*********************************************************************/
/* Redefine some C standard routines with our replacements           */
/*********************************************************************/

#define MALLOC(s)    sys_malloc(WorkBlock,s)
#define FREE(p)      sys_free(WorkBlock,p)
#define REALLOC(p,s) sys_realloc(WorkBlock,p,s)


/*********************************************************************/
/* Constants for Object REXX                                         */
/*********************************************************************/

 extern int which_system_is_running();

 #define RUNNING_WIN31 (!which_system_is_running()) // returns true only for Windows 3.1
 #define RUNNING_95 (which_system_is_running()==2)  // returns true only for Windows 95
 #define RUNNING_NT (which_system_is_running())     // returns true for Windows NT and Windows 95
 #define WIN32S  0x800000000             // high bit set if running WIN32S on 3.1

/*********************************************************************/
/* Constants for system specific error messages                      */
/*********************************************************************/
#define MSG_PROGRAM_UNREADABLE  3

/*********************************************************************/
/* Integral types for string lengths                                 */
/*********************************************************************/
typedef unsigned short int SLENTYPE;

typedef unsigned short int LENSTRING;  /* # of chars in a data block */

/* Environment */
#define MAXENVSIZE          250
#undef far
#undef near
#define far
#define near
#define _CDECL

/* file handling */

#define LINT_ARGS 1

/* debug capability */

#ifdef DEBUG
#define TRACE0(w) printv(w)
#define TRACE1(w,x) printv(w,x)
#define TRACE2(w,x,y) printv(w,x,y)
#define TRACE3(w,x,y,z) printv(w,x,y,z)
#define TRACE4(w,x,y,z,a) printv(w,x,y,z,a)
#define TRACE5(w,x,y,z,a,b) printv(w,x,y,z,a,b)
#else
#define TRACE0(w)
#define TRACE1(w,x)
#define TRACE2(w,x,y)
#define TRACE3(w,x,y,z)
#define TRACE4(w,x,y,z,a)
#define TRACE5(w,x,y,z,a,b)
#endif


/*********************************************************************/
/* PTRDIFF : C defines the difference of two pointers to be an       */
/* integer. for IBM C2 compiler this means a maximum difference of   */
/* 32k. We need 64k. So use the following macro to give you the      */
/* number of elements between two pointers where the byte difference */
/* can be more than 32k. p1 is the 'bigger' pointer;  p2 is the      */
/* 'smaller' pointer. s is the size of elements these pointers point */
/* to.                                                               */
/* IS_IN_BUFF : Gives a true result if the pointer p is between the  */
/* first pointer f and the last pointer l. This is used because the  */
/* code generated for a simple comparison assumes same segement for  */
/* all pointers. This macro assumes the same segment for f and l but */
/* different for p.                                                  */
/*********************************************************************/

#define ptr_diff(p1,p2,s)   (((ULONG)(p1)-(ULONG)(p2))/(ULONG)(s))
#define ptr_diffc(p1,p2)    ((ULONG)(p1)-(ULONG)(p2))

#define IS_IN_BUFF(f,l,p)   ((f( <= (p) && (p) <= (l))


/*********************************************************************/
/* PCODE : These defines allow the pcode buffer routines to be used  */
/* in expandable buffers and/or linked list mode.                    */
/* PCODE_BUFSIZ   - Number of tokens to allocate per expansion       */
/* PCODE_REALLOCS - Number of times a buffer can be expanded by      */
/*                  PCODE_BUFSIZ amounts.                            */
/* PCODE_LINKS    - Number of links in pcode buffer linked list      */
/*                                                                   */
/* RECOMMENDED VALUES :                                              */
/* ON OS2 : PCODE_BUFSIZ = YYYY, PCODE_REALLOCS=1, PCODE_LINKS= XXXX */
/* ON AS4 : PCODE_BUFSIZ = YYYY, PCODE_REALLOCS=XXXX PCODE_LINKS= 1  */
/* Where (sizeof(PCODE)*YYYY*XXXXX) is less than maximum memory      */
/* available. YYYY can be calculated as 64k/sizeof(PCODE).           */
/*********************************************************************/

#define PCODE_BUFSIZ    8192           /* 64k / sizeof(pcode) i.e 8  */
#define PCODE_REALLOCS  256            /* (16*1024*1024)/(8192*8)    */
#define PCODE_LINKS     (USHORT)1      /* force a single link chain  */
#define PCODE_NOLINKS                  /* force simple macros        */
#define SINGLE_BUFFERS                 /* force flat model code      */

#include "setjmp.h"
#include "stdio.h"
#include "ctype.h"
#include "process.h"

#include "rexx.h"
#include "stdlib.h"
#include "string.h"

typedef int *PINT;                     /* 32BIT DEF USED BY  dlm */


/***    SHVEXIT enable flags (possible value for shvexit_flag and    */
/*      shvexit_value                                                */

#define  DISABLED      0               /* SHVEXIT Disabled           */
#define  FULLY_ENABLED 1               /* SHVEXIT Enabled            */

#define  FULL          1               /* full vpi access            */
#define  PARTIAL       2               /* only SHVEXIT access        */

#define  NO_VALUE      0               /* No value from SHVEXIT      */
#define  VALUE_SET     1               /* SHVEXIT value set          */

/*********************************************************************/
/*  The following definitions are used for external function         */
/*  interfacing:                                                     */
/*********************************************************************/

#define  SLASH        "\\"      /* path element internal separator   */
#define  RXLIBPATH    "PATH"    /* environment path variable         */

#define GIVEN_PATH    0         /* path given in filename            */
#define CURRENT_EXT   1         /* search for current extension      */
#define DEFAULT_EXT   2         /* search for default extension      */
/*-- end --*/

#define SYSID "Windows "

typedef int LJ;

#define PS(p,n)      ((p)+(n))

/* byte order within longs */
#define LBPOS(i)        (i)

#define SHORT_LEN(l)  ((l) && (l) <= 0x10000L)
#define HUGE_LEN(l)   ((l) > 0x10000L)
#define  FULLSEG       65536L          /* 64K constant               */
#define  MAX_SEG       65536L-16       /* maximum single segment     */

/*********************************************************************/
/* CONFIGURATION CONSTANTS                                           */
/*                                                                   */
/* MAXTERMS : Maximum terms waiting to be evaluated on the           */
/*            expression stack.                                      */
/*            Add 1 more to allow for underflow of this stack, and 1 */
/*            more for hidden OP_FENCE.                              */
/*                                                                   */
/* MAXCALL  : Maximum number of calls allowed. Note that this is     */
/*            used to control maximum number of dictionaries also. So*/
/*            allow for labels dictionary as well as variables       */
/*            dictionary.  We will allow for 1 labels dictionary     */
/*            only. Although it will not be enough.                  */
/*                                                                   */
/* MAXCONTROL : Maximum number of active control structures. This is */
/*              the real requirement of the language.                */
/*********************************************************************/

#define MAXLINE              132     /* max linesize (general use)   */
#define MAXTERMS              27     /* max outstanding terms in expr*/
#define MAXCALL              101     /* max CALL nesting/dictionaries*/
#define MAXCONTROL           101     /* max Control structures nestng*/
#define MAX_BIN_LIT_SIZE     100L    /* max size of packed bin literl*/
#define MAX_HEX_LIT_SIZE     250L    /* max size of packed hex literl*/

/*********************************************************************/
/*  The macro is executed at clause boundaries.                      */
/*********************************************************************/

#define SYS_CLAUSE() \
  if (EI_ClauseExit) \
    sys_ExClauseBoundry(WorkBlock);\
  else {\
    if (next_trace_setting == ta_keep_current_setting);\
    else if (next_trace_setting == ta_enable_trace) {\
      ES_Traceflag = 0;\
      traceSet( WorkBlock, &trace_on, YES);\
      next_trace_setting = ta_keep_current_setting;\
    }\
    else {\
      ES_Traceflag = 0;\
      next_trace_setting = ta_keep_current_setting;\
    }\
    if (ES_break_flag)\
      raise_cond(WorkBlock, SIG_HALT,\
        &ES_H_ptr_empty, MSG_PROGRAM_INTERRUPTED);\
  }

#endif

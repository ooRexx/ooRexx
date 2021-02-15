/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2009-2010 Rexx Language Association. All rights reserved.    */
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
/* Authors;                                                                   */
/*       W. David Ashley <dashley@us.ibm.com>                                 */
/*                                                                            */
/*----------------------------------------------------------------------------*/


#ifndef HOSTEMU_INCLUDED
#define HOSTEMU_INCLUDED


/*--------------------------------------------------------------------*/
/*                                                                    */
/* Prototype definitions for external functions                       */
/*                                                                    */
/*--------------------------------------------------------------------*/

int yyparse (
   void);                        /* no arguments                      */
RexxReturnCode RexxEntry GrxHost(PCONSTRXSTRING command,
                                 unsigned short int *flags,
                                 PRXSTRING retc);


/*--------------------------------------------------------------------*/
/*                                                                    */
/* Global definitions                                                 */
/*                                                                    */
/*--------------------------------------------------------------------*/

// Use the following to get debug messages to stdout
// #define HOSTEMU_DEBUG

/* https://www.ibm.com/support/knowledgecenter/SSB27U_6.2.0/com.ibm.zvm.v620.dmsb4/excio.htm
0       Finished correctly
1       Truncated
2       EOF before specified number of lines were read
3       Count ran out without successful pattern match
24      Bad PLIST
31      Error caused a rollback of a shared file(s)
41      Insufficient free storage to load EXECIO
55      APPC/VM communication error
70      SFS sharing conflict
76      SFS authorization error
99      Insufficient virtual storage for file pool repository
1nn     100 + return code from I/O operation (if nonzero)
2008    Variable name supplied on STEM or VAR option was not valid
nnnn    2000 + return code from EXECCOMM command (if nonzero)
x1nnn   1000 + return code from CP command (if nonzero), where x is 0, 1, 2, or 3
1xnnnn  100000 + return code from CP command (if nonzero), where x is 0, 1, 2, or 3 
*/
#define ERR_EXECIO_EOF 2
#define ERR_EXECIO_BAD_PLIST 24
#define ERR_EXECIO_NO_STORAGE 41
#define ERR_EXECIO_VAR_INVALID 2008

#define SYMTABLESIZE 15
#define EXECIO_STMT  0
#define HI_STMT      1
#define TE_STMT      2
#define TS_STMT      3
typedef struct _EXECIO_OPTIONS
   {
   long lRcdCnt;                    /* # of records to be processed   */
   bool fRW;                        /* DISKR or DISKW                 */
   char aFilename [1024];           /* ddname (filename)              */
   char aStem [251];                /* stem variable name             */
   bool fFinis;                     /* FINIS option                   */
   long lStartRcd;                  /* start record number            */
   long lDirection;                 /* FIFO, LIFO or SKIP option      */
   } EXECIO_OPTIONS;


/*--------------------------------------------------------------------*/
/*                                                                    */
/* Extern definitions                                                 */
/*                                                                    */
/*--------------------------------------------------------------------*/

extern const char *   szVersion;
extern const char *   szDate;
extern EXECIO_OPTIONS ExecIO_Options;
extern PCONSTRXSTRING prxCmd;
extern long           lCmdPtr;
extern unsigned long  ulNumSym;
extern char *         pszSymbol [SYMTABLESIZE];
extern long           lStmtType;

#endif /* HOSTEMU_INCLUDED */

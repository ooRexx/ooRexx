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
/******************************************************************************/
/* REXX OS/2 Support                                            aixcmd.h      */
/*                                                                            */
/* AIX specific command processing routines                                   */
/*                                                                            */
/******************************************************************************/
/********************************************************************/
/*                                                                  */
/* MODULE NAME: aixcmd.h                                            */
/*                                                                  */
/* FUNCTION:    Type Definitions for AIX REXX Commands.             */
/*                                                                  */
/* NOTES:       The registration table is an array of               */
/*              structures.  We first typedef the REG_ENVTABLE      */
/*              structure, then we declare and initialize           */
/*              a static array of these structures.                 */
/*                                                                  */
/*              The typedef is useful for pointer                   */
/*              declaration.                                        */
/*                                                                  */
/*              We also declare an enumerated type that             */
/*              tracks the command types, and other data            */
/*              structures used by the command processor.           */
/*              Please see below for details.                       */
/********************************************************************/
#ifndef AIXCMD_H_INCL
#define AIXCMD_H_INCL
/********************************************************************/
/*                                                                  */
/* Define symbolic constants:  various maxima, etc.                 */
/*                                                                  */
/********************************************************************/

#define NAME_TOKEN_LEN 255    /* Max length of environment pgm name */
#define ENV_STG_LEN 10        /* Current max builtin env name length*/

/********************************************************************/
/*                                                                  */
/* Define the command types.                                        */
/*                                                                  */
/********************************************************************/

typedef enum {
               cmd_sh,                 /* Shell                     */
               cmd_ksh,                /* Korn shell                */
               cmd_bsh,                /* Bourne shell              */
               cmd_csh,                /* C shell                   */
               cmd_bash,               /* Bourne Again SHell        */
               cmd_cmd,                /* No shell - direct to AIX  */
               cmd_pgm                 /* User-defined program      */
} CMD_TYPE ;

/********************************************************************/
/*                                                                  */
/* Define the structure that contains command address data.         */
/*                                                                  */
/********************************************************************/

typedef struct {
   CMD_TYPE  cmdtype ;                 /* Command type.             */
   char      cmdprgm[                  /* Command_program.      */
                      NAME_TOKEN_LEN ] ;
} CMD_STRUCTURE ;

/********************************************************************/
/*                                                                  */
/* Define the Command Registration Structure.                       */
/*                                                                  */
/********************************************************************/

typedef struct {
   char     envname[ENV_STG_LEN];      /* Environment name.          */
   CMD_TYPE envtype;                   /* Command Type.              */
} REG_ENVTABLE ;

static REG_ENVTABLE reg_envtable[] =   /* The registration table.   */
   {
      {
         "sh",                         /* Shell                     */
         cmd_sh                        /*                           */
      },
      {
         "ksh",                        /* Korn shell                */
         cmd_ksh                       /*                           */
      },
      {
         "bsh",                        /* Bourne shell              */
         cmd_bsh                       /*                           */
      },
      {
         "csh",                        /* C shell                   */
         cmd_csh                       /*                           */
      },
      {
         "bash",                       /* Bourne Again shell        */
         cmd_bash                       /*                           */
      },
      {
         "SH",                         /* Korn shell                */
         cmd_sh                        /*                           */
      },
      {
         "KSH",                        /* Korn shell                */
         cmd_ksh                       /*                           */
      },
      {
         "BSH",                        /* Bourne shell              */
         cmd_bsh                       /*                           */
      },
      {
         "CSH",                        /* C shell                   */
         cmd_csh                       /*                           */
      },
      {
         "BASH",                        /* Bourne Again shell       */
         cmd_bash                       /*                           */
      },
      {
         "command",                    /* No shell - direct to AIX  */
         cmd_cmd                       /*                           */
      },
      {
         "COMMAND",                    /* No shell - direct to AIX  */
         cmd_cmd                       /*                           */
      }
   } ;                                 /* Note:  add commands here. */

#define REG_ENVTABLE_SIZE sizeof(reg_envtable)/sizeof(reg_envtable[0])
/* Message numbers */
#define MSG_TOO_MANY_CMD_ARGS 2
#define MSG_PROGRAM_UNREADABLE 3
#define SIG_ERROR      1
#define SIG_FAILURE    2
#define SIG_HALT       3
#define SIG_NOVALUE    4
#define SIG_NOTREADY   5
#define SIG_SYNTAX     6
#define SIG_MAXCOND    6


#endif        //AIXCMD_H_INCL



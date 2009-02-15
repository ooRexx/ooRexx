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
/******************************************************************************/
/* REXX AIX Support                                            aixmain.c      */
/*                                                                            */
/* Main interface to the REXX interpreter                                     */
/*                                                                            */
/******************************************************************************/
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>

#if defined( HAVE_SCHED_H )
# include <sched.h>
#endif

#include "RexxCore.h"                    /* bring in global defines           */
#include "StringClass.hpp"
#include "BufferClass.hpp"
#include "MethodClass.hpp"
#include "RexxCode.hpp"
#include "ArrayClass.hpp"
#include "RexxNativeActivation.hpp"
#include "RexxActivation.hpp"
#include "RexxActivity.hpp"
#include "SourceFile.hpp"

#include "APIDefinitions.h"
#include "Interpreter.hpp"


/******************************************************************************/
/* Name:       ApiRexxStart                                  (like RexxStart) */
/*                                                                            */
/* Arguments:  argcount - Number of args in arglist                           */
/*             arglist - Array of args (array of RXSTRINGs)                   */
/*             programname - REXX program to run                              */
/*             instore - Instore array (array of 2 RXSTRINGs)                 */
/*             envname - Initial cmd environment                              */
/*             calltype - How the program is called                           */
/*             exits - Array of system exit names (array of RXSTRINGs)        */
/*                                                                            */
/* Returned:   result - Result returned from program                          */
/*             rc - Return code from program                                  */
/*                                                                            */
/* Notes:  Primary path into Object REXX.  Makes sure Object REXX is up       */
/*   and runs the requested program.                                          */
/*                                                                            */
/*   Mainline path looks like this:                                           */
/*     RexxStart => server_RexxStart                                          */
/******************************************************************************/
int ApiRexxStart(
  size_t argcount,                     /* Number of args in arglist         */
  PCONSTRXSTRING arglist,              /* Array of args                     */
  const char *programname,             /* REXX program to run               */
  PRXSTRING instore,                   /* Instore array                     */
  const char *envname,                 /* Initial cmd environment           */
  int    calltype,                     /* How the program is called         */
  PRXSYSEXIT exits,                    /* Array of system exit names        */
  short * retcode,                     /* Integer form of result            */
  PRXSTRING result)                    /* Result returned from program      */

{
    return RexxStart(argcount, arglist, programname, instore, envname, calltype, exits, retcode, result);
}



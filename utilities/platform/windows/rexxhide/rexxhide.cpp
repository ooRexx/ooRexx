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
/*********************************************************************/
/*                                                                   */
/*  File Name:          REXXHIDE.C                                   */
/*                                                                   */
/*  Calling REXX without creating any console                        */
/*********************************************************************/

#include <windows.h>
#include <rexx.h>                           /* needed for RexxStart()     */
#include <malloc.h>
#include <string.h>                         /* needed for strlen()        */
#include <stdio.h>
#include <io.h>

#include "ArgumentParser.h"  /* defines getArguments and freeArguments */

//
//  MAIN program
//
int WINAPI WinMain(
    HINSTANCE hInstance,	// handle to current instance
    HINSTANCE hPrevInstance,	// handle to previous instance
    LPSTR lpCmdLine,	// pointer to command line
    int nCmdShow)
{
  short    rexxrc = 0;                 /* return code from rexx             */
  LONG  rc;                            /* actually running program RC       */
  const char *program_name;            /* name to run                       */
  CHAR  arg_buffer[1024];              /* starting argument buffer         */
  CONSTRXSTRING arguments;             /* rexxstart argument                */
  size_t argcount;
  RXSTRING rxretbuf;                   // program return buffer

  rc = 0;                              /* set default return                */

  strcpy(arg_buffer, lpCmdLine);
  getArguments(&program_name, arg_buffer, &argcount, &arguments);

  if (program_name == NULL) {
                                       /* give a simple error message       */
    MessageBox(NULL, "Syntax: REXXHIDE ProgramName [parameter_1....parameter_n]\n", "Wrong Arguments", MB_OK | MB_ICONHAND);
    return -1;
  }
  else {                               /* real program execution            */
    rxretbuf.strlength = 0L;           /* initialize return to empty*/

   /* Here we call the interpreter.  We don't really need to use     */
   /* all the casts in this call; they just help illustrate          */
   /* the data types used.                                           */
   rc=REXXSTART(argcount,      /* number of arguments   */
                &arguments,    /* array of arguments    */
                program_name,  /* name of REXX file     */
                0,             /* No INSTORE used       */
                "CMD",         /* Command env. name     */
                RXCOMMAND,     /* Code for how invoked  */
				NULL,          /* No system exits */
                &rexxrc,       /* Rexx program output   */
                &rxretbuf );   /* Rexx program output   */

   if ((rc==0) && rxretbuf.strptr) RexxFreeMemory(rxretbuf.strptr);        /* Release storage only if*/
   freeArguments(program_name, &arguments);

   if (rc < 0)
   {
       sprintf(arg_buffer, "Open Object Rexx program execution failure: rc = %d",rc);
       MessageBox(NULL, arg_buffer, "Execution Error", MB_OK | MB_ICONHAND);
   }
  }
                                             // return interpeter or
 return rc ? rc : rexxrc;                    // rexx program return cd
}





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
/* Oryx Kernel                                                  orx.c         */
/*                                                                            */
/* Executor (EXE file)                                                        */
/*                                                                            */
/*   main entry point to REXX    for LINUX and AIX                            */
/*                                                                            */
/*   MVS - this file will be basically commented out in favor                 */
/*         of just being used to create an image.  It can later               */
/*         be used to run a program if necessary (ie, the                     */
/*         RexxStart api is not yet running.  See Windows pgm                 */
/*         REXXC.C for a mechanism for doing this).                           */
/*                                                                            */
/*         So, this file will simulate getting -ib as input to                */
/*         cause the image construction/saving.                               */
/*         It is simulated instead of real input because va_arg               */
/*         argument passing does not look the same on MVS as OS/2.            */
/*                                                                            */
/*         We will also set tracing on to see what happens.                   */
/******************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "rexx.h"

#if defined(AIX)
#define SYSINITIALADDRESS "ksh"
#elif defined(OPSYS_SUN)
#define SYSINITIALADDRESS "sh"
#else
#define SYSINITIALADDRESS "bash"
#endif

                                       /* semaphore type changed from HEV to OSEM */
                                       /* for AIX.                          */
int main (int argc, char **argv)
{
  int   i;                             /* loop counter                      */
  int   rc;                            /* actually running program RC       */
  const char *program_name;            /* name to run                       */
  char  arg_buffer[8192];              /* starting argument buffer          */
  const char *cp;                      /* option character pointer          */
  CONSTRXSTRING argument;              /* rexxstart argument                */
  size_t argCount;
  char *ptr;
  short rexxrc = 0;                    /* exit List array                   */
  bool from_string = false;            /* running from command line string? */
  bool real_argument = true;           /* running from command line string? */
  RXSTRING instore[2];

  rc = 0;                              /* set default return                */
  argCount = 0;                        /* argument to RexxMain              */
  arg_buffer[0] = '\0';                /* default to no argument string     */
  program_name = NULL;                 /* no program to run yet             */
  for (i = 1; i < argc; i++) {         /* loop through the arguments        */
                                       /* is this option a switch?          */
    if (program_name == NULL && (*(cp=*(argv+i)) == '-'))
      switch (*++cp) {
        case 'e': case 'E':            /* execute from string               */
          from_string = true;          /* hit the startup flags             */
          if ( argc == i+1 ) {
            break;
          }
          program_name = "INSTORE";
          instore[0].strptr = argv[i+1];
          instore[0].strlength = strlen(instore[0].strptr);
          instore[1].strptr = NULL;
          instore[1].strlength = 0;
          real_argument = false;
          break;

        case 'v': case 'V':            /* display version string            */
          ptr = RexxGetVersionInformation();
          fprintf(stderr, ptr);
          if (ptr) free(ptr);
          break;

        default:                       /* ignore other switches             */
          break;
      }
    else {                             /* convert into an argument string   */
      if (program_name == NULL)        /* no name yet?                      */
        program_name = argv[i];        /* program is first non-option       */
      else if (real_argument) {
        if (arg_buffer[0] != '\0')     /* not the first one?                */
          strcat(arg_buffer, " ");     /* add an blank                      */
        strcat(arg_buffer, argv[i]);   /* add this to the argument string   */
        ++argCount;
      }
    real_argument = true;
    }
  }
                                       /* missing a program name?           */
  if (program_name == NULL)
  {
                                       /* give a simple error message       */
    fprintf(stderr,"\n");
    fprintf(stderr,"Syntax is \"rexx [-v] filename [arguments]\"\n");
    fprintf(stderr,"or        \"rexx [-e] program_string [arguments]\".\n");
    return -1;
  }

  argCount = (argCount==0) ? 0 : 1;  /* is there an argument ?            */
                                     /* make an argument                  */
  MAKERXSTRING(argument, arg_buffer, strlen(arg_buffer));
                                     /* run this via RexxStart            */

  if (from_string)
  {
    rc = RexxStart(argCount,         /* number of arguments    */
                   &argument,        /* array of arguments     */
                   program_name,     /* INSTORE                */
                   instore,          /* rexx code from -e      */
                   SYSINITIALADDRESS,/* command env. name      */
                   RXCOMMAND,        /* code for how invoked   */
                   NULL,
                   &rexxrc,          /* REXX program output    */
                   NULL);            /* REXX program output    */
  }
  else
  {
    rc = RexxStart(argCount,         /* number of arguments    */
                   &argument,        /* array of arguments     */
                   program_name,     /* name of REXX file      */
                   0,                /* no instore used        */
                   SYSINITIALADDRESS,/* command env. name      */
                   RXCOMMAND,        /* code for how invoked   */
                   NULL,
                   &rexxrc,          /* REXX program output    */
                   NULL);            /* REXX program output    */
  }
  return rc ? rc : rexxrc;

}

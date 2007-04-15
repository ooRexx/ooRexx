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

#if !defined(AIX) && !defined(LINUX)
#define  INCL_DOSSEMAPHORES
#include "os2.h"

#else
#include "RexxLibrary.h"
//#include "aixrexx.h"

#ifdef AIX
#include "rexx.h"
#endif
#endif // AIX and LINUX

#define INCL_REXXSAA
#include SYSREXXSAA

//int    APIENTRY RexxTerminate (void);

#ifdef __cplusplus
extern "C" {
#endif
BOOL   APIENTRY RexxInitialize (void);
#ifdef __cplusplus
}
#endif
extern BOOL RexxStartedByApplication;  /* Global inducator                  */
extern BOOL ProcessRestoreImage;
extern BOOL ProcessSaveImage;
extern HEV  RexxTerminated;            /* Termination complete semaphore.   */
                                       /* semaphore type changed from HEV to OSEM */
                                       /* for AIX.                          */
int main (int argc, char **argv)
{
  INT   i;                             /* loop counter                      */
  LONG  rc;                            /* actually running program RC       */
  PCHAR program_name;                  /* name to run                       */
  CHAR  arg_buffer[8192];              /* starting argument buffer          */
  char *cp;                            /* option character pointer          */
  RXSTRING argument;                   /* rexxstart argument                */
  LONG  argCount;
  PCHAR ptr;
  SHORT rexxrc = 0;                    /* exit List array                   */
  BOOL from_string = FALSE;            /* running from command line string? */
  BOOL real_argument = TRUE;           /* running from command line string? */
  RXSTRING instore[2];

  RexxStartedByApplication = FALSE;    /* Call NOT from internal            */
  rc = 0;                              /* set default return                */
  argCount = 0;                        /* argument to RexxMain              */
  arg_buffer[0] = '\0';                /* default to no argument string     */
  program_name = NULL;                 /* no program to run yet             */
  for (i = 1; i < argc; i++) {         /* loop through the arguments        */
                                       /* is this option a switch?          */
    if (program_name == NULL && (*(cp=*(argv+i)) == '-'))
      switch (*++cp) {
        case 'i': case 'I':            /* image build                       */
          ProcessRestoreImage = FALSE; /* hit the startup flags             */
          ProcessSaveImage = TRUE;     /* say this is a save image          */
          break;

        case 'e': case 'E':            /* execute from string               */
          from_string = TRUE;          /* hit the startup flags             */
          if ( argc == i+1 ) {
            break;
          }
          program_name = "INSTORE";
          instore[0].strptr = argv[i+1];
          instore[0].strlength = strlen(instore[0].strptr);
          instore[1].strptr = NULL;
          instore[1].strlength = 0;
          real_argument = FALSE;
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
    real_argument = TRUE;
    }
  }
                                       /* missing a program name?           */
  if (program_name == NULL && !ProcessSaveImage) {
                                       /* give a simple error message       */
    fprintf(stderr,"Syntax is \"rexx [-v] filename [arguments]\"\n");
    fprintf(stderr,"or        \"rexx [-e] program_string [arguments]\".\n");
    return -1;
  }

  if (program_name == NULL) {          /* no program to run?                */
                                       /* This is a Saveimage ...           */
    setbuf(stdout,NULL);               /* no buffering                      */
    RexxInitialize();                  /* do normal REXX init               */
  }
  else {                               /* real program execution            */
    argCount = (argCount==0) ? 0 : 1;  /* is there an argument ?            */
                                       /* make an argument                  */
    MAKERXSTRING(argument, arg_buffer, strlen(arg_buffer));
                                       /* run this via RexxStart            */

    if (from_string)
    {
      rc = RexxStart((LONG)       argCount,         /* number of arguments    */
                     (PRXSTRING)  &argument,        /* array of arguments     */
                     (PSZ)        program_name,     /* INSTORE                */
                     (PRXSTRING)  instore,          /* rexx code from -e      */
                     (PSZ)        SYSINITIALADDRESS,/* command env. name      */
                     (LONG)       RXCOMMAND,        /* code for how invoked   */
                                  NULL,
                     (PSHORT)     &rexxrc,          /* REXX program output    */
                                  NULL);            /* REXX program output    */
    }
    else
    {
      rc = RexxStart((LONG)       argCount,         /* number of arguments    */
                     (PRXSTRING)  &argument,        /* array of arguments     */
                     (PSZ)        program_name,     /* name of REXX file      */
                     (PRXSTRING)  0,                /* no instore used        */
                     (PSZ)        SYSINITIALADDRESS,/* command env. name      */
                     (LONG)       RXCOMMAND,        /* code for how invoked   */
                                  NULL,
                     (PSHORT)     &rexxrc,          /* REXX program output    */
                                  NULL);            /* REXX program output    */
    }

    RexxWaitForTermination();

//    EVWAIT(RexxTerminated);
//    EVCL(RexxTerminated);
//    DosWaitEventSem(RexxTerminated,SEM_INDEFINITE_WAIT);
//    DosCloseEventSem(RexxTerminated);
  }
  return rc ? rc : rexxrc;

}

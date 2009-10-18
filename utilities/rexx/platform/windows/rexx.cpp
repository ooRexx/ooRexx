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
/*  File Name:          REXX.C                                   */
/*                                                                   */
/*  Description:        Provides a sample call to the REXX           */
/*                      interpreter, passing in an environment name, */
/*                      a file name, and a single argument string.   */
/*                                                                   */
/*  Entry Points:       main - main entry point                      */
/*                                                                   */
/*  Input:              None                                         */
/*                                                                   */
/*  Output:             returns 0 in all cases.                      */
/*                                                                   */
/*********************************************************************/


#include <windows.h>
#include <rexx.h>                           /* needed for RexxStart()     */
#include <malloc.h>
#include <stdio.h>                          /* needed for printf()        */
#include <string.h>                         /* needed for strlen()        */

//
//  Prototypes
//
int __cdecl main(int argc, char *argv[]);  /* main entry point           */
LONG REXXENTRY MY_IOEXIT( LONG ExitNumber, LONG Subfunction, PEXIT ParmBlock);

#include "ArgumentParser.h"  /* defines getArguments and freeArguments */

//
//  MAIN program
//
int __cdecl main(int argc, char *argv[])
{
  short    rexxrc = 0;                 /* return code from rexx             */
  INT   i;                             /* loop counter                      */
  LONG  rc;                            /* actually running program RC       */
  const char *program_name;            /* name to run                       */
  char  arg_buffer[8192];              /* starting argument buffer          */
  char *cp;                            /* option character pointer          */
  CONSTRXSTRING arguments;             /* rexxstart argument                */
  size_t argcount;
  RXSTRING rxretbuf;                   // program return buffer
  BOOL from_string = FALSE;            /* running from command line string? */
  BOOL real_argument = TRUE;           /* running from command line string? */
  RXSTRING instore[2];

  rc = 0;                              /* set default return                */

   /*
    * Convert the input array into a single string for the Object REXX
    * argument string. Initialize the RXSTRING variable to point to this
    * string. Keep the string null terminated so we can print it for debug.
    * First argument is name of the REXX program
    * Next argument(s) are parameters to be passed
   */

  arg_buffer[0] = '\0';                /* default to no argument string     */
  program_name = NULL;                 /* no program to run yet             */

  for (i = 1; i < argc; i++) {         /* loop through the arguments        */
                                       /* is this an option switch?         */
    if ((*(cp=*(argv+i)) == '-' || *cp == '/'))
      switch (*++cp) {

        case 'e': case 'E':            /* execute from string               */
          if (from_string == FALSE) {    /* only treat 1st -e differently     */
            from_string = TRUE;
            if ( argc == i+1 ) {
              break;
            }
            program_name = "INSTORE";
            instore[0].strptr = argv[i+1];
            instore[0].strlength = strlen(instore[0].strptr);
            instore[1].strptr = NULL;
            instore[1].strlength = 0;
            real_argument = FALSE;
          }
          break;

        case 'v': case 'V':            /* version display */
            {
                char *ptr = RexxGetVersionInformation();
                if (ptr) {
                    printf(ptr, "Interpreter");
                    RexxFreeMemory(ptr);
                }
            }
            return 0;

        default:                       /* ignore other switches             */
          break;
      }
    else {                             /* convert into an argument string   */
      if (program_name == NULL)        /* no name yet?                      */
      {
        program_name = argv[i];        /* program is first non-option       */
        break;      /* end parsing after program_name has been resolved */
      }
      else if ( real_argument ) {      /* part of the argument string       */
        if (arg_buffer[0] != '\0')     /* not the first one?                */
          strcat(arg_buffer, " ");     /* add an blank                      */
        strcat(arg_buffer, argv[i]);   /* add this to the argument string   */
      }
    real_argument = TRUE;
    }
  }

  if (program_name == NULL) {
                                       /* give a simple error message       */
    #undef printf
    printf("\n");
    fprintf(stderr,"Syntax is \"rexx filename [arguments]\"\n");
    fprintf(stderr,"or        \"rexx -e program_string [arguments]\"\n");
    fprintf(stderr,"or        \"rexx -v\".\n");
    return -1;
  }
  else {                               /* real program execution            */
    strcpy(arg_buffer, GetCommandLine());
    getArguments(NULL, arg_buffer, &argcount, &arguments);
    rxretbuf.strlength = 0L;           /* initialize return to empty*/

    #ifdef REXXC_DEBUG
    printf("program_name = %s\n", program_name);
    printf("argv 0 = %s\n", argv[0]);
    printf("argv 1 = %s\n", argv[1]);
    printf("argv 2 = %s\n", argv[2]);
    printf("argument.strptr = %s\n", argument.strptr);
    printf("argument.strlenth = %lu\n", argument.strlength);
    #endif


    if (from_string)
    {
      /* Here we call the interpreter.  We don't really need to use     */
      /* all the casts in this call; they just help illustrate          */
      /* the data types used.                                           */
      rc=REXXSTART(argcount,       /* number of arguments   */
                   &arguments,     /* array of arguments   */
                   program_name,   /* name of REXX file     */
                   instore,        /* rexx code from command line */
                   "CMD",          /* Command env. name     */
                   RXCOMMAND,      /* Code for how invoked  */
                   NULL,
                   &rexxrc,        /* Rexx program output   */
                   &rxretbuf );    /* Rexx program output   */
    }
    else
    {
      rc=REXXSTART(argcount,      /* number of arguments   */
                   &arguments,    /* array of arguments   */
                   program_name,  /* name of REXX file     */
                   NULL,          /* No INSTORE used       */
                   "CMD",         /* Command env. name     */
                   RXCOMMAND,     /* Code for how invoked  */
                   NULL,
                   &rexxrc,       /* Rexx program output   */
                   &rxretbuf );   /* Rexx program output   */
    }
                         /* rexx procedure executed*/
    if ((rc==0) && rxretbuf.strptr)
    {
        RexxFreeMemory(rxretbuf.strptr);        /* Release storage only if*/
    }
    freeArguments(NULL, &arguments);

  }
                                              // return interpeter or
  return rc ? rc : rexxrc;                    // rexx program return cd
}




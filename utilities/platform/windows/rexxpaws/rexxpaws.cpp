/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2014 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* https://www.oorexx.org/license.html                         */
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
/*  File Name:          REXXPAWS.C                                   */
/*                                                                   */
/*  Calling REXX and pausing at exit                                 */
/*********************************************************************/


#include <windows.h>
#include <oorexxapi.h>                          /* needed for rexx stuff      */
#include <malloc.h>
#include <stdio.h>                          /* needed for printf()        */
#include <string.h>                         /* needed for strlen()        */

//
//  Prototypes
//
int __cdecl main(int argc, char *argv[]);  /* main entry point           */
LONG REXXENTRY MY_IOEXIT( LONG ExitNumber, LONG Subfunction, PEXIT ParmBlock);

#include "ArgumentParser.h"  /* defines getArguments and freeArguments */

/*
 * These functions are used to allow ooRexx to display "Press ENTER key to exit..."
 */
static void __cdecl do_pause_at_exit( void )
{
   int ch;
   printf("\nPress ENTER key to exit...");
   fflush( stdout );
   ch = getchar();
}

void __cdecl set_pause_at_exit( void )
{
   atexit( do_pause_at_exit );
}

//
//  MAIN program
//
int __cdecl main(int argc, char *argv[])
{
    int32_t   i;                         /* loop counter                      */
    int32_t  rc;                         /* actually running program RC       */
    const char *program_name;            /* name to run                       */
    char  arg_buffer[8192];              /* starting argument buffer          */

    rc = 0;                              /* set default return                */

    /*
     * Convert the input array into a single string for the Object REXX
     * argument string. First argument is name of the REXX program Next
     * argument(s) are parameters to be passed.  Note that rexxpaws does not
     * currently accept any options, so the parsing is straight forward.
    */
    set_pause_at_exit();

    arg_buffer[0] = '\0';                /* default to no argument string     */
    program_name = NULL;                 /* no program to run yet             */

    for (i = 1; i < argc; i++)           /* loop through the arguments        */
    {
        if (program_name == NULL)        /* no name yet?                      */
        {
            program_name = argv[i];      /* program is first non-option       */
        }
        else                             /* part of the argument string       */
        {
            if (arg_buffer[0] != '\0')   /* not the first one?                */
            {
                strcat(arg_buffer, " "); /* add an blank                      */
            }
            strcat(arg_buffer, argv[i]); /* add this to the argument string   */
        }
    }

    if (program_name == NULL)
    {
        /* give a simple error message       */
#undef printf
        printf("Syntax: REXXPAWS ProgramName [parameter_1....parameter_n]\n");
        return -1;
    }
    else                                 /* real program execution            */
    {
        RexxInstance        *pgmInst;
        RexxThreadContext   *pgmThrdInst;
        RexxArrayObject      rxargs, rxcargs;
        RexxDirectoryObject  dir;
        RexxObjectPtr        result;

        RexxCreateInterpreter(&pgmInst, &pgmThrdInst, NULL);

        // configure the traditional single argument string
        if ( arg_buffer[0] != '\0' )
        {
            rxargs = pgmThrdInst->NewArray(1);
            pgmThrdInst->ArrayPut(rxargs, pgmThrdInst->String(arg_buffer), 1);
        }
        else
        {
            rxargs = pgmThrdInst->NewArray(0);
        }

        // set up the C args into the .local environment
        dir = (RexxDirectoryObject)pgmThrdInst->GetLocalEnvironment();
        if ( argc > 2 )
        {
            rxcargs = pgmThrdInst->NewArray(argc - 2);
        }
        else
        {
            rxcargs = pgmThrdInst->NewArray(0);
        }
        for (i = 2; i < argc; i++)
        {
            pgmThrdInst->ArrayPut(rxcargs,
                                  pgmThrdInst->NewStringFromAsciiz(argv[i]),
                                  i - 1);
        }
        pgmThrdInst->DirectoryPut(dir, rxcargs, "SYSCARGS");
        // call the interpreter
        result = pgmThrdInst->CallProgram(program_name, rxargs);
        // display any error message if there is a condition.  if there was an
        // error, then that will be our return code. we know the return code
        // will fit in an int32_t.
        rc = (int32_t)pgmThrdInst->DisplayCondition();
        if (rc != 0)
        {
            pgmInst->Terminate();
            return -rc;   // well, the negation of the error number is the return code
        }
        if (result != NULL)
        {
            pgmThrdInst->ObjectToInt32(result, &rc);
        }

        pgmInst->Terminate();
    }

    return rc;
}


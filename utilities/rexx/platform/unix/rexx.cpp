/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2012-2017 Rexx Language Association. All rights reserved.    */
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
/*----------------------------------------------------------------------------*/
/******************************************************************************/
/*                                                                            */
/*   main entry point to REXX for unix-based systems                          */
/*                                                                            */
/******************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "oorexxapi.h"


int main (int argc, char **argv) {
    int   i;                             /* loop counter                      */
    int   rc = 0;                        /* actually running program RC       */
    const char *program_name = NULL;     /* name to run                       */
    char  arg_buffer[8192];              /* starting argument buffer          */
    const char *cp;                      /* option character pointer          */
    CONSTRXSTRING argument;              /* rexxstart argument                */
    size_t argCount = 0;
    char *ptr;
    short rexxrc = 0;                    /* exit List array                   */
    bool from_string = false;            /* running from command line string? */
    bool real_argument = true;           /* running from command line string? */
    RXSTRING instore[2];

    RexxInstance        *pgmInst;
    RexxThreadContext   *pgmThrdInst;
    RexxArrayObject      rxargs, rxcargs;
    RexxDirectoryObject  dir;
    RexxObjectPtr        result;

    arg_buffer[0] = '\0';                /* default to no argument string     */
    for (i = 1; i < argc; i++) {         /* loop through the arguments        */
                                         /* is this option a switch?          */
        if (program_name == NULL && (*(cp=*(argv+i)) == '-')) {
            switch (*++cp) {
                case 'e': case 'E':      /* execute from string               */
                    from_string = true;  /* hit the startup flags             */
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

                case 'v': case 'V':      /* display version string            */
                    ptr = RexxGetVersionInformation();
                    fprintf(stdout, "%s\n", ptr);
                    RexxFreeMemory(ptr);
                    return 0;

                default:                 /* ignore other switches             */
                    break;
            }
        } else {                         /* convert into an argument string   */
            if (program_name == NULL) {  /* no name yet?                      */
                program_name = argv[i];  /* program is first non-option       */
            } else if (real_argument) {
                if (arg_buffer[0] != '\0') /* not the first one?              */
                    strcat(arg_buffer, " "); /* add an blank                  */
                strcat(arg_buffer, argv[i]);  /* add this to the arg string   */
                ++argCount;
            }
            real_argument = true;
        }
    }
                                         /* missing a program name?           */
    if (program_name == NULL)  {
                                         /* give a simple error message       */
        fprintf(stderr,"\n");
        fprintf(stderr,"Syntax is \"rexx filename [arguments]\"\n");
        fprintf(stderr,"or        \"rexx -e program_string [arguments]\"\n");
        fprintf(stderr,"or        \"rexx -v\".\n");
        return -1;
    }

    argCount = (argCount==0) ? 0 : 1;    /* is there an argument ?            */
                                         /* make an argument                  */
    MAKERXSTRING(argument, arg_buffer, strlen(arg_buffer));
                                         /* run this via RexxStart            */

    if (from_string) {
        rc = RexxStart(argCount,         /* number of arguments    */
                       &argument,        /* array of arguments     */
                       program_name,     /* INSTORE                */
                       instore,          /* rexx code from -e      */
                       NULL,             // use the default initial ADDRESS() environment
                       RXCOMMAND,        /* code for how invoked   */
                       NULL,
                       &rexxrc,          /* REXX program output    */
                       NULL);            /* REXX program output    */
    }
    else {
        RexxCreateInterpreter(&pgmInst, &pgmThrdInst, NULL);
        // configure the traditional single argument string
        if (argCount > 0) {
            rxargs = pgmThrdInst->NewArray(1);
            pgmThrdInst->ArrayPut(rxargs,
                                  pgmThrdInst->NewStringFromAsciiz(arg_buffer), 1);
        } else {
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
        for (i = 2; i < argc; i++) {
            pgmThrdInst->ArrayPut(rxcargs,
                                  pgmThrdInst->NewStringFromAsciiz(argv[i]),
                                  i - 1);
        }
        pgmThrdInst->DirectoryPut(dir, rxcargs, "SYSCARGS");
        // call the interpreter
        result = pgmThrdInst->CallProgram(program_name, rxargs);
        // display any error message if there is a condition.
        // if there was an error, then that will be our return code
        rc = pgmThrdInst->DisplayCondition();
        if (rc != 0) {
            pgmInst->Terminate();
            return -rc;   // well, the negation of the error number is the return code
        }
        if (result != NULL) {
            pgmThrdInst->ObjectToInt32(result, &rc);
        }

        pgmInst->Terminate();

        return rc;
    }
    return rc ? rc : rexxrc;

}


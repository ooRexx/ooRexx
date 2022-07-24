/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2022 Rexx Language Association. All rights reserved.    */
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
/*********************************************************************/
/*                                                                   */
/*  File Name:          REXXHIDE.C                                   */
/*                                                                   */
/*  Calling REXX without creating any console                        */
/*********************************************************************/

#include <windows.h>
#include <oorexxapi.h>                      /* needed for RexxStart()     */
#include <malloc.h>
#include <string.h>                         /* needed for strlen()        */
#include <stdio.h>
#include <io.h>
#include <time.h>                           // time(), localtime(), strftime()

#include "ArgumentParser.h"  /* defines getArguments and freeArguments */

//
//  MAIN program
//
int WINAPI WinMain(
    HINSTANCE hInstance,                 // handle to current instance
    HINSTANCE hPrevInstance,             // handle to previous instance
    LPSTR lpCmdLine,                     // pointer to command line
    int nCmdShow)
{
    int32_t rc;                          /* actually running program RC       */
    const char *program_name;            /* name to run                       */
    CHAR  arg_buffer[8192];              /* starting argument buffer         */
    CONSTRXSTRING arguments;             /* rexxstart argument                */
    size_t argcount;

    rc = 0;                              /* set default return                */

    strcpy(arg_buffer, lpCmdLine);
    getArguments(&program_name, arg_buffer, &argcount, &arguments);

    if (program_name == NULL)
    {
        /* give a simple error message       */
        MessageBox(NULL, "Syntax: REXXHIDE ProgramName [parameter_1....parameter_n]\n", "Wrong Arguments", MB_OK | MB_ICONHAND);
        return -1;
    }
    else                               /* real program execution            */
    {
        RexxInstance        *pgmInst;
        RexxThreadContext   *pgmThrdInst;
        RexxArrayObject      rxargs, rxcargs;
        RexxDirectoryObject  dir;
        RexxObjectPtr        result;
        int i;
        int argc;                            // parsed count of arguments
        PCHAR *argv;                         // parsed out arguments

        // parse the arguments into argv/argc format
        argv = CommandLineToArgvA(lpCmdLine, &argc);

        RexxCreateInterpreter(&pgmInst, &pgmThrdInst, NULL);
        // configure the traditional single argument string
        if ( arguments.strptr != NULL )
        {
            rxargs = pgmThrdInst->NewArray(1);
            pgmThrdInst->ArrayPut(rxargs, pgmThrdInst->NewString(arguments.strptr, arguments.strlength), 1);
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
            pgmThrdInst->ArrayPut(rxcargs, pgmThrdInst->NewStringFromAsciiz(argv[i]), i - 1);
        }

        pgmThrdInst->DirectoryPut(dir, rxcargs, "SYSCARGS");

        GlobalFree(argv);        // release the parsed arguments
        // call the interpreter
        result = pgmThrdInst->CallProgram(program_name, rxargs);
        // display any error message if there is a condition.  if there was an
        // error, then that will be our return code. we know error code will fit
        // in an int32_t. This just writes it out to the configured error stream. We
        // will want to give full information in the popup as well.
        rc = (int32_t)pgmThrdInst->DisplayCondition();
        if (rc != 0)
        {
            RexxDirectoryObject condition = pgmThrdInst->GetConditionInfo();
            RexxCondition conditionInfo;

            pgmThrdInst->DecodeConditionInfo(condition, &conditionInfo);
            wholenumber_t minorCode = conditionInfo.code - (conditionInfo.rc * 1000);

            sprintf(arg_buffer, "Error %zd.%1zd running %s line %zd\n\n%s\n%s", conditionInfo.rc, minorCode,
                pgmThrdInst->StringData(conditionInfo.program), conditionInfo.position,
                pgmThrdInst->StringData(conditionInfo.errortext), pgmThrdInst->StringData(conditionInfo.message));

            char title[64];
            time_t now = time(NULL);
            struct tm *local = localtime(&now);
            strftime(title, sizeof(title) - 1, "%F %T  Open Object Rexx Execution Error", local);

            MessageBox(NULL, arg_buffer, title, MB_OK | MB_ICONHAND);

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


/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2019 Rexx Language Association. All rights reserved.    */
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
/*                                                                            */
/* translate a program and save to an output file                             */
/*                                                                            */
/******************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "windows.h"
#include "rexx.h"
#include "PlatformDefinitions.h"
#include "RexxErrorCodes.h"
#include "RexxInternalApis.h"

void displayError(int msgid)
{
    // retrieve the message from the central catalog
    const char *message = RexxGetErrorMessage(msgid);

    printf("%s\n", message);    /* print the message                 */
}


/**
 * Handle syntax errors for the rexxc command
 */
void syntaxError()
{
    displayError(Error_REXXC_wrongNrArg);
    displayError(Error_REXXC_SynCheckInfo);
    exit(-1);
}


int SysCall main(int argc, char **argv)
{
    bool  silent = false;
    bool  encode = false;

    const char *input;
    const char *output = NULL;

    // first possible flag argument
    int firstFlag = 2;

    // the source file name is required
    if (argc < 2)
    {
        syntaxError();
    }

    input = argv[1];

    // the output file is optional, but it cannot be a possible flag
    if (argc > 2)
    {
        // if not a possible flag, this is the output file
        if ((argv[2][0] != '/') && (argv[2][0] != '-'))
        {
            output = argv[2];
            // first possible flage is the third argument
            firstFlag = 3;
        }
    }

    // process any remaining arguments as flags.
    for (int j = firstFlag; j < argc; j++)
    {
        if ((argv[j][0] == '/') || (argv[j][0] == '-'))
        {
            if ((argv[j][1] == 's') || (argv[j][1] == 'S'))
            {
                silent = j;
            }
            else if ((argv[j][1] == 'e') || (argv[j][1] == 'E'))
            {
                encode = j;
            }
            // unknown flag
            else
            {
                displayError(Error_REXXC_cmd_parm_incorrect);
                syntaxError();
            }
        }
        else
        {
            displayError(Error_REXXC_cmd_parm_incorrect);
            syntaxError();
        }
    }

    // if not silent, output the banner
    if (!silent)
    {
        char *ptr = RexxGetVersionInformation();
        printf("%s\n", ptr);
        RexxFreeMemory(ptr);
    }


    // if we have an output file, it must be different than the input file
    if (output != NULL && stricmp(input, output) == 0)
    {
        displayError(Error_REXXC_outDifferent);
        exit(-2);
    }

    RexxCompileProgram(input, output, NULL, encode);
}

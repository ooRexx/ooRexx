/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2020 Rexx Language Association. All rights reserved.    */
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
/* Descriptive Name:   Unix Command Line Program for Subcommand      */
/*                     Interface.                                    */
/*                                                                   */
/*********************************************************************/
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <limits.h>
#include <string.h>
#include <stdio.h>

#include "rexx.h"
#include "RexxInternalApis.h"
#include "RexxErrorCodes.h"

#define CASE(x) if(!strcasecmp(x, argv[1]))

void parmerr(int);

int main(int argc, char *argv[], char *envp[])
{
    int args = argc - 2;               // number of RXSUBCOM [ACTION] args

    if (args < 0)                      // no RXSUBCOM argument specified
    {
        // The RXSUBCOM parameters are incorrect
        parmerr(Error_RXSUBC_general);
    }

    CASE("REGISTER")
    {
        if (args != 3)                 // REGISTER envname dllname procname
        {
            // The RXSUBCOM REGISTER parameters are incorrect
            parmerr(Error_RXSUBC_register);
        }
        return RexxRegisterSubcomDll(argv[2], argv[3], argv[4], NULL, RXSUBCOM_DROPPABLE);
    }

    CASE("QUERY")
    {
        if (args < 1 || args > 2)      // QUERY envname [dllname]
        {
            // The RXSUBCOM QUERY parameters are incorrect
            parmerr(Error_RXSUBC_query);
        }
        unsigned short flags;
        return RexxQuerySubcom(argv[2], args == 1 ? NULL : argv[3], &flags, NULL);
    }

    CASE("DROP")
    {
        if (args < 1 || args > 2)      // DROP envname [dllname]
        {
            // The RXSUBCOM DROP parameters are incorrect
            parmerr(Error_RXSUBC_drop);
        }
        return RexxDeregisterSubcom(argv[2], args == 1 ? NULL : argv[3]);
    }

    CASE("LOAD")
    {
        if (args < 1 || args > 2)      // LOAD envname [dllname]
        {
            // The RXSUBCOM LOAD parameters are incorrect
            parmerr(Error_RXSUBC_load);
        }
        return RexxLoadSubcom(argv[2], args == 1 ? NULL : argv[3]);
    }

    // The RXSUBCOM parameters are incorrect
    parmerr(Error_RXSUBC_general);
    return 0;                          // this will never be reached
}

void parmerr(int msgid )
{
    // retrieve the message from the central catalog and print it
    const char *message = RexxGetErrorMessage(msgid);
    printf("%s\n", message);

    exit(-1);
}

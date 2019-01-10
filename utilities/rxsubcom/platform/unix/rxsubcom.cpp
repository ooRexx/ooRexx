/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2019 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* http://www.oorexx.org/license.html                                         */
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
/* Descriptive Name:   unix Command Line Program for Subcommand      */
/*                     Interface.                                    */
/*                                                                   */
/*********************************************************************/
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <limits.h>
#include <string.h>                    /* Include string functions   */
#include <stdio.h>

#include "rexx.h"                      /* for REXXSAA functionality  */
                                       /* old msg constants replaced!*/
#include "RexxInternalApis.h"          /* Get private REXXAPI API's         */
#include "RexxErrorCodes.h"

#define BUFFERLEN         256          /* Length of message bufs used*/
                                       /* Macro for argv[1] compares */
#define CASE(x) if(!strcasecmp(x,argv[1]))

void parmerr(int );

int main( int argc, char *argv[ ], char *envp[ ] )
{                                      /*                            */
    const char * scbname;               /* registration name          */
    const char * scbdll_name;           /* DLL file name              */
    const char * scbdll_proc;           /* DLL procedure name         */

    /* Must be at lease 1 argument*/
    if (argc<2)
    {
        parmerr(Error_RXSUBC_general);
    }

    CASE("REGISTER")
    {                   /* Registration check         */
        /* requires 4 parameters */
        if (argc<5)
        {
            parmerr(Error_RXSUBC_register);
        }
        scbname=argv[2];                  /* Should be Environment Name */
        scbdll_name=argv[3];              /* Should be Dll Name         */
        scbdll_proc=argv[4];              /* Should be Function Name    */
                                          /* Go register the environment*/
        return RexxRegisterSubcomDll(argv[2], argv[3], argv[4], NULL, RXSUBCOM_DROPPABLE);
    }                                 /*                            */
    CASE("QUERY")
    {                      /* Query Check                */
        /* requires 3 parameters */
        if (argc<3)
        {
            parmerr(Error_RXSUBC_query);
        }
        /* if only 3 passed, dummy 4  */
        if (argc<4)
        {
            scbdll_name = "";
        }
        else
        {
            scbdll_name = argv[3];
        }
        unsigned short flags;
        return RexxQuerySubcom(argv[2], scbdll_name, &flags, NULL);
    }                                 /*                            */
    CASE("DROP")
    {                       /* Drop Check                 */
        /* Must pass at least 3 args*/
        if (argc<3)
        {
            parmerr(Error_RXSUBC_drop);
        }
        /* if only 3 passed, dummy 4  */
        if (argc<4)
        {
            scbdll_name = "";
        }
        else
        {
            scbdll_name = argv[3];
        }
        return RexxDeregisterSubcom(argv[2], scbdll_name);
    }
    CASE("LOAD")
    {
        if (argc<3)
        {
            parmerr(Error_RXSUBC_load);
        }
        if (argc<4)
        {
            scbdll_name = "";
        }
        else
        {
            scbdll_name = argv[3];
        }
        return RexxLoadSubcom(argv[2], argv[3]);
    }
    parmerr(Error_RXSUBC_general);      /* Otherwise, must be a error */
    return 0;                           /* dummy return               */
}

void parmerr( int   msgid )            /* removed useless code       */
{
    // retrieve the message from the central catalog
    const char *message = RexxGetErrorMessage(msgid);

    printf("\n%s\n", message);    /* print the message                 */

    exit(-1);
}

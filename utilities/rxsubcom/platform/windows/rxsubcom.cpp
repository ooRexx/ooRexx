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
/*     Program Name:   WINRXCMD.C                                    */
/*                                                                   */
/* Descriptive Name:   Windows Command Line Program for Subcommand   */
/*                     Interface.                                    */
/*                                                                   */
/*********************************************************************/
#include <string.h>                    /* Include string functions   */
#include <stdio.h>
#include "rexx.h"                      /* for REXX functionality     */
#include "RexxInternalApis.h"          /* Get private REXXAPI API's         */
#include "RexxErrorCodes.h"            /* error constants            */

#define BUFFERLEN         256          /* Length of message bufs used*/
#define DLLNAME   "rexx.dll"
                                       /* Macro for argv[1] compares */
#define CASE(x) if(!_stricmp(x,argv[1]))


void parmerr(int);

int __cdecl main( int argc, char *argv[ ], char *envp[ ] )
{                                      /*                            */
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
            argv[3]="";
        }
        unsigned short flags;
        return RexxQuerySubcom(argv[2], argv[3], &flags, NULL);
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
            argv[3]="";
        }
        return RexxDeregisterSubcom(argv[2], argv[3]);
    }
    CASE("LOAD")
    {
        if (argc<3)
        {
            parmerr(Error_RXSUBC_load);
        }
        if (argc<4)
        {
            argv[3]="";             /* if only 3 passed, dummy 4  */
        }
        return RexxLoadSubcom(argv[2], argv[3]);
    }
    parmerr(Error_RXSUBC_general);      /* Otherwise, must be a error */
    return 0;                           /* dummy return               */
}


void parmerr(int arg )
{
    char     dataarea[BUFFERLEN];        /* buf addr to return message   */
    size_t   msglen = 0;                 /* length of returned message   */
    HINSTANCE hDll=NULL;

    memset(dataarea,0,BUFFERLEN);

    hDll = LoadLibrary(DLLNAME);
    if (hDll)
    {
        if (!LoadString(hDll, arg, dataarea, BUFFERLEN))
        {
            strcpy(dataarea,"Error, but no error message available.");
        }
    }
    else
    {
        strcpy(dataarea,"Error, but no error message available because REXX.DLL not loaded.");
    }

    printf("REX%d: %s",arg, dataarea);
    FreeLibrary(hDll);
    exit(-1);
}

/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2008-2024 Rexx Language Association. All rights reserved.    */
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
/* THIS SOFTWARE IS PROVIDED BY THE COPYright HOLDERS AND CONTRIBUTORS        */
/* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT          */
/* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS          */
/* FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYright   */
/* OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,      */
/* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED   */
/* TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,        */
/* OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY     */
/* OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING    */
/* NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS         */
/* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.               */
/*                                                                            */
/*----------------------------------------------------------------------------*/

#include <rexx.h>
#include <string.h>
#include <stdio.h>


size_t REXXENTRY MyTestExtFunc(const char *Name, long Argc, CONSTRXSTRING Argv[],
                               const char *Queuename, PRXSTRING Retstr) {
    int retc = 0;
    snprintf(Retstr->strptr, 256, "%d", 0);
    Retstr->strlength = strlen(Retstr->strptr);

    if (Argc > 0) {
        retc = RexxDeregisterFunction("MyTestExtFunc");
        if (retc) {
            snprintf(Retstr->strptr, 256, "%d", -2);
            Retstr->strlength = strlen(Retstr->strptr);
        }
    }
    return 0;
}


size_t REXXENTRY TestExternalFunction(const char *Name, long Argc, CONSTRXSTRING Argv[],
                                    const char *Queuename, PRXSTRING Retstr) {
    int retc = 0;
    int i;
    snprintf(Retstr->strptr, 256, "%d", 0);
    Retstr->strlength = strlen(Retstr->strptr);

    if (strcmp(Name, "TESTEXTERNALFUNCTION")) {
        snprintf(Retstr->strptr, 256, "%d", -1);
        Retstr->strlength = strlen(Retstr->strptr);
        return 0;
    }
    if (strcmp(Queuename, "SESSION")) {
        snprintf(Retstr->strptr, 256, "%d", -1);
        Retstr->strlength = strlen(Retstr->strptr);
        return 0;
    }
    if (Argc == 0) {
        retc = RexxRegisterFunctionDll("MyTestExtFunc", "orxclassic1", "MyTestExtFunc");
        if (retc) {
            snprintf(Retstr->strptr, 256, "%d", -2);
            Retstr->strlength = strlen(Retstr->strptr);
            return 0;
        }
    }
    else {
        for (i = 0; i < Argc; i++) {
            if (!RXVALIDSTRING(Argv[i])) {
                snprintf(Retstr->strptr, 256, "%d", -4);
                Retstr->strlength = strlen(Retstr->strptr);
                return 0;
            }
        }
        retc = RexxRegisterFunctionExe("MyTestExtFunc", MyTestExtFunc);
        if (retc) {
            snprintf(Retstr->strptr, 256, "%d", -2);
            Retstr->strlength = strlen(Retstr->strptr);
            return 0;
        }
    }
    retc = RexxQueryFunction("MyTestExtFunc");
    if (retc) {
        snprintf(Retstr->strptr, 256, "%d", -3);
        Retstr->strlength = strlen(Retstr->strptr);
        return 0;
    }
    return 0;
}


RexxReturnCode REXXENTRY MyTestSubcomHandler(CONSTRXSTRING *Cmd, unsigned short *flags,
                                             PRXSTRING Retstr) {

    if (Cmd->strlength > RXAUTOBUFLEN - 1) {
        *flags = RXSUBCOM_ERROR;
        return 30;
    }
    *flags = RXSUBCOM_OK;
    strcpy(Retstr->strptr, Cmd->strptr);
    Retstr->strlength = Cmd->strlength;
    return 0;
}


size_t REXXENTRY TestSubcomHandler(const char *Name, long Argc, CONSTRXSTRING Argv[],
                                   const char *Queuename, PRXSTRING Retstr) {
    unsigned short flags;
    char userarea[2*sizeof(void *)];  // different size on 64-bit systems
    int retc;
    snprintf(Retstr->strptr, 256, "%d", 0);
    Retstr->strlength = strlen(Retstr->strptr);

    if (Argc != 2) {
        snprintf(Retstr->strptr, 256, "%d", -1);
        Retstr->strlength = strlen(Retstr->strptr);
        return 0;
    }
    if (*Argv[0].strptr == 'R') {
        retc = RexxRegisterSubcomDll(Argv[1].strptr, "orxclassic1",
                                     "MyTestSubcomHandler",
                                     (char*)malloc(8), RXSUBCOM_DROPPABLE);
        if (retc != 0) {
            snprintf(Retstr->strptr, 256, "%d", -2);
            Retstr->strlength = strlen(Retstr->strptr);
            return 0;
        }
        retc = RexxQuerySubcom(Argv[1].strptr, "orxclassic1", &flags, userarea);
        if (retc != 0) {
            snprintf(Retstr->strptr, 256, "%d", -2);
            Retstr->strlength = strlen(Retstr->strptr);
            return 0;
        }
    }
    else if (*Argv[0].strptr == 'E') {
        retc = RexxRegisterSubcomExe(Argv[1].strptr, (REXXPFN)MyTestSubcomHandler,
                                     (char*)malloc(8));
        if (retc != 0) {
            snprintf(Retstr->strptr, 256, "%d", -2);
            Retstr->strlength = strlen(Retstr->strptr);
            return 0;
        }
        retc = RexxQuerySubcom(Argv[1].strptr, NULL, &flags, userarea);
        if (retc != 0) {
            snprintf(Retstr->strptr, 256, "%d", -2);
            Retstr->strlength = strlen(Retstr->strptr);
            return 0;
        }
    }
    else if (*Argv[0].strptr == 'D') {
        retc = RexxDeregisterSubcom(Argv[1].strptr, "orxclassic1");
        if (retc != 0) {
            snprintf(Retstr->strptr, 256, "%d", -2);
            Retstr->strlength = strlen(Retstr->strptr);
            return 0;
        }
        retc = RexxQuerySubcom(Argv[1].strptr, "orxclassic1", &flags, userarea);
        if (retc != 0) {
            snprintf(Retstr->strptr, 256, "%d", -2);
            Retstr->strlength = strlen(Retstr->strptr);
            return 0;
        }
    }
    else if (*Argv[0].strptr == 'X') {
        retc = RexxDeregisterSubcom(Argv[1].strptr, NULL);
        if (retc != 0) {
            snprintf(Retstr->strptr, 256, "%d", -2);
            Retstr->strlength = strlen(Retstr->strptr);
            return 0;
        }
        retc = RexxQuerySubcom(Argv[1].strptr, NULL, &flags, userarea);
        if (retc != 0) {
            snprintf(Retstr->strptr, 256, "%d", -2);
            Retstr->strlength = strlen(Retstr->strptr);
            return 0;
        }
    }
    else {
        snprintf(Retstr->strptr, 256, "%d", -2);
        Retstr->strlength = strlen(Retstr->strptr);
        return 0;
    }
    return 0;
}




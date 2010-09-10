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
/******************************************************************************/
/* REXX Kernel                                                                */
/*                                                                            */
/* Version Identification                                                     */
/*                                                                            */
/******************************************************************************/
#include "RexxCore.h"
#include "StringClass.hpp"
#include "Interpreter.hpp"

const char *  build_date = __DATE__;  /* date of last build                */

RexxString *Interpreter::versionNumber = OREF_NULL;

RexxString *Interpreter::getVersionNumber()
/******************************************************************************/
/* Arguments:  None                                                           */
/*                                                                            */
/*  Returned:  Version string                                                 */
/******************************************************************************/
{
    if (versionNumber == OREF_NULL)
    {
        char     buffer[100];                /* buffer for building the string    */
        char     work[20];                   /* working buffer                    */

        strcpy(work, build_date);            /* copy the build date               */
        char *month = strtok(work, " ");     /* get the month                     */
        char *day = strtok(NULL, " ");       /* get the build day                 */
        char *year = strtok(NULL, " ");      /* and the year                      */
        if (*day == '0')                     /* day have a leading zero?          */
        {
            day++;                             /* step over it                      */
        }
                                               /* format the result                 */
        sprintf(buffer, "REXX-ooRexx_%d.%d.%d(MT) 6.04 %s %s %s", ORX_VER, ORX_REL, ORX_MOD, day, month, year);
        versionNumber = new_string(buffer);  /* return as a rexx string           */
    }
    return versionNumber;
}


/**
 * Get the interpreter version level as a binary number
 * to be returned in the APIs.
 *
 * @return The binary interpreter version level.
 */
size_t Interpreter::getInterpreterVersion()
{
    return REXX_CURRENT_INTERPRETER_VERSION;
}

/**
 * Return the current language level implemented by this
 * interpreter version.
 *
 * @return The current defined language level.
 */
size_t Interpreter::getLanguageLevel()
{
    return REXX_CURRENT_LANGUAGE_LEVEL;
}

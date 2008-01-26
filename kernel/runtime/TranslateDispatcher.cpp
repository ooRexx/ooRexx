/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2006 Rexx Language Association. All rights reserved.    */
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

#include "RexxCore.h"
#include "TranslateDispatcher.hpp"
#include "MethodClass.hpp"
#include "ProtectedObject.hpp"


/**
 * Default virtual method for handling a run() methods on
 * an activity dispatcher.
 */
void TranslateDispatcher::run()
{
    ProtectedSet savedObjects;

    RexxString *name = OREF_NULLSTRING;     // name of the invoked program
    RexxMethod *method;

    if (programName != NULL)       /* have an actual name?              */
    {
        /* get string version of the name    */
        name = new_string(programName);
    }

    savedObjects.add(name);              /* protect from garbage collect      */

    if (instore == NULL)                     /* no instore request?               */
    {
        /* go resolve the name               */
        RexxString *fullname = SysResolveProgramName(name, OREF_NULL);
        if (fullname == OREF_NULL)         /* not found?                        */
        {
            /* got an error here                 */
            reportException(Error_Program_unreadable_notfound, name);
        }
        savedObjects.add(fullname);
        /* go translate the image            */
        method = TheMethodClass->newFile(fullname);
        savedObjects.add(method);
        SysSaveProgram(fullname, method);/* go save this method               */
    }
    else                                 /* have an instore program           */
    {
        /* go handle instore parms           */
        method = RexxMethod::processInstore(instore, name);
        if (method == OREF_NULL)           /* couldn't get it?                  */
        {
            /* got an error here                 */
            reportException(Error_Program_unreadable_name, name);
        }
        savedObjects.add(method);
    }
    if (outputName != NULL)              /* want to save this to a file?      */
    {
                                         /* go save this method               */
        SysSaveTranslatedProgram(outputName, method);
    }
}


/**
 * Default handler for any error conditions.  This just sets the
 * condition information in the dispatch unit.
 *
 * @param c      The condition information for the error.
 */
void TranslateDispatcher::handleError(wholenumber_t r, RexxDirectory *c)
{
    // use the base error handling and set our return code to the negated error code.
    ActivityDispatcher::handleError(rc, c);
    rc = -r;
}


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

#include "RexxCore.h"
#include "TranslateDispatcher.hpp"
#include "RoutineClass.hpp"
#include "ProtectedObject.hpp"
#include "NativeActivation.hpp"
#include "LanguageParser.hpp"
#include "GlobalNames.hpp"
#include "ActivityManager.hpp"


/**
 * Default virtual method for handling a run() methods on
 * an activity dispatcher.
 */
void TranslateDispatcher::run()
{
    Protected<RoutineClass> program;

    // default name is a null string if one not given
    Protected<RexxString> name = GlobalNames::NULLSTRING;

    // have an actual name?
    if (programName != NULL)
    {
        // get string version of the name
        name = new_string(programName);
    }

    // if not an instore request, then this must be a resolvable name
    if (instore == NULL)
    {
        Protected<RexxString> fullname = activity->resolveProgramName(name, OREF_NULL, OREF_NULL);
        if (fullname == OREF_NULL)
        {
            reportException(Error_Program_unreadable_notfound, name);
        }

        // go translate the image            */
        program = LanguageParser::createProgram(fullname);
    }
    // this is an instore program, so it needs to be translated as is
    else
    {
        // this is handled by the language parser
        program = LanguageParser::processInstore(instore, name);
        // not valid, so give the unreadable error
        if (program == OREF_NULL)
        {
            reportException(Error_Program_unreadable_name, name);
        }
    }

    // is this a request to save to a file?
    if (outputName != NULL)
    {
        // go save this program using the indicated format
        program->save(outputName, encode);
    }
}


/**
 * Default handler for any error conditions.  This just sets the
 * condition information in the dispatch unit.
 *
 * @param c      The condition information for the error.
 */
void TranslateDispatcher::handleError(wholenumber_t r, DirectoryClass *c)
{
    // use the base error handling and set our return code to the negated error code.
    ActivityDispatcher::handleError(rc, c);
    rc = -r;
    // process the error to display the error message.
    activity->error(activation, conditionData);
}


/**
 * Default virtual method for handling a run() methods on
 * an activity dispatcher.
 */
void TranslateInstoreDispatcher::run()
{
    Protected<RexxString> name = GlobalNames::NULLSTRING;     // name of the invoked program
    // have a name given, then use that instead
    if (programName != NULL)
    {
        name = new_string(programName);
    }

    RXSTRING instore[2];

    MAKERXSTRING(instore[0], const_cast<char *>(source->strptr), source->strlength);
    MAKERXSTRING(instore[1], NULL, 0);

    // do the translation
    Protected<RoutineClass> program = LanguageParser::processInstore(instore, name);
    if (program == OREF_NULL)
    {
        reportException(Error_Program_unreadable_name, name);
    }

    // copy the image information back
    image->strptr = instore[1].strptr;
    image->strlength = instore[1].strlength;
}


/**
 * Default handler for any error conditions.  This just sets the
 * condition information in the dispatch unit.
 *
 * @param c      The condition information for the error.
 */
void TranslateInstoreDispatcher::handleError(wholenumber_t r, DirectoryClass *c)
{
    // use the base error handling and set our return code to the negated error code.
    ActivityDispatcher::handleError(rc, c);
    rc = -r;
}


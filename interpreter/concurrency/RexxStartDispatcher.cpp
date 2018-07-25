/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2018 Rexx Language Association. All rights reserved.    */
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
#include "Activity.hpp"
#include "RexxStartDispatcher.hpp"
#include "ProtectedObject.hpp"
#include "RoutineClass.hpp"
#include "SystemInterpreter.hpp"
#include "InterpreterInstance.hpp"
#include "NativeActivation.hpp"
#include "LanguageParser.hpp"
#include "GlobalNames.hpp"
#include "ActivityManager.hpp"


/**
 * Default virtual method for handling a run() methods on
 * an activity dispatcher.
 */
void RexxStartDispatcher::run()
{
    ProtectedSet savedObjects;

    // set default return values
    rc = 0;
    retcode = 0;

    RexxString *name = GlobalNames::NULLSTRING;     // name of the invoked program
    RexxString *fullname = name;            // default the fulllength name to the simple name

    // if we've been given an actual name, get the string version of it
    if (programName != NULL)
    {
        name = new_string(programName);
        savedObjects.add(name);
    }

    // get an array version of the arguments and protect
    ArrayClass *new_arglist = new_array(argcount);
    savedObjects.add(new_arglist);

    // for compatibility reasons, if this is a command invocation and there is a leading blank
    // on the only argument, then remove that leading blank from the argument
    if (calltype == RXCOMMAND && argcount == 1 && arglist[0].strlength > 1 && arglist[0].strptr != NULL && arglist[0].strptr[0] == ' ')
    {
        new_arglist->put(new_string(arglist[0].strptr + 1, arglist[0].strlength - 1), 1);
    }
    // we need to create an array argument list from the RXSTRINGs
    else
    {
        for (size_t i = 0; i < argcount; i++)
        {
            // only add real arguments
            if (arglist[i].strptr != NULL)
            {
                new_arglist->put(new_string(arglist[i]), i + 1);
            }
        }
    }

    RexxString *source_calltype;

    // now get the calltype as a character string to used in
    // the source string. .
    switch (calltype)
    {
        case  RXCOMMAND:
            source_calltype = GlobalNames::COMMAND;
            break;

        case  RXFUNCTION:
            source_calltype = GlobalNames::FUNCTION;
            break;

        case  RXSUBROUTINE:
            source_calltype = GlobalNames::SUBROUTINE;
            break;

        // if not specified, call it a COMMAND.
        default:
            source_calltype = GlobalNames::COMMAND;
            break;
    }

    Protected<RoutineClass> program;

    // if not an instore request, we load this from a file.
    if (instore == NULL)
    {
        fullname = activity->resolveProgramName(name, OREF_NULL, OREF_NULL);
        if (fullname == OREF_NULL)
        {
            reportException(Error_Program_unreadable_notfound, name);
        }
        savedObjects.add(fullname);
        program = LanguageParser::createProgramFromFile(fullname);
    }
    // we either need to parse the instore source or restore from a
    // previous image.
    else
    {
        program = LanguageParser::processInstore(instore, name);
        if (program.isNull())
        {
            reportException(Error_Program_unreadable_name, name);
        }
    }

    RexxString *initial_address = activity->getInstance()->getDefaultEnvironment();
    // actually need to run this?
    if (!program.isNull())
    {
        ProtectedObject program_result;
        // call the program
        program->runProgram(activity, source_calltype, initial_address, new_arglist->messageArgs(), argcount, program_result);

        // provided for a return result (that's optional)
        if (result != NULL)
        {
            // actually have a result to return?
            if (!program_result.isNull())
            {
                // force to a string value
                program_result = program_result->stringValue();
                // copy this into the result RXSTRING
                ((RexxString *)program_result)->copyToRxstring(*result);
            }
            // nothing to return
            else
            {
                MAKERXSTRING(*result, NULL, 0);
            }
        }

        // if we have a return result and it is an integer value, return that as a return code.
        if (!program_result.isNull())
        {
            wholenumber_t return_code;

            if (program_result->numberValue(return_code) && return_code <= SHRT_MAX && return_code >= SHRT_MIN)
            {
                retcode = (short)return_code;
            }
        }
    }
}


/**
 * Default handler for any error conditions.  This just sets the
 * condition information in the dispatch unit.
 *
 * @param c      The condition information for the error.
 */
void RexxStartDispatcher::handleError(wholenumber_t r, DirectoryClass *c)
{
    // use the base error handling and set our return code to the negated error code.
    ActivityDispatcher::handleError(-r, c);
    retcode = (short)rc;
    // process the error to display the error message.
    activity->error(activation, conditionData);
}



/**
 * Run a routine for a thread context API call.
 */
void CallRoutineDispatcher::run()
{
    if (arguments != OREF_NULL)
    {
        // we use a null string for the name when things are called directly
        routine->call(activity, GlobalNames::NULLSTRING, arguments->messageArgs(), arguments->messageArgCount(), result);
    }
    else
    {
        // we use a null string for the name when things are called directly
        routine->call(activity, GlobalNames::NULLSTRING, NULL, 0, result);
    }
}



/**
 * Run a routine for a thread context API call.
 */
void CallProgramDispatcher::run()
{
    RexxString *targetName = new_string(program);
    //we are resolving from a program name
    RexxString *name = activity->resolveProgramName(targetName, OREF_NULL, OREF_NULL);
    if (name == OREF_NULL)
    {
        reportException(Error_Program_unreadable_notfound, targetName);
    }
    ProtectedObject p(name);
    // create a routine from this file
    Protected<RoutineClass> routine = LanguageParser::createProgramFromFile(name);

    if (arguments != OREF_NULL)
    {
        // use the provided name for the call name
        routine->runProgram(activity, arguments->messageArgs(), arguments->messageArgCount(), result);
    }
    else
    {
        // we use a null string for the name when things are called directly
        routine->runProgram(activity, NULL, 0, result);
    }
}

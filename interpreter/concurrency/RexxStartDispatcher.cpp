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

#include "RexxCore.h"
#include "RexxStartDispatcher.hpp"
#include "ProtectedObject.hpp"
#include "RoutineClass.hpp"
#include "SystemInterpreter.hpp"
#include "InterpreterInstance.hpp"
#include "RexxNativeActivation.hpp"


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

    RexxString *name = OREF_NULLSTRING;     // name of the invoked program
    RexxString *fullname = name;            // default the fulllength name to the simple name

    if (programName != NULL)       /* have an actual name?              */
    {
        /* get string version of the name    */
        name = new_string(programName);
    }

    savedObjects.add(name);              /* protect from garbage collect      */
    // get an array version of the arguments and protect
    RexxArray *new_arglist = new_array(argcount);
    savedObjects.add(new_arglist);
    /* loop through the argument list    */
    for (size_t i = 0; i < argcount; i++)
    {
        /* have a real argument?             */
        if (arglist[i].strptr != NULL)
        {
            /* add to the argument array         */
            new_arglist->put(new_string(arglist[i]), i + 1);
        }
    }
    RexxString *source_calltype;

    switch (calltype)                      /* turn calltype into a string       */
    {
        case  RXCOMMAND:                   /* command invocation                */
            source_calltype = OREF_COMMAND;  /* this is the 'COMMAND' string      */
            break;

        case  RXFUNCTION:                  /* function invocation               */
            /* 'FUNCTION' string                 */
            source_calltype = OREF_FUNCTIONNAME;
            break;

        case  RXSUBROUTINE:                /* subroutine invocation             */
            /* 'SUBROUTINE' string               */
            source_calltype = OREF_SUBROUTINE;
            break;

        default:
            source_calltype = OREF_COMMAND;  /* this is the 'COMMAND' string      */
            break;
    }

    RoutineClass *program = OREF_NULL;

    if (instore == NULL)                     /* no instore request?               */
    {
        /* go resolve the name               */
        fullname = activity->resolveProgramName(name, OREF_NULL, OREF_NULL);
        if (fullname == OREF_NULL)         /* not found?                        */
        {
            /* got an error here                 */
            reportException(Error_Program_unreadable_notfound, name);
        }
        savedObjects.add(fullname);
                                           /* try to restore saved image        */
        program = RoutineClass::fromFile(fullname);
    }
    else                                 /* have an instore program           */
    {
        /* go handle instore parms           */
        program = RoutineClass::processInstore(instore, name);
        if (program == OREF_NULL)        /* couldn't get it?                  */
        {
            /* got an error here                 */
            reportException(Error_Program_unreadable_name, name);
        }
    }

    RexxString *initial_address = activity->getInstance()->getDefaultEnvironment();
    /* actually need to run this?        */
    if (program != OREF_NULL)
    {
        ProtectedObject program_result;
        // call the program
        program->runProgram(activity, source_calltype, initial_address, new_arglist->data(), argcount, program_result);
        if (result != NULL)          /* if return provided for            */
        {
            /* actually have a result to return? */
            if ((RexxObject *)program_result != OREF_NULL)
            {
                /* force to a string value           */
                program_result = ((RexxObject *)program_result)->stringValue();
                // copy this into the result RXSTRING
                ((RexxString *)program_result)->copyToRxstring(*result);
            }
            else                             /* make this an invalid string       */
            {
                MAKERXSTRING(*result, NULL, 0);
            }
        }
                                             /* If there is a return val...       */
        if ((RexxObject *)program_result != OREF_NULL)
        {
            wholenumber_t return_code;

            /* if a whole number...              */
            if (((RexxObject *)program_result)->numberValue(return_code) && return_code <= SHRT_MAX && return_code >= SHRT_MIN)
            {
                /* ...copy to return code.           */
                retcode = (short)return_code;
            }
            // set the RC to the retcode value if there's no errors.
            rc = retcode;
        }
    }
}


/**
 * Default handler for any error conditions.  This just sets the
 * condition information in the dispatch unit.
 *
 * @param c      The condition information for the error.
 */
void RexxStartDispatcher::handleError(wholenumber_t r, RexxDirectory *c)
{
    // use the base error handling and set our return code to the negated error code.
    ActivityDispatcher::handleError(-r, c);
    retcode = (short)rc;
    // process the error to display the error message.
    activity->error(activation);
}



/**
 * Run a routine for a thread context API call.
 */
void CallRoutineDispatcher::run()
{
    if (arguments != OREF_NULL)
    {
        // we use a null string for the name when things are called directly
        routine->call(activity, OREF_NULLSTRING, arguments->data(), arguments->size(), result);
    }
    else
    {
        // we use a null string for the name when things are called directly
        routine->call(activity, OREF_NULLSTRING, NULL, 0, result);
    }
}



/**
 * Run a routine for a thread context API call.
 */
void CallProgramDispatcher::run()
{
    RexxString *name = new_string(program);

    // get a routine from the file source first
    RoutineClass *routine = new RoutineClass(name);

    ProtectedObject p(routine);

    if (arguments != OREF_NULL)
    {
        // use the provided name for the call name
        routine->runProgram(activity, arguments->data(), arguments->size(), result);
    }
    else
    {
        // we use a null string for the name when things are called directly
        routine->runProgram(activity, NULL, 0, result);
    }
}

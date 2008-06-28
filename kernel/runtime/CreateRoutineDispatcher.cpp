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
#include "CreateRoutineDispatcher.hpp"
#include "Interpreter.hpp"
#include "InterpreterInstance.hpp"
#include "RexxActivity.hpp"
#include "RoutineClass.hpp"
#include "DirectoryClass.hpp"
#include "BufferClass.hpp"
#include "ProtectedObject.hpp"
#include "RexxNativeActivation.hpp"



/**
 * Default handler for any error conditions.  This just sets the
 * condition information in the dispatch unit.
 *
 * @param c      The condition information for the error.
 */
void ConditionDispatcher::handleError(wholenumber_t r, RexxDirectory *c)
{
    // use the base error handling and set our return code
    ActivityDispatcher::handleError(rc, c);

    // fill in the condition information

    memset(translatedCondition, 0, sizeof(RexxConditionData));
    translatedCondition->code = message_number((RexxString *)conditionData->at(OREF_CODE));

    translatedCondition->rc = message_number((RexxString *)conditionData->at(OREF_RC));

    RexxString *message = (RexxString *)conditionData->at(OREF_NAME_MESSAGE);
    if ((RexxObject*) message != TheNilObject)
    {
        message->copyToRxstring(translatedCondition->message);
    }

    RexxString *errortext = (RexxString *)conditionData->at(OREF_ERRORTEXT);
    errortext->copyToRxstring(translatedCondition->errortext);
    RexxString *program = (RexxString *)conditionData->at(OREF_PROGRAM);
    program->copyToRxstring(translatedCondition->program);
    // we set the dispatcher return code to the negated value
    rc = -translatedCondition->rc;
}


/**
 * Default virtual method for handling a run() methods on
 * an activity dispatcher.
 */
void CreateRoutineDispatcher::run()
{
                                          /* get a buffer object               */
    RexxBuffer *source_buffer = new_buffer(programBuffer);
    ProtectedObject p(source_buffer);
                                          /* translate this source             */
    translatedRoutine = (REXXOBJECT)new RoutineClass(OREF_NULLSTRING, source_buffer);

    RexxString *saveTarget = new_string(contextName);
    RexxDirectory *locked_objects = (RexxDirectory *)ActivityManager::localEnvironment->at(saveTarget);
    // the value used in our directory is the string value of the
    // method pointer
    char buffer[32];
    sprintf(buffer, "0x%p", translatedRoutine);
    locked_objects->put((RexxObject *)translatedRoutine, new_string(buffer));
}


/**
 * Run a routine for a RexxRunRoutine() API call.
 */
void RunRoutineDispatcher::run()
{
    RexxArray *new_arglist = OREF_NULL;

    // callback activated?
    if (argumentCallback != NULL)
    {
        RunRoutineArgumentCallback callback(argumentCallback, callbackArguments);

        activity->run(callback);

        new_arglist = callback.argumentList;
        activation->createLocalReference(new_arglist);
    }
    // use dummy argument array
    else
    {
        // this is just the null argument list
        new_arglist = TheNullArray;
    }

    // get the default environment from the instance
    RexxString *initial_address = activity->getInstance()->getDefaultEnvironment();

    // configure the method security manager
    if (securityManager != NULLOBJECT)
    {
        ((RoutineClass *)routine)->setSecurityManager((RexxObject *)securityManager);
    }

    /* run and get the result            */
    ProtectedObject program_result;
    // call the program
    ((RoutineClass *)routine)->runProgram(activity, OREF_SCRIPT, initial_address, new_arglist->data(), new_arglist->size(), program_result);
    result = (REXXOBJECT)program_result;
    if (result != NULLOBJECT)
    {
        RexxString *saveTarget = new_string(contextName);
        RexxDirectory *locked_objects = (RexxDirectory *)ActivityManager::localEnvironment->at(saveTarget);
        // the value used in our directory is the string value of the
        // method pointer
        char buffer[32];
        sprintf(buffer, "0x%p", result);
        locked_objects->put((RexxObject *)result, new_string(buffer));
    }
}


/**
 * Process a callout to an argument processing function.
 */
void RunRoutineArgumentCallback::run()
{
    // this just dispatches the callback in the correct state
    argumentList = (RexxArray *)callback(arguments);
}

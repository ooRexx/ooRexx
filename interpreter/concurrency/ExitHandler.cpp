/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2009 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* http://www.ibm.com/developerworks/oss/CPLv1.0.htm                          */
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

#include "RexxCore.h"
#include "ExitHandler.hpp"
#include "RexxActivity.hpp"
#include "ActivityManager.hpp"
#include "RexxInternalApis.h"


/**
 * Call an exit
 *
 * @param activity   The current activity.
 * @param activation The top-most activation.
 * @param function   The exit major function code.
 * @param subfunction
 *                   The exit minor function code.
 * @param parms      The parameter block passed to the exit.
 *
 * @return The exit handler return code.
 */
int ExitHandler::call(RexxActivity *activity, RexxActivation *activation, int function, int subfunction, void *parms)
{
    if (type == REGISTERED_NAME)
    {
        ExitHandlerDispatcher dispatcher(entryPoint, function, subfunction, parms);

        // run this and give back the return code
        activity->run(dispatcher);
        return dispatcher.rc;
    }
    else
    {
        ContextExitHandlerDispatcher dispatcher(entryPoint, function, subfunction, parms);

        // run this and give back the return code
        activity->run(dispatcher);
        return dispatcher.rc;
    }
}


/**
 * Resolve a classic-style exit handler to the actual target
 * entry point address and invocation style.
 *
 * @param name   The registered exit name.
 */
void ExitHandler::resolve(const char *name)
{
    RexxResolveExit(name, &entryPoint);
    type = REGISTERED_NAME;
}


/**
 * Resolve a classic-style exit handler to the actual target
 * entry point address and invocation style.
 *
 * @param name   The registered exit name.
 */
void ExitHandler::resolve(RexxContextExitHandler *handler)
{
    entryPoint = (REXXPFN)handler;
    type = DIRECT;
}



/**
 * Process a callout to a system exit function.
 */
void ExitHandlerDispatcher::run()
{
    RexxExitHandler *exit_address = (RexxExitHandler *)entryPoint;
    rc = (int)(*exit_address)(major, minor, (PEXIT)parms);
}



/**
 * Process a callout to a system exit function.
 */
void ContextExitHandlerDispatcher::run()
{
    RexxContextExitHandler *exit_address = (RexxContextExitHandler *)entryPoint;

    ExitContext context;

    // build a context pointer to pass out
    activity->createExitContext(context, activation);

    rc = (int)(*exit_address)(&context.threadContext, major, minor, (PEXIT)parms);
}

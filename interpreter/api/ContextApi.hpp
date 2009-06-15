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
/* REXX API support                                                           */
/*                                                                            */
/* Stub functions for all APIs accessed via the NativeFunctionContext         */
/*                                                                            */
/******************************************************************************/

#ifndef ContextApi_Included
#define ContextApi_Included

#include "RexxCore.h"
#include "RexxNativeActivation.hpp"
#include "ActivationApiContexts.hpp"
#include "RexxActivity.hpp"

/**
 * A stack-based API context object used for API stubs.
 */
class ApiContext
{
public:
    /**
     * Initialize an API context from a thread context.
     *
     * @param c      The source context.
     */
    inline ApiContext(RexxThreadContext *c)
    {
        // we need to cleanup on exit
        releaseLock = true;
        activity = contextToActivity(c);
        context = activity->getApiContext();
        context->enableConditionTraps();
        // go acquire the kernel lock and take care of nesting
        activity->enterCurrentThread();
        // we need to validate the thread call context to ensure this
        // is the correct thread
        activity->validateThread();
    }


    /**
     * Initialize an API context from a thread context.
     * this is a nonblocking context.  The extra argument allows
     * the overloads to work
     *
     * @param c      The source context.
     */
    inline ApiContext(RexxThreadContext *c, bool blocking)
    {

        // we need to cleanup on exit
        releaseLock = blocking;
        activity = contextToActivity(c);
        context = activity->getApiContext();
        context->enableConditionTraps();
    }

    /**
     * Initialize an API context from a call context.
     *
     * @param c      The source context.
     */
    inline ApiContext(RexxCallContext *c)
    {
        // we need to cleanup on exit
        releaseLock = true;
        activity = contextToActivity(c);
        context = contextToActivation(c);
        context->enableConditionTraps();
        // go acquire the kernel lock and take care of nesting
        activity->enterCurrentThread();
        // we need to validate the thread call context to ensure this
        // is the correct thread
        activity->validateThread();
    }

    /**
     * Initialize an API context from an exit context.
     *
     * @param c      The source context.
     */
    inline ApiContext(RexxExitContext *c)
    {
        // we need to cleanup on exit
        releaseLock = true;
        activity = contextToActivity(c);
        context = contextToActivation(c);
        context->enableConditionTraps();
        // go acquire the kernel lock and take care of nesting
        activity->enterCurrentThread();
        // we need to validate the thread call context to ensure this
        // is the correct thread
        activity->validateThread();
    }

    /**
     * Initialize an API context from a method context.
     *
     * @param c      The source context.
     */
    inline ApiContext(RexxMethodContext *c)
    {
        // we need to cleanup on exit
        releaseLock = true;
        activity = contextToActivity(c);
        context = contextToActivation(c);
        context->enableConditionTraps();
        // go acquire the kernel lock and take care of nesting
        activity->enterCurrentThread();
        // we need to validate the thread call context to ensure this
        // is the correct thread
        activity->validateThread();
    }

    /**
     * Destructor for an API context.  Releases the interpreter
     * access lock on exit.
     */
    inline ~ApiContext()
    {
        // we only do this sort of cleanup if we really entered on the
        // activity
        if (releaseLock)
        {
            context->disableConditionTraps();
            activity->exitCurrentThread();
        }
    }

    inline RexxObjectPtr ret(RexxObject *o)
    {
        context->createLocalReference(o);
        return (RexxObjectPtr)o;
    }

    /**
     * The activity used for the API callback.
     */
    RexxActivity *activity;
    /**
     * The top-level API context.
     */
    RexxNativeActivation *context;

    /**
     * Indicates whether we need to release the lock on return.
     */
    boolean releaseLock;
};


/**
 * A stack-based API context object used for instance stubs.
 */
class InstanceApiContext
{
public:
    /**
     * Initialize an API context from an instance context.
     *
     * @param c      The source context.
     */
    inline InstanceApiContext(RexxInstance *c)
    {
        instance = ((InstanceContext *)c)->instance;
    }

    /**
     * Destructor for an API context.  Releases the interpreter
     * access lock on exit.
     */
    inline ~InstanceApiContext()
    {
    }

    /**
     * The top-level API context.
     */
    InterpreterInstance *instance;
};

#endif

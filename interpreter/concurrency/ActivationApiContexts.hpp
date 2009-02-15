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
/* REXX Kernel                                                                */
/*                                                                            */
/* Definition of contexts used for API call vector states.                    */
/*                                                                            */
/******************************************************************************/
#ifndef ActivationApiContexts_Included
#define ActivationApiContexts_Included

#include "RexxCore.h"

class RexxActivity;
class RexxNativeActivation;
class InterpreterInstance;

// structure used to hand out a thread context structure for this
// activity.  This stucture contains a RexxThreadContext structure
// followed by a self-referential pointer back to the containing
// activity.  This gives us a quick mapping back to the activity
// associated with the thread context.

typedef struct
{
    RexxInstance instanceContext;           // externalized instance context
    InterpreterInstance *instance;          // the instance this represents
} InstanceContext;

typedef struct
{
    RexxThreadContext threadContext;        // the thread context structure used for the API
    RexxActivity *owningActivity;           // a pointer back to the owning activity
} ActivityContext;

// and similar structures for other API context structures
typedef struct
{
    RexxMethodContext threadContext;        // the thread context structure used for the API
    RexxNativeActivation *context;          // a pointer back to the owning activation
} MethodContext;

typedef struct
{
    RexxCallContext   threadContext;        // the thread context structure used for the API
    RexxNativeActivation *context;          // a pointer back to the owning activation
} CallContext;

typedef struct
{
    RexxExitContext   threadContext;        // the thread context structure used for the API
    RexxNativeActivation *context;          // a pointer back to the owning activation
} ExitContext;


/**
 * Convert a context into the activity the context is associated with.
 *
 * @param c      The calling thread context.
 *
 * @return The activity object the context is associated with.
 */
inline RexxActivity *contextToActivity(RexxThreadContext *c)
{
    return ((ActivityContext *)c)->owningActivity;
}


/**
 * Convert a context into the activity the context is associated with.
 *
 * @param c      The calling thread context.
 *
 * @return The activity object the context is associated with.
 */
inline RexxActivity *contextToActivity(RexxCallContext *c)
{
    return ((ActivityContext *)(c->threadContext))->owningActivity;
}


/**
 * Convert a context into the activity the context is associated with.
 *
 * @param c      The calling thread context.
 *
 * @return The activity object the context is associated with.
 */
inline RexxActivity *contextToActivity(RexxMethodContext *c)
{
    return ((ActivityContext *)(c->threadContext))->owningActivity;
}


/**
 * Convert a context into the activity the context is associated with.
 *
 * @param c      The calling thread context.
 *
 * @return The activity object the context is associated with.
 */
inline RexxActivity *contextToActivity(RexxExitContext *c)
{
    return ((ActivityContext *)(c->threadContext))->owningActivity;
}

#endif


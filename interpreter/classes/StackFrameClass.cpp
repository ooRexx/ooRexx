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
/* Primitive Rexx execution context                                           */
/*                                                                            */
/******************************************************************************/
#include "RexxCore.h"
#include "StackFrameClass.hpp"

RexxClass *StackFrameClass::classInstance = OREF_NULL;   // singleton class instance

/**
 * Create initial bootstrap objects
 */
void StackFrameClass::createInstance()
{
    CLASS_CREATE(StackFrame, "StackFrame", RexxClass);
}


/**
 * Allocate a new RexxContext object
 *
 * @param size   The size of the object.
 *
 * @return The newly allocated object.
 */
void *StackFrameClass::operator new(size_t size)
{
    /* Get new object                    */
    return new_object(size, T_StackFrame);
}


/**
 * Constructor for a RexxContext object.
 *
 * @param a      The activation we're attached to.
 */
StackFrameClass::StackFrameClass(const char *ty, RexxString *n, BaseExecutable *e, RexxObject *tg, RexxArray *a, RexxString *t, size_t l)
{
    type = ty;
    name = n;
    executable = e;
    target = tg;
    arguments = a;
    traceLine = t;
    line = l;
}


/**
 * The Rexx accessible class NEW method.  This raises an
 * error because StackFrame objects can only be created by the
 * internal interpreter.
 *
 * @param args   The NEW args
 * @param argc   The count of arguments
 *
 * @return Never returns.
 */
RexxObject *StackFrameClass::newRexx(RexxObject **args, size_t argc)
{
    // we do not allow these to be allocated from Rexx code...
    reportException(Error_Unsupported_new_method, ((RexxClass *)this)->getId());
    return TheNilObject;
}


void StackFrameClass::live(size_t liveMark)
/******************************************************************************/
/* Function:  Normal garbage collection live marking                          */
/******************************************************************************/
{
    memory_mark(this->name);
    memory_mark(this->executable);
    memory_mark(this->traceLine);
    memory_mark(this->arguments);
    memory_mark(this->target);
}

void StackFrameClass::liveGeneral(int reason)
/******************************************************************************/
/* Function:  Generalized object marking                                      */
/******************************************************************************/
{
    memory_mark_general(this->name);
    memory_mark_general(this->executable);
    memory_mark_general(this->traceLine);
    memory_mark_general(this->arguments);
    memory_mark_general(this->target);
}

void StackFrameClass::flatten(RexxEnvelope *envelope)
/******************************************************************************/
/* Function:  Flatten an object                                               */
/******************************************************************************/
{
  setUpFlatten(StackFrameClass)

  newThis->name = OREF_NULL;   // this never should be getting flattened, so sever the connection
  newThis->executable = OREF_NULL;
  newThis->traceLine = OREF_NULL;
  newThis->arguments = OREF_NULL;
  newThis->target = OREF_NULL;

  cleanUpFlatten
}


/**
 * Return the frame type used to invoke the executable
 *
 * @return The string name of the frame type.
 */
RexxString *StackFrameClass::getType()
{
    return new_string(type);
}


/**
 * Return the name used to invoke the executable
 *
 * @return The message or routine name (or program name, for a
 *         top-level invocation
 */
RexxString *StackFrameClass::getName()
{
    return name;
}


/**
 * Return a trace line for this stack frame
 *
 * @return The message or routine name (or program name, for a
 *         top-level invocation
 */
RexxString *StackFrameClass::getTraceLine()
{
    return traceLine;
}


/**
 * Return the stack frame current line position.
 *
 * @return The current line number of the context.
 */
RexxObject *StackFrameClass::getExecutable()
{
    if (executable == OREF_NULL)
    {
        return TheNilObject;
    }
    return executable;
}

/**
 * Return the execution context current line position.
 *
 * @return The current line number of the context.
 */
RexxObject *StackFrameClass::getLine()
{
    if (line == SIZE_MAX)
    {
        return TheNilObject;
    }
    else
    {
        return new_integer(line);
    }
}

/**
 * Return an array of arguments to the current activation.
 *
 * @return An array of arguments.  Returns an empty array if no
 *         arguments.
 */
RexxArray *StackFrameClass::getArguments()
{
    if (arguments == OREF_NULL)
    {
        return new_array((size_t)0);
    }
    else
    {
        return arguments;
    }
}


/**
 * Get the source object associated with the stack frame.
 *
 * @return The Source object instance for the stack frame.
 */
RexxSource *StackFrameClass::getSourceObject()
{
    if (executable == OREF_NULL)
    {
        return OREF_NULL;
    }

    return executable->getSourceObject();
}


/**
 * Get the message target if this is a method call.
 *
 * @return The target object, or .nil if this stack frame is not for a method call.
 */
RexxObject *StackFrameClass::getTarget()
{
    if (target == OREF_NULL)
    {
        return TheNilObject;
    }
    return target;
}

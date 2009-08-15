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
/* Primitive Rexx exception base class                                        */
/*                                                                            */
/******************************************************************************/
#include "RexxCore.h"
#include "ExceptionClass.hpp"
#include "ProtectedObject.hpp"
#include "DirectoryClass.hpp"
#include "StackFrameClass.hpp"
#include "SourceFile.hpp"
#include "PackageClass.hpp"

RexxClass *ExceptionClass::classInstance = OREF_NULL;   // singleton class instance

/**
 * Create initial bootstrap objects
 */
void ExceptionClass::createInstance()
{
    CLASS_CREATE(Exception, "Exception", RexxClass);
}


/**
 * Allocate a new Exception object
 *
 * @param size   The size of the object.
 *
 * @return The newly allocated object.
 */
void *ExceptionClass::operator new(size_t size)
{
    /* Get new object                    */
    return new_object(size, T_Exception);
}


/**
 * Constructor for an exception object.
 */
ExceptionClass::ExceptionClass()
{
    type = OREF_NULL;
    message = OREF_NULL;
    cause = OREF_NULL;
    stackFrames = OREF_NULL;
    traceBack = OREF_NULL;
}


/**
 * The Rexx accessible class NEW method.
 *
 * @param args   The NEW args
 * @param argc   The count of arguments
 *
 * @return Never returns.
 */
RexxObject *ExceptionClass::newRexx(RexxObject **args, size_t argc)
{
    // create a new weakReference
    RexxObject *newObj = new ExceptionClass();
    // override the behaviour in case this is a subclass
    newObj->setBehaviour(((RexxClass *)this)->getInstanceBehaviour());
    if (((RexxClass *)this)->hasUninitDefined())
    {
        newObj->hasUninit();
    }
    ProtectedObject p(newObj);

                                         /* Initialize the new instance       */
    newObj->sendMessage(OREF_INIT, args, argc);
    return newObj;                       /* return the new instance           */
}


RexxObject *ExceptionClass::init(RexxString *t, RexxString *m, RexxString *d, RexxObject *a, ExceptionClass *c)
{
    type = t;
    if (type == OREF_NULL)
    {
        type = OREF_EXCEPTION;
    }

    message = m;
    if (message == OREF_NULL)
    {
        message = OREF_NULLSTRING;
    }

    description = d;
    if (description == OREF_NULL)
    {
        description = OREF_NULLSTRING;
    }

    additional = a;
    if (additional == OREF_NULL)
    {
        additional = TheNilObject;
    }

    cause = c;
    if (cause == OREF_NULL)
    {
        cause = (ExceptionClass *)TheNilObject;
    }
    fillInStackTrace();
    return OREF_NULL;
}


void ExceptionClass::live(size_t liveMark)
/******************************************************************************/
/* Function:  Normal garbage collection live marking                          */
/******************************************************************************/
{
    memory_mark(this->type);
    memory_mark(this->message);
    memory_mark(this->description);
    memory_mark(this->additional);
    memory_mark(this->cause);
    memory_mark(this->stackFrames);
    memory_mark(this->traceBack);
    memory_mark(this->condition);
}

void ExceptionClass::liveGeneral(int reason)
/******************************************************************************/
/* Function:  Generalized object marking                                      */
/******************************************************************************/
{
    memory_mark_general(this->type);
    memory_mark_general(this->message);
    memory_mark_general(this->description);
    memory_mark_general(this->additional);
    memory_mark_general(this->cause);
    memory_mark_general(this->stackFrames);
    memory_mark_general(this->traceBack);
    memory_mark_general(this->condition);
}

void ExceptionClass::flatten(RexxEnvelope *envelope)
/******************************************************************************/
/* Function:  Flatten an object                                               */
/******************************************************************************/
{
  setUpFlatten(ExceptionClass)

  flatten_reference(newThis->type, envelope);
  flatten_reference(newThis->message, envelope);
  flatten_reference(newThis->description, envelope);
  flatten_reference(newThis->additional, envelope);
  flatten_reference(newThis->cause, envelope);
  flatten_reference(newThis->stackFrames, envelope);
  flatten_reference(newThis->traceBack, envelope);
  flatten_reference(newThis->condition, envelope);

  cleanUpFlatten
}


/**
 * Return the exception type name.
 *
 * @return The string name of the exception type.
 */
RexxString *ExceptionClass::getType()
{
    return type;
}


/**
 * Return the exception message
 *
 * @return The string message associated with the exception
 */
RexxString *ExceptionClass::getMessage()
{
    return message;
}


/**
 * Return the nested exception cause.
 *
 * @return The nested exception object.
 */
ExceptionClass *ExceptionClass::getCause()
{
    return cause;
}


/**
 * Return the stack frames tracing back from the exception
 * creation
 *
 * @return A list of the stack frames.
 */
RexxList *ExceptionClass::getStackFrames()
{
    return stackFrames;
}


/**
 * Get the traceback lines associated with this exception.
 *
 * @return The list of Traceback location lines.
 */
RexxList *ExceptionClass::getTraceBack()
{
    // if we've not generated this yet, do it now
    if (traceBack == OREF_NULL)
    {
        traceBack = new_list();
        for (size_t listIndex = stackFrames->firstIndex();
             listIndex != LIST_END;
             listIndex = stackFrames->nextIndex(listIndex) )
        {
            StackFrameClass *frame = (StackFrameClass *)stackFrames->getValue(listIndex);
            traceBack->append(frame->getTraceLine());
        }
    }
    return traceBack;
}


/**
 * Fill in the stack trace information for the exception.
 */
RexxObject *ExceptionClass::fillInStackTrace()
{
    stackFrames = ActivityManager::currentActivity->generateStackFrames();
    traceBack = OREF_NULL;
    return OREF_NULL;
}


RexxDirectory *ExceptionClass::getCondition()
{
    if (condition == OREF_NULL)
    {
        condition = new_directory();
        // the type is the condition type.
        condition->put(type, OREF_CONDITION);
        condition->put(message, OREF_NAME_MESSAGE);
        condition->put(description, OREF_DESCRIPTION);
        condition->put(additional, OREF_ADDITIONAL);
        // add in the exception and traceback items
        condition->put(stackFrames, OREF_STACKFRAMES);
        condition->put(getTraceBack(), OREF_TRACEBACK);
        StackFrameClass *frame = (StackFrameClass *)stackFrames->firstItem();
        RexxObject *lineNumber = frame->getLine();
        if (lineNumber != TheNilObject)
        {
            // add the line number information
            condition->put(lineNumber, OREF_POSITION);
        }
        RexxSource *source = frame->getSourceObject();
        // fill in the source information if we have it.
        if (source != OREF_NULL)
        {
            condition->put(source->getProgramName(), OREF_PROGRAM);
            condition->put(source->getPackage(), OREF_PACKAGE);
        }
        else
        {
            // if not available, then this is explicitly a NULLSTRINg.
            condition->put(OREF_NULLSTRING, OREF_PROGRAM);
        }
        // fill in the propagation status
        condition->put(TheFalseObject, OREF_PROPAGATED);
    }
    return condition;
}


/**
 * Get the exception description information.
 *
 * @return The description string.
 */
RexxString *ExceptionClass::getDescription()
{
    return description;
}


/**
 * Get the exception additional information.
 *
 * @return The additional information
 */
RexxObject *ExceptionClass::getAdditional()
{
    return additional;
}

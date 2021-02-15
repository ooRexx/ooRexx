/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2020 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* https://www.oorexx.org/license.html                                        */
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
#include "ContextClass.hpp"
#include "RexxActivation.hpp"
#include "SupplierClass.hpp"
#include "DirectoryClass.hpp"
#include "MethodArguments.hpp"

RexxClass *RexxContext::classInstance = OREF_NULL;   // singleton class instance

/**
 * Create initial bootstrap objects
 */
void RexxContext::createInstance()
{
    CLASS_CREATE(RexxContext);
}


/**
 * Allocate a new RexxContext object
 *
 * @param size   The size of the object.
 *
 * @return The newly allocated object.
 */
void *RexxContext::operator new(size_t size)
{
    return new_object(size, T_RexxContext);
}


/**
 * Constructor for a RexxContext object.
 *
 * @param a      The activation we're attached to.
 */
RexxContext::RexxContext(RexxActivation *a)
{
    activation = a;
}


/**
 * The Rexx accessible class NEW method.  This raises an
 * error because RexxContext objects can only be created
 * by the internal interpreter.
 *
 * @param args   The NEW args
 * @param argc   The count of arguments
 *
 * @return Never returns.
 */
RexxObject *RexxContext::newRexx(RexxObject **args, size_t argc)
{
    // we do not allow these to be allocated from Rexx code...
    reportException(Error_Unsupported_new_method, ((RexxClass *)this)->getId());
    return TheNilObject;
}


/**
 * Generalized object marking.
 *
 * @param reason The reason for this live marking operation.
 */
void RexxContext::live(size_t liveMark)
{
    memory_mark(objectVariables);
    memory_mark(activation);
}


/**
 * Generalized object marking.
 *
 * @param reason The reason for this live marking operation.
 */
void RexxContext::liveGeneral(MarkReason reason)
{
    memory_mark_general(objectVariables);
    memory_mark_general(activation);
}


/**
 * An override for the copy method to keep RexxContext
 * objects from being copied.
 *
 * @return Never returns.
 */
RexxObject *RexxContext::copyRexx()
{
    // we do not allow these to be allocated from Rexx code...
    reportException(Error_Unsupported_copy_method, this);
    return TheNilObject;
}


/**
 * Check that the backing RexxActivation is still available.
 */
void RexxContext::checkValid()
{
    if (activation == OREF_NULL)
    {
        reportException(Error_Execution_context_not_active);
    }
}


/**
 * Return the package object for the code that's currently
 * executing.
 *
 * @return The package instance.
 */
PackageClass *RexxContext::getPackage()
{
    checkValid();
    return activation->getPackage();
}


/**
 * Return the current digits setting for the running context
 *
 * @return The current digits value
 */
RexxObject *RexxContext::getDigits()
{
    checkValid();
    return new_integer(activation->digits());
}


/**
 * Return the current fuzz setting for the running context
 *
 * @return The current fuzz value
 */
RexxObject *RexxContext::getFuzz()
{
    checkValid();
    return new_integer(activation->fuzz());
}


/**
 * Return the current form setting for the running context
 *
 * @return The current form value
 */
RexxObject *RexxContext::getForm()
{
    checkValid();
    return activation->form() == Numerics::FORM_SCIENTIFIC ? GlobalNames::SCIENTIFIC : GlobalNames::ENGINEERING;
}


/**
 * Return a supplier for all of the variables in the current
 * context.
 *
 * @return A supplier object for iterating over the variables
 */
RexxObject *RexxContext::getVariables()
{
    checkValid();
    return activation->getAllLocalVariables();
}


/**
 * Return the executable backing the current context
 *
 * @return The executable object (either a method or routine)
 */
RexxObject *RexxContext::getExecutable()
{
    checkValid();
    return activation->getExecutable();
}


/**
 * Return the executable backing the current context
 *
 * @return The executable object (either a method or routine)
 */
RexxObject *RexxContext::getArgs()
{
    checkValid();
    RexxObject **arglist = activation->getMethodArgumentList();
    size_t size = activation->getMethodArgumentCount();
    return new_array(size, arglist);
}


/**
 * Return the current executable condition information
 *
 * @return The condition information
 */
RexxObject *RexxContext::getCondition()
{
    checkValid();
    // we return a copy of the current condition if we have any
    RexxObject *conditionobj = activation->getConditionObj();
    return conditionobj == OREF_NULL ? TheNilObject : (RexxObject *)conditionobj->copy();
}


/**
 * Return the execution context current line position.
 *
 * @return The current line number of the context.
 */
RexxObject *RexxContext::getLine()
{
    checkValid();
    return activation->getContextLine();
}


/**
 * Return the execution context return status
 *
 * @return The .RS value of the context.
 */
RexxObject *RexxContext::getRS()
{
    checkValid();
    return activation->getContextReturnStatus();
}


/**
 * Retrieve the name associated with the current context.
 * If this is the top level, then the name of the package
 * is returned.  For internal call contexts, the label name
 * is returned, and for routines or methods, the name used
 * to invoke the code is returned.
 *
 * @return The appropriate name for this context.
 */
RexxObject *RexxContext::getName()
{
    checkValid();
    return resultOrNil(activation->getCallname());
}

/**
 * Retrieve the stack frames from the current context.
 *
 * @return A list of the current stack frames.
 */
RexxObject *RexxContext::getStackFrames()
{
    checkValid();
    // we don't want to include the stackframes frame in the list, so ask
    // that it be skipped.
    return activation->getStackFrames(true);
}

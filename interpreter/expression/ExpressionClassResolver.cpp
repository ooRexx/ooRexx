/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2014 Rexx Language Association. All rights reserved.    */
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
/* REXX Translator                                                            */
/*                                                                            */
/* Object for resolving class name references.                                */
/*                                                                            */
/******************************************************************************/
#include "RexxCore.h"
#include "StringClass.hpp"
#include "RexxActivation.hpp"
#include "ExpressionClassResolver.hpp"


/**
 * Allocate a new class resolver expression object.
 *
 * @param size   The size of object.
 *
 * @return Storage for creating a dot variable object.
 */
void *ClassResolver::operator new(size_t size)
{
    return new_object(size, T_ClassResolver);
}


/**
 * Construct a Class resolver object
 *
 */
ClassResolver::ClassResolver(RexxString *n, RexxString *c)
{
    namespaceName = n;
    className = c;
}


/**
 * Perform garbage collection on a live object.
 *
 * @param liveMark The current live mark.
 */
void ClassResolver::live(size_t liveMark)
{
    memory_mark(namespaceName);
    memory_mark(className);
}


/**
 * Perform generalized live marking on an object.  This is
 * used when mark-and-sweep processing is needed for purposes
 * other than garbage collection.
 *
 * @param reason The reason for the marking call.
 */
void ClassResolver::liveGeneral(MarkReason reason)
{
    memory_mark_general(namespaceName);
    memory_mark_general(className);
}


/**
 * Flatten a source object.
 *
 * @param envelope The envelope that will hold the flattened object.
 */
void ClassResolver::flatten(Envelope * envelope)
{
    setUpFlatten(ClassResolver)

    flattenRef(namespaceName);
    flattenRef(className);

    cleanUpFlatten
}


/**
 * Evaluate a dot variable symbol.
 *
 * @param context The current evaluation context.
 * @param stack   The current evaluation stack.
 *
 * @return The dot symbol value.
 */
RexxObject *ClassResolver::evaluate(RexxActivation *context, ExpressionStack *stack )
{
    PackageClass *package = context->getPackage();
    // do the lookup, including raising error messages for failures.  We only
    // get called for evaluate because this is a qualified dot-variable lookup.
    // Error messages will have been issued if this did not resolve.
    RexxClass *resolvedClass = lookup(package);

    // evaluate always pushes on the stack.
    stack->push(resolvedClass);
    // trace this if tracing intermediates
    context->traceClassResolution(namespaceName, className, resolvedClass);
    return resolvedClass;
}


/**
 * Resolve a class reference, including handling namespace
 * qualification.  This raises errors for any failure to locate
 * a class.
 *
 * @param package The the source package.
 *
 * @return A resolved class object.  Raises errors for any failures.
 */
RexxClass *ClassResolver::lookup(PackageClass *package)
{
    // if this is not qualified by namespace, do the full class search.
    if (namespaceName == OREF_NULL)
    {
        RexxObject *t = OREF_NULL;   // required for the findClass call

        RexxClass *resolvedClass = package->findClass(className, t);

        // We found something, but it must be a real class object
        if (resolvedClass != OREF_NULL && !resolvedClass->isInstanceOf(TheClassClass))
        {
            reportException(Error_Translation_bad_class, className);
        }
        return resolvedClass;
    }
    // namespace qualified
    else
    {
        PackageClass *namespacePackage = package->findNamespace(namespaceName);
        if (namespacePackage == OREF_NULL)
        {
            reportException(Error_Execution_no_namespace, namespaceName, package->getProgramName());
        }
        // we only look for public classes in the namespaces
        RexxClass *resolvedClass = namespacePackage->findPublicClass(className);
        // we give a specific error for this one
        if (resolvedClass == OREF_NULL)
        {
            reportException(Error_Execution_no_namespace_class, className, namespaceName);
        }

        // We found something, but it must be a real class object
        if (resolvedClass != OREF_NULL && !resolvedClass->isInstanceOf(TheClassClass))
        {
            reportException(Error_Translation_bad_class, className);
        }
        return resolvedClass;
    }
}



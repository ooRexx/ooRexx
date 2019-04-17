/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2019 Rexx Language Association. All rights reserved.    */
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
/******************************************************************************/
/* REXX API support                                                           */
/*                                                                            */
/* Stub functions for all APIs accessed via the NativeMethodContext           */
/*                                                                            */
/******************************************************************************/

#include "RexxCore.h"
#include "ContextApi.hpp"
#include "Activity.hpp"
#include "StringClass.hpp"
#include "IntegerClass.hpp"
#include "BufferClass.hpp"
#include "SupplierClass.hpp"
#include "ArrayClass.hpp"
#include "DirectoryClass.hpp"
#include "MethodClass.hpp"
#include "RoutineClass.hpp"
#include "PackageClass.hpp"
#include "Interpreter.hpp"
#include "InterpreterInstance.hpp"
#include "SystemInterpreter.hpp"
#include "PointerClass.hpp"
#include "ActivityManager.hpp"
#include "RexxStartDispatcher.hpp"
#include "PackageManager.hpp"
#include "MutableBufferClass.hpp"
#include "MethodArguments.hpp"
#include "LanguageParser.hpp"
#include "StemClass.hpp"
#include "NumberStringClass.hpp"
#include "VariableReference.hpp"
#include "StringTableClass.hpp"

BEGIN_EXTERN_C()

void RexxEntry DetachThread(RexxThreadContext *c)
{
    // we do this one without grabbing the lock because we're going away.
    ApiContext context(c, false);
    try
    {
        context.activity->detachThread();
    }
    catch (NativeActivation *)
    {
    }
}


/**
 * API stub for raising a halt condition on a thread.
 */
void RexxEntry HaltThread(RexxThreadContext *c)
{
    ApiContext context(c);
    try
    {
        context.activity->halt(OREF_NULL);
    }
    catch (NativeActivation *)
    {
    }
}


void RexxEntry SetThreadTrace(RexxThreadContext *c, logical_t setting)
{
    ApiContext context(c);
    try
    {
        context.activity->setTrace(setting != 0);
    }
    catch (NativeActivation *)
    {
    }
}


RexxObjectPtr RexxEntry RequestGlobalReference(RexxThreadContext *c, RexxObjectPtr o)
{
    ApiContext context(c);
    try
    {
        context.activity->getInstance()->addGlobalReference((RexxObject *)o);
        return o;
    }
    catch (NativeActivation *)
    {
    }
    return NULLOBJECT;
}


void RexxEntry ReleaseGlobalReference(RexxThreadContext *c, RexxObjectPtr o)
{
    ApiContext context(c);
    try
    {
        context.activity->getInstance()->removeGlobalReference((RexxObject *)o);
    }
    catch (NativeActivation *)
    {
    }
}


void RexxEntry ReleaseLocalReference(RexxThreadContext *c, RexxObjectPtr o)
{
    ApiContext context(c);
    try
    {
        context.context->removeLocalReference((RexxObject *)o);
    }
    catch (NativeActivation *)
    {
    }
}


//NB:  The name "SendMessage" has a conflict with a Windows API, so this name differs from
// the call vector version.
RexxObjectPtr RexxEntry SendMessageArray(RexxThreadContext * c, RexxObjectPtr o, CSTRING m, RexxArrayObject a)
{
    ApiContext context(c);
    try
    {
        Protected<RexxString> message = new_upper_string(m);
        ProtectedObject p;
        return context.ret(((RexxObject *)o)->sendMessage(message, (ArrayClass *)a, p));
    }
    catch (NativeActivation *)
    {
    }
    return NULLOBJECT;
}


RexxObjectPtr RexxEntry SendMessageScoped(RexxThreadContext * c, RexxObjectPtr o, CSTRING m, RexxClassObject s, RexxArrayObject a)
{
    ApiContext context(c);
    try
    {
        Protected<RexxString> message = new_upper_string(m);
        ProtectedObject p;
        return context.ret(((RexxObject *)o)->sendMessage(message, (RexxClass *)s, (ArrayClass *)a, p));
    }
    catch (NativeActivation *)
    {
    }
    return NULLOBJECT;
}



RexxObjectPtr RexxEntry SendMessage0(RexxThreadContext * c, RexxObjectPtr o, CSTRING m)
{
    ApiContext context(c);
    try
    {
        Protected<RexxString> message = new_upper_string(m);
        ProtectedObject p;
        return context.ret(((RexxObject *)o)->sendMessage(message, p));
    }
    catch (NativeActivation *)
    {
    }
    return NULLOBJECT;
}


RexxObjectPtr RexxEntry SendMessage1(RexxThreadContext * c, RexxObjectPtr o, CSTRING m, RexxObjectPtr a1)
{
    ApiContext context(c);
    try
    {
        Protected<RexxString> message = new_upper_string(m);
        ProtectedObject p;
        return context.ret(((RexxObject *)o)->sendMessage(message, (RexxObject *)a1, p));
    }
    catch (NativeActivation *)
    {
    }
    return NULLOBJECT;
}

RexxObjectPtr RexxEntry SendMessage2(RexxThreadContext * c, RexxObjectPtr o, CSTRING m, RexxObjectPtr a1, RexxObjectPtr a2)
{
    ApiContext context(c);
    try
    {
        Protected<RexxString> message = new_upper_string(m);
        ProtectedObject p;
        return context.ret(((RexxObject *)o)->sendMessage(message, (RexxObject *)a1, (RexxObject *)a2, p));
    }
    catch (NativeActivation *)
    {
    }
    return NULLOBJECT;
}

RexxDirectoryObject RexxEntry GetLocalEnvironment(RexxThreadContext *c)
{
    ApiContext context(c);
    try
    {
        return (RexxDirectoryObject)context.activity->getLocal();
    }
    catch (NativeActivation *)
    {
    }
    return NULLOBJECT;
}

RexxDirectoryObject RexxEntry GetGlobalEnvironment(RexxThreadContext *c)
{
    ApiContext context(c);
    try
    {
        return (RexxDirectoryObject)TheEnvironment;
    }
    catch (NativeActivation *)
    {

    }
    return NULLOBJECT;
}

logical_t RexxEntry IsInstanceOf(RexxThreadContext *c, RexxObjectPtr o, RexxClassObject cl)
{
    ApiContext context(c);
    try
    {
        return ((RexxObject *)o)->isInstanceOf((RexxClass *)cl);
    }
    catch (NativeActivation *)
    {
    }
    return 0;
}

logical_t RexxEntry IsOfType(RexxThreadContext *c, RexxObjectPtr o, CSTRING cn)
{
    ApiContext context(c);
    try
    {
        // convert the name to a string instance, and get the class object from
        // our current context
        Protected<RexxString> name = new_upper_string(cn);

        RexxClass *classObject = context.context->findClass(name);
        // if not found, this is always false
        if (classObject == OREF_NULL)
        {
            return false;
        }
        return ((RexxObject *)o)->isInstanceOf(classObject);
    }
    catch (NativeActivation *)
    {
    }
    return 0;
}


logical_t RexxEntry HasMethod(RexxThreadContext *c, RexxObjectPtr o, CSTRING n)
{
    ApiContext context(c);
    try
    {
        Protected<RexxString> name = new_upper_string(n);
        // convert the name to a string instance, and check the environments.
        return ((RexxObject *)o)->hasMethod(name);

    }
    catch (NativeActivation *)
    {
    }
    return 0;
}


RexxPackageObject RexxEntry LoadPackage(RexxThreadContext *c, CSTRING n)
{
    ApiContext context(c);
    try
    {
        Protected<RexxString> name = new_string(n);
        RexxString *resolvedName = context.activity->resolveProgramName(name, OREF_NULL, OREF_NULL);

        // convert the name to a string instance, and check the environments.
        return (RexxPackageObject)context.ret(context.activity->getInstance()->loadRequires(context.activity, name, resolvedName));
    }
    catch (NativeActivation *)
    {
    }
    return NULLOBJECT;
}


RexxPackageObject RexxEntry LoadPackageFromData(RexxThreadContext *c, CSTRING n, CSTRING d, size_t l)
{
    ApiContext context(c);
    try
    {
        Protected<RexxString> name = new_string(n);
        return (RexxPackageObject)context.ret(context.activity->getInstance()->loadRequires(context.activity, name, d, l));
    }
    catch (NativeActivation *)
    {
    }
    return NULLOBJECT;
}


logical_t RexxEntry LoadLibraryPackage(RexxThreadContext *c, CSTRING n)
{
    ApiContext context(c);
    try
    {
        Protected<RexxString> name = new_string(n);

        // convert the name to a string instance, and check the environments.
        return PackageManager::loadLibrary(name) != OREF_NULL;
    }
    catch (NativeActivation *)
    {
    }
    return false;
}


logical_t RexxEntry RegisterLibrary(RexxThreadContext *c, CSTRING n, RexxPackageEntry *e)
{
    ApiContext context(c);
    try
    {
        Protected<RexxString> name = new_string(n);

        // convert the name to a string instance, and check the environments.
        return PackageManager::registerPackage(name, e);
    }
    catch (NativeActivation *)
    {
    }
    return false;
}


RexxClassObject RexxEntry FindClass(RexxThreadContext *c, CSTRING n)
{
    ApiContext context(c);
    try
    {
        // convert the name to a string instance, and check the environments.
        Protected<RexxString> name = new_upper_string(n);

        return (RexxClassObject)context.ret(context.context->findClass(name));

    }
    catch (NativeActivation *)
    {
    }
    return NULLOBJECT;
}


RexxClassObject RexxEntry FindClassFromPackage(RexxThreadContext *c, RexxPackageObject m, CSTRING n)
{
    ApiContext context(c);
    try
    {
        // convert the name to a string instance, and check the environments.
        Protected<RexxString> name = new_upper_string(n);

        return (RexxClassObject)context.ret(((PackageClass *)m)->findClass(name));

        RexxObject *t = OREF_NULL;   // required for the findClass call
        return (RexxClassObject)context.ret(((PackageClass *)m)->findClass(name, t));

    }
    catch (NativeActivation *)
    {
    }
    return NULLOBJECT;
}


RexxDirectoryObject RexxEntry GetPackageRoutines(RexxThreadContext *c, RexxPackageObject m)
{
    ApiContext context(c);
    try
    {
        return (RexxDirectoryObject)context.ret(((PackageClass *)m)->getRoutinesRexx());
    }
    catch (NativeActivation *)
    {
    }
    return NULLOBJECT;
}


RexxDirectoryObject RexxEntry GetPackagePublicRoutines(RexxThreadContext *c, RexxPackageObject m)
{
    ApiContext context(c);
    try
    {
        return (RexxDirectoryObject)context.ret(((PackageClass *)m)->getPublicRoutinesRexx());
    }
    catch (NativeActivation *)
    {
    }
    return NULLOBJECT;
}

RexxDirectoryObject RexxEntry GetPackageClasses(RexxThreadContext *c, RexxPackageObject m)
{
    ApiContext context(c);
    try
    {
        return (RexxDirectoryObject)context.ret(((PackageClass *)m)->getClassesRexx());
    }
    catch (NativeActivation *)
    {
    }
    return NULLOBJECT;
}

RexxDirectoryObject RexxEntry GetPackagePublicClasses(RexxThreadContext *c, RexxPackageObject m)
{
    ApiContext context(c);
    try
    {
        return (RexxDirectoryObject)context.ret(((PackageClass *)m)->getPublicClassesRexx());
    }
    catch (NativeActivation *)
    {
    }
    return NULLOBJECT;
}

RexxDirectoryObject RexxEntry GetPackageMethods(RexxThreadContext *c, RexxPackageObject m)
{
    ApiContext context(c);
    try
    {
        return (RexxDirectoryObject)context.ret(((PackageClass *)m)->getMethodsRexx());
    }
    catch (NativeActivation *)
    {
    }
    return NULLOBJECT;
}


RexxObjectPtr RexxEntry CallRoutine(RexxThreadContext *c, RexxRoutineObject r, RexxArrayObject a)
{
    ApiContext context(c);
    try
    {
        CallRoutineDispatcher dispatcher((RoutineClass *)r, (ArrayClass *)a);
        context.activity->run(dispatcher);
        return (RexxObjectPtr)context.ret(dispatcher.result);
    }
    catch (NativeActivation *)
    {
    }
    return NULLOBJECT;
}


RexxObjectPtr RexxEntry CallProgram(RexxThreadContext *c, const char * p, RexxArrayObject a)
{
    ApiContext context(c);
    try
    {
        CallProgramDispatcher dispatcher(p, (ArrayClass *)a);
        context.activity->run(dispatcher);
        return (RexxObjectPtr)context.ret(dispatcher.result);
    }
    catch (NativeActivation *)
    {
    }
    return NULLOBJECT;
}


RexxMethodObject RexxEntry NewMethod(RexxThreadContext *c, CSTRING n, CSTRING s, stringsize_t l)
{
    ApiContext context(c);
    try
    {
        Protected<RexxString> name = new_string(n);
        Protected<BufferClass> source = new_buffer(s, l);
        // convert the name to a string instance, and check the environments.
        return (RexxMethodObject)context.ret(LanguageParser::createMethod(name, source));
    }
    catch (NativeActivation *)
    {
    }
    return NULLOBJECT;
}


RexxRoutineObject RexxEntry NewRoutine(RexxThreadContext *c, CSTRING n, CSTRING s, stringsize_t l)
{
    ApiContext context(c);
    try
    {
        Protected<RexxString> name = new_string(n);
        Protected<BufferClass> source = new_buffer(s, l);

        // convert the name to a string instance, and check the environments.
        return (RexxRoutineObject)context.ret(LanguageParser::createRoutine(name, source, OREF_NULL));
    }
    catch (NativeActivation *)
    {
    }
    return NULLOBJECT;
}


logical_t RexxEntry IsRoutine(RexxThreadContext *c, RexxObjectPtr o)
{
    ApiContext context(c);
    try
    {
        // convert the name to a string instance, and check the environments.
        return (logical_t)((RexxObject *)o)->isInstanceOf(TheRoutineClass);
    }
    catch (NativeActivation *)
    {
    }
    return false;
}


logical_t RexxEntry IsMethod(RexxThreadContext *c, RexxObjectPtr o)
{
    ApiContext context(c);
    try
    {
        // convert the name to a string instance, and check the environments.
        return (logical_t)((RexxObject *)o)->isInstanceOf(TheMethodClass);
    }
    catch (NativeActivation *)
    {
    }
    return false;
}


RexxPackageObject RexxEntry GetRoutinePackage(RexxThreadContext *c, RexxRoutineObject o)
{
    ApiContext context(c);
    try
    {
        // convert the name to a string instance, and check the environments.
        return (RexxPackageObject)context.ret(((RoutineClass *)o)->getPackage());
    }
    catch (NativeActivation *)
    {
    }
    return (RexxPackageObject)NULL;
}


RexxPackageObject RexxEntry GetMethodPackage(RexxThreadContext *c, RexxMethodObject o)
{
    ApiContext context(c);
    try
    {
        // convert the name to a string instance, and check the environments.
        return (RexxPackageObject)context.ret(((MethodClass *)o)->getPackage());
    }
    catch (NativeActivation *)
    {
    }
    return (RexxPackageObject)NULL;
}


POINTER RexxEntry ObjectToCSelf(RexxThreadContext *c, RexxObjectPtr o)
{
    ApiContext context(c);
    try
    {
        // ask the object to figure this out
        return ((RexxObject *)o)->getCSelf();
    }
    catch (NativeActivation *)
    {
    }
    return NULL;
}


POINTER RexxEntry ObjectToCSelfScoped(RexxThreadContext *c, RexxObjectPtr o, RexxObjectPtr s)
{
    ApiContext context(c);
    try
    {
        // ask the object to figure this out
        return ((RexxObject *)o)->getCSelf((RexxClass *)s);
    }
    catch (NativeActivation *)
    {
    }
    return NULL;
}


RexxObjectPtr RexxEntry WholeNumberToObject(RexxThreadContext *c, wholenumber_t n)
{
    ApiContext context(c);
    try
    {
        return context.ret(Numerics::wholenumberToObject((wholenumber_t)n));
    }
    catch (NativeActivation *)
    {
    }
    return NULLOBJECT;
}

RexxObjectPtr RexxEntry UintptrToObject(RexxThreadContext *c, uintptr_t n)
{
    ApiContext context(c);
    try
    {
        return context.ret(Numerics::uintptrToObject(n));
    }
    catch (NativeActivation *)
    {
    }
    return NULLOBJECT;
}

RexxObjectPtr RexxEntry IntptrToObject(RexxThreadContext *c, intptr_t n)
{
    ApiContext context(c);
    try
    {
        return context.ret(Numerics::intptrToObject(n));
    }
    catch (NativeActivation *)
    {
    }
    return NULLOBJECT;
}


RexxObjectPtr RexxEntry ValueToObject(RexxThreadContext *c, ValueDescriptor *d)
{
    ApiContext context(c);
    try
    {
        return (RexxObjectPtr)context.ret(context.context->valueToObject(d));
    }
    catch (NativeActivation *)
    {
        // we want this to fail without raising an error
        context.context->clearException();
    }
    return NULLOBJECT;
}


RexxArrayObject RexxEntry ValuesToObject(RexxThreadContext *c, ValueDescriptor *d, size_t count)
{
    ApiContext context(c);
    try
    {
        return (RexxArrayObject)context.ret(context.context->valuesToObject(d, count));
    }
    catch (NativeActivation *)
    {
        // we want this to fail without raising an error
        context.context->clearException();
    }
    return NULLOBJECT;
}


logical_t RexxEntry ObjectToValue(RexxThreadContext *c, RexxObjectPtr o, ValueDescriptor *d)
{
    ApiContext context(c);
    try
    {
        return context.context->objectToValue((RexxObject *)o, d);
    }
    catch (NativeActivation *)
    {
        // some conversion failures result in an exception...cancel that, and
        // just return FALSE;
        context.context->clearException();
    }
    return false;
}

RexxObjectPtr RexxEntry StringSizeToObject(RexxThreadContext *c, stringsize_t n)
{
    ApiContext context(c);
    try
    {
        return context.ret(Numerics::stringsizeToObject((stringsize_t)n));
    }
    catch (NativeActivation *)
    {
    }
    return NULLOBJECT;
}


logical_t RexxEntry ObjectToWholeNumber(RexxThreadContext *c, RexxObjectPtr o, wholenumber_t *n)
{
    ApiContext context(c);
    try
    {
        wholenumber_t temp;
        // this uses the entire value range
        // NB:  SSIZE_MIN appears to be defined as 0 for some bizarre reason on some platforms,
        // so we'll make things relative to SIZE_MAX.
        if (Numerics::objectToWholeNumber((RexxObject *)o, temp, Numerics::MAX_WHOLENUMBER, Numerics::MIN_WHOLENUMBER))
        {
            *n = (wholenumber_t)temp;
            return true;
        }
        return false;
    }
    catch (NativeActivation *)
    {
    }
    return 0;
}


RexxObjectPtr RexxEntry Int32ToObject(RexxThreadContext *c, int32_t n)
{
    ApiContext context(c);
    try
    {
        return context.ret(Numerics::wholenumberToObject((wholenumber_t)n));
    }
    catch (NativeActivation *)
    {
    }
    return NULLOBJECT;
}


RexxObjectPtr RexxEntry UnsignedInt32ToObject(RexxThreadContext *c, uint32_t n)
{
    ApiContext context(c);
    try
    {
        return context.ret(Numerics::stringsizeToObject((uint32_t)n));
    }
    catch (NativeActivation *)
    {
    }
    return NULLOBJECT;
}


logical_t RexxEntry ObjectToStringSize(RexxThreadContext * c, RexxObjectPtr o, stringsize_t * n)
{
    ApiContext context(c);
    try
    {
        stringsize_t temp;
        // this uses the entire value range
        if (Numerics::objectToStringSize((RexxObject *)o, temp, Numerics::MAX_WHOLENUMBER))
        {
            *n = (stringsize_t)temp;
            return true;
        }
        return false;
    }
    catch (NativeActivation *)
    {
    }
    return 0;
}


logical_t RexxEntry ObjectToInt32(RexxThreadContext *c, RexxObjectPtr o, int32_t *n)
{
    ApiContext context(c);
    try
    {
        ssize_t temp;
        // this uses the entire value range
        // NB:  SSIZE_MIN appears to be defined as 0 for some bizarre reason on some platforms,
        // so we'll make things relative to SIZE_MAX.
        if (Numerics::objectToSignedInteger((RexxObject *)o, temp, INT32_MAX, INT32_MIN))
        {
            *n = (int32_t)temp;
            return true;
        }
        return false;
    }
    catch (NativeActivation *)
    {
    }
    return 0;
}


logical_t RexxEntry ObjectToUnsignedInt32(RexxThreadContext * c, RexxObjectPtr o, uint32_t *n)
{
    ApiContext context(c);
    try
    {
        size_t temp;
        // this uses the entire value range
        if (Numerics::objectToUnsignedInteger((RexxObject *)o, temp, UINT32_MAX))
        {
            *n = (uint32_t)temp;
            return true;
        }
        return false;
    }
    catch (NativeActivation *)
    {
    }
    return 0;
}


RexxObjectPtr RexxEntry Int64ToObject(RexxThreadContext *c, int64_t n)
{
    ApiContext context(c);
    try
    {
        return context.ret(Numerics::int64ToObject(n));
    }
    catch (NativeActivation *)
    {
    }
    return NULLOBJECT;
}

RexxObjectPtr RexxEntry UnsignedInt64ToObject(RexxThreadContext * c, uint64_t n)
{
    ApiContext context(c);
    try
    {
        return context.ret(Numerics::uint64ToObject(n));
    }
    catch (NativeActivation *)
    {
    }
    return NULLOBJECT;
}

logical_t RexxEntry ObjectToInt64(RexxThreadContext *c, RexxObjectPtr o, int64_t * n)
{
    ApiContext context(c);
    try
    {
        // this uses the entire value range
        return Numerics::objectToInt64((RexxObject *)o, *n);
    }
    catch (NativeActivation *)
    {
    }
    return 0;
}

logical_t RexxEntry ObjectToUnsignedInt64(RexxThreadContext *c, RexxObjectPtr o, uint64_t *n)
{
    ApiContext context(c);
    try
    {
        // this uses the entire value range
        return Numerics::objectToUnsignedInt64((RexxObject *)o, *n);
    }
    catch (NativeActivation *)
    {
    }
    return 0;
}

logical_t RexxEntry ObjectToUintptr(RexxThreadContext * c, RexxObjectPtr o, uintptr_t * n)
{
    ApiContext context(c);
    try
    {
        // this uses the entire value range
        return Numerics::objectToUintptr((RexxObject *)o, *n);
    }
    catch (NativeActivation *)
    {
    }
    return 0;
}

logical_t RexxEntry ObjectToIntptr(RexxThreadContext * c, RexxObjectPtr o, intptr_t * n)
{
    ApiContext context(c);
    try
    {
        // this uses the entire value range
        return Numerics::objectToIntptr((RexxObject *)o, *n);
    }
    catch (NativeActivation *)
    {
    }
    return 0;
}

logical_t RexxEntry ObjectToLogical(RexxThreadContext * c, RexxObjectPtr o, logical_t * n)
{
    ApiContext context(c);
    try
    {
        // this uses the entire value range
        return ((RexxObject *)o)->logicalValue(*n);
    }
    catch (NativeActivation *)
    {
    }
    return 0;
}

RexxObjectPtr RexxEntry LogicalToObject(RexxThreadContext *c, logical_t n)
{
    ApiContext context(c);
    try
    {
        return (RexxObjectPtr)booleanObject(n != 0);
    }
    catch (NativeActivation *)
    {
    }
    return NULLOBJECT;
}

RexxObjectPtr RexxEntry DoubleToObject(RexxThreadContext *c, double n)
{
    ApiContext context(c);
    try
    {
        return context.ret(new_numberstringFromDouble(n));
    }
    catch (NativeActivation *)
    {
    }
    return NULLOBJECT;
}

RexxObjectPtr RexxEntry DoubleToObjectWithPrecision(RexxThreadContext *c, double n, size_t precision)
{
    ApiContext context(c);
    try
    {
        return context.ret(new_numberstringFromDouble(n, precision));
    }
    catch (NativeActivation *)
    {
    }
    return NULLOBJECT;
}

logical_t RexxEntry ObjectToDouble(RexxThreadContext *c, RexxObjectPtr o, double *n)
{
    ApiContext context(c);
    try
    {
        return ((RexxObject *)o)->doubleValue(*n);
    }
    catch (NativeActivation *)
    {
    }
    return 0;
}

RexxStringObject RexxEntry ObjectToString(RexxThreadContext *c, RexxObjectPtr o)
{
    ApiContext context(c);
    try
    {
        return (RexxStringObject)context.ret(((RexxObject *)o)->requestString());
    }
    catch (NativeActivation *)
    {
    }
    return NULLOBJECT;
}

CSTRING RexxEntry ObjectToStringValue(RexxThreadContext *c, RexxObjectPtr o)
{
    ApiContext context(c);
    try
    {
        RexxString *temp = ((RexxObject *)o)->requestString();
        context.ret(temp);
        return (CSTRING)temp->getStringData();
    }
    catch (NativeActivation *)
    {
    }
    return NULL;
}

size_t RexxEntry StringGet(RexxThreadContext *c, RexxStringObject s, size_t o, POINTER r, size_t l)
{
    ApiContext context(c);
    try
    {
        RexxString *temp = (RexxString *)s;
        return temp->copyData(o - 1, (char *)r, l);
    }
    catch (NativeActivation *)
    {
    }
    return 0;
}

size_t RexxEntry StringLength(RexxThreadContext *c, RexxStringObject s)
{
    ApiContext context(c);
    try
    {
        RexxString *temp = (RexxString *)s;
        return temp->getLength();
    }
    catch (NativeActivation *)
    {
    }
    return 0;
}

CSTRING RexxEntry StringData(RexxThreadContext *c, RexxStringObject s)
{
    ApiContext context(c);
    try
    {
        RexxString *temp = (RexxString *)s;
        return (CSTRING)temp->getStringData();
    }
    catch (NativeActivation *)
    {
    }
    return NULL;
}

RexxStringObject RexxEntry NewString(RexxThreadContext *c, CSTRING s, size_t l)
{
    ApiContext context(c);
    try
    {
        return (RexxStringObject)context.ret(new_string(s, (stringsize_t)l));
    }
    catch (NativeActivation *)
    {
    }
    return NULLOBJECT;
}

RexxStringObject RexxEntry NewStringFromAsciiz(RexxThreadContext *c, CSTRING s)
{
    ApiContext context(c);
    try
    {
        return (RexxStringObject)context.ret(new_string((const char *)s));
    }
    catch (NativeActivation *)
    {
    }
    return NULLOBJECT;
}

RexxStringObject RexxEntry StringUpper(RexxThreadContext *c, RexxStringObject s)
{
    ApiContext context(c);
    try
    {
        RexxString *temp = (RexxString *)s;
        return (RexxStringObject)context.ret(temp->upper());
    }
    catch (NativeActivation *)
    {
    }
    return NULLOBJECT;
}

RexxStringObject RexxEntry StringLower(RexxThreadContext *c, RexxStringObject s)
{
    ApiContext context(c);
    try
    {
        RexxString *temp = (RexxString *)s;
        return (RexxStringObject)context.ret(temp->lower());
    }
    catch (NativeActivation *)
    {
    }
    return NULLOBJECT;
}

logical_t RexxEntry IsString(RexxThreadContext *c, RexxObjectPtr o)
{
    ApiContext context(c);
    try
    {
        return isString((RexxObject *)o);
    }
    catch (NativeActivation *)
    {
    }
    return 0;
}

RexxBufferStringObject RexxEntry NewBufferString(RexxThreadContext * c, size_t l)
{
    ApiContext context(c);
    try
    {
        return (RexxBufferStringObject)context.ret(raw_string((stringsize_t)l));
    }
    catch (NativeActivation *)
    {
    }
    return NULLOBJECT;
}

size_t  RexxEntry BufferStringLength(RexxThreadContext *c, RexxBufferStringObject s)
{
    ApiContext context(c);
    try
    {
        RexxString *temp = (RexxString *)s;
        return temp->getLength();
    }
    catch (NativeActivation *)
    {
    }
    return 0;
}

POINTER RexxEntry BufferStringData(RexxThreadContext *c, RexxBufferStringObject s)
{
    ApiContext context(c);
    try
    {
        RexxString *temp = (RexxString *)s;
        return (POINTER)temp->getWritableData();
    }
    catch (NativeActivation *)
    {
    }
    return NULL;
}

RexxStringObject RexxEntry FinishBufferString(RexxThreadContext *c, RexxBufferStringObject s, size_t l)
{
    ApiContext context(c);
    try
    {
        RexxString *temp = (RexxString *)s;
        temp->finish(l);
        return s;
    }
    catch (NativeActivation *)
    {
    }
    return NULL;
}

void  RexxEntry DirectoryPut(RexxThreadContext *c, RexxDirectoryObject t, RexxObjectPtr o, CSTRING i)
{
    ApiContext context(c);
    try
    {
        Protected<RexxString> index = new_string(i);
        ((DirectoryClass *)t)->put((RexxObject *)o, index);
    }
    catch (NativeActivation *)
    {
    }
}

RexxObjectPtr RexxEntry DirectoryAt(RexxThreadContext *c, RexxDirectoryObject t, CSTRING i)
{
    ApiContext context(c);
    try
    {
        Protected<RexxString> index = new_string(i);
        return context.ret(((DirectoryClass *)t)->get(index));
    }
    catch (NativeActivation *)
    {
    }
    return OREF_NULL;
}

RexxObjectPtr RexxEntry DirectoryRemove(RexxThreadContext *c, RexxDirectoryObject t, CSTRING i)
{
    ApiContext context(c);
    try
    {
        Protected<RexxString> index = new_string(i);
        return context.ret(((DirectoryClass *)t)->remove(index));
    }
    catch (NativeActivation *)
    {
    }
    return OREF_NULL;
}

RexxDirectoryObject RexxEntry NewDirectory(RexxThreadContext *c)
{
    ApiContext context(c);
    try
    {
        return (RexxDirectoryObject)context.ret(new_directory());
    }
    catch (NativeActivation *)
    {
    }
    return OREF_NULL;
}

logical_t RexxEntry IsDirectory(RexxThreadContext *c, RexxObjectPtr o)
{
    ApiContext context(c);
    try
    {
        return isOfClass(Directory, (RexxObject *)o);
    }
    catch (NativeActivation *)
    {
    }
    return false;
}

void  RexxEntry StringTablePut(RexxThreadContext *c, RexxStringTableObject t, RexxObjectPtr o, CSTRING i)
{
    ApiContext context(c);
    try
    {
        Protected<RexxString> index = new_string(i);
        ((StringTable *)t)->put((RexxObject *)o, index);
    }
    catch (NativeActivation *)
    {
    }
}

RexxObjectPtr RexxEntry StringTableAt(RexxThreadContext *c, RexxStringTableObject t, CSTRING i)
{
    ApiContext context(c);
    try
    {
        Protected<RexxString> index = new_string(i);
        return context.ret(((StringTable *)t)->get(index));
    }
    catch (NativeActivation *)
    {
    }
    return OREF_NULL;
}

RexxObjectPtr RexxEntry StringTableRemove(RexxThreadContext *c, RexxStringTableObject t, CSTRING i)
{
    ApiContext context(c);
    try
    {
        Protected<RexxString> index = new_string(i);
        return context.ret(((StringTable *)t)->remove(index));
    }
    catch (NativeActivation *)
    {
    }
    return OREF_NULL;
}

RexxStringTableObject RexxEntry NewStringTable(RexxThreadContext *c)
{
    ApiContext context(c);
    try
    {
        return (RexxStringTableObject)context.ret(new_string_table());
    }
    catch (NativeActivation *)
    {
    }
    return OREF_NULL;
}

logical_t RexxEntry IsStringTable(RexxThreadContext *c, RexxObjectPtr o)
{
    ApiContext context(c);
    try
    {
        return isOfClass(StringTable, (RexxObject *)o);
    }
    catch (NativeActivation *)
    {
    }
    return false;
}

RexxObjectPtr RexxEntry ArrayAt(RexxThreadContext *c, RexxArrayObject a, size_t i)
{
    ApiContext context(c);
    try
    {
        if (i == 0)
        {
            reportException(Error_Incorrect_method_positive, 1);
        }
        return (RexxObjectPtr)context.ret(((ArrayClass *)a)->safeGet(i));
    }
    catch (NativeActivation *)
    {
    }
    return OREF_NULL;
}


void RexxEntry ArrayPut(RexxThreadContext *c, RexxArrayObject a, RexxObjectPtr o, size_t i)
{
    ApiContext context(c);
    try
    {
        if (i == 0)
        {
            reportException(Error_Incorrect_method_positive, 2);
        }
        ((ArrayClass *)a)->put((RexxObject *)o, i);
    }
    catch (NativeActivation *)
    {
    }
}


size_t RexxEntry ArrayAppend(RexxThreadContext *c, RexxArrayObject a, RexxObjectPtr o)
{
    ApiContext context(c);
    try
    {
        return ((ArrayClass *)a)->append((RexxObject *)o);
    }
    catch (NativeActivation *)
    {
        return 0;
    }
}


size_t RexxEntry ArrayAppendString(RexxThreadContext *c, RexxArrayObject a, CSTRING s, size_t l)
{
    ApiContext context(c);
    try
    {
        Protected<RexxString> str = new_string(s, (stringsize_t)l);
        return ((ArrayClass *)a)->append(str);
    }
    catch (NativeActivation *)
    {
        return 0;
    }
}


size_t RexxEntry ArraySize(RexxThreadContext *c, RexxArrayObject a)
{
    ApiContext context(c);
    try
    {
        return ((ArrayClass *)a)->size();
    }
    catch (NativeActivation *)
    {
    }
    return 0;
}


size_t RexxEntry ArrayItems(RexxThreadContext *c, RexxArrayObject a)
{
    ApiContext context(c);
    try
    {
        return ((ArrayClass *)a)->items();
    }
    catch (NativeActivation *)
    {
    }
    return 0;
}

size_t RexxEntry ArrayDimension(RexxThreadContext *c, RexxArrayObject a)
{
    ApiContext context(c);
    try
    {
        return ((ArrayClass *)a)->getDimensions();
    }
    catch (NativeActivation *)
    {
    }
    return 0;
}

RexxArrayObject RexxEntry NewArray(RexxThreadContext *c, size_t s)
{
    ApiContext context(c);
    try
    {
        return (RexxArrayObject)context.ret(new_array(s));
    }
    catch (NativeActivation *)
    {
    }
    return NULLOBJECT;
}

RexxArrayObject RexxEntry ArrayOfOne(RexxThreadContext *c, RexxObjectPtr o)
{
    ApiContext context(c);
    try
    {
        return (RexxArrayObject)context.ret(new_array((RexxObject *)o));
    }
    catch (NativeActivation *)
    {
    }
    return NULLOBJECT;
}

RexxArrayObject RexxEntry ArrayOfTwo(RexxThreadContext *c, RexxObjectPtr o1, RexxObjectPtr o2)
{
    ApiContext context(c);
    try
    {
        return (RexxArrayObject)context.ret(new_array((RexxObject *)o1, (RexxObject *)o2));
    }
    catch (NativeActivation *)
    {
    }
    return NULLOBJECT;
}

RexxArrayObject RexxEntry ArrayOfThree(RexxThreadContext *c, RexxObjectPtr o1, RexxObjectPtr o2, RexxObjectPtr o3)
{
    ApiContext context(c);
    try
    {
        return (RexxArrayObject)context.ret(new_array((RexxObject *)o1, (RexxObject *)o2, (RexxObject *)o3));
    }
    catch (NativeActivation *)
    {
    }
    return NULLOBJECT;
}

RexxArrayObject RexxEntry ArrayOfFour(RexxThreadContext *c, RexxObjectPtr o1, RexxObjectPtr o2, RexxObjectPtr o3, RexxObjectPtr o4)
{
    ApiContext context(c);
    try
    {
        return (RexxArrayObject)context.ret(new_array((RexxObject *)o1, (RexxObject *)o2, (RexxObject *)o3, (RexxObject *)o4));
    }
    catch (NativeActivation *)
    {
    }
    return NULLOBJECT;
}


logical_t RexxEntry IsArray(RexxThreadContext *c, RexxObjectPtr o)
{
    ApiContext context(c);
    try
    {
        return isArray((RexxObject *)o);
    }
    catch (NativeActivation *)
    {
    }
    return false;
}

POINTER RexxEntry BufferData(RexxThreadContext *c, RexxBufferObject b)
{
    ApiContext context(c);
    try
    {
        return (POINTER)((BufferClass *)b)->getData();
    }
    catch (NativeActivation *)
    {
    }
    return NULL;
}

size_t RexxEntry BufferLength(RexxThreadContext *c, RexxBufferObject b)
{
    ApiContext context(c);
    try
    {
        return ((BufferClass *)b)->getDataLength();
    }
    catch (NativeActivation *)
    {
    }
    return 0;
}

RexxBufferObject RexxEntry NewBuffer(RexxThreadContext *c, size_t l)
{
    ApiContext context(c);
    try
    {
        return (RexxBufferObject)context.ret((RexxObject *)new_buffer(l));
    }
    catch (NativeActivation *)
    {
    }
    return NULLOBJECT;
}

logical_t RexxEntry IsBuffer(RexxThreadContext *c, RexxObjectPtr o)
{
    ApiContext context(c);
    try
    {
        return isOfClass(Buffer, (RexxObject *)o);
    }
    catch (NativeActivation *)
    {
    }
    return false;
}

POINTER RexxEntry PointerValue(RexxThreadContext *c, RexxPointerObject o)
{
    ApiContext context(c);
    try
    {
        return (POINTER)((PointerClass *)o)->pointer();
    }
    catch (NativeActivation *)
    {
    }
    return NULL;
}

RexxPointerObject RexxEntry NewPointer(RexxThreadContext *c, POINTER p)
{
    ApiContext context(c);
    try
    {
        return (RexxPointerObject)context.ret(new_pointer(p));
    }
    catch (NativeActivation *)
    {
    }
    return NULLOBJECT;
}

logical_t RexxEntry IsPointer(RexxThreadContext *c, RexxObjectPtr o)
{
    ApiContext context(c);
    try
    {
        return isOfClass(Pointer, (RexxObject *)o);
    }
    catch (NativeActivation *)
    {
    }
    return false;
}

logical_t RexxEntry IsVariableReference(RexxThreadContext *c, RexxObjectPtr o)
{
    ApiContext context(c);
    try
    {
        return isOfClass(VariableReference, (RexxObject *)o);
    }
    catch (NativeActivation *)
    {
    }
    return false;
}

RexxStringObject RexxEntry VariableReferenceName(RexxThreadContext *c, RexxVariableReferenceObject o)
{
    ApiContext context(c);
    try
    {
        return (RexxStringObject)context.ret(((VariableReference *)o)->getName());
    }
    catch (NativeActivation *)
    {
    }
    return NULLOBJECT;
}

RexxObjectPtr RexxEntry VariableReferenceValue(RexxThreadContext *c, RexxVariableReferenceObject o)
{
    ApiContext context(c);
    try
    {
        return context.ret(((VariableReference *)o)->getValue());
    }
    catch (NativeActivation *)
    {
    }
    return NULLOBJECT;
}

void RexxEntry SetVariableReferenceValue(RexxThreadContext *c, RexxVariableReferenceObject o, RexxObjectPtr v)
{
    ApiContext context(c);
    try
    {
        ((VariableReference *)o)->setValue((RexxObject *)v);
    }
    catch (NativeActivation *)
    {
    }
}

RexxObjectPtr RexxEntry SupplierItem(RexxThreadContext *c, RexxSupplierObject o)
{
    ApiContext context(c);
    try
    {
        return context.ret(((SupplierClass *)o)->item());
    }
    catch (NativeActivation *)
    {
    }
    return NULLOBJECT;
}

RexxObjectPtr RexxEntry SupplierIndex(RexxThreadContext *c, RexxSupplierObject o)
{
    ApiContext context(c);
    try
    {
        return context.ret(((SupplierClass *)o)->index());
    }
    catch (NativeActivation *)
    {
    }
    return NULLOBJECT;
}

logical_t RexxEntry SupplierAvailable(RexxThreadContext *c, RexxSupplierObject o)
{
    ApiContext context(c);
    try
    {
        return ((SupplierClass *)o)->isAvailable();
    }
    catch (NativeActivation *)
    {
    }
    return false;
}

void RexxEntry SupplierNext(RexxThreadContext *c, RexxSupplierObject o)
{
    ApiContext context(c);
    try
    {
        ((SupplierClass *)o)->next();
    }
    catch (NativeActivation *)
    {
    }
}

RexxSupplierObject RexxEntry NewSupplier(RexxThreadContext *c, RexxArrayObject values, RexxArrayObject names)
{
    ApiContext context(c);
    try
    {
        return (RexxSupplierObject)context.ret(new_supplier((ArrayClass *)values, (ArrayClass *)names));
    }
    catch (NativeActivation *)
    {
    }
    return NULLOBJECT;
}

RexxStemObject RexxEntry NewStem(RexxThreadContext *c, CSTRING n)
{
    ApiContext context(c);
    try
    {
        if (n == NULL)
        {
            return (RexxStemObject)context.ret(new StemClass(OREF_NULL));
        }
        else
        {
            Protected<RexxString> name = new_string(n);
            return (RexxStemObject)context.ret(new StemClass(name));
        }

    }
    catch (NativeActivation *)
    {
    }
    return NULLOBJECT;
}

void RexxEntry SetStemElement(RexxThreadContext *c, RexxStemObject s, CSTRING n, RexxObjectPtr v)
{
    ApiContext context(c);
    try
    {
        ((StemClass *)s)->setElement(n, (RexxObject *)v);
    }
    catch (NativeActivation *)
    {
    }
}

RexxObjectPtr RexxEntry GetStemElement(RexxThreadContext *c, RexxStemObject s, CSTRING n)
{
    ApiContext context(c);
    try
    {
        return context.ret(((StemClass *)s)->getElement(n));
    }
    catch (NativeActivation *)
    {
    }
    return NULLOBJECT;
}

void RexxEntry DropStemElement(RexxThreadContext *c, RexxStemObject s, CSTRING n)
{
    ApiContext context(c);
    try
    {
        ((StemClass *)s)->dropElement(n);
    }
    catch (NativeActivation *)
    {
    }
}

void RexxEntry SetStemArrayElement(RexxThreadContext *c, RexxStemObject s, size_t i, RexxObjectPtr v)
{
    ApiContext context(c);
    try
    {
        ((StemClass *)s)->setElement((size_t )i, (RexxObject *)v);
    }
    catch (NativeActivation *)
    {
    }
}

RexxObjectPtr RexxEntry GetStemArrayElement(RexxThreadContext *c, RexxStemObject s, size_t i)
{
    ApiContext context(c);
    try
    {
        return context.ret(((StemClass *)s)->getElement((size_t)i));
    }
    catch (NativeActivation *)
    {
    }
    return NULLOBJECT;
}

void RexxEntry DropStemArrayElement(RexxThreadContext *c, RexxStemObject s, size_t i)
{
    ApiContext context(c);
    try
    {
        ((StemClass *)s)->dropElement((size_t)i);
    }
    catch (NativeActivation *)
    {
    }
}

RexxDirectoryObject RexxEntry GetAllStemElements(RexxThreadContext *c, RexxStemObject s)
{
    ApiContext context(c);
    try
    {
        return (RexxDirectoryObject)context.ret(((StemClass *)s)->toDirectory());
    }
    catch (NativeActivation *)
    {
    }
    return NULLOBJECT;
}

RexxObjectPtr RexxEntry GetStemValue(RexxThreadContext *c, RexxStemObject s)
{
    ApiContext context(c);
    try
    {
        return context.ret(((StemClass *)s)->getStemValue());
    }
    catch (NativeActivation *)
    {
    }
    return NULLOBJECT;
}

logical_t RexxEntry IsStem(RexxThreadContext *c, RexxObjectPtr o)
{
    ApiContext context(c);
    try
    {
        return isStem((RexxObject *)o);
    }
    catch (NativeActivation *)
    {
    }
    return false;
}

void RexxEntry RaiseException0(RexxThreadContext *c, size_t n)
{
    ApiContext context(c);
    try
    {
        reportException((RexxErrorCodes)n);
    }
    catch (NativeActivation *)
    {
    }
}

void RexxEntry RaiseException1(RexxThreadContext *c, size_t n, RexxObjectPtr o1)
{
    ApiContext context(c);
    try
    {
        reportException((RexxErrorCodes)n, (RexxObject *)o1);
    }
    catch (NativeActivation *)
    {
    }
}

void RexxEntry RaiseException2(RexxThreadContext *c, size_t n, RexxObjectPtr o1, RexxObjectPtr o2)
{
    ApiContext context(c);
    try
    {
        reportException((RexxErrorCodes)n, (RexxObject *)o1, (RexxObject *)o2);
    }
    catch (NativeActivation *)
    {
    }
}

void RexxEntry APIRaiseException(RexxThreadContext *c, size_t n, RexxArrayObject a)
{
    ApiContext context(c);
    try
    {
        reportException((RexxErrorCodes)n, (ArrayClass *)a);
    }
    catch (NativeActivation *)
    {
    }
}

void RexxEntry RaiseCondition(RexxThreadContext *c, CSTRING n, RexxStringObject desc, RexxObjectPtr add, RexxObjectPtr result)
{
    ApiContext context(c);
    try
    {
        Protected<RexxString> name = new_upper_string(n);
        context.context->enableConditionTrap();
        context.activity->raiseCondition(name, OREF_NULL, (RexxString *)desc, (RexxObject *)add, (RexxObject *)result);
    }
    catch (NativeActivation *)
    {
    }
}

logical_t RexxEntry CheckCondition(RexxThreadContext *c)
{
    ApiContext context(c);
    try
    {
        return context.context->getConditionInfo() != OREF_NULL;
    }
    catch (NativeActivation *)
    {
    }
    return false;
}

wholenumber_t RexxEntry DisplayCondition(RexxThreadContext *c)
{
    ApiContext context(c);
    try
    {
        return context.activity->displayCondition(context.context->getConditionInfo());
    }
    catch (NativeActivation *)
    {
    }
    return Error_Interpretation/1000;   // this is a default one if something goes wrong
}

RexxDirectoryObject RexxEntry GetConditionInfo(RexxThreadContext *c)
{
    ApiContext context(c);
    try
    {
        return (RexxDirectoryObject)context.ret(context.context->getConditionInfo());
    }
    catch (NativeActivation *)
    {
    }
    return OREF_NULL;
}


void RexxEntry DecodeConditionInfo(RexxThreadContext *c, RexxDirectoryObject d, RexxCondition *cd)
{
    ApiContext context(c);
    try
    {
        Interpreter::decodeConditionData((DirectoryClass *)d, cd);
    }
    catch (NativeActivation *)
    {
    }
}

void RexxEntry ClearCondition(RexxThreadContext *c)
{
    ApiContext context(c);
    try
    {
        context.context->clearException();
    }
    catch (NativeActivation *)
    {
    }
}

POINTER RexxEntry MutableBufferData(RexxThreadContext *c, RexxMutableBufferObject b)
{
    ApiContext context(c);
    try
    {
        return (POINTER)((MutableBuffer *)b)->getData();
    }
    catch (NativeActivation *)
    {
    }
    return NULL;
}

size_t RexxEntry MutableBufferLength(RexxThreadContext *c, RexxMutableBufferObject b)
{
    ApiContext context(c);
    try
    {
        return ((MutableBuffer *)b)->getLength();
    }
    catch (NativeActivation *)
    {
    }
    return 0;
}

size_t RexxEntry SetMutableBufferLength(RexxThreadContext *c, RexxMutableBufferObject b, size_t length)
{
    ApiContext context(c);
    try
    {
        return ((MutableBuffer *)b)->setDataLength(length);
    }
    catch (NativeActivation *)
    {
    }
    return 0;
}

size_t RexxEntry MutableBufferCapacity(RexxThreadContext *c, RexxMutableBufferObject b)
{
    ApiContext context(c);
    try
    {
        return ((MutableBuffer *)b)->getCapacity();
    }
    catch (NativeActivation *)
    {
    }
    return 0;
}

POINTER RexxEntry SetMutableBufferCapacity(RexxThreadContext *c, RexxMutableBufferObject b, size_t length)
{
    ApiContext context(c);
    try
    {
        return (POINTER)((MutableBuffer *)b)->setCapacity(length);
    }
    catch (NativeActivation *)
    {
    }
    return 0;
}

RexxMutableBufferObject RexxEntry NewMutableBuffer(RexxThreadContext *c, size_t l)
{
    ApiContext context(c);
    try
    {
        return (RexxMutableBufferObject)context.ret((RexxObject *)new MutableBuffer(l, l));
    }
    catch (NativeActivation *)
    {
    }
    return NULLOBJECT;
}

logical_t RexxEntry IsMutableBuffer(RexxThreadContext *c, RexxObjectPtr o)
{
    ApiContext context(c);
    try
    {
        return isOfClass(MutableBuffer, (RexxObject *)o);
    }
    catch (NativeActivation *)
    {
    }
    return false;
}

END_EXTERN_C()

RexxThreadInterface Activity::threadContextFunctions =
{
    THREAD_INTERFACE_VERSION,
    DetachThread,
    HaltThread,
    SetThreadTrace,
    RequestGlobalReference,
    ReleaseGlobalReference,
    ReleaseLocalReference,

    SendMessageArray,
    SendMessage0,
    SendMessage1,
    SendMessage2,

    GetLocalEnvironment,
    GetGlobalEnvironment,

    IsInstanceOf,
    IsOfType,
    HasMethod,
    LoadPackage,
    LoadPackageFromData,
    LoadLibraryPackage,
    RegisterLibrary,
    FindClass,
    FindClassFromPackage,
    GetPackageRoutines,
    GetPackagePublicRoutines,
    GetPackageClasses,
    GetPackagePublicClasses,
    GetPackageMethods,
    CallRoutine,
    CallProgram,

    NewMethod,
    NewRoutine,
    IsRoutine,
    IsMethod,
    GetRoutinePackage,
    GetMethodPackage,

    ObjectToCSelf,
    WholeNumberToObject,
    UintptrToObject,
    IntptrToObject,
    ValueToObject,
    ValuesToObject,
    ObjectToValue,
    StringSizeToObject,
    ObjectToWholeNumber,
    ObjectToStringSize,
    Int64ToObject,
    UnsignedInt64ToObject,
    ObjectToInt64,
    ObjectToUnsignedInt64,
    Int32ToObject,
    UnsignedInt32ToObject,
    ObjectToInt32,
    ObjectToUnsignedInt32,
    ObjectToUintptr,
    ObjectToIntptr,
    ObjectToLogical,
    LogicalToObject,
    DoubleToObject,
    DoubleToObjectWithPrecision,
    ObjectToDouble,

    ObjectToString,
    ObjectToStringValue,
    StringGet,
    StringLength,
    StringData,
    NewString,
    NewStringFromAsciiz,
    StringUpper,
    StringLower,
    IsString,

    NewBufferString,
    BufferStringLength,
    BufferStringData,
    FinishBufferString,

    DirectoryPut,
    DirectoryAt,
    DirectoryRemove,
    NewDirectory,
    IsDirectory,

    ArrayAt,
    ArrayPut,
    ArrayAppend,
    ArrayAppendString,
    ArraySize,
    ArrayItems,
    ArrayDimension,
    NewArray,
    ArrayOfOne,
    ArrayOfTwo,
    ArrayOfThree,
    ArrayOfFour,
    IsArray,

    BufferData,
    BufferLength,
    NewBuffer,
    IsBuffer,

    PointerValue,
    NewPointer,
    IsPointer,

    SupplierItem,
    SupplierIndex,
    SupplierAvailable,
    SupplierNext,
    NewSupplier,

    NewStem,
    SetStemElement,
    GetStemElement,
    DropStemElement,
    SetStemArrayElement,
    GetStemArrayElement,
    DropStemArrayElement,
    GetAllStemElements,
    GetStemValue,
    IsStem,

    RaiseException0,
    RaiseException1,
    RaiseException2,
    APIRaiseException,
    RaiseCondition,
    CheckCondition,
    GetConditionInfo,
    DecodeConditionInfo,
    ClearCondition,

    OREF_NULL,
    OREF_NULL,
    OREF_NULL,
    OREF_NULL,
    ObjectToCSelfScoped,
    DisplayCondition,

    MutableBufferData,
    MutableBufferLength,
    SetMutableBufferLength,
    NewMutableBuffer,
    IsMutableBuffer,
    MutableBufferCapacity,
    SetMutableBufferCapacity,

    VariableReferenceName,
    VariableReferenceValue,
    SetVariableReferenceValue,
    IsVariableReference,

    StringTablePut,
    StringTableAt,
    StringTableRemove,
    NewStringTable,
    IsStringTable,
    SendMessageScoped,
};

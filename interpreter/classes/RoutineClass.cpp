/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2014 Rexx Language Association. All rights reserved.    */
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
/* REXX Kernel                                             RoutineClass.cpp   */
/*                                                                            */
/* Primitive Routine Class                                                    */
/*                                                                            */
/******************************************************************************/
#include "RexxCore.h"
#include "StringClass.hpp"
#include "ArrayClass.hpp"
#include "RexxCode.hpp"
#include "RexxNativeCode.hpp"
#include "RexxActivity.hpp"
#include "RexxActivation.hpp"
#include "RexxNativeActivation.hpp"
#include "RoutineClass.hpp"
#include "PackageClass.hpp"
#include "SourceFile.hpp"
#include "DirectoryClass.hpp"
#include "ProtectedObject.hpp"
#include "BufferClass.hpp"
#include "RexxInternalApis.h"
#include "RexxSmartBuffer.hpp"
#include "ProgramMetaData.hpp"
#include "Utilities.hpp"
#include "SystemInterpreter.hpp"
#include "PackageManager.hpp"
#include "InterpreterInstance.hpp"
#include "LanguageParser.hpp"

// singleton class instance
RexxClass *RoutineClass::classInstance = OREF_NULL;


/**
 * Create initial class object at bootstrap time.
 */
void RoutineClass::createInstance()
{
    CLASS_CREATE(Routine, "Routine", RexxClass);
}


/**
 * Initialize a Routine object from a generated code object. Generally
 * used for routines generated from ::ROUTINE directives.
 *
 * @param name    The routine name.
 * @param codeObj The associated code object.
 */
RoutineClass::RoutineClass(RexxString *name, BaseCode *codeObj)
{
    code = codeObj;
    executableName = name;
}


/**
 * Perform garbage collection on a live object.
 *
 * @param liveMark The current live mark.
 */
void RoutineClass::live(size_t liveMark)
{
    memory_mark(code);
    memory_mark(executableName);
    memory_mark(objectVariables);
}


/**
 * Perform generalized live marking on an object.  This is
 * used when mark-and-sweep processing is needed for purposes
 * other than garbage collection.
 *
 * @param reason The reason for the marking call.
 */
void RoutineClass::liveGeneral(int reason)
{
    memory_mark_general(code);
    memory_mark_general(executableName);
    memory_mark_general(objectVariables);
}


/**
 * Flatten a source object.
 *
 * @param envelope The envelope that will hold the flattened object.
 */
void RoutineClass::flatten(RexxEnvelope *envelope)
/******************************************************************************/
/* Function:  Flatten an object                                               */
/******************************************************************************/
{
    setUpFlatten(RoutineClass)

     flattenRef(code);
     flattenRef(executableName);
     flattenRef(objectVariables);

    cleanUpFlatten
}


/**
 * Call a Routine as a top-level program or an external method call.
 *
 * @param activity The current activity.
 * @param routineName The name of the program or routine.
 * @param argPtr   The pointer to the call arguments.
 * @param argcount The argument count.
 * @param result   The result object used to pass the value back.
 */
void RoutineClass::call(RexxActivity *activity, RexxString *routineName, RexxObject**argPtr,
    size_t argcount, ProtectedObject &result)
{
    // just forward this to the code object
    code->call(activity, this, routineName, argPtr, argcount, result);
}


/**
 * Call a routine as a top-level program or external call
 * context.
 *
 * @param activity The activity we're running on.
 * @param routineName
 *                 The name of the routine.
 * @param argPtr   The pointer to the call arguments.
 * @param argcount The count of arguments.
 * @param calltype The string CALLTYPE (ROUTINE, FUNCTION, etc.)
 * @param environment
 *                 The initial address name.
 * @param context  The context of the call (internal call, interpret, etc.)
 * @param result
 */
void RoutineClass::call(RexxActivity *activity, RexxString *routineName,
    RexxObject**argPtr, size_t argcount, RexxString *calltype,
    RexxString *environment, int context, ProtectedObject &result)
{
    // just forward this to the code object
    code->call(activity, this, routineName, argPtr, argcount, calltype, environment, context, result);
}


/**
 * Call a routine object from Rexx-level code.
 *
 * @param args   The call arguments.
 * @param count  The count of arguments.
 *
 * @return The call result (if any).
 */
RexxObject *RoutineClass::callRexx(RexxObject **args, size_t count)
{
    ProtectedObject result;

    code->call(ActivityManager::currentActivity, this, executableName, args, count, result);
    return (RexxObject *)result;
}


/**
 * Call a routine object from Rexx-level code.
 *
 * @param args   The call arguments.
 *
 * @return The call result (if any).
 */
RexxObject *RoutineClass::callWithRexx(RexxArray *args)
{
    // this is required and must be an array
    args = arrayArgument(args, ARG_ONE);

    ProtectedObject result;
    code->call(ActivityManager::currentActivity, this, executableName, args->data(), args->size(), result);
    return (RexxObject *)result;
}



/**
 * Run a Routine object as a top-level program.
 *
 * @param activity  The current activity.
 * @param calltype  The string call type.
 * @param environment
 *                  The initial environment name.
 * @param arguments Poiner to the arguments.
 * @param argCount  The count of arguments.
 * @param result    A protected object used to pass back the result.
 */
void RoutineClass::runProgram(RexxActivity *activity, RexxString * calltype,
    RexxString * environment, RexxObject **arguments, size_t       argCount,
    ProtectedObject &result)
{
    code->call(activity, this, executableName, arguments, argCount, calltype, environment, PROGRAMCALL, result);
}


/**
 * Run a program via simpler means, allowing most details
 * to default.
 *
 * @param activity  The current activity.
 * @param arguments The program arguments.
 * @param argCount  The count of arguments.
 * @param result    A ProtectedObject used to return (and protect) the result.
 */
void RoutineClass::runProgram(RexxActivity *activity, RexxObject **arguments,
    size_t argCount, ProtectedObject &result)
{
    code->call(activity, this, executableName, arguments, argCount, OREF_COMMAND, activity->getInstance()->getDefaultEnvironment(), PROGRAMCALL, result);
}


RexxObject *RoutineClass::setSecurityManager(
    RexxObject *manager)               /* supplied security manager         */
/******************************************************************************/
/* Function:  Associate a security manager with a method's source             */
/******************************************************************************/
{
    return code->setSecurityManager(manager);
}


/**
 * Translate the translated program into a buffer.
 *
 * @return The flattened object.
 */
RexxBuffer *RoutineClass::save()
{
    // detach the source from this routine object before saving
    detachSource();

    RexxEnvelope *envelope = new RexxEnvelope;
    ProtectedObject p(envelope);
    // now pack into an envelope;
    return envelope->pack(this);
}


/**
 * Save a routine into an externalized buffer form in an RXSTRING.
 *
 * @param outBuffer The target output RXSTRING.
 */
void RoutineClass::save(PRXSTRING outBuffer)
{
    ProtectedObject p(this);
    RexxBuffer *methodBuffer = save();  /* flatten the routine               */
    // create a full buffer of the data, plus the information header.
    ProgramMetaData *data = new (methodBuffer) ProgramMetaData(methodBuffer);
    // we just hand this buffer of data right over...that's all, we're done.
    outBuffer->strptr = (char *)data;
    outBuffer->strlength = data->getDataSize();
}


/**
 * Save a routine to a target file.
 *
 * @param filename The name of the file (fully resolved already).
 */
void RoutineClass::save(const char *filename)
{
    FILE *handle = fopen(filename, "wb");/* open the output file              */
    if (handle == NULL)                  /* get an open error?                */
    {
        /* got an error here                 */
        reportException(Error_Program_unreadable_output_error, filename);
    }
    ProtectedObject p(this);

    // save to a flattened buffer
    RexxBuffer *buffer = save();
    ProtectedObject p2(buffer);

    // create an image header
    ProgramMetaData metaData(buffer->getDataLength());
    {
        UnsafeBlock releaser;

        // write out the header information
        metaData.write(handle, buffer);
        fclose(handle);
    }
}


/**
 * Allocate memory for a new Routine instance.
 *
 * @param size   The size of the object.
 *
 * @return An allocated object configured for a Routine object.
 */
void *RoutineClass::operator new (size_t size)
{
    return new_object(size, T_Routine);
}


/**
 * Create a new method from REXX code contained in a string or an
 * array
 *
 * @param init_args The array of arguments to the new method.
 * @param argCount  The argument count.
 *
 * @return A new method object.
 */
RoutineClass *RoutineClass::newRexx(RexxObject **init_args, size_t argCount)
{
    // this class is defined on the object class, but this is actually attached
    // to a class object instance.  Therefore, any use of the this pointer
    // will be touching the wrong data.  Use the classThis pointer for calling
    // any methods on this object from this method.
    RexxClass *classThis = (RexxClass *)this;

    RexxString *programName;
    Protected<RexxArray> sourceArray;
    PackageClass *sourceContext;

    // parse all of the options
    processNewExecutableArgs(init_args, argCount, programName, sourceArray, sourceContext);

    // go create a method from whatever we were given for source.
    Protected<RoutineClass> newRoutine = LanguageParser::createRoutine(programName, sourceArray, sourceContext);

    // finish up the object creation.  Set the correct instance behavior (this could
    // be a subclass), check for uninit methods, and finally, send an init message using any
    // left over arguments.
    classThis->completeNewObject(newRoutine, init_args, argCount);
    return newRoutine;
}


/**
 * Create a method object from a file.
 *
 * @param filename The target file name.
 *
 * @return The created method object.
 */
RoutineClass *RoutineClass::newFileRexx(RexxString *filename)
{
    // this class is defined on the object class, but this is actually attached
    // to a class object instance.  Therefore, any use of the this pointer
    // will be touching the wrong data.  Use the classThis pointer for calling
    // any methods on this object from this method.
    RexxClass *classThis = (RexxClass *)this;

    // get the method name as a string
    filename = stringArgument(filename, ARG_ONE);

    RoutineClass *newRoutine = LanguageParser::createRoutine(filename);
    ProtectedObject p(newRoutine);

    // complete the initialization
    classThis->completeNewObject(newRoutine);
    return newRoutine;
}

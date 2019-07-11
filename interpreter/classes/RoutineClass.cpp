/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation.wri All rights reserved.             */
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
/* REXX Kernel                                             RoutineClass.cpp   */
/*                                                                            */
/* Primitive Routine Class                                                    */
/*                                                                            */
/******************************************************************************/
#include "RexxCore.h"
#include "StringClass.hpp"
#include "ArrayClass.hpp"
#include "RexxCode.hpp"
#include "NativeCode.hpp"
#include "Activity.hpp"
#include "RexxActivation.hpp"
#include "NativeActivation.hpp"
#include "RoutineClass.hpp"
#include "PackageClass.hpp"
#include "DirectoryClass.hpp"
#include "ProtectedObject.hpp"
#include "BufferClass.hpp"
#include "SmartBuffer.hpp"
#include "ProgramMetaData.hpp"
#include "Utilities.hpp"
#include "PackageManager.hpp"
#include "InterpreterInstance.hpp"
#include "LanguageParser.hpp"
#include "MethodArguments.hpp"
#include <stdio.h>
#include "ProgramSource.hpp"
#include "SysFile.hpp"


// singleton class instance
RexxClass *RoutineClass::classInstance = OREF_NULL;


/**
 * Create initial class object at bootstrap time.
 */
void RoutineClass::createInstance()
{
    CLASS_CREATE(Routine);
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
    memory_mark(annotations);
}


/**
 * Perform generalized live marking on an object.  This is
 * used when mark-and-sweep processing is needed for purposes
 * other than garbage collection.
 *
 * @param reason The reason for the marking call.
 */
void RoutineClass::liveGeneral(MarkReason reason)
{
    memory_mark_general(code);
    memory_mark_general(executableName);
    memory_mark_general(objectVariables);
    memory_mark_general(annotations);
}


/**
 * Flatten a source object.
 *
 * @param envelope The envelope that will hold the flattened object.
 */
void RoutineClass::flatten(Envelope *envelope)
{
    setUpFlatten(RoutineClass)

     flattenRef(code);
     flattenRef(executableName);
     flattenRef(objectVariables);
     flattenRef(annotations);

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
void RoutineClass::call(Activity *activity, RexxString *routineName, RexxObject**argPtr,
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
void RoutineClass::call(Activity *activity, RexxString *routineName,
    RexxObject**argPtr, size_t argcount, RexxString *calltype,
    RexxString *environment, ActivationContext context, ProtectedObject &result)
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
    return result;
}


/**
 * Call a routine object from Rexx-level code.
 *
 * @param args   The call arguments.
 *
 * @return The call result (if any).
 */
RexxObject *RoutineClass::callWithRexx(ArrayClass *args)
{
    // this is required and must be an array
    Protected<ArrayClass> argList = arrayArgument(args, ARG_ONE);

    ProtectedObject result;
    code->call(ActivityManager::currentActivity, this, executableName, argList->messageArgs(), argList->messageArgCount(), result);
    return result;
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
void RoutineClass::runProgram(Activity *activity, RexxString * calltype,
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
void RoutineClass::runProgram(Activity *activity, RexxObject **arguments,
    size_t argCount, ProtectedObject &result)
{
    code->call(activity, this, executableName, arguments, argCount, GlobalNames::COMMAND, activity->getInstance()->getDefaultEnvironment(), PROGRAMCALL, result);
}


/**
 * Associate a security manager with a routine's source package.
 *
 * @param manager The security manager object.
 *
 * @return The return value from the code object.
 */
RexxObject *RoutineClass::setSecurityManager(RexxObject *manager)
{
    return code->setSecurityManager(manager);
}


/**
 * Translate the translated program into a buffer.
 *
 * @return The flattened object.
 */
BufferClass *RoutineClass::save()
{
    // detach the source from this routine object before saving
    Protected<ProgramSource> source = detachSource();

    Protected<Envelope> envelope = new Envelope;
    // now pack into an envelope;
    Protected<BufferClass> result = envelope->pack(this);
    // reattach the source because we might still execute this
    attachSource(source);
    return result;
}


/**
 * Save a routine into an externalized buffer form in an RXSTRING.
 *
 * @param outBuffer The target output RXSTRING.
 */
void RoutineClass::save(PRXSTRING outBuffer)
{
    ProtectedObject p(this);
    // flatten the routine
    Protected<BufferClass> methodBuffer = save();
    // create a full buffer of the data, plus the information header.
    ProgramMetaData *data = new (methodBuffer) ProgramMetaData(getLanguageLevel(), methodBuffer);
    // we just hand this buffer of data right over...that's all, we're done...
    outBuffer->strptr = (char *)data;
    outBuffer->strlength = data->getDataSize();
}

/**
 * Retrieve the language level necessary to interpret this routine.
 *
 * @return The required language leve.
 */
LanguageLevel RoutineClass::getLanguageLevel()
{
    return getPackageObject()->getLanguageLevel();
}


/**
 * Save a routine to a target file.
 *
 * @param fileName
 * @param encode   Indicates if the file should be saved in binary form or base64 encoded string form.
 */
void RoutineClass::save(const char *fileName, bool encode)
{
    // make sure this is protected from GC during all of this processing
    ProtectedObject p(this);

    SysFile target;

    if (!target.open(fileName, RX_O_CREAT | RX_O_TRUNC | RX_O_WRONLY, RX_S_IREAD | RX_S_IWRITE, RX_SH_DENYRW))
    {
        // got an error here
        reportException(Error_Program_unreadable_output_error, fileName);
    }

    // save to a flattened buffer
    Protected<BufferClass> buffer = save();

    // create an image header
    ProgramMetaData metaData(getLanguageLevel(), buffer->getDataLength());

    // write out the header information
    metaData.write(target, buffer, encode);
    target.close();
}


/**
 * Restore a saved routine directly from character data.
 *
 * @param data   The data pointer.
 * @param length the data length.
 *
 * @return The unflattened routine object.
 */
RoutineClass *RoutineClass::restore(const char *data, size_t length)
{
    // create a buffer object and restore from it
    BufferClass *buffer = new_buffer(data, length);
    ProtectedObject p(buffer);
    return restore(buffer, buffer->getData(), length);
}


/**
 * Unflatten a saved Routine object.
 *
 * @param buffer The buffer containing the saved data.
 * @param startPointer
 *               The pointer to the start of the flattened data.
 * @param length The length of the flattened data.
 *
 * @return A restored Routine object.
 */
RoutineClass *RoutineClass::restore(BufferClass *buffer, char *startPointer, size_t length)
{
    // get an envelope and puff up the object
    Protected<Envelope> envelope  = new Envelope;
    envelope->puff(buffer, startPointer, length);
    // the envelope receiver object is our return value.
    return (RoutineClass *)envelope->getReceiver();
}


/**
 * Restore a program from a simple buffer.
 *
 * @param fileName The file name of the program we're restoring from.
 * @param buffer   The source buffer.  This contains all of the saved metadata
 *                 ahead of the the flattened object.
 *
 * @return The inflated Routine object, if valid.
 */
RoutineClass* RoutineClass::restore(RexxString *fileName, BufferClass *buffer)
{
    // ProgramMetaData handles all of the details here
    return ProgramMetaData::restore(fileName, buffer);
}


/**
 * Restore a routine object from a previously saved instore buffer.
 *
 * @param inData The input data (in RXSTRING form).
 *
 * @return The unflattened object.
 */
RoutineClass* RoutineClass::restore(RXSTRING *inData, RexxString *name)
{
    // because we're restoring from instore data that might need decoding,
    // we need to get this information in a buffer that we can modify.
    Protected<BufferClass> buffer = new_buffer(inData->strptr, inData->strlength);

    // ProgramMetaData handles all of the details here
    return ProgramMetaData::restore(name, buffer);
}


/**
 * Create a new routine from REXX code contained in a string or an
 * array
 *
 * @param init_args The array of arguments to the new routine.
 * @param argCount  The argument count.
 *
 * @return A new routine object.
 */
RoutineClass *RoutineClass::newRexx(RexxObject **init_args, size_t argCount)
{
    // this class is defined on the object class, but this is actually attached
    // to a class object instance.  Therefore, any use of the this pointer
    // will be touching the wrong data.  Use the classThis pointer for calling
    // any methods on this object from this method.
    RexxClass *classThis = (RexxClass *)this;

    RexxString *programName;
    Protected<ArrayClass> sourceArray;
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
 * Create a routine object from a file.
 *
 * @param filename The target file name.
 * @param scope    The scope that the new routine object will be given.
 *
 * @return The created routine object.
 */
RoutineClass *RoutineClass::newFileRexx(RexxString *filename, PackageClass *sourceContext)
{
    // this class is defined on the object class, but this is actually attached
    // to a class object instance.  Therefore, any use of the this pointer
    // will be touching the wrong data.  Use the classThis pointer for calling
    // any methods on this object from this method.
    RexxClass *classThis = (RexxClass *)this;

    // parse all of the options
    processNewFileExecutableArgs(filename, sourceContext);

    // go create a routine from filename
    Protected<RoutineClass> newRoutine = LanguageParser::createRoutine(filename, sourceContext);

    // complete the initialization
    classThis->completeNewObject(newRoutine);
    return newRoutine;
}


/**
 * Create a routine from an external library source.
 *
 * @param name   The routine name.
 *
 * @return The resolved routine object, or OREF_NULL if unable to load
 *         the routine.
 */
RoutineClass *RoutineClass::loadExternalRoutine(RexxString *name, RexxString *descriptor)
{
    Protected<RexxString> routineName = stringArgument(name, "name");
    Protected<RexxString> libraryDescriptor = stringArgument(descriptor, "descriptor");

    // convert external into words
    Protected<ArrayClass> words = StringUtil::words(libraryDescriptor->getStringData(), libraryDescriptor->getLength());
    // "LIBRARY libbar [foo]"
    if (words->size() > 0 && ((RexxString *)(words->get(1)))->strCompare("LIBRARY"))
    {
        RexxString *library = OREF_NULL;
        // the default entry point name is the internal name
        RexxString *entry = routineName;

        // full library with entry name version?
        if (words->size() == 3)
        {
            library = (RexxString *)words->get(2);
            entry = (RexxString *)words->get(3);
        }
        else if (words->size() == 2)
        {
            library = (RexxString *)words->get(2);
        }
        else  // wrong number of tokens
        {
            reportException(Error_Translation_bad_external, descriptor);
        }

        // create a new native method
        RoutineClass *routine = PackageManager::loadRoutine(library, entry);
        return (RoutineClass *)resultOrNil(routine);
    }
    else
    {
        // unknown external type
        reportException(Error_Translation_bad_external, descriptor);
    }
    return OREF_NULL;
}

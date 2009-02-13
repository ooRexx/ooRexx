/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2006 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* http://www.oorexx.org/license.html                          */
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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "RexxCore.h"
#include "StringClass.hpp"
#include "ArrayClass.hpp"
#include "RexxCode.hpp"
#include "RexxNativeCode.hpp"
#include "RexxActivity.hpp"
#include "RexxActivation.hpp"
#include "RexxNativeActivation.hpp"
#include "MethodClass.hpp"
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
#include <ctype.h>


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
    OrefSet(this, this->code, codeObj);  /* store the code                    */
    OrefSet(this, this->executableName, name);
}


/**
 * Initialize a RoutineClass object from a file source.
 *
 * @param name   The routine name (and the resolved name of the file).
 */
RoutineClass::RoutineClass(RexxString *name)
{
    // we need to protect this object until the constructor completes.
    // the code generation step will create lots of new objects, giving a
    // pretty high probability that it will be collected.
    ProtectedObject p(this);
    OrefSet(this, this->executableName, name);
    // get a source object to generat this from
    RexxSource *_source = new RexxSource(name);
    ProtectedObject p2(_source);
    // generate our code object and make the file hook up.
    RexxCode *codeObj = _source->generateCode(false);
    OrefSet(this, this->code, codeObj);
}


/**
 * Initialize a Routine object using a buffered source.
 *
 * @param name   The name of the routine.
 * @param source the source buffer.
 */
RoutineClass::RoutineClass(RexxString *name, RexxBuffer *s)
{
    // we need to protect this object until the constructor completes.
    // the code generation step will create lots of new objects, giving a
    // pretty high probability that it will be collected.
    ProtectedObject p(this);
    OrefSet(this, this->executableName, name);
    // get a source object to generat this from
    RexxSource *_source = new RexxSource(name, s);
    ProtectedObject p2(_source);
    // generate our code object and make the file hook up.
    RexxCode *codeObj = _source->generateCode(false);
    OrefSet(this, this->code, codeObj);
}


/**
 * Initialize a Routine object using directly provided source.
 *
 * @param name   The name of the routine.
 * @param data   The source data buffer pointer.
 * @param length the length of the source buffer.
 */
RoutineClass::RoutineClass(RexxString *name, const char *data, size_t length)
{
    // we need to protect this object until the constructor completes.
    // the code generation step will create lots of new objects, giving a
    // pretty high probability that it will be collected.
    ProtectedObject p(this);
    OrefSet(this, this->executableName, name);
    // get a source object to generat this from
    RexxSource *_source = new RexxSource(name, data, length);
    ProtectedObject p2(_source);
    // generate our code object and make the file hook up.
    RexxCode *codeObj = _source->generateCode(false);
    OrefSet(this, this->code, codeObj);
}


/**
 * Initialize a Routine object using an array source.
 *
 * @param name   The name of the routine.
 * @param source the source buffer.
 */
RoutineClass::RoutineClass(RexxString *name, RexxArray *s)
{
    // we need to protect this object until the constructor completes.
    // the code generation step will create lots of new objects, giving a
    // pretty high probability that it will be collected.
    ProtectedObject p(this);
    OrefSet(this, this->executableName, name);
    // get a source object to generat this from
    RexxSource *_source = new RexxSource(name, s);
    ProtectedObject p2(_source);
    // generate our code object and make the file hook up.
    RexxCode *codeObj = _source->generateCode(false);
    OrefSet(this, this->code, codeObj);
}


void RoutineClass::live(size_t liveMark)
/******************************************************************************/
/* Function:  Normal garbage collection live marking                          */
/******************************************************************************/
{
    memory_mark(this->code);
    memory_mark(this->executableName);
    memory_mark(this->objectVariables);
}

void RoutineClass::liveGeneral(int reason)
/******************************************************************************/
/* Function:  Generalized object marking                                      */
/******************************************************************************/
{
    memory_mark_general(this->code);
    memory_mark_general(this->executableName);
    memory_mark_general(this->objectVariables);
}

void RoutineClass::flatten(RexxEnvelope *envelope)
/******************************************************************************/
/* Function:  Flatten an object                                               */
/******************************************************************************/
{
  setUpFlatten(RoutineClass)

   flatten_reference(newThis->code, envelope);
   flatten_reference(newThis->executableName, envelope);
   flatten_reference(newThis->objectVariables, envelope);

  cleanUpFlatten
}


void RoutineClass::call(
    RexxActivity *activity,            /* activity running under            */
    RexxString *msgname,               /* message to be run                 */
    RexxObject**argPtr,                /* arguments to the method           */
    size_t      argcount,              /* the count of arguments            */
    ProtectedObject &result)           // the method result
/******************************************************************************/
/* Function:  Call a method as a top level program or external function call  */
/******************************************************************************/
{
    ProtectedObject p(this);           // belt-and-braces to make sure this is protected
    // just forward this to the code object
    code->call(activity, this, msgname, argPtr, argcount, result);
}


void RoutineClass::call(
    RexxActivity *activity,            /* activity running under            */
    RexxString *msgname,               /* message to be run                 */
    RexxObject**argPtr,                /* arguments to the method           */
    size_t      argcount,              /* the count of arguments            */
    RexxString *calltype,              /* COMMAND/ROUTINE/FUNCTION          */
    RexxString *environment,           /* initial command environment       */
    int   context,                     /* type of context                   */
    ProtectedObject &result)           // the method result
/******************************************************************************/
/* Function:  Call a method as a top level program or external function call  */
/******************************************************************************/
{
    ProtectedObject p(this);           // belt-and-braces to make sure this is protected
    // just forward this to the code object
    code->call(activity, this, msgname, argPtr, argcount, calltype, environment, context, result);
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



void RoutineClass::runProgram(
    RexxActivity *activity,
    RexxString * calltype,             /* type of invocation                */
    RexxString * environment,          /* initial address                   */
    RexxObject **arguments,            /* array of arguments                */
    size_t       argCount,             /* the number of arguments           */
    ProtectedObject &result)           // the method result
/****************************************************************************/
/* Function:  Run a method as a program                                     */
/****************************************************************************/
{
    ProtectedObject p(this);           // belt-and-braces to make sure this is protected
    code->call(activity, this, OREF_NONE, arguments, argCount, calltype, environment, PROGRAMCALL, result);
}


void RoutineClass::runProgram(
    RexxActivity *activity,
    RexxObject **arguments,            /* array of arguments                */
    size_t       argCount,             /* the number of arguments           */
    ProtectedObject &result)           // the method result
/****************************************************************************/
/* Function:  Run a method as a program                                     */
/****************************************************************************/
{
    ProtectedObject p(this);           // belt-and-braces to make sure this is protected
    code->call(activity, this, OREF_NONE, arguments, argCount, OREF_COMMAND, activity->getInstance()->getDefaultEnvironment(), PROGRAMCALL, result);
}


RexxObject *RoutineClass::setSecurityManager(
    RexxObject *manager)               /* supplied security manager         */
/******************************************************************************/
/* Function:  Associate a security manager with a method's source             */
/******************************************************************************/
{
    return code->setSecurityManager(manager);
}


RexxBuffer *RoutineClass::save()
/******************************************************************************/
/* Function: Flatten translated method into a buffer for storage into EA's etc*/
/******************************************************************************/
{
                                         /* Get new envelope object           */
    RexxEnvelope *envelope = new RexxEnvelope;
    ProtectedObject p(envelope);
                                         /* now pack up the envelope for      */
                                         /* saving.                           */
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


void *RoutineClass::operator new (size_t size)
/******************************************************************************/
/* Function:  create a new method instance                                    */
/******************************************************************************/
{
                                         /* get a new method object           */
    return new_object(size, T_Routine);
}


/**
 * Construct a Routine using different forms of in-memory
 * source file.
 *
 * @param pgmname  The name of the program.
 * @param source   The program source.  This can be a string or an array of strings.
 * @param position The argument position used for error reporting.
 * @param parentSource
 *                 A parent source context used to provide additional class and
 *                 routine definitions.
 *
 * @return A constructed Routine object.
 */
RoutineClass *RoutineClass::newRoutineObject(RexxString *pgmname, RexxObject *source, RexxObject *position, RexxSource *parentSource)
{
    // request this as an array.  If not convertable, then we'll use it as a string
    RexxArray *newSourceArray = source->requestArray();
    /* couldn't convert?                 */
    if (newSourceArray == (RexxArray *)TheNilObject)
    {
        /* get the string representation     */
        RexxString *sourceString = source->makeString();
        /* got back .nil?                    */
        if (sourceString == (RexxString *)TheNilObject)
        {
            /* raise an error                    */
            reportException(Error_Incorrect_method_no_method, position);
        }
        /* wrap an array around the value    */
        newSourceArray = new_array(sourceString);
    }
    else                                 /* have an array, make sure all      */
    {
        /* is it single dimensional?         */
        if (newSourceArray->getDimension() != 1)
        {
            /* raise an error                    */
            reportException(Error_Incorrect_method_noarray, position);
        }
        /*  element are strings.             */
        /* Make a source array safe.         */
        ProtectedObject p(newSourceArray);
        /* Make sure all elements in array   */
        for (size_t counter = 1; counter <= newSourceArray->size(); counter++)
        {
            /* Get element as string object      */
            RexxString *sourceString = newSourceArray ->get(counter)->makeString();
            /* Did it convert?                   */
            if (sourceString == (RexxString *)TheNilObject)
            {
                /* and report the error.             */
                reportException(Error_Incorrect_method_nostring_inarray, IntegerTwo);
            }
            else
            {
                /* itsa string add to source array   */
                newSourceArray ->put(sourceString, counter);
            }
        }
    }

    // create the routine
    RoutineClass *result = new RoutineClass(pgmname, newSourceArray);
    ProtectedObject p(result);

    p = result;    // switch the protectiong

    // if we've been provided with a scope, use it
    if (parentSource == OREF_NULL)
    {
        // see if we have an active context and use the current source as the basis for the lookup
        RexxActivation *currentContext = ActivityManager::currentActivity->getCurrentRexxFrame();
        if (currentContext != OREF_NULL)
        {
            parentSource = currentContext->getSourceObject();
        }
    }

    // if there is a parent source, then merge in the scope information
    if (parentSource != OREF_NULL)
    {
        result->getSourceObject()->inheritSourceContext(parentSource);
    }

    return result;
}


/**
 * Construct a Routine using different forms of in-memory
 * source file.
 *
 * @param pgmname  The name of the program.
 * @param source   The program source.  This can be a string or an array of strings.
 * @param position The argument position used for error reporting.
 * @param parentSource
 *                 A parent source context used to provide additional class and
 *                 routine definitions.
 *
 * @return A constructed Routine object.
 */
RoutineClass *RoutineClass::newRoutineObject(RexxString *pgmname, RexxArray *source, RexxObject *position)
{
    // request this as an array.  If not convertable, then we'll use it as a string
    RexxArray *newSourceArray = source->requestArray();
    /* couldn't convert?                 */
    if (newSourceArray == (RexxArray *)TheNilObject)
    {
       /* raise an error                    */
       reportException(Error_Incorrect_method_no_method, position);
    }
    else                                 /* have an array, make sure all      */
    {
        /* is it single dimensional?         */
        if (newSourceArray->getDimension() != 1)
        {
            /* raise an error                    */
            reportException(Error_Incorrect_method_noarray, position);
        }
        /*  element are strings.             */
        /* Make a source array safe.         */
        ProtectedObject p(newSourceArray);
        /* Make sure all elements in array   */
        for (size_t counter = 1; counter <= newSourceArray->size(); counter++)
        {
            /* Get element as string object      */
            RexxString *sourceString = newSourceArray ->get(counter)->makeString();
            /* Did it convert?                   */
            if (sourceString == (RexxString *)TheNilObject)
            {
                /* and report the error.             */
                reportException(Error_Incorrect_method_nostring_inarray, IntegerTwo);
            }
            else
            {
                /* itsa string add to source array   */
                newSourceArray ->put(sourceString, counter);
            }
        }
    }
    // create the routine
    return new RoutineClass(pgmname, newSourceArray);
}


RoutineClass *RoutineClass::newRexx(
    RexxObject **init_args,            /* subclass init arguments           */
    size_t       argCount)             /* number of arguments passed        */
/******************************************************************************/
/* Function:  Create a new method from REXX code contained in a string or an  */
/*            array                                                           */
/******************************************************************************/
{
    RexxObject *pgmname;                 /* method name                       */
    RexxObject *_source;                 /* Array or string object            */
    RexxObject *option = OREF_NULL;
    size_t initCount = 0;                /* count of arguments we pass along  */

                                         /* break up the arguments            */

    RexxClass::processNewArgs(init_args, argCount, &init_args, &initCount, 2, (RexxObject **)&pgmname, (RexxObject **)&_source);
    /* get the method name as a string   */
    RexxString *nameString = stringArgument(pgmname, ARG_ONE);
    requiredArgument(_source, ARG_TWO);          /* make sure we have the second too  */

    RexxSource *sourceContext = OREF_NULL;
    // retrieve extra parameter if exists
    if (initCount != 0)
    {
        RexxClass::processNewArgs(init_args, initCount, &init_args, &initCount, 1, (RexxObject **)&option, NULL);
        if (isOfClass(Method, option))
        {
            sourceContext = ((RexxMethod *)option)->getSourceObject();
        }
        if (isOfClass(Routine, option))
        {
            sourceContext = ((RoutineClass *)option)->getSourceObject();
        }
        else if (isOfClass(Package, option))
        {
            sourceContext = ((PackageClass *)option)->getSourceObject();
        }
        else
        {
            reportException(Error_Incorrect_method_argType, IntegerThree, "Method, Routine, or Package object");
        }
    }

    RoutineClass *newRoutine = newRoutineObject(nameString, _source, IntegerTwo, sourceContext);
    ProtectedObject p(newRoutine);
    /* Give new object its behaviour     */
    newRoutine->setBehaviour(((RexxClass *)this)->getInstanceBehaviour());
    if (((RexxClass *)this)->hasUninitDefined())
    {
        newRoutine->hasUninit();         /* Make sure everyone is notified.   */
    }
    /* now send an INIT message          */
    newRoutine->sendMessage(OREF_INIT, init_args, initCount);
    return newRoutine;                   /* return the new method             */
}


RoutineClass *RoutineClass::newFileRexx(
    RexxString *filename)              /* name of the target file           */
/******************************************************************************/
/* Function:  Create a method from a fully resolved file name                 */
/******************************************************************************/
{
                                       /* get the method name as a string   */
  filename = stringArgument(filename, ARG_ONE);
                                       /* finish up processing of this      */
  RoutineClass * newMethod = new RoutineClass(filename);
  ProtectedObject p2(newMethod);
                                       /* Give new object its behaviour     */
  newMethod->setBehaviour(((RexxClass *)this)->getInstanceBehaviour());
  /* does object have an UNINT method  */
   if (((RexxClass *)this)->hasUninitDefined())
   {
     newMethod->hasUninit();           /* Make sure everyone is notified.   */
   }
                                       /* now send an INIT message          */
  newMethod->sendMessage(OREF_INIT);
  return newMethod;
}


/**
 * Create a routine from a macrospace source.
 *
 * @param name   The name of the macrospace item.
 *
 * @return The inflatted macrospace routine.
 */
RoutineClass *RoutineClass::restoreFromMacroSpace(RexxString *name)
{
    RXSTRING buffer;                     /* instorage buffer                  */

    MAKERXSTRING(buffer, NULL, 0);
    /* get the image of function         */
    RexxResolveMacroFunction(name->getStringData(), &buffer);
    /* unflatten the method now          */
    RoutineClass *routine = restore(&buffer, name);
    // release the buffer memory
    SystemInterpreter::releaseResultMemory(buffer.strptr);
    return routine;
}


RoutineClass *RoutineClass::processInstore(PRXSTRING instore, RexxString * name )
/******************************************************************************/
/* Function:  Process instorage execution arguments                           */
/******************************************************************************/
{
    // just a generic empty one indicating that we should check the macrospace?
    if (instore[0].strptr == NULL && instore[1].strptr == NULL)
    {
        unsigned short temp;

        /* see if this exists                */
        if (!RexxQueryMacro(name->getStringData(), &temp))
        {
            return restoreFromMacroSpace(name);
        }
        return OREF_NULL;         // not found
    }
    if (instore[1].strptr != NULL)       /* have an image                     */
    {
        /* go convert into a method          */
        RoutineClass *routine = restore(&instore[1], name);
        if (routine != OREF_NULL)
        {         /* did it unflatten successfully?    */
            if (instore[0].strptr != NULL)   /* have source also?                 */
            {
                /* get a buffer object               */
                RexxBuffer *source_buffer = new_buffer(instore[0]);
                /* reconnect this with the source    */
                routine->getSourceObject()->setBufferedSource(source_buffer);
            }
            return routine;                  /* go return it                      */
        }
    }
    if (instore[0].strptr != NULL)       /* have instorage source             */
    {
        /* get a buffer object               */
        RexxBuffer *source_buffer = new_buffer(instore[0]);
        if (source_buffer->getData()[0] == '#' && source_buffer->getData()[1] == '!')
        {
            memcpy(source_buffer->getData(), "--", 2);
        }

        /* translate this source             */
        RoutineClass *routine = new RoutineClass(name, source_buffer);
        ProtectedObject p(routine);
        /* return this back in instore[1]    */
        routine->save(&instore[1]);
        return routine;                    /* return translated source          */
    }
    return OREF_NULL;                    /* processing failed                 */
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
    RexxBuffer *buffer = new_buffer(data, length);
    ProtectedObject p(buffer);
    return restore(buffer, buffer->getData(), length);
}


RoutineClass *RoutineClass::restore(
    RexxBuffer *buffer,                /* buffer containing the method      */
    char *startPointer,                /* first character of the method     */
    size_t length)                     // length of data to unflatten
/******************************************************************************/
/* Function: Unflatten a translated method.  Passed a buffer object containing*/
/*           the method                                                       */
/******************************************************************************/
{
                                       /* Get new envelope object           */
  RexxEnvelope *envelope  = new RexxEnvelope;
  ProtectedObject p(envelope);
                                       /* now puff up the method object     */
  envelope->puff(buffer, startPointer, length);
                                       /* The receiver object is an envelope*/
                                       /* whose receiver is the actual      */
                                       /* method object we're restoring     */
  return (RoutineClass *)envelope->getReceiver();
}


/**
 * Restore a program from a simple buffer.
 *
 * @param buffer The source buffer.
 *
 * @return The inflated Routine object, if valid.
 */
RoutineClass *RoutineClass::restore(RexxString *fileName, RexxBuffer *buffer)
{
    const char *data = buffer->getData();

    // does this start with a hash-bang?  Need to scan forward to the first
    // newline character
    if (data[0] == '#' && data[1] == '!')
    {
        data = Utilities::strnchr(data, buffer->getDataLength(), '\n');
        if (data == OREF_NULL)
        {
            return OREF_NULL;
        }
        // step over the linend
        data++;
    }

    ProgramMetaData *metaData = (ProgramMetaData *)data;
    bool badVersion = false;
    // make sure this is valid for interpreter
    if (!metaData->validate(badVersion))
    {
        // if the failure was due to a version mismatch, this is an error condition.
        if (badVersion)
        {
            reportException(Error_Program_unreadable_version, fileName);
        }
        return OREF_NULL;
    }
    // this should be valid...try to restore.
    RoutineClass *routine = restore(buffer, metaData->getImageData(), metaData->getImageSize());
    routine->getSourceObject()->setProgramName(fileName);
    return routine;
}


/**
 * Restore a routine object from a previously saved instore buffer.
 *
 * @param inData The input data (in RXSTRING form).
 *
 * @return The unflattened object.
 */
RoutineClass *RoutineClass::restore(RXSTRING *inData, RexxString *name)
{
    const char *data = inData->strptr;

    // does this start with a hash-bang?  Need to scan forward to the first
    // newline character
    if (data[0] == '#' && data[1] == '!')
    {
        data = Utilities::strnchr(data, inData->strlength, '\n');
        if (data == OREF_NULL)
        {
            return OREF_NULL;
        }
        // step over the linend
        data++;
    }


    ProgramMetaData *metaData = (ProgramMetaData *)data;
    bool badVersion;
    // make sure this is valid for interpreter
    if (!metaData->validate(badVersion))
    {
        // if the failure was due to a version mismatch, this is an error condition.
        if (badVersion)
        {
            reportException(Error_Program_unreadable_version, name);
        }
        return OREF_NULL;
    }
    RexxBuffer *bufferData = metaData->extractBufferData();
    ProtectedObject p(bufferData);
    // we're restoring from the beginning of this.
    RoutineClass *routine = restore(bufferData, bufferData->getData(), metaData->getImageSize());
    // if this restored properly (and it should), reconnect it to the source file
    if (routine != OREF_NULL)
    {
        routine->getSourceObject()->setProgramName(name);
    }
    return routine;
}


/**
 * Retrieve a routine object from a file.  This will first attempt
 * to restore a previously translated image, then will try to
 * translate the source if that fails.
 *
 * @param filename The target file name.
 *
 * @return A resulting Routine object, if possible.
 */
RoutineClass *RoutineClass::fromFile(RexxString *filename)
{
                                         /* load the program file             */
    RexxBuffer *program_buffer = SystemInterpreter::readProgram(filename->getStringData());
    if (program_buffer == OREF_NULL)     /* Program not found or read error?  */
    {
        /* report this                       */
        reportException(Error_Program_unreadable_name, filename);
    }

    // try to restore a flattened program first
    RoutineClass *routine = restore(filename, program_buffer);
    if (routine != OREF_NULL)
    {
        return routine;
    }

    // process this from the source
    return new RoutineClass(filename, program_buffer);
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
    name = stringArgument(name, "name");
    descriptor = stringArgument(descriptor, "descriptor");
    /* convert external into words       */
    RexxArray *_words = StringUtil::words(descriptor->getStringData(), descriptor->getLength());
    ProtectedObject p(_words);
    // "LIBRARY libbar [foo]"
    if (((RexxString *)(_words->get(1)))->strCompare(CHAR_LIBRARY))
    {
        RexxString *library = OREF_NULL;
        // the default entry point name is the internal name
        RexxString *entry = name;

        // full library with entry name version?
        if (_words->size() == 3)
        {
            library = (RexxString *)_words->get(2);
            entry = (RexxString *)_words->get(3);
        }
        else if (_words->size() == 2)
        {
            library = (RexxString *)_words->get(2);
        }
        else  // wrong number of tokens
        {
            /* this is an error                  */
            reportException(Error_Translation_bad_external, descriptor);
        }

                                     /* create a new native method        */
        RoutineClass *routine = PackageManager::loadRoutine(library, entry);
        // raise an exception if this entry point is not found.
        if (routine == OREF_NULL)
        {
             return (RoutineClass *)TheNilObject;
        }
        return routine;
    }
    else
    {
        /* unknown external type             */
        reportException(Error_Translation_bad_external, descriptor);
    }
    return OREF_NULL;
}


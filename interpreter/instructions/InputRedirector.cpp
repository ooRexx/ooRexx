/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2005-2018 Rexx Language Association. All rights reserved.    */
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
/* InputRedirector implementations for the ADDRESS WITH instruction           */
/*                                                                            */
/******************************************************************************/

#include "RexxCore.h"
#include "Activity.hpp"
#include "ActivityManager.hpp"
#include "InputRedirector.hpp"
#include "ProtectedObject.hpp"
#include "PackageClass.hpp"
#include "NativeActivation.hpp"
#include "ConditionTrappingDispatcher.hpp"
#include "MutableBufferClass.hpp"
#include "SysFileSystem.hpp"
#include "DirectoryClass.hpp"


/**
 * A dispatcher for trapping and handling conditions and errors from
 * method calls used in redirection. This will ensure the errors get
 * propagated to the base native activation for the command call.
 */
class RedirectionDispatcher : public ConditionTrappingDispatcher
{
public:
    inline RedirectionDispatcher(TrapInvoker &t) : ConditionTrappingDispatcher(t)
    {

    }
    virtual ~RedirectionDispatcher() { ; }

    virtual void handleError(DirectoryClass *conditionObj)
    {
        // we're only interested in NOTREADY or SYNTAX conditions. We swallow the
        // NOTREADY's, and propagate the SYNTAX conditions
        RexxString *conditionName = (RexxString *)conditionObj->get(GlobalNames::CONDITION);
        if (conditionName->strCompare(GlobalNames::SYNTAX))
        {
            // set this in the base command callout activation so the error will get raised
            // when the command completes.
            ((NativeActivation *)(activation->getPreviousStackFrame()))->setConditionInfo(conditionObj);
        }

        // now take care of normal trapping processes.
        ConditionTrappingDispatcher::handleError(conditionObj);
    }
};


/**
 * Read all of the input data as a single buffer.
 *
 * @param data   The returned pointer to the data
 * @param length The returned data length.
 */
void InputRedirector::readBuffered(NativeActivation *context, const char *&data, size_t &length)
{
    // ok, called a second time, we can return this data again.
    if (inputBuffer != OREF_NULL)
    {
        data = inputBuffer->getStringData();
        length = inputBuffer->getLength();
        return;
    }

    // create a new mutable buffer instance to write into
    inputBuffer = new MutableBuffer(defaultBufferSize, defaultBufferSize);
    // now process all of the lines
    for (;;)
    {
        // read the next line and break if we have nothing
        RexxString *newLine = read(context);
        if (newLine == OREF_NULL)
        {
            break;
        }
        inputBuffer->append(newLine);
        inputBuffer->append(SysFileSystem::EOL_Marker);
    }
    data = inputBuffer->getStringData();
    length = inputBuffer->getLength();
}


/**
 * Allocate a new stem input source
 *
 * @param size   The size of the object.
 *
 * @return Storage for creating the object.
 */
void *StemInputSource::operator new(size_t size)
{
    return new_object(size, T_StemInputSource);
}


/**
 * Construct a output target ogject
 *
 * @param stem      The stem oubect being used
 */
StemInputSource::StemInputSource(StemClass *s)
{
    stem = s;
}


/**
 * Perform garbage collection on a live object.
 *
 * @param liveMark The current live mark.
 */
void StemInputSource::live(size_t liveMark)
{
    memory_mark(inputBuffer);
    memory_mark(stem);
    memory_mark(lastValue);
}


/**
 * Perform generalized live marking on an object.  This is
 * used when mark-and-sweep processing is needed for purposes
 * other than garbage collection.
 *
 * @param reason The reason for the marking call.
 */
void StemInputSource::liveGeneral(MarkReason reason)
{
    memory_mark_general(inputBuffer);
    memory_mark_general(stem);
    memory_mark_general(lastValue);
}


/**
 * Perform initial setup on this input source.
 */
void StemInputSource::init()
{
    // get the stem.0 value
    RexxObject *count = stem->getElement(size_t(0));
    // if completely not there, then we'll just treat this like a replace
    if (count == OREF_NULL)
    {
        // this is an error for input
        reportException(Error_Execution_missing_stem_array_index, stem->getName());
    }
    // the stem.0 value must be an integer size. We'll start writing at the next position
    else
    {
        // a valid whole number index
        if (!count->unsignedNumberValue(arraySize, Numerics::ARGUMENT_DIGITS))
        {
            reportException(Error_Invalid_whole_number_stem_array_index, stem->getName(), count);
        }
    }
    // we always start at element 1
    index = 1;
}


/**
 * Read the next line from the stem source
 *
 * @param context The NativeActivation context for the command handler, which
 *                will handler error/condition traps for this callback.
 */
RexxString *StemInputSource::read(NativeActivation *context)
{
    // if we've reached the end, done reading, so return nothing.
    if (index > arraySize)
    {
        lastValue = OREF_NULL;
        return OREF_NULL;
    }
    // get the next element
    RexxObject *value = stem->getFullElement(index++);
    // this is not supposed to be a sparse array, but for safety,
    // we'll turn this into a null string
    if (value == OREF_NULL)
    {
        return GlobalNames::NULLSTRING;
    }
    // Conceptually, everything in here is supposed to be a string.
    // However, if we unalaterally assume it is a string, they very bad things
    // happen. Additionally, this value might be an integer or numberstring object
    // pretending to be a string, so we need to force this to be a real string.

    // Additionally, we keep a reference to this in case it is a newly
    // allocated object that could be garbage collected unless we hold on
    // to a reference
    lastValue = value->requestString();

    return lastValue;
}


class LineinInvoker : public TrapInvoker
{
public:
    LineinInvoker(RexxObject *s, RexxString *&returned) : stream(s), nextLine(returned) { ; }

    virtual void invoke()
    {
        // read a line from the stream
        ProtectedObject result;
        nextLine = (RexxString *)stream->sendMessage(GlobalNames::LINEIN, result);
    }

    RexxObject *stream;         // the stream we're reading from
    RexxString *&nextLine;       // the read line
};

/**
 * Allocate a new stream input source
 *
 * @param size   The size of the object.
 *
 * @return Storage for creating the object.
 */
void *StreamObjectInputSource::operator new(size_t size)
{
    return new_object(size, T_StreamObjectInputSource);
}


/**
 * Construct a output target ogject
 *
 * @param stem      The supplied stream object
 */
StreamObjectInputSource::StreamObjectInputSource(RexxObject *s)
{
    stream = s;
}


/**
 * Perform garbage collection on a live object.
 *
 * @param liveMark The current live mark.
 */
void StreamObjectInputSource::live(size_t liveMark)
{
    memory_mark(inputBuffer);
    memory_mark(stream);
    memory_mark(lastValue);
}


/**
 * Perform generalized live marking on an object.  This is
 * used when mark-and-sweep processing is needed for purposes
 * other than garbage collection.
 *
 * @param reason The reason for the marking call.
 */
void StreamObjectInputSource::liveGeneral(MarkReason reason)
{
    memory_mark_general(inputBuffer);
    memory_mark_general(stream);
    memory_mark_general(lastValue);
}


/**
 * Read the next line from the stem source
 *
 * @param context The NativeActivation context for the command handler, which
 *                will handler error/condition traps for this callback.
 */
RexxString *StreamObjectInputSource::read(NativeActivation *context)
{
    // if we previously hit the end, then return nothing
    if (hitEnd)
    {
        lastValue = OREF_NULL;
        return OREF_NULL;
    }

    // read a line from the stream
    ProtectedObject result;

    // invoke LINEIN with trap protection
    LineinInvoker invoker(stream, lastValue);
    RedirectionDispatcher dispatcher(invoker);

    // invoke the method with appropriate trapping
    context->getActivity()->run(dispatcher);

    // this terminated due to a trapped condition (likely NOTREADY)
    // treat this as an eof
    if (dispatcher.conditionOccurred())
    {
        hitEnd = true;
        return OREF_NULL;
    }

    // return the read string
    return lastValue;
}


/**
 * Allocate a new stream input source
 *
 * @param size   The size of the object.
 *
 * @return Storage for creating the object.
 */
void *StreamInputSource::operator new(size_t size)
{
    return new_object(size, T_StreamInputSource);
}


/**
 * Construct a output target ogject
 *
 * @param n      The supplied stream name
 */
StreamInputSource::StreamInputSource(RexxString *n)
{
    name = n;
}


/**
 * Perform garbage collection on a live object.
 *
 * @param liveMark The current live mark.
 */
void StreamInputSource::live(size_t liveMark)
{
    memory_mark(inputBuffer);
    memory_mark(stream);
    memory_mark(lastValue);
    memory_mark(name);
}


/**
 * Perform generalized live marking on an object.  This is
 * used when mark-and-sweep processing is needed for purposes
 * other than garbage collection.
 *
 * @param reason The reason for the marking call.
 */
void StreamInputSource::liveGeneral(MarkReason reason)
{
    memory_mark_general(inputBuffer);
    memory_mark_general(stream);
    memory_mark_general(lastValue);
    memory_mark_general(name);
}


/**
 * Perform initialization of a stream input source
 */
void StreamInputSource::init()
{
    // create an instance of the stream class for the given name
    RexxClass *streamClass = TheRexxPackage->findClass(GlobalNames::STREAM);
    ProtectedObject result;
    stream = streamClass->sendMessage(GlobalNames::NEW, name, result);

    // open for input only
    RexxString *openResult = (RexxString *)stream->sendMessage(GlobalNames::OPEN, GlobalNames::READ, result);
    // the open should return ready
    if (!openResult->strCompare(GlobalNames::OPENREADY))
    {
        reportException(Error_Execution_file_not_readable, name, openResult);
    }
}


/**
 * Perform post-execution cleanup on the streams
 */
void StreamInputSource::cleanup()
{
    ProtectedObject result;
    stream->sendMessage(GlobalNames::CLOSE, result);
}


/**
 * Allocate a new array input source
 *
 * @param size   The size of the object.
 *
 * @return Storage for creating the object.
 */
void *ArrayInputSource::operator new(size_t size)
{
    return new_object(size, T_ArrayInputSource);
}


/**
 * Construct a output target ogject
 *
 * @param n      The supplied stream name
 */
ArrayInputSource::ArrayInputSource(ArrayClass *a)
{
    array = a;
}


/**
 * Perform garbage collection on a live object.
 *
 * @param liveMark The current live mark.
 */
void ArrayInputSource::live(size_t liveMark)
{
    memory_mark(inputBuffer);
    memory_mark(array);
    memory_mark(lastValue);
}


/**
 * Perform generalized live marking on an object.  This is
 * used when mark-and-sweep processing is needed for purposes
 * other than garbage collection.
 *
 * @param reason The reason for the marking call.
 */
void ArrayInputSource::liveGeneral(MarkReason reason)
{
    memory_mark_general(inputBuffer);
    memory_mark_general(array);
    memory_mark_general(lastValue);
}


/**
 * Perform initialization of an array input source
 */
void ArrayInputSource::init()
{
    // we always read from the beginning
    index = 1;
    // up to the end of the items
    arraySize = array->items();
}


/**
 * Read the next line from the array
 *
 * @param context The NativeActivation context for the command handler, which
 *                will handler error/condition traps for this callback.
 *
 * @return The next line or OREF_NULL if we've reached the end
 */
RexxString *ArrayInputSource::read(NativeActivation *context)
{
    // if we've already reached the end, return a null
    if (index > arraySize)
    {
        lastValue = OREF_NULL;
        return OREF_NULL;
    }
    // Conceptually, everything in here is supposed to be a string.
    // However, if we unalaterally assume it is a string, they very bad things
    // happen. Additionally, this value might be an integer or numberstring object
    // pretending to be a string, so we need to force this to be a real string.

    // Additionally, we keep a reference to this in case it is a newly
    // allocated object that could be garbage collected unless we hold on
    // to a reference
    lastValue = (RexxString *)array->get(index++)->requestString();

    return lastValue;
}


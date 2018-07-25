/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2005-2018 Rexx Language Association. All rights reserved.    */
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
/*                                                       CommandIOContext.cpp */
/*                                                                            */
/* The processing context for ADDRESS WITH I/O interaction                    */
/*                                                                            */
/******************************************************************************/
#include "RexxCore.h"
#include "Activity.hpp"
#include "CommandIOContext.hpp"
#include "InputRedirector.hpp"
#include "OutputRedirector.hpp"
#include "ProtectedObject.hpp"


/**
 * Allocate a CommandIOContext object
 *
 * @param size   The base size of this object.
 *
 * @return A newly allocated Rexx object.
 */
void  *CommandIOContext::operator new(size_t size)
{
    return new_object(size, T_CommandIOContext);
}


/**
 * Perform garbage collection on a live object.
 *
 * @param liveMark The current live mark.
 */
void CommandIOContext::live(size_t liveMark)
{
    memory_mark(input);
    memory_mark(output);
    memory_mark(error);
}


/**
 * Perform generalized live marking on an object.  This is
 * used when mark-and-sweep processing is needed for purposes
 * other than garbage collection.
 *
 * @param reason The reason for the marking call.
 */
void CommandIOContext::liveGeneral(MarkReason reason)
{
    memory_mark_general(input);
    memory_mark_general(output);
    memory_mark_general(error);
}


/**
 * Now resolve conflicts between the input and output
 * streams and check for duplicate output targets.
 */
void CommandIOContext::resolveConflicts()
{
    // not yet determined that we have dual output
    bool dualOutput = false;

    // if we have both error and output specified, we need to check that these
    // are the same and collapse them to a single target
    if (error != OREF_NULL && output != OREF_NULL && output->isSameTarget(error))
    {
        error = output;
        // if we detect a conflict between the input and output, then
        // we need to adjust both.
        dualOutput = true;
    }

    // now check if we have an input conflict
    if (input != OREF_NULL)
    {
        // now see if these are the same target
        if (output != OREF_NULL && output->needsBuffering(input))
        {
            // make this buffered until the command returns
            output = new BufferingOutputTarget(output);
            // if we've already established that output and error
            // are the same, then adjust error to point to the buffered
            // input
            if (dualOutput)
            {
                error = output;
            }
        }
        // now we still could have a conflict beteen the input ane error, so check that too
        else if (error != OREF_NULL && error->needsBuffering(input))
        {
            // make this buffered until the command returns
            output = new BufferingOutputTarget(output);
        }
    }
}


/**
 * Initialize the various elements of the IO context.
 */
void CommandIOContext::init()
{
    // initialize each element. Input, if it exists should come first
    if (input != OREF_NULL)
    {
        input->init();
    }
    if (output != OREF_NULL)
    {
        output->init();
    }
    if (error != OREF_NULL)
    {
        error->init();
    }
}


/**
 * Perform post-command cleanup of the various elements of the
 * IO context.
 */
void CommandIOContext::cleanup()
{
    // initialize each element. Input, if it exists should come first
    if (input != OREF_NULL)
    {
        input->cleanup();
    }
    if (output != OREF_NULL)
    {
        output->cleanup();
    }
    if (error != OREF_NULL)
    {
        error->cleanup();
    }
}


/**
 * return the next line from the ADDRESS instruction
 * context as a CSTRING.
 *
 * @param context The NativeActivation context for the command handler, which
 *                will handler error/condition traps for this callback.
 * @return The CSTRING value for the next line or NULL if we've hit EOF.
 */
RexxString *CommandIOContext::readInput(NativeActivation *context)
{
    // first make sure we have a source object
    if (input == OREF_NULL)
    {
        return OREF_NULL;
    }
    return input->read(context);
}


/**
 * Read all of the input data into a buffer, with appropriate
 * line separation
 *
 * @param context The NativeActivation context for the command handler, which
 *                will handler error/condition traps for this callback.
 * @param data    The returned buffer pointer
 * @param length  the returned data length
 */
void CommandIOContext::readInputBuffered(NativeActivation *context, const char *&data, size_t &length)
{
    // first make sure we have a source object
    if (input != OREF_NULL)
    {
        input->readBuffered(context, data, length);
    }
}


/**
 * Write a line to the command output catcher
 *
 * @param context The NativeActivation context for the command handler, which
 *                will handler error/condition traps for this callback.
 * @param data   Pointer to the output data
 * @param len    Length of the output data
 */
void CommandIOContext::writeOutput(NativeActivation *context, const char *data, size_t len)
{
    // this shouldn't happen, but if not redirected, don't crash!
    if (output != OREF_NULL)
    {
        Protected<RexxString> value = new_string(data, len);
        output->write(value);
    }
}


/**
 * Write a line to the command error catcher
 *
 * @param context The NativeActivation context for the command handler, which
 *                will handler error/condition traps for this callback.
 * @param data   Pointer to the output data
 * @param len    Length of the output data
 */
void CommandIOContext::writeError(NativeActivation *context, const char *data, size_t len)
{
    // this shouldn't happen, but if not redirected, don't crash!
    if (error != OREF_NULL)
    {
        Protected<RexxString> value = new_string(data, len);
        error->write(value);
    }
}


/**
 * Write a a buffer of data to the output target
 *
 * @param context The NativeActivation context for the command handler, which
 *                will handler error/condition traps for this callback.
 * @param data   Pointer to the output data
 * @param len    Length of the output data
 */
void CommandIOContext::writeOutputBuffer(NativeActivation *context, const char *data, size_t len)
{
    // this shouldn't happen, but if not redirected, don't crash!
    if (output != OREF_NULL)
    {
        output->writeBuffer(data, len);
    }
}


/**
 * Write a buffer to the command error catcher
 *
 * @param context The NativeActivation context for the command handler, which
 *                will handler error/condition traps for this callback.
 * @param data   Pointer to the output data
 * @param len    Length of the output data
 */
void CommandIOContext::writeErrorBuffer(NativeActivation *context, const char *data, size_t len)
{
    // this shouldn't happen, but if not redirected, don't crash!
    if (error != OREF_NULL)
    {
        error->writeBuffer(data, len);
    }
}

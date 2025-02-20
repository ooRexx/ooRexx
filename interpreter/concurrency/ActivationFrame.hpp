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
#ifndef ActivationFrame_Included
#define ActivationFrame_Included

#include "Activity.hpp"

class RexxActivation;
class NativeActivation;
class MethodClass;
class StackFrameClass;
class LanguageParser;


/**
 * The base class for all stack frames.
 */
class ActivationFrame
{
friend class Activity;
 public:
    inline ActivationFrame(Activity *a) : activity(a)
    {
        // it would be better to have the activity class do this, but because
        // we're doing this with inline methods, we run into a bit of a
        // circular reference problem
        next = activity->activationFrames;
        activity->activationFrames = this;
    }

    // explicitly disable a frame for situations where we no longer
    // wish to have the frame show up in the tracebacks.
    inline void disableFrame()
    {
        // only do this once
        if (activity != OREF_NULL)
        {
            // remove ourselves from the list
            activity->activationFrames = next;
            // this will keep us from attempting to do this twice
            activity = OREF_NULL;
        }

    }

    inline ~ActivationFrame()
    {
        // make this no longer active
        disableFrame();
    }

    virtual RexxString *messageName() = 0;
    virtual BaseExecutable *executable() = 0;
    virtual StackFrameClass *createStackFrame() = 0;
    virtual PackageClass *getPackage() = 0;

 protected:

    ActivationFrame *next;             // the next activation frame in the chain
    Activity *activity;                // the activity we're running on
};


/**
 * A stack frame representing running Rexx code.
 */
class RexxActivationFrame : public ActivationFrame
{
 public:
    inline RexxActivationFrame(Activity *a, RexxActivation *context) : ActivationFrame(a), activation(context) { }

    RexxString *messageName() override;
    BaseExecutable *executable() override;
    StackFrameClass *createStackFrame() override;
    PackageClass *getPackage() override;
    RexxObject *getContextObject();

 protected:

    RexxActivation *activation;        // the activation backing this frame
};


/**
 * A stack frame representing running native code.
 */
class NativeActivationFrame : public ActivationFrame
{
 public:
    inline NativeActivationFrame(Activity *a, NativeActivation *context) : ActivationFrame(a), activation(context) { }

    RexxString *messageName() override;
    BaseExecutable *executable() override;
    StackFrameClass *createStackFrame() override;
    PackageClass *getPackage() override;

 protected:

    NativeActivation *activation;        // the activation backing this frame
};


/**
 * A frame representing an internal C++ method call.
 */
class InternalActivationFrame : public ActivationFrame
{
 public:
    inline InternalActivationFrame(Activity *a, RexxString *n, RexxObject *t, MethodClass *m, RexxObject **args, size_t c)
        : ActivationFrame(a), name(n), target(t), frameMethod(m), argPtr(args), count(c) { }

    RexxString *messageName() override;
    BaseExecutable *executable() override;
    StackFrameClass *createStackFrame() override;
    PackageClass *getPackage() override;

 protected:

    RexxString *name;                        // message name associated with the invocation
    MethodClass *frameMethod;                // the backing method object
    RexxObject *target;                      // method target
    RexxObject **argPtr;                     // arguments passed to this instance
    size_t       count;                      // count of arguments
};


/**
 * A frame representing a source file being translated.
 * A lot of syntax errors are generated with one of these
 * at the top of the stack.
 */
class CompileActivationFrame : public ActivationFrame
{
 public:
    inline CompileActivationFrame(Activity *a, LanguageParser *p) : ActivationFrame(a), parser(p) { }

    RexxString *messageName() override;
    BaseExecutable *executable() override;
    StackFrameClass *createStackFrame() override;
    PackageClass *getPackage() override;

protected:

    LanguageParser *parser;                  // the parser instance handling source translation.
};

#endif

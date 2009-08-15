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
#ifndef ActivationFrame_Included
#define ActivationFrame_Included

#include "RexxActivity.hpp"

class RexxActivation;
class RexxNativeActivation;
class RexxMethod;
class StackFrameClass;
class RexxSource;

class ActivationFrame
{
friend class RexxActivity;
public:
    inline ActivationFrame(RexxActivity *a) : activity(a)
    {
        // it would be better to have the activity class do this, but because
        // we're doing this with inline methods, we run into a bit of a
        // circular reference problem
        next = activity->activationFrames;
        activity->activationFrames = this;
    }

    inline ~ActivationFrame()
    {
        // remove ourselves from the list and give this object a
        // little hold protection.
        activity->activationFrames = next;
    }

    virtual RexxString *messageName() = 0;
    virtual RexxMethod *method() = 0;
    virtual StackFrameClass *createStackFrame() = 0;
    virtual RexxSource *getSource() = 0;

protected:
    ActivationFrame *next;             // the next activation frame in the chain
    RexxActivity *activity;            // the activity we're running on
};


class RexxActivationFrame : public ActivationFrame
{
public:
    inline RexxActivationFrame(RexxActivity *a, RexxActivation *context) : ActivationFrame(a), activation(context) { }

    virtual RexxString *messageName();
    virtual RexxMethod *method();
    virtual StackFrameClass *createStackFrame();
    virtual RexxSource *getSource();

protected:
    RexxActivation *activation;        // the activation backing this frame
};


class NativeActivationFrame : public ActivationFrame
{
public:
    inline NativeActivationFrame(RexxActivity *a, RexxNativeActivation *context) : ActivationFrame(a), activation(context) { }

    virtual RexxString *messageName();
    virtual RexxMethod *method();
    virtual StackFrameClass *createStackFrame();
    virtual RexxSource *getSource();

protected:
    RexxNativeActivation *activation;        // the activation backing this frame
};


class InternalActivationFrame : public ActivationFrame
{
public:
    inline InternalActivationFrame(RexxActivity *a, RexxString *n, RexxObject *t, RexxMethod *m, RexxObject **args, size_t c)
        : ActivationFrame(a), name(n), target(t), frameMethod(m), argPtr(args), count(c) { }

    virtual RexxString *messageName();
    virtual RexxMethod *method();
    virtual StackFrameClass *createStackFrame();
    virtual RexxSource *getSource();

protected:
    RexxString *name;                        // message name associated with the invocation
    RexxMethod *frameMethod;                 // the backing method object
    RexxObject *target;                      // method target
    RexxObject **argPtr;                     // arguments passed to this instance
    size_t       count;                      // count of arguments
};


class ParseActivationFrame : public ActivationFrame
{
public:
    inline ParseActivationFrame(RexxActivity *a, RexxSource *s) : ActivationFrame(a), source(s) { }

    virtual RexxString *messageName();
    virtual RexxMethod *method();
    virtual StackFrameClass *createStackFrame();
    virtual RexxSource *getSource();

protected:
    RexxSource *source;                      // the source object being parsed.
};

#endif

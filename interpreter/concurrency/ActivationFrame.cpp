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

#include "RexxCore.h"
#include "ActivationFrame.hpp"
#include "RexxActivation.hpp"
#include "RexxNativeActivation.hpp"
#include "StackFrameClass.hpp"

RexxString *RexxActivationFrame::messageName()
{
    return activation->getMessageName();
}

RexxMethod *RexxActivationFrame::method()
{
    return (RexxMethod *)activation->getExecutableObject();
}

StackFrameClass *RexxActivationFrame::createStackFrame()
{
    return activation->createStackFrame();
}

RexxSource *RexxActivationFrame::getSource()
{
    return activation->getEffectiveSourceObject();
}

RexxString *NativeActivationFrame::messageName()
{
    return activation->getMessageName();
}

RexxMethod *NativeActivationFrame::method()
{
    return (RexxMethod *)activation->getExecutableObject();
}

StackFrameClass *NativeActivationFrame::createStackFrame()
{
    return activation->createStackFrame();
}

RexxSource *NativeActivationFrame::getSource()
{
    return activation->getSourceObject();
}

RexxString *InternalActivationFrame::messageName()
{
    return name;
}

RexxMethod *InternalActivationFrame::method()
{
    return frameMethod;
}

StackFrameClass *InternalActivationFrame::createStackFrame()
{
    return new StackFrameClass(FRAME_METHOD, name, frameMethod, new_array(count, argPtr), new_string(COMPILED_MARKER), SIZE_MAX);
}

RexxSource *InternalActivationFrame::getSource()
{
    return OREF_NULL;
}

RexxString *ParseActivationFrame::messageName()
{
    return OREF_NULL;
}

RexxMethod *ParseActivationFrame::method()
{
    return OREF_NULL;
}

StackFrameClass *ParseActivationFrame::createStackFrame()
{
    return source->createStackFrame();
}

RexxSource *ParseActivationFrame::getSource()
{
    return source;
}

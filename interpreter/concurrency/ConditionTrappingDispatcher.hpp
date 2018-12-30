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
/* REXX Kernel                                                                */
/*                                                                            */
/* Allows calls to be made to methods with from internal code with condition  */
/* trapping.                                                                  */
/*                                                                            */
/******************************************************************************/
#ifndef Included_ConditionTrappingDispatcher_hpp
#define Included_ConditionTrappingDispatcher_hpp

#include "RexxCore.h"
#include "TrappingDispatcher.hpp"

class Activity;
class DirectoryClass;

/**
 * A class used to wrap a message operation fed into a
 * Trapping dispatcher. The calling context should create
 * a subclass of this that holds whatever data is needed
 * to make a method call.
 */
class TrapInvoker
{
public:
    virtual void invoke() { ; }
};


/**
 * A generalized dispatcher that allows internal code to
 * make a call with condition trapping.
 */
class ConditionTrappingDispatcher : public TrappingDispatcher
{
public:
    inline ConditionTrappingDispatcher(TrapInvoker &t) : invoker(t), errorCode(0) { }
    virtual ~ConditionTrappingDispatcher() { ; }

    void run() override;
    void handleError(wholenumber_t, DirectoryClass *) override;
    void handleError(DirectoryClass *) override;
    bool trapConditions() override { return true; }

    bool errorOccurred()  { return errorCode != 0; }
    bool conditionOccurred() { return conditionData != OREF_NULL; }
    DirectoryClass *getCondition() { return conditionData; }

protected:
    TrapInvoker &invoker;          // that invokes the method call.
    wholenumber_t errorCode;       // any trapped error code
};

#endif


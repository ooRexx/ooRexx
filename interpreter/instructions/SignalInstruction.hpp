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
/******************************************************************************/
/* REXX Kernel                                          SignalInstruction.hpp */
/*                                                                            */
/* Primitive SIGNAL instruction Class Definitions                             */
/*                                                                            */
/******************************************************************************/
#ifndef Included_RexxInstructionSignal
#define Included_RexxInstructionSignal

#include "RexxInstruction.hpp"
#include "CallInstruction.hpp"

/**
 * Instruction object for a "normal" SIGNAL instruction
 * that jumps to a label location.
 */
class RexxInstructionSignal : public RexxInstructionCallBase
{
 public:
    RexxInstructionSignal(RexxString *);
    inline RexxInstructionSignal(RESTORETYPE restoreType) { ; };

    void live(size_t) override;
    void liveGeneral(MarkReason reason) override;
    void flatten(Envelope *) override;

    void execute(RexxActivation *, ExpressionStack *) override;
    void resolve (StringTable *) override;
};


/**
 * An instruction object for a dynamic SIGNAL instruction
 * (SIGNAL VALUE or SIGNAL expr).  This resolves the
 * target label from an expression result at run time.
 */
class RexxInstructionDynamicSignal : public RexxInstructionDynamicCallBase
{
 public:
    RexxInstructionDynamicSignal(RexxInternalObject *);
    inline RexxInstructionDynamicSignal(RESTORETYPE restoreType) { ; };

    void live(size_t) override;
    void liveGeneral(MarkReason reason) override;
    void flatten(Envelope*) override;

    void execute(RexxActivation *, ExpressionStack *) override;
};


/**
 * An instruction object to handle the basics of the SIGNAL
 * ON/OFF instruction.
 */
class RexxInstructionSignalOn : public RexxInstructionTrapBase
{
 public:
    RexxInstructionSignalOn(RexxString*, RexxString *);
    inline RexxInstructionSignalOn(RESTORETYPE restoreType) { ; };

    void live(size_t) override;
    void liveGeneral(MarkReason reason) override;
    void flatten(Envelope*) override;

    void execute(RexxActivation *, ExpressionStack *) override;
    void resolve(StringTable *) override;

    void trap(RexxActivation *context, DirectoryClass  *conditionObj) override;
};
#endif

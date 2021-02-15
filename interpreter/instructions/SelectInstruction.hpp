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
/* REXX Kernel                                          SelectInstruction.hpp */
/*                                                                            */
/* Primitive SELECT instruction Class Definitions                             */
/*                                                                            */
/******************************************************************************/
#ifndef Included_RexxInstructionSelect
#define Included_RexxInstructionSelect

#include "RexxInstruction.hpp"

class LanguageParser;
class RexxInstructionOtherwise;

/**
 * Base SELECT instruction.  The WHEN clauses really do
 * all of the work.
 */
class RexxInstructionSelect : public RexxBlockInstruction
{
public:
    inline RexxInstructionSelect() { }
    RexxInstructionSelect(RexxString *);
    inline RexxInstructionSelect(RESTORETYPE restoreType) { ; };

    void live(size_t) override;
    void liveGeneral(MarkReason reason) override;
    void flatten(Envelope*) override;

    // required by RexxInstructon
    void execute(RexxActivation *, ExpressionStack *) override;

    // required by RexxBlockinstruction;
    void matchEnd(RexxInstructionEnd *, LanguageParser *) override;
    bool isLoop() override;
    void terminate(RexxActivation *, DoBlock *) override;

    // all select instructions do the same thing here
    EndBlockType getEndStyle() override { return SELECT_BLOCK; }

    // specific to the SELECT instruction
    void addWhen(RexxInstructionIf *);
    void setOtherwise(RexxInstructionOtherwise *);

    QueueClass                *whenList;  // list of WHEN end targets
    RexxInstructionOtherwise *otherwise; // OTHERWISE matching the SELECT
};


/**
 * A SELECT CASE instruction.  This evaluates an expression
 * that all of the WHEN clauses use to compare.
 */
class RexxInstructionSelectCase : public RexxInstructionSelect
{
public:
    RexxInstructionSelectCase(RexxString *label, RexxInternalObject *condition);
    inline RexxInstructionSelectCase(RESTORETYPE restoreType) { ; };

    void live(size_t) override;
    void liveGeneral(MarkReason reason) override;
    void flatten(Envelope*) override;

    // required by RexxInstructon
    void execute(RexxActivation *, ExpressionStack *) override;

    RexxInternalObject *caseExpr;  // the SELECT CASE expression.
};
#endif

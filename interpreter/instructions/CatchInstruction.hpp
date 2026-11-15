/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2026 Rexx Language Association. All rights reserved.         */
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
/******************************************************************************/
/* REXX Kernel                                                                */
/*                                                                            */
/* CATCH clause of a DO, LOOP or SELECT block (RFE #31)                       */
/*                                                                            */
/******************************************************************************/
#ifndef Included_RexxInstructionCatch
#define Included_RexxInstructionCatch

#include "RexxInstruction.hpp"

class DoBlock;
class RexxVariableBase;
class DirectoryClass;

/**
 * A CATCH clause.  The clauses of a block follow its body and
 * precede its END.  A block that has CATCH clauses gets an END of
 * style CATCH_BLOCK; the CATCH clauses remember the style the END
 * would have had (the style used when the body runs into the first
 * CATCH clause), the block they belong to and the next CATCH clause.
 */
class RexxInstructionCatch : public RexxInstruction
{
 public:
    inline RexxInstructionCatch(RESTORETYPE restoreType) { ; };
    RexxInstructionCatch(ArrayClass *c, RexxVariableBase *v = OREF_NULL, ArrayClass *q = OREF_NULL);

    void live(size_t) override;
    void liveGeneral(MarkReason reason) override;
    void flatten(Envelope *) override;

    // a CATCH clause is closed by the END of its block
    bool isBlock() override { return true; }
    void execute(RexxActivation *, ExpressionStack *) override;

    bool handles(RexxString *condition, DirectoryClass *conditionObj, RexxActivation *context);
    static bool sameCode(RexxObject *code, RexxObject *rc, RexxActivation *context);
    static wholenumber_t errorNumber(RexxString *text);
    static RexxObject *readCode(RexxObject *code, RexxActivation *context);
    void assignCondition(RexxActivation *context, DirectoryClass *conditionObj);
    static RexxInstructionCatch *findClause(DoBlock *doblock, RexxString *condition, DirectoryClass *conditionObj, RexxActivation *context);
    static RexxInstructionCatch *firstClause(RexxBlockInstruction *block);
    static bool hasCatches(RexxBlockInstruction *block);

    inline void setOwner(RexxBlockInstruction *o) { owner = o; }
    inline RexxBlockInstruction *getOwner() { return owner; }
    inline void setNextCatch(RexxInstructionCatch *c) { nextCatch = c; }
    inline RexxInstructionCatch *getNextCatch() { return nextCatch; }
    inline void setFirstCatch(RexxInstructionCatch *c) { firstCatch = c; }
    inline RexxInstructionCatch *getFirstCatch() { return firstCatch; }
    inline void setOwnerStyle(EndBlockType s) { ownerStyle = s; }

 protected:

    ArrayClass           *conditions;  // the condition names this clause catches
    RexxBlockInstruction *owner;       // the DO, LOOP or SELECT this clause belongs to
    RexxInstructionCatch *nextCatch;   // the next CATCH clause of the same block
    RexxInstructionCatch *firstCatch;  // the first CATCH clause of the same block
    EndBlockType          ownerStyle;  // the END style the block would have without CATCH
    RexxVariableBase     *variable;    // CATCH variable = ...: gets the condition object
    ArrayClass           *codes;       // CATCH SYNTAX number, ERROR code, FAILURE code: one per condition (.nil if none), or null
};
#endif

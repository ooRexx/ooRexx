/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
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
/* REXX Kernel                                            RexxInstruction.hpp */
/*                                                                            */
/* Primitive Abstract Instruction Class Definitions                           */
/*                                                                            */
/******************************************************************************/
#ifndef Included_RexxInstruction
#define Included_RexxInstruction

#include "Token.hpp"

class RexxInstructionEnd;
class RexxInstructionEndIf;
class RexxClause;
class LanguageParser;
class RexxVariableBase;

#include "SourceLocation.hpp"

/**
 * Base class for all instruction objects.  Defines
 * behavior common to all instructions, such as having
 * a successor instruction and recording the instruction
 * location.
 */
class RexxInstruction : public RexxInternalObject
{
 friend class RexxActivation;
 public:
           void *operator new(size_t);
    inline void  operator delete(void *) { }

    RexxInstruction(RexxClause *clause, InstructionKeyword type);
    inline RexxInstruction(RESTORETYPE restoreType) { ; };
    inline RexxInstruction() { ; }

    void live(size_t) override;
    void liveGeneral(MarkReason reason) override;
    void flatten(Envelope *) override;

    virtual void execute(RexxActivation *, ExpressionStack *) { ; };
    // indicates whether this is a block instruction type that requires
    // a matching END
    virtual bool isBlock() { return false; }
    // indicates if this is a complex control construct.
    virtual bool isControl() { return false; }

    inline const SourceLocation &getLocation() { return instructionLocation; }
    inline void  setLocation(const SourceLocation &l) { instructionLocation = l; }

    // NOTE:  This method is only used during program translation, so we can skip using
    // OrefSet to set this variable.
    inline void setNext(RexxInstruction *next) { nextInstruction = next; };
    inline RexxInstruction *next() { return nextInstruction; }
    inline bool isLast() { return nextInstruction == OREF_NULL; }
    void        setStart(size_t line, size_t off) { instructionLocation.setStart(line, off); }
    void        setEnd(size_t line, size_t off) { instructionLocation.setEnd(line, off); }
    inline      void        setType(InstructionKeyword type) { instructionType = type; };
    inline      InstructionKeyword getType()     { return instructionType;  };
    inline      bool        isType(InstructionKeyword type)  { return instructionType == type; }
    inline      size_t      getLineNumber()      { return instructionLocation.getLineNumber(); }
    static      void evaluateArguments(RexxActivation *context, ExpressionStack *stack, RexxInternalObject **argArray, size_t argCount);

    InstructionKeyword  instructionType;    // name of the instruction
    SourceLocation    instructionLocation;  // location of the instruction in its source
    RexxInstruction  *nextInstruction;      // the next instruction object in the assembled chain.
};

// types of END terminators
typedef enum
{
    DO_BLOCK,
    SELECT_BLOCK,
    OTHERWISE_BLOCK,
    LOOP_BLOCK,
    LABELED_SELECT_BLOCK,
    LABELED_OTHERWISE_BLOCK,
    LABELED_DO_BLOCK,
} EndBlockType;

class DoBlock;

/**
 * Base class for all Block instruction types.  This
 * defines the interface that all block instructions must
 * implement to handle resolutions.
 */
class RexxBlockInstruction : public RexxInstruction
{
 public:
     RexxBlockInstruction() {;};
     RexxBlockInstruction(RESTORETYPE restoreType) {; };

     // virtual functions required by subclasses to override.

     bool isBlock()override { return true; }
     // all block instructions are also control instructions.
     bool isControl()override { return true; }

     virtual EndBlockType getEndStyle() = 0;
     virtual bool isLoop() { return false; };
     virtual void matchEnd(RexxInstructionEnd *, LanguageParser *) {; };
     virtual void terminate(RexxActivation *, DoBlock *) {; };
     virtual RexxVariableBase* getCountVariable() { return OREF_NULL; }

     // inherited behaviour
     // NOTE: tokens on ends and label instructions are interned strings, so
     // we can just do pointer compares
     bool isLabel(RexxString *name) { return name == label; }
     RexxString* getLabel() { return label; };
     void handleDebugPause(RexxActivation *context, DoBlock *doblock);

    RexxString *label;         // the block instruction label
    RexxInstructionEnd *end;   // the END matching the block instruction
};


/**
 * Base instruction for instructions that need to have an end
 * position set.
 */
class RexxInstructionSet : public RexxInstruction
{
 public:
    RexxInstructionSet() {;};
    RexxInstructionSet(RESTORETYPE restoreType) { ; };

    virtual void setEndInstruction(RexxInstructionEndIf *) {;}
};

/**
 * Common definition for instructions that are a keyword
 * with a single expression (SAY, INTERPRET, etc.).  This is
 * common enough to warrant a subclass.
 */
class RexxInstructionExpression : public RexxInstruction
{
 public:
    RexxInstructionExpression() { ; };
    RexxInstructionExpression(RESTORETYPE restoreType) { ; };

    void live(size_t) override;
    void liveGeneral(MarkReason reason) override;
    void flatten(Envelope *) override;

    RexxObject *evaluateExpression(RexxActivation *context, ExpressionStack *stack);
    RexxString *evaluateStringExpression(RexxActivation *context, ExpressionStack *stack);

 protected:
    RexxInternalObject *expression;     // expression to evaluate
};

// a convenience macro for initializing instruction/expression objects.
// for many of the instructions, they have a variable size array
// that are initialized in reverse order using items pulled from a
// provided queue.  This simplifies this initialization process and
// ensures it is done correctly.  NOTE:  This decrements the count
// variable so make sure you use the one from the argument list!!!!!!

#define initializeObjectArray(count, array, type, queue) \
{                                                  \
    while (count > 0)                              \
    {                                              \
        array[--count] = (type *)queue->pop();     \
    }                                              \
}

#endif

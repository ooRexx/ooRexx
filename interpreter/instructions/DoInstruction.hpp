/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2014 Rexx Language Association. All rights reserved.    */
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
/* REXX Kernel                                             DoInstruction.hpp  */
/*                                                                            */
/* Primitive DO instruction Class Definitions                                 */
/*                                                                            */
/******************************************************************************/
#ifndef Included_RexxInstructionDo
#define Included_RexxInstructionDo

#include "RexxInstruction.hpp"
#include "DoBlockComponents.hpp"
#include "EndInstruction.hpp"

class LanguageParser;


/**
 * The base class for a DO instruction.  This implements all of
 * the common END-matching behavior and label definitions.
 *
 */
class RexxInstructionBaseDo : public RexxBlockInstruction
{
 public:
    inline RexxInstructionBaseDo() { ; }

    virtual void live(size_t);
    virtual void liveGeneral(MarkReason reason);
    virtual void flatten(Envelope *);

    // required by RexxInstruction.  For most subclasses, the default
    // is sufficient.
    virtual void execute(RexxActivation *, ExpressionStack *);

    // methods required by RexxBlockInstruction;
    virtual void matchEnd(RexxInstructionEnd *, LanguageParser *);
    // most DO blocks are loops.  The simple styles will need to override.
    virtual EndBlockType getEndStyle() { return LOOP_BLOCK; }
    // Most DOs are loops...Simple DO will override again.
    virtual bool isLoop() { return true; };

    // specific to Do loops.  Most subclasses can rely on the default
    virtual void reExecute(RexxActivation *, ExpressionStack *, DoBlock *);
    virtual void terminate(RexxActivation *, DoBlock *);

    // most loops will want to override these two
    virtual void setup(RexxActivation *context, ExpressionStack *stack, DoBlock *doblock);
    virtual bool iterate(RexxActivation *context, ExpressionStack *stack, DoBlock *doblock, bool first);

    void matchLabel(RexxInstructionEnd *end, LanguageParser *source );
    void handleDebugPause(RexxActivation *context, DoBlock *doblock);
    void endLoop(RexxActivation *context);
};


/**
 * The simplest form of DO BLOCK.  This is a non-looping block.
 * It may have a label, but no other expressions to handle.
 */
class RexxInstructionSimpleDo : public RexxInstructionBaseDo
{
 public:
    inline RexxInstructionSimpleDo() { ; }
    inline RexxInstructionSimpleDo(RESTORETYPE restoreType) { ; };
           RexxInstructionSimpleDo(RexxString *l);

    // required by RexxInstruction
    virtual void execute(RexxActivation *, ExpressionStack *);

    // methods required by RexxBlockInstruction;
    virtual bool isLoop() { return false; }
    // most DO blocks are loops.  The simple styles will need to override.
    virtual EndBlockType getEndStyle() { return DO_BLOCK; }
};


/**
 * The DO FOREVER instruction.  Checks no state, it just loops
 * until terminated via other means.
 */
class RexxInstructionDoForever : public RexxInstructionBaseDo
{
 public:
    inline RexxInstructionDoForever() { ; }
    inline RexxInstructionDoForever(RESTORETYPE restoreType) { ; };
           RexxInstructionDoForever(RexxString *l);
};


/**
 * The DO OVER instruction.  Takes a snap shot of an object via
 * makearray method then iterates over the array
 */
class RexxInstructionDoOver : public RexxInstructionBaseDo
{
 public:
    inline RexxInstructionDoOver() { ; }
    inline RexxInstructionDoOver(RESTORETYPE restoreType) { ; };
           RexxInstructionDoOver(RexxString *l, OverLoop &o);

    virtual void live(size_t);
    virtual void liveGeneral(MarkReason reason);
    virtual void flatten(Envelope *);

    // Methods needed for loop iteration
    virtual void setup(RexxActivation *context, ExpressionStack *stack, DoBlock *doblock);
    virtual bool iterate(RexxActivation *context, ExpressionStack *stack, DoBlock *doblock, bool first);

 protected:

    OverLoop overLoop;          // handles control logic for a DO OVER
};


/**
 * The DO OVER UNTIL cond instruction.  Takes a snap shot of an
 * object via makearray method then iterates over the array
 */
class RexxInstructionDoOverUntil : public RexxInstructionDoOver
{
 public:
    inline RexxInstructionDoOverUntil() { ; }
    inline RexxInstructionDoOverUntil(RESTORETYPE restoreType) { ; };
           RexxInstructionDoOverUntil(RexxString *l, OverLoop &o, WhileUntilLoop &w);

    virtual void live(size_t);
    virtual void liveGeneral(MarkReason reason);
    virtual void flatten(Envelope *);

    // Methods needed for loop iteration
    virtual bool iterate(RexxActivation *context, ExpressionStack *stack, DoBlock *doblock, bool first);

protected:

    WhileUntilLoop whileLoop;   // handles the conditional part
};


/**
 * The DO OVER WHILE cond instruction.  Takes a snap shot of an
 * object via makearray method then iterates over the array
 */
class RexxInstructionDoOverWhile : public RexxInstructionDoOverUntil
{
 public:
    inline RexxInstructionDoOverWhile() { ; }
    inline RexxInstructionDoOverWhile(RESTORETYPE restoreType) { ; };
           RexxInstructionDoOverWhile(RexxString *l, OverLoop &o, WhileUntilLoop &w);

    // Methods needed for loop iteration
    virtual bool iterate(RexxActivation *context, ExpressionStack *stack, DoBlock *doblock, bool first);
};


/**
 * The controled DO instruction.  Sets an iteration variable on
 * each pass through the loop.
 */
class RexxInstructionControlledDo: public RexxInstructionBaseDo
{
 public:
    inline RexxInstructionControlledDo() { ; }
    inline RexxInstructionControlledDo(RESTORETYPE restoreType) { ; };
           RexxInstructionControlledDo(RexxString *l, ControlledLoop &c);

    virtual void live(size_t);
    virtual void liveGeneral(MarkReason reason);
    virtual void flatten(Envelope *);

    // Methods needed for loop iteration
    virtual void setup(RexxActivation *context, ExpressionStack *stack, DoBlock *doblock);
    virtual bool iterate(RexxActivation *context, ExpressionStack *stack, DoBlock *doblock, bool first);

 protected:

    ControlledLoop controlLoop;          // handles control logic for a controlled loop
};


/**
 * The DO i = x UNTIL cond instruction.
 */
class RexxInstructionControlledDoUntil : public RexxInstructionControlledDo
{
 public:
    inline RexxInstructionControlledDoUntil() { ; }
    inline RexxInstructionControlledDoUntil(RESTORETYPE restoreType) { ; };
           RexxInstructionControlledDoUntil(RexxString *l, ControlledLoop &c, WhileUntilLoop &w);

    virtual void live(size_t);
    virtual void liveGeneral(MarkReason reason);
    virtual void flatten(Envelope *);

    // Methods needed for loop iteration
    virtual bool iterate(RexxActivation *context, ExpressionStack *stack, DoBlock *doblock, bool first);

 protected:

    WhileUntilLoop whileLoop;   // handles the conditional part
};


/**
 * The DO i = x WHILE cond instruction.
 */
class RexxInstructionControlledDoWhile : public RexxInstructionControlledDoUntil
{
 public:
    inline RexxInstructionControlledDoWhile() { ; }
    inline RexxInstructionControlledDoWhile(RESTORETYPE restoreType) { ; };
           RexxInstructionControlledDoWhile(RexxString *l, ControlledLoop &c, WhileUntilLoop &w);

    virtual bool iterate(RexxActivation *context, ExpressionStack *stack, DoBlock *doblock, bool first);
};


/**
 * The DO WHILE loop.  Loops while a condition is true.
 */
class RexxInstructionDoWhile: public RexxInstructionBaseDo
{
 public:
    inline RexxInstructionDoWhile() { ; }
    inline RexxInstructionDoWhile(RESTORETYPE restoreType) { ; };
           RexxInstructionDoWhile(RexxString *l, WhileUntilLoop &w);

    virtual void live(size_t);
    virtual void liveGeneral(MarkReason reason);
    virtual void flatten(Envelope *);

    // Methods needed for loop iteration
    virtual bool iterate(RexxActivation *context, ExpressionStack *stack, DoBlock *doblock, bool first);

 protected:

    WhileUntilLoop whileLoop;                 // handles condition logic
};


/**
 * The DO UNTIL cond instruction.  Loops until a condition is
 * true
 */
class RexxInstructionDoUntil : public RexxInstructionDoWhile
{
 public:
    inline RexxInstructionDoUntil() { ; }
    inline RexxInstructionDoUntil(RESTORETYPE restoreType) { ; };
           RexxInstructionDoUntil(RexxString *l, WhileUntilLoop &w);

    virtual bool iterate(RexxActivation *context, ExpressionStack *stack, DoBlock *doblock, bool first);
};


/**
 * The DO COUNT instruction.  Just loops for a number of
 * iterations without setting a control variable.
 */
class RexxInstructionDoCount : public RexxInstructionBaseDo
{
 public:
    inline RexxInstructionDoCount() { ; }
    inline RexxInstructionDoCount(RESTORETYPE restoreType) { ; };
           RexxInstructionDoCount(RexxString *l, ForLoop &f);

    virtual void live(size_t);
    virtual void liveGeneral(MarkReason reason);
    virtual void flatten(Envelope *);

    // Methods needed for loop iteration
    virtual void setup(RexxActivation *context, ExpressionStack *stack, DoBlock *doblock);
    virtual bool iterate(RexxActivation *context, ExpressionStack *stack, DoBlock *doblock, bool first);

 protected:

    ForLoop forLoop;          // handles control logic for a DO count
};


/**
 * The DO count UNTIL cond instruction.
 */
class RexxInstructionDoCountUntil : public RexxInstructionDoCount
{
 public:
    inline RexxInstructionDoCountUntil() { ; }
    inline RexxInstructionDoCountUntil(RESTORETYPE restoreType) { ; };
           RexxInstructionDoCountUntil(RexxString *l, ForLoop &f, WhileUntilLoop &w);

    virtual void live(size_t);
    virtual void liveGeneral(MarkReason reason);
    virtual void flatten(Envelope *);

    // Methods needed for loop iteration
    virtual bool iterate(RexxActivation *context, ExpressionStack *stack, DoBlock *doblock, bool first);

 protected:

    WhileUntilLoop whileLoop;   // handles the conditional part
};


/**
 * The DO count WHILE cond instruction.  Takes a snap shot of an
 * object via makearray method then iterates over the array
 */
class RexxInstructionDoCountWhile : public RexxInstructionDoCountUntil
{
 public:
    inline RexxInstructionDoCountWhile() { ; }
    inline RexxInstructionDoCountWhile(RESTORETYPE restoreType) { ; };
           RexxInstructionDoCountWhile(RexxString *l, ForLoop &f, WhileUntilLoop &w);

    // Methods needed for loop iteration
    virtual bool iterate(RexxActivation *context, ExpressionStack *stack, DoBlock *doblock, bool first);
};
#endif

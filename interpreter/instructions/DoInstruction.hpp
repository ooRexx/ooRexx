/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2023 Rexx Language Association. All rights reserved.    */
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

class RexxBaseBlockInstruction : public RexxBlockInstruction
{
 public:
     inline RexxBaseBlockInstruction() {; }
     inline RexxBaseBlockInstruction(RESTORETYPE restoreType) {; };

     // methods required by RexxBlockInstruction;
     void terminate(RexxActivation *, DoBlock *)override;
     void matchEnd(RexxInstructionEnd *, LanguageParser *)override;
     void matchLabel(RexxInstructionEnd *_end, LanguageParser *parser);
};

/**
 * The simplest form of DO BLOCK.  This is a non-looping block.
 * It may have a label, but no other expressions to handle.
 */
class RexxInstructionSimpleDo : public RexxBaseBlockInstruction
{
 public:
     inline RexxInstructionSimpleDo() {; }
     inline RexxInstructionSimpleDo(RESTORETYPE restoreType) {; };
     RexxInstructionSimpleDo(RexxString *l);

     // required by RexxInstruction
     void execute(RexxActivation *, ExpressionStack *)override;

     // methods required by RexxBlockInstruction;
     bool isLoop()override { return false; }
     // most DO blocks are loops.  The simple styles will need to override.
     EndBlockType getEndStyle()override { return getLabel() == OREF_NULL ? DO_BLOCK : LABELED_DO_BLOCK; }
};


/**
 * The base class for a DO instruction.  This implements all of
 * the common END-matching behavior and label definitions.
 *
 */
class RexxInstructionBaseLoop : public RexxBaseBlockInstruction
{
 public:
     inline RexxInstructionBaseLoop() {;}

     void live(size_t)override;
     void liveGeneral(MarkReason reason)override;
     void flatten(Envelope *)override;

     // required by RexxInstruction.  For most subclasses, the default
     // is sufficient.
     void execute(RexxActivation *, ExpressionStack *)override;

     // most DO blocks are loops.  The simple styles will need to override.
     EndBlockType getEndStyle()override { return LOOP_BLOCK; }
     // Most DOs are loops...Simple DO will override again.
     bool isLoop()override { return true; };
     RexxVariableBase* getCountVariable()override { return countVariable; }

     // specific to Do loops.  Most subclasses can rely on the default
     virtual void reExecute(RexxActivation *, ExpressionStack *, DoBlock *);

     // most loops will want to override these two
     virtual void setup(RexxActivation *context, ExpressionStack *stack, DoBlock *doblock);
     virtual bool iterate(RexxActivation *context, ExpressionStack *stack, DoBlock *doblock, bool first);

     void endLoop(RexxActivation *context);
     inline void setCountVariable(RexxVariableBase *v) { countVariable = v; }

 protected:
     RexxVariableBase *countVariable;       // optional variable for a counter
};


/**
 * The DO FOREVER instruction.  Checks no state, it just loops
 * until terminated via other means.
 */
class RexxInstructionDoForever : public RexxInstructionBaseLoop
{
 public:
    inline RexxInstructionDoForever() { ; }
    inline RexxInstructionDoForever(RESTORETYPE restoreType) { ; };
           RexxInstructionDoForever(RexxString *l, RexxVariableBase *c);
};


/**
 * The DO OVER instruction.  Takes a snap shot of an object via
 * makearray method then iterates over the array
 */
class RexxInstructionDoOver : public RexxInstructionBaseLoop
{
 public:
    inline RexxInstructionDoOver() { ; }
    inline RexxInstructionDoOver(RESTORETYPE restoreType) { ; };
           RexxInstructionDoOver(RexxString *l, RexxVariableBase *c, OverLoop &o);

    void live(size_t) override;
    void liveGeneral(MarkReason reason) override;
    void flatten(Envelope *) override;

    // Methods needed for loop iteration
    void setup(RexxActivation *context, ExpressionStack *stack, DoBlock *doblock) override;
    bool iterate(RexxActivation *context, ExpressionStack *stack, DoBlock *doblock, bool first) override;

 protected:

    OverLoop overLoop;          // handles control logic for a DO OVER
};


/**
 * The DO OVER col FOR n instruction.  Takes a snap shot of an
 * object via makearray method then iterates over the array
 */
class RexxInstructionDoOverFor : public RexxInstructionDoOver
{
 public:
    inline RexxInstructionDoOverFor() { ; }
    inline RexxInstructionDoOverFor(RESTORETYPE restoreType) { ; };
           RexxInstructionDoOverFor(RexxString *l, RexxVariableBase *c, OverLoop &o, ForLoop &f);

    void live(size_t) override;
    void liveGeneral(MarkReason reason) override;
    void flatten(Envelope *) override;

    // Methods needed for loop iteration
    void setup(RexxActivation *context, ExpressionStack *stack, DoBlock *doblock) override;
    bool iterate(RexxActivation *context, ExpressionStack *stack, DoBlock *doblock, bool first) override;

 protected:

    ForLoop forLoop;          // handles control logic for the FOR portion
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
           RexxInstructionDoOverUntil(RexxString *l, RexxVariableBase *c, OverLoop &o, WhileUntilLoop &w);

    void live(size_t) override;
    void liveGeneral(MarkReason reason) override;
    void flatten(Envelope *) override;

    // Methods needed for loop iteration
    bool iterate(RexxActivation *context, ExpressionStack *stack, DoBlock *doblock, bool first) override;

protected:

    WhileUntilLoop whileLoop;   // handles the conditional part
};


/**
 * The DO OVER FOR n UNTIL cond instruction.  Takes a snap shot
 * of an object via makearray method then iterates over the
 * array
 */
class RexxInstructionDoOverForUntil : public RexxInstructionDoOverFor
{
 public:
    inline RexxInstructionDoOverForUntil() { ; }
    inline RexxInstructionDoOverForUntil(RESTORETYPE restoreType) { ; };
           RexxInstructionDoOverForUntil(RexxString *l, RexxVariableBase *c, OverLoop &o, ForLoop &f, WhileUntilLoop &w);

    void live(size_t) override;
    void liveGeneral(MarkReason reason) override;
    void flatten(Envelope *) override;

    // Methods needed for loop iteration
    bool iterate(RexxActivation *context, ExpressionStack *stack, DoBlock *doblock, bool first) override;

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
           RexxInstructionDoOverWhile(RexxString *l, RexxVariableBase *c, OverLoop &o, WhileUntilLoop &w);

    // Methods needed for loop iteration
    bool iterate(RexxActivation *context, ExpressionStack *stack, DoBlock *doblock, bool first) override;
};


/**
 * The DO OVER FOR n WHILE cond instruction.  Takes a snap shot
 * of an object via makearray method then iterates over the
 * array
 */
class RexxInstructionDoOverForWhile : public RexxInstructionDoOverForUntil
{
 public:
    inline RexxInstructionDoOverForWhile() { ; }
    inline RexxInstructionDoOverForWhile(RESTORETYPE restoreType) { ; };
           RexxInstructionDoOverForWhile(RexxString *l, RexxVariableBase *c, OverLoop &o, ForLoop &f, WhileUntilLoop &w);

    // Methods needed for loop iteration
    bool iterate(RexxActivation *context, ExpressionStack *stack, DoBlock *doblock, bool first) override;
};


/**
 * The controled DO instruction.  Sets an iteration variable on
 * each pass through the loop.
 */
class RexxInstructionControlledDo: public RexxInstructionBaseLoop
{
 public:
    inline RexxInstructionControlledDo() { ; }
    inline RexxInstructionControlledDo(RESTORETYPE restoreType) { ; };
           RexxInstructionControlledDo(RexxString *l, RexxVariableBase *cv, ControlledLoop &c);

    void live(size_t) override;
    void liveGeneral(MarkReason reason) override;
    void flatten(Envelope *) override;

    // Methods needed for loop iteration
    void setup(RexxActivation *context, ExpressionStack *stack, DoBlock *doblock) override;
    bool iterate(RexxActivation *context, ExpressionStack *stack, DoBlock *doblock, bool first) override;

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
           RexxInstructionControlledDoUntil(RexxString *l, RexxVariableBase *cv, ControlledLoop &c, WhileUntilLoop &w);

    void live(size_t) override;
    void liveGeneral(MarkReason reason) override;
    void flatten(Envelope *) override;

    // Methods needed for loop iteration
    bool iterate(RexxActivation *context, ExpressionStack *stack, DoBlock *doblock, bool first) override;

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
           RexxInstructionControlledDoWhile(RexxString *l, RexxVariableBase *cv, ControlledLoop &c, WhileUntilLoop &w);

    bool iterate(RexxActivation *context, ExpressionStack *stack, DoBlock *doblock, bool first) override;
};


/**
 * The DO WHILE loop.  Loops while a condition is true.
 */
class RexxInstructionDoWhile: public RexxInstructionBaseLoop
{
 public:
    inline RexxInstructionDoWhile() { ; }
    inline RexxInstructionDoWhile(RESTORETYPE restoreType) { ; };
           RexxInstructionDoWhile(RexxString *l, RexxVariableBase *c, WhileUntilLoop &w);

    void live(size_t) override;
    void liveGeneral(MarkReason reason) override;
    void flatten(Envelope *) override;

    // Methods needed for loop iteration
    bool iterate(RexxActivation *context, ExpressionStack *stack, DoBlock *doblock, bool first) override;

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
           RexxInstructionDoUntil(RexxString *l, RexxVariableBase *c, WhileUntilLoop &w);

    bool iterate(RexxActivation *context, ExpressionStack *stack, DoBlock *doblock, bool first) override;
};


/**
 * The DO COUNT instruction.  Just loops for a number of
 * iterations without setting a control variable.
 */
class RexxInstructionDoCount : public RexxInstructionBaseLoop
{
 public:
    inline RexxInstructionDoCount() { ; }
    inline RexxInstructionDoCount(RESTORETYPE restoreType) { ; };
           RexxInstructionDoCount(RexxString *l, RexxVariableBase *c, ForLoop &f);

    void live(size_t) override;
    void liveGeneral(MarkReason reason) override;
    void flatten(Envelope *) override;

    // Methods needed for loop iteration
    void setup(RexxActivation *context, ExpressionStack *stack, DoBlock *doblock) override;
    bool iterate(RexxActivation *context, ExpressionStack *stack, DoBlock *doblock, bool first) override;

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
           RexxInstructionDoCountUntil(RexxString *l, RexxVariableBase *c, ForLoop &f, WhileUntilLoop &w);

    void live(size_t) override;
    void liveGeneral(MarkReason reason) override;
    void flatten(Envelope *) override;

    // Methods needed for loop iteration
    bool iterate(RexxActivation *context, ExpressionStack *stack, DoBlock *doblock, bool first) override;

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
           RexxInstructionDoCountWhile(RexxString *l, RexxVariableBase *c, ForLoop &f, WhileUntilLoop &w);

    // Methods needed for loop iteration
    bool iterate(RexxActivation *context, ExpressionStack *stack, DoBlock *doblock, bool first) override;
};


/**
 * The DO WITH instruction.  Takes a snap shot of an object via
 * supplier method then iterates over the supplier
 */
class RexxInstructionDoWith : public RexxInstructionBaseLoop
{
 public:
    inline RexxInstructionDoWith() { ; }
    inline RexxInstructionDoWith(RESTORETYPE restoreType) { ; };
           RexxInstructionDoWith(RexxString *l, RexxVariableBase *c, WithLoop &o);

    void live(size_t) override;
    void liveGeneral(MarkReason reason) override;
    void flatten(Envelope *) override;

    // Methods needed for loop iteration
    void setup(RexxActivation *context, ExpressionStack *stack, DoBlock *doblock) override;
    bool iterate(RexxActivation *context, ExpressionStack *stack, DoBlock *doblock, bool first) override;

 protected:

    WithLoop withLoop;          // handles control logic for a DO WITH
};


/**
 * The DO WITH col FOR n instruction.  Takes a snap shot of an
 * object via makearray method then iterates over the array
 */
class RexxInstructionDoWithFor : public RexxInstructionDoWith
{
 public:
    inline RexxInstructionDoWithFor() { ; }
    inline RexxInstructionDoWithFor(RESTORETYPE restoreType) { ; };
           RexxInstructionDoWithFor(RexxString *l, RexxVariableBase *c, WithLoop &o, ForLoop &f);

    void live(size_t) override;
    void liveGeneral(MarkReason reason) override;
    void flatten(Envelope *) override;

    // Methods needed for loop iteration
    void setup(RexxActivation *context, ExpressionStack *stack, DoBlock *doblock) override;
    bool iterate(RexxActivation *context, ExpressionStack *stack, DoBlock *doblock, bool first) override;

 protected:

    ForLoop forLoop;          // handles control logic for the FOR portion
};


/**
 * The DO WITH UNTIL cond instruction.  Takes a snap shot of an
 * object via makearray method then iterates over the array
 */
class RexxInstructionDoWithUntil : public RexxInstructionDoWith
{
 public:
    inline RexxInstructionDoWithUntil() { ; }
    inline RexxInstructionDoWithUntil(RESTORETYPE restoreType) { ; };
           RexxInstructionDoWithUntil(RexxString *l, RexxVariableBase *c, WithLoop &o, WhileUntilLoop &w);

    void live(size_t) override;
    void liveGeneral(MarkReason reason) override;
    void flatten(Envelope *) override;

    // Methods needed for loop iteration
    bool iterate(RexxActivation *context, ExpressionStack *stack, DoBlock *doblock, bool first) override;

protected:

    WhileUntilLoop whileLoop;   // handles the conditional part
};


/**
 * The DO WITH FOR n UNTIL cond instruction.  Takes a snap shot
 * of an object via makearray method then iterates over the
 * array
 */
class RexxInstructionDoWithForUntil : public RexxInstructionDoWithFor
{
 public:
    inline RexxInstructionDoWithForUntil() { ; }
    inline RexxInstructionDoWithForUntil(RESTORETYPE restoreType) { ; };
           RexxInstructionDoWithForUntil(RexxString *l, RexxVariableBase *c, WithLoop &o, ForLoop &f, WhileUntilLoop &w);

    void live(size_t) override;
    void liveGeneral(MarkReason reason) override;
    void flatten(Envelope *) override;

    // Methods needed for loop iteration
    bool iterate(RexxActivation *context, ExpressionStack *stack, DoBlock *doblock, bool first) override;

protected:

    WhileUntilLoop whileLoop;   // handles the conditional part
};


/**
 * The DO WITH WHILE cond instruction.  Takes a snap shot of an
 * object via makearray method then iterates over the array
 */
class RexxInstructionDoWithWhile : public RexxInstructionDoWithUntil
{
 public:
    inline RexxInstructionDoWithWhile() { ; }
    inline RexxInstructionDoWithWhile(RESTORETYPE restoreType) { ; };
           RexxInstructionDoWithWhile(RexxString *l, RexxVariableBase *c, WithLoop &o, WhileUntilLoop &w);

    // Methods needed for loop iteration
    bool iterate(RexxActivation *context, ExpressionStack *stack, DoBlock *doblock, bool first) override;
};


/**
 * The DO WITH FOR n WHILE cond instruction.  Takes a snap shot
 * of an object via makearray method then iterates over the
 * array
 */
class RexxInstructionDoWithForWhile : public RexxInstructionDoWithForUntil
{
 public:
    inline RexxInstructionDoWithForWhile() { ; }
    inline RexxInstructionDoWithForWhile(RESTORETYPE restoreType) { ; };
           RexxInstructionDoWithForWhile(RexxString *l, RexxVariableBase *c, WithLoop &o, ForLoop &f, WhileUntilLoop &w);

    // Methods needed for loop iteration
    bool iterate(RexxActivation *context, ExpressionStack *stack, DoBlock *doblock, bool first) override;
};
#endif

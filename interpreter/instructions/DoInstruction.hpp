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
#include "EndInstruction.hpp"


/**
 * Identifier for different DO loop types.
 */
enum
{
    SIMPLE_DO,
    DO_COUNT,
    DO_FOREVER,
    DO_WHILE,
    DO_UNTIL,
    CONTROLLED_DO,
    CONTROLLED_WHILE,
    CONTROLLED_UNTIL,
    DO_OVER,
    DO_OVER_WHILE,
    DO_OVER_UNTIL,
    DO_COUNT_WHILE,
    DO_COUNT_UNTIL
} DoInstructionType;


/**
 * The base class for a DO instruction.  This implements all of
 * the common END-matching behavior and label definitions.
 *
 */
class RexxInstructionBaseDo : public RexxBlockInstruction
{
 public:

    inline void *operator new(size_t size, void *ptr) {return ptr;}
    inline void operator delete(void *) { }
    inline void operator delete(void *, void *) { }

    inline RexxInstructionBaseDo(void) { ; }
    inline RexxInstructionBaseDo(RESTORETYPE restoreType) { ; };

    virtual void live(size_t);
    virtual void liveGeneral(int reason);
    virtual void flatten(RexxEnvelope *);

    // methods required by RexxBlockInstruction;
    virtual void matchEnd(RexxInstructionEnd *, RexxSource *);
    // most DO blocks are loops.  The simple styles will need to override.
    virtual EndBlockType getEndStyle() { return LOOP_BLOCK; }
    // Most DOs are loops...Simple DO will override again.
    virtual bool isLoop() { return true };

    // specific to Do loops.
    virtual void reExecute(RexxActivation *, RexxExpressionStack *, RexxDoBlock *);
    virtual void terminate(RexxActivation *, RexxDoBlock *);

    void matchLabel(RexxInstructionEnd *end, RexxSource *source );
    void handleDebugPause(RexxActivation *context, RexxDoBlock *doblock);
    void endLoop(RexxActivation *context);
};


/**
 * The Ultimate DO loop instruction, capable of handling all of
 * the DO LOOP options.  We create one of these to hold
 * everything during parsing and once we determine this is one
 * of our simplier types, we create a version specialied to just
 * that type of instruction.  The specialialed versions save
 * both image size and execution cycle.
 */
class RexxInstructionDo : public RexxInstructionBaseDo
{
 public:

     inline void *operator new(size_t size, void *ptr) {return ptr;}
     inline void operator delete(void *) { }
     inline void operator delete(void *, void *) { }

     inline RexxInstructionDo(void) { ; }
     inline RexxInstructionDo(RESTORETYPE restoreType) { ; };

     virtual void live(size_t);
     virtual void liveGeneral(int reason);
     virtual void flatten(RexxEnvelope *);

     // required by RexxInstruction
     virtual void execute(RexxActivation *, RexxExpressionStack *);
     void reExecute(RexxActivation *, RexxExpressionStack *, RexxDoBlock *);

     // methods required by RexxBlockInstruction;
     virtual void matchEnd(RexxInstructionEnd *, RexxSource *);

     // required by loop instructions
     virtual void reExecute(RexxActivation *, RexxExpressionStack *, RexxDoBlock *);

     DoInstructionType type;              // the type of DO instruction

     ForLoop           forLoop;           // used for simple counting loops
     ControlledLoop    controlLoop;       // our information for a controlled loop
     OverLoop          overLoop;          // our information for a DO OVER
     WhileUntilLoop    whileLoop;         // information for WHILE or UNTIL
};


/**
 * The simplest form of DO BLOCK.  This is a non-looping block.
 * It may have a label, but no other expressions to handle.
 */
class RexxInstructionSimpleDo : public RexxInstructionBaseDo
{
 public:

    inline void *operator new(size_t size, void *ptr) {return ptr;}
    inline void operator delete(void *) { }
    inline void operator delete(void *, void *) { }

    inline RexxInstructionSimpleDo(void) { ; }
    inline RexxInstructionSimpleDo(RESTORETYPE restoreType) { ; };
           RexxInstructionSimpleDo(RexxInstructionDo *parent);

    // required by RexxInstruction
    virtual void execute(RexxActivation *, RexxExpressionStack *);

    // methods required by RexxBlockInstruction;
    virtual bool isLoop() { return false; }
};


/**
 * The DO FOREVER instruction.  Checks no state, it just loops
 * until terminated via other means.
 */
class RexxInstructionDoForever : public RexxInstructionBaseDo
{
 public:

    inline void *operator new(size_t size, void *ptr) {return ptr;}
    inline void operator delete(void *) { }
    inline void operator delete(void *, void *) { }

    inline RexxInstructionDoForever(void) { ; }
    inline RexxInstructionDoForever(RESTORETYPE restoreType) { ; };
           RexxInstructionDoForever(RexxInstructionDo *parent);

    // required by RexxInstruction
    virtual void execute(RexxActivation *, RexxExpressionStack *);

    // methods required by RexxBlockInstruction;
    virtual void reExecute(RexxActivation *, RexxExpressionStack *, RexxDoBlock *);
};
#endif

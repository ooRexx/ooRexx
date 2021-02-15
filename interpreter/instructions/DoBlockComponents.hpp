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
/* REXX Kernel                                          DoBlockComponent.hpp  */
/*                                                                            */
/* A set of helper classes for implementing different DO/LOOP instructions.   */
/* These allows different combinations of keyword types without having        */
/* to duplicate code across all of the instruction classes.                   */
/*                                                                            */
/******************************************************************************/
#ifndef Included_DoBlockComponent
#define Included_DoBlockComponent

#include "RexxCore.h"
#include "ExpressionBaseVariable.hpp"

class DoBlock;

typedef enum
{
    EXP_NONE,
    EXP_TO,
    EXP_BY,
    EXP_FOR
} ControlExpressionOrder;

/**
 * An embedded object that handles the details of a FOR count
 * loop expression.
 */
class ForLoop
{
 public:
     inline ForLoop() : forCount(OREF_NULL) { }

     // helper memory marking methods for embedding classes to call.
     inline void live(size_t liveMark)
     {
         memory_mark(forCount);
     }
     inline void liveGeneral(MarkReason reason)
     {
         memory_mark_general(forCount);
     }

     void setup(RexxActivation *context, ExpressionStack *stack, DoBlock *doblock, bool forKeyword);

    RexxInternalObject *forCount;          // number of iterations
};


/**
 * An embedded object that manages the details of controlled
 * loop execution (all keywords).
 */
class ControlledLoop : public ForLoop
{
 public:
     inline ControlledLoop(): control(OREF_NULL), initial(OREF_NULL), to(OREF_NULL), by(OREF_NULL), ForLoop()
     {
         for (size_t i = 0; i < 3; i++)
         {
             expressions[i] = EXP_NONE;
         }
     }

     // helper memory marking methods for embedding classes to call.
     inline void live(size_t liveMark)
     {
         memory_mark(forCount);
         memory_mark(control);
         memory_mark(initial);
         memory_mark(to);
         memory_mark(by);
     }
     inline void liveGeneral(MarkReason reason)
     {
         memory_mark_general(forCount);
         memory_mark_general(control);
         memory_mark_general(initial);
         memory_mark_general(to);
         memory_mark_general(by);
     }

     /**
      * An assignment override that ensures that we don't pick up any padding from the assigned instance.
      *
      * @param c      The assignment source.
      */
     inline void operator=(const ControlledLoop &c)
     {
         forCount = c.forCount;
         control = c.control;
         initial = c.initial;
         to = c.to;
         by = c.by;

         for (size_t i = 0; i < 3; i++)
         {
             expressions[i] = c.expressions[i];
         }
     }



    void setup(RexxActivation *context, ExpressionStack *stack, DoBlock *doblock);

    RexxVariableBase *control;           // control variable retriever
    RexxInternalObject *initial;         // initial control expression
    RexxInternalObject *to;              // final target value
    RexxInternalObject *by;              // control increment value
    uint8_t           expressions[3];    // controlled loop expression order
};


/**
 * An embedded object that manages the details of a
 * DO/LOOP OVER type.
 */
class OverLoop
{
 public:
     inline OverLoop() : control(OREF_NULL), target(OREF_NULL) { }
    // helper memory marking methods for embedding classes to call.
    inline void live(size_t liveMark)
    {
        memory_mark(control);
        memory_mark(target);
    }
    inline void liveGeneral(MarkReason reason)
    {
        memory_mark_general(control);
        memory_mark_general(target);
    }

    void setup(RexxActivation *context,
        ExpressionStack *stack, DoBlock *doblock);

    RexxVariableBase *control;           // control variable retriever
    RexxInternalObject *target;          // supplier for the array we do over
};


/**
 * An embedded object that can process WHILE or UNTIL
 * loop conditionals.
 */
class WhileUntilLoop
{
public:
    inline WhileUntilLoop() : conditional(OREF_NULL) { }

    // helper memory marking methods for embedding classes to call.
    inline void live(size_t liveMark)
    {
        memory_mark(conditional);
    }

    inline void liveGeneral(MarkReason reason)
    {
        memory_mark_general(conditional);
    }

    bool checkWhile(RexxActivation *context, ExpressionStack *stack);
    bool checkUntil(RexxActivation *context, ExpressionStack *stack);

    RexxInternalObject *conditional;      // a while or until condition
};


/**
 * An embedded object that can process WITH iteration loops.
 */
class WithLoop
{
public:
    inline WithLoop() : indexVar(OREF_NULL), itemVar(OREF_NULL), supplierSource(OREF_NULL) { }

    // helper memory marking methods for embedding classes to call.
    inline void live(size_t liveMark)
    {
        memory_mark(indexVar);
        memory_mark(itemVar);
        memory_mark(supplierSource);
    }

    inline void liveGeneral(MarkReason reason)
    {
        memory_mark_general(indexVar);
        memory_mark_general(itemVar);
        memory_mark_general(supplierSource);
    }

    void setup(RexxActivation *context, ExpressionStack *stack, DoBlock *doblock);
    bool checkIteration(RexxActivation *context, ExpressionStack *stack, DoBlock *doblock, bool first);

    RexxVariableBase *indexVar;     // variable assigned to supplier indexes
    RexxVariableBase *itemVar;      // variable assigned to supplier items
                                    // expression to evaluate for a supplier instance.
    RexxInternalObject *supplierSource;
};


#endif


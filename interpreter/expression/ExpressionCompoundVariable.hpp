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
/* REXX Kernel                               ExpressionCompoundVariable.hpp   */
/*                                                                            */
/* Primitive Expression Compound Variable Class Definitions                   */
/*                                                                            */
/******************************************************************************/
#ifndef Included_RexxCompoundVariable
#define Included_RexxCompoundVariable

#include "ExpressionBaseVariable.hpp"

class QueueClass;

/**
 * Expression object representing a compound variable.  This
 * handles all expression evaluation as well as all
 * variable operations.
 */
class RexxCompoundVariable : public RexxVariableBase
{
 public:
    void *operator new(size_t, size_t);
    inline void  operator delete(void *) { ; }

    RexxCompoundVariable(RexxString *, size_t, QueueClass *, size_t);
    inline RexxCompoundVariable(RESTORETYPE restoreType) { ; };

    void live(size_t) override;
    void liveGeneral(MarkReason reason) override;
    void flatten(Envelope *) override;

    // evaluation method from RexxInternalObject
    RexxObject *evaluate(RexxActivation *, ExpressionStack *) override;
    RexxObject *getValue(RexxActivation *context) override;
    RexxObject *getValue(VariableDictionary *context) override;
    RexxObject *getRealValue(VariableDictionary *) override;
    RexxObject *getRealValue(RexxActivation *) override;

    // RexxVariabeBase methods to perform variable operations
    bool exists(RexxActivation *) override;
    void set(RexxActivation *, RexxObject *)  override;
    void set(VariableDictionary *, RexxObject *) override;
    void assign(RexxActivation *, RexxObject *) override;
    void drop(RexxActivation *) override;
    void drop(VariableDictionary *) override;
    void setGuard(RexxActivation *) override;
    void clearGuard(RexxActivation *) override;
    void expose(RexxActivation *,  VariableDictionary *) override;
    void procedureExpose(RexxActivation *, RexxActivation *) override;

    // implemented only for compound variables.
    inline RexxString * variableStem() { return stemName; };

protected:

    RexxString *stemName;                // the stem variable name
    size_t      stemIndex;               // stem variable lookaside table index
    size_t      tailCount;               // count of tails.
    RexxInternalObject *tails[1];        // array of tail elements
};
#endif

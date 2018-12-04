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
/* REXX Kernel                                      ExpressionVariable.hpp    */
/*                                                                            */
/* Primitive Expression Variable Class Definitions                            */
/*                                                                            */
/******************************************************************************/
#ifndef Included_RexxSimpleVariable
#define Included_RexxSimpleVariable

#include "ExpressionBaseVariable.hpp"

class RexxSimpleVariable : public RexxVariableBase
{
 public:
    void *operator new(size_t);
    inline void  operator delete(void *) { ; }

    inline RexxSimpleVariable(RESTORETYPE restoreType) { ; };
    RexxSimpleVariable(RexxString *, size_t);

    virtual void live(size_t);
    virtual void liveGeneral(MarkReason reason);
    virtual void flatten(Envelope *);

    // RexxInternalObject evaluation methods
    virtual RexxObject *evaluate(RexxActivation *, ExpressionStack *);
    virtual RexxObject *getValue(VariableDictionary *);
    virtual RexxObject *getValue(RexxActivation *);
    virtual RexxObject *getRealValue(VariableDictionary *);
    virtual RexxObject *getRealValue(RexxActivation *);
    virtual VariableReference *getVariableReference(VariableDictionary *);
    virtual VariableReference *getVariableReference(RexxActivation *);

    // RexxVariableBase variable methods
    virtual bool exists(RexxActivation *);
    virtual void set(RexxActivation *, RexxObject *) ;
    virtual void set(VariableDictionary *, RexxObject *) ;
    virtual void assign(RexxActivation *, RexxObject *);
    virtual void drop(RexxActivation *);
    virtual void drop(VariableDictionary *);
    virtual void setGuard(RexxActivation *);
    virtual void clearGuard(RexxActivation *);
    virtual void setGuard(VariableDictionary *);
    virtual void clearGuard(VariableDictionary *);
    virtual void expose(RexxActivation *, VariableDictionary *);
    virtual void procedureExpose(RexxActivation *, RexxActivation *);
    virtual void alias(RexxActivation *, RexxVariable *);

    RexxString *getName();

protected:

    RexxString *variableName;            // name of the variable
    size_t      index;                   // lookaside table index
};
#endif

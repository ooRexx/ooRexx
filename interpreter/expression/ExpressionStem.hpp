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
/* REXX Kernel                                           ExpressionStem.hpp   */
/*                                                                            */
/* Primitive Expression Stem Class Definitions                                */
/*                                                                            */
/******************************************************************************/
#ifndef Included_RexxStemVariable
#define Included_RexxStemVariable

#include "ExpressionBaseVariable.hpp"

/**
 * A "retriever" class representing a Stem variable.  This
 * performs all of the normal operations expected from
 * an expression term or variable accessor.
 */
class RexxStemVariable : public RexxVariableBase
{
 public:
    void *operator new(size_t);
    inline void  operator delete(void *) { ; }

    inline RexxStemVariable(RESTORETYPE restoreType) { ; };
    RexxStemVariable(RexxString *, size_t);

    virtual void live(size_t);
    virtual void liveGeneral(MarkReason reason);
    virtual void flatten(Envelope *);

    // overrides of RexxInternalObject evaluaton methods
    virtual RexxObject *evaluate(RexxActivation *, ExpressionStack *);
    RexxObject *getValue(VariableDictionary *);
    RexxObject *getValue(RexxActivation *);
    RexxObject *getRealValue(VariableDictionary *);
    RexxObject *getRealValue(RexxActivation *);

    // overrides of RexxVariableBase methods
    virtual bool exists(RexxActivation *);
    virtual void set(RexxActivation *, RexxObject *) ;
    virtual void set(VariableDictionary *, RexxObject *) ;
    virtual void assign(RexxActivation *, RexxObject *);
    virtual void drop(RexxActivation *);
    virtual void drop(VariableDictionary *);
    virtual void setGuard(RexxActivation *);
    virtual void clearGuard(RexxActivation *);
    virtual void expose(RexxActivation *, VariableDictionary *);
    virtual void procedureExpose(RexxActivation *, RexxActivation *);

    // class-specific methods
    bool sort(RexxActivation *context, RexxString *prefix, int order, int type, size_t start, size_t end, size_t firstcol, size_t lastcol);
    inline size_t getIndex() {return stemIndex;};

 protected:

    RexxString  *stemName;                   // the stem variable name
    size_t       stemIndex;                  // lookaside table index
};
#endif

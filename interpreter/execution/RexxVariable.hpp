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
/* REXX Kernel                                           RexxVariable.hpp     */
/*                                                                            */
/* Primitive Variable Class Definition                                        */
/*                                                                            */
/******************************************************************************/
#ifndef Included_RexxVariable
#define Included_RexxVariable

#include "StringClass.hpp"
#include "ObjectClass.hpp"

class VariableReference;

class RexxVariable : public RexxInternalObject
{
 public:
    void *operator new(size_t);
    inline void  operator delete(void *) { }

    inline RexxVariable() : variableName(OREF_NULL), variableValue(OREF_NULL), creator(OREF_NULL), dependents(OREF_NULL) {;};
    inline RexxVariable(RexxString *n) : variableName(n), variableValue(OREF_NULL), creator(OREF_NULL), dependents(OREF_NULL) {;};
    inline RexxVariable(RESTORETYPE restoreType) { ; };

    void live(size_t) override;
    void liveGeneral(MarkReason reason) override;
    void flatten(Envelope *) override;

    void inform(Activity *);
    void drop();
    void notify();
    void uninform(Activity *);
    void setStem(RexxObject *);

    inline void set(RexxObject *value)
    {
        setField(variableValue, value);
        if (dependents != OREF_NULL)
        {
            notify();
        }
    };

    void setValue(RexxObject *value);

    inline void setCreator(RexxActivation *creatorActivation) { creator = creatorActivation; }
    inline bool isLocal() { return creator != OREF_NULL; }
           bool isAliasable();
    inline RexxObject *getVariableValue() { return variableValue; };
    inline RexxObject *getResolvedValue() { return variableValue != OREF_NULL ? variableValue : variableName; };
    inline RexxString *getName() { return variableName; }
    inline void setName(RexxString *name) { setField(variableName, name); }
    inline bool isDropped() { return variableValue == OREF_NULL; }

    // Note:  This does not use setField() since it will only occur with
    // local variables that can never be part of oldspace;
    inline RexxVariable *getNext() { return (RexxVariable *)variableValue; }
    inline bool isStem() { return variableName->endsWith('.'); }
    VariableReference *createReference();

protected:

    RexxString *variableName;            // the name of the variable
    RexxObject *variableValue;           // the assigned value of the variable.
    RexxActivation *creator;             // the activation that created this variable
    IdentityTable  *dependents;          // guard expression dependents
};


inline RexxVariable *new_variable(RexxString *n) { return new RexxVariable(n); }

#endif

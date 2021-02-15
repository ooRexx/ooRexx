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
/* REXX Kernel                                    ExpressionBaseVariable.h    */
/*                                                                            */
/* Polymorphic declarations for all translator variable parse classes         */
/*                                                                            */
/******************************************************************************/
#ifndef Included_RexxVariableBase
#define Included_RexxVariableBase

class RexxVariable;
class VariableReference;

/**
 * Base class definition for "variable" objects.  These
 * perform retrieval, assignment, etc. operations on
 * different types of variables, hiding the details of
 * the different types of variables.
 */
class RexxVariableBase : public RexxInternalObject
{
 public:
    RexxVariableBase() { ; };

    virtual bool exists(RexxActivation *) { return false; }
    virtual void set(RexxActivation *, RexxObject *) {;}
    virtual void set(VariableDictionary *, RexxObject *) {;}
    virtual void assign(RexxActivation *, RexxObject *) {;}
    virtual void drop(RexxActivation *) {;}
    virtual void drop(VariableDictionary *) {;}
    virtual void setGuard(RexxActivation *) {;}
    virtual void clearGuard(RexxActivation *) {;}
    virtual void setGuard(VariableDictionary *) {;}
    virtual void clearGuard(VariableDictionary *) {;}
    virtual void expose(RexxActivation *, VariableDictionary *) {;}
    virtual void procedureExpose(RexxActivation *, RexxActivation *) {;}
    virtual void alias(RexxActivation *, RexxVariable *) {;}
    virtual VariableReference *getVariableReference(RexxActivation *) { return OREF_NULL; }
    virtual VariableReference *getVariableReference(VariableDictionary *) { return OREF_NULL; }
};

#endif

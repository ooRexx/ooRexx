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
/* REXX Kernel                                     CompoundTableElement.hpp   */
/*                                                                            */
/* An element that is part of the balanced binary tree managed by a           */
/* CompoundVariableTable                                                      */
/*                                                                            */
/******************************************************************************/
#ifndef Included_CompoundTableElement
#define Included_CompoundTableElement

#include "RexxVariable.hpp"
#include "StringClass.hpp"

class CompoundTableElement : public RexxVariable
{
 friend class CompoundVariableTable;        // allow the compound table to access innards.

 public:
           void *operator new(size_t size);
    inline void  operator delete(void *) { ; }

    inline CompoundTableElement(RexxString *name) { variableName = name; }
    inline CompoundTableElement(RESTORETYPE restoreType) { ; };

    void live(size_t) override;
    void liveGeneral(MarkReason reason) override;
    void flatten(Envelope *) override;

    inline CompoundTableElement *realVariable() { return realElement != OREF_NULL ? realElement : this; }
    inline void setParent(CompoundTableElement *parentElement) { setField(parent, parentElement); }
    inline void setLeft(CompoundTableElement *leftChild) { setField(left, leftChild); }
    inline void setRight(CompoundTableElement *rightChild) { setField(right, rightChild); }

    inline bool isRightChild(CompoundTableElement *n)  { return right == n; }
    inline bool isLeftChild(CompoundTableElement *n)  { return left == n; }
    // hook up a node to a child based on last traversal position.
    inline void setChild(int rc, CompoundTableElement *child)
    {
        if (rc > 0)
        {
            setRight(child);
        }
        else
        {
            setLeft(child);
        }
    }
    inline void expose(CompoundTableElement *real) { setField(realElement, real); }
    inline RexxString *createCompoundName(RexxString *stemName) { return stemName->concat(getName()); }
    inline void setValue(RexxObject *value) { set(value); }

    static CompoundTableElement *newInstance(RexxString *name);

protected:

    CompoundTableElement *left;             // the left child
    CompoundTableElement *right;            // the right child
    CompoundTableElement *parent;           // the parent entry to this node
    unsigned short leftDepth;               // depth on the left side
    unsigned short rightDepth;              // depth on the right side
    CompoundTableElement *realElement;      // a potential expose indirection
};


inline CompoundTableElement *new_compoundElement(RexxString *s) { return new CompoundTableElement(s); }

#endif

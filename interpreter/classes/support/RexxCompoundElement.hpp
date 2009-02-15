/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2009 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* http://www.oorexx.org/license.html                          */
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
/* REXX Kernel                                     RexxCompoundElement.hpp    */
/*                                                                            */
/* Primitive CompoundVariable Class Definition                                */
/*                                                                            */
/******************************************************************************/
#ifndef Included_RexxCompoundElement
#define Included_RexxCompoundElement

#include "RexxVariable.hpp"
#include "StringClass.hpp"

class RexxCompoundElement : public RexxVariable {
 friend class RexxCompoundTable;        // allow the compound table to access innards.

 public:
  inline void *operator new(size_t size, void *ptr) { return ptr; };
  inline void  operator delete(void *) { ; }
  inline void  operator delete(void *, void *) { ; }

  inline RexxCompoundElement(RESTORETYPE restoreType) { ; };
  void         live(size_t);
  void         liveGeneral(int reason);
  void         flatten(RexxEnvelope *);

  inline RexxCompoundElement *realVariable() { return real_element != OREF_NULL ? real_element : this; }
  inline void setParent(RexxCompoundElement *parentElement) { OrefSet(this, this->parent, parentElement); }
  inline void setLeft(RexxCompoundElement *leftChild) { OrefSet(this, this->left, leftChild); }
  inline void setRight(RexxCompoundElement *rightChild) { OrefSet(this, this->right, rightChild); }
  inline void expose(RexxCompoundElement *real) { OrefSet(this, this->real_element, real); }
  inline RexxString *createCompoundName(RexxString *stemName) { return stemName->concat(getName()); }
  inline void setValue(RexxObject *value) { set(value); }

  static RexxCompoundElement *newInstance(RexxString *name);

protected:

  RexxCompoundElement *left;             /* the left child */
  RexxCompoundElement *right;            /* the right child */
  RexxCompoundElement *parent;           /* the parent entry to this node */
  unsigned short leftdepth;               /* depth on the left side */
  unsigned short rightdepth;              /* depth on the right side */
  RexxCompoundElement *real_element;      /* a potential expose indirection */
};


inline RexxCompoundElement *new_compoundElement(RexxString *s) { return RexxCompoundElement::newInstance(s); }

#endif

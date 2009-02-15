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
/* REXX Kernel                               ExpressionCompoundVariable.hpp   */
/*                                                                            */
/* Primitive Expression Compound Variable Class Definitions                   */
/*                                                                            */
/******************************************************************************/
#ifndef Included_RexxCompoundVariable
#define Included_RexxCompoundVariable

#include "ExpressionBaseVariable.hpp"

class RexxQueue;

/* Prototypes of external subroutine */

class RexxCompoundVariable : public RexxVariableBase {
 public:
  void *operator new(size_t, size_t);
  inline void *operator new(size_t size, void *ptr) {return ptr;};
  inline void  operator delete(void *) { ; }
  inline void  operator delete(void *, size_t) { ; }
  inline void  operator delete(void *, void *) { ; }

  RexxCompoundVariable(RexxString *, size_t, RexxQueue *, size_t);
  inline RexxCompoundVariable(RESTORETYPE restoreType) { ; };
  void live(size_t);
  void liveGeneral(int reason);
  void flatten(RexxEnvelope *);
  RexxObject *evaluate(RexxActivation *, RexxExpressionStack *);
  RexxObject *getValue(RexxActivation *context);
  RexxObject *getValue(RexxVariableDictionary *context);
  RexxObject *getRealValue(RexxVariableDictionary *);
  RexxObject *getRealValue(RexxActivation *);
  bool exists(RexxActivation *);
  void set(RexxActivation *, RexxObject *) ;
  void set(RexxVariableDictionary *, RexxObject *);
  void assign(RexxActivation *, RexxExpressionStack *, RexxObject *);
  void drop(RexxActivation *);
  void drop(RexxVariableDictionary *);
  void setGuard(RexxActivation *);
  void clearGuard(RexxActivation *);
  void expose(RexxActivation *, RexxExpressionStack *, RexxVariableDictionary *);
  void procedureExpose(RexxActivation *, RexxActivation *, RexxExpressionStack *);
  inline RexxString * variableStem() {return this->stemName;};

  RexxString *stemName;                // the stem variable name
  size_t      index;                   /* lookaside table index             */
  size_t      tailCount;               /* count of tails.                   */
  RexxObject *tails[1];                /* array of tail elements            */
};
#endif

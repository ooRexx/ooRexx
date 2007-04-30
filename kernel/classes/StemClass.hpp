/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2006 Rexx Language Association. All rights reserved.    */
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
/* REXX Kernel                                                  StemClass.hpp    */
/*                                                                            */
/* Primitive Stem Class Definitions                                           */
/*                                                                            */
/******************************************************************************/
#ifndef Included_RexxStem
#define Included_RexxStem

#include "RexxCompoundElement.hpp"
#include "RexxCompoundTable.hpp"
#include "RexxCompoundTail.hpp"
#include "ExpressionStem.hpp"

#define SORT_CASESENSITIVE 0
#define SORT_CASEIGNORE    1

#define SORT_ASCENDING 0
#define SORT_DECENDING 1

class RexxSupplier;

 class RexxStem : public RexxObject {
  public:
   void *operator new (size_t);
   inline void *operator new(size_t size, void *ptr) {return ptr;};
   RexxStem(RexxString *);
  inline RexxStem(RESTORETYPE restoreType) { ; };
  void live();
  void liveGeneral();
  void flatten(RexxEnvelope*);
  RexxObject * copy();

  long         longValue(size_t);
  long         longValueNoNOSTRING(size_t);
  RexxNumberString *numberString();
  double       doubleValue();
  double       doubleValueNoNOSTRING();
  RexxInteger *integerValue(size_t);
  RexxString  *stringValue();
  RexxArray   *makeArray();
  RexxArray   *allItems();
  RexxArray   *allIndexes();
  RexxSupplier *supplier();
  RexxObject  *request(RexxString *);
  RexxObject   *empty();
  RexxObject   *isEmpty();
  arraysize_t  items();

  void        dropValue();
  RexxObject *unknown (RexxString *, RexxArray *);
  RexxObject *bracket (RexxObject **, size_t);
  RexxObject *bracketEqual(RexxObject **, size_t);

  RexxObject *hasIndex(RexxObject **, size_t);
  RexxObject *remove(RexxObject **, size_t);
  RexxObject *hasItem(RexxObject *);
  RexxObject *index(RexxObject *);
  RexxObject *itemsRexx();
  RexxObject *removeItem(RexxObject *);


  RexxString *tail(RexxArray *, long);
  RexxObject *newRexx(RexxObject **, size_t);
  RexxObject *evaluateCompoundVariableValue(RexxActivation *context, RexxCompoundTail *resolved_tail);
  RexxObject *getCompoundVariableValue(RexxCompoundTail *resolved_tail);
  RexxObject *realCompoundVariableValue(RexxCompoundTail *resolved_tail);
  RexxCompoundElement *getCompoundVariable(RexxCompoundTail *name);
  RexxCompoundElement *exposeCompoundVariable(RexxCompoundTail *name);
  RexxCompoundElement *findCompoundVariable(RexxCompoundTail *name);
  RexxCompoundElement *findByValue(RexxObject *target);
  void        dropCompoundVariable(RexxCompoundTail *name);
  void        setCompoundVariable(RexxCompoundTail *name, RexxObject *value);
  void        setValue(RexxObject *value);
  RexxArray  *tailArray();
  RexxCompoundElement *nextVariable(RexxNativeActivation *activation);
  RexxObject *handleNovalue(RexxString *name, RexxActivation *context);
  void        expose(RexxCompoundElement *variable);
//  BOOL        sort(INT order, INT type, size_t start, size_t end, size_t firstcol, size_t lastcol);
  BOOL        sort(RexxString *prefix, INT order, INT type, size_t start, size_t end, size_t firstcol, size_t lastcol);

  inline BOOL compoundVariableExists(RexxCompoundTail *resolved_tail) { return realCompoundVariableValue(resolved_tail) != OREF_NULL; }
  inline RexxString *getName() { return u_name; }
  inline RexxCompoundElement *first() { return tails.first(); }
  inline RexxString *createCompoundName(RexxCompoundTail *tailPart) { return tailPart->createCompoundName(u_name); }


  RexxCompoundTable tails;            /* the table of compound tails */
  RexxObject *value;                  /* value of the stem                 */
  BOOL dropped;                       /* stem has no explicit value        */

 };
 #endif

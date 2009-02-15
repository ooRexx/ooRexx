/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2009 Rexx Language Association. All rights reserved.    */
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
/* REXX Kernel                                   RexxVariableDictionary.hpp   */
/*                                                                            */
/* Primitive Variable Dictionary Class Definition                             */
/*                                                                            */
/******************************************************************************/
#ifndef Included_RexxVariableDictionary
#define Included_RexxVariableDictionary

#include "RexxVariable.hpp"
#include "StemClass.hpp"
#include "RexxHashTable.hpp"

class RexxSupplier;

#define DEFAULT_OBJECT_DICTIONARY_SIZE 7

class RexxVariableDictionary : public RexxInternalObject {
 public:
  inline void  *operator new(size_t size, void *ptr) { return ptr; };
  inline void  operator delete(void *) { }
  inline void  operator delete(void *, void *) { }

  inline RexxVariableDictionary(RESTORETYPE restoreType) { ; };

  void         live(size_t);
  void         liveGeneral(int reason);
  void         flatten(RexxEnvelope *envelope);
  RexxObject  *copy();
  void         copyValues();

  RexxObject  *realValue(RexxString *name);
  void         add(RexxVariable *, RexxString *);
  void         put(RexxVariable *, RexxString *);
  inline RexxStem    *getStem(RexxString *stemName) { return (RexxStem *)getStemVariable(stemName)->getVariableValue(); }
  RexxVariable *createStemVariable(RexxString *stemName);
  RexxVariable *createVariable(RexxString *stemName);

  inline RexxVariable *resolveVariable(RexxString *name)
  {
      return (RexxVariable *)contents->stringGet(name);
  }

  inline RexxVariable *getVariable(RexxString *name)
    {
      RexxVariable *variable;              /* resolved variable item            */

      /* find the entry */
      variable = resolveVariable(name);
      if (variable == OREF_NULL) {         /* not in the table?                 */
          /* create a new one */
          variable = createVariable(name);
      }
      return variable;                     /* return the stem                   */
    }

  inline RexxVariable *getStemVariable(RexxString *stemName)
    {
      RexxVariable *variable;              /* resolved variable item            */

                                           /* find the stem entry               */
      variable = resolveVariable(stemName);
      if (variable == OREF_NULL) {         /* not in the table?                 */
          /* create a new one */
          variable = createStemVariable(stemName);
      }
      return variable;                     /* return the stem                   */
    }

  void setCompoundVariable(RexxString *stemName, RexxObject **tail, size_t tailCount, RexxObject *value);
  void dropCompoundVariable(RexxString *stemName, RexxObject **tail, size_t tailCount);
  RexxDirectory *getAllVariables();
  inline void remove(RexxString *n) { contents->remove(n); }

  RexxVariable *nextVariable(RexxNativeActivation *);
  void         set(RexxString *, RexxObject *);
  void         drop(RexxString *);
  void         dropStemVariable(RexxString *);
  void         reserve(RexxActivity *);
  void         release(RexxActivity *);
  bool         transfer(RexxActivity *);

  RexxCompoundElement *getCompoundVariable(RexxString *stemName, RexxObject **tail, size_t tailCount);
  RexxObject  *getCompoundVariableValue(RexxString *stemName, RexxObject **tail, size_t tailCount);
  RexxObject  *getCompoundVariableRealValue(RexxString *stem, RexxObject **tail, size_t tailCount);

  RexxObject  *realStemValue(RexxString *stemName);

  inline bool isScope(RexxObject *otherScope) { return this->scope == otherScope; }
  inline RexxVariableDictionary *getNextDictionary() { return next; }
  inline RexxActivity *getReservingActivity() { return reservingActivity; }

  void setNextDictionary(RexxVariableDictionary *next);

  static RexxVariableBase *getVariableRetriever(RexxString  *variable);
  static RexxVariableBase *getDirectVariableRetriever(RexxString  *variable);
  static RexxObject *buildCompoundVariable(RexxString * variable_name, bool direct);

  static RexxVariableDictionary *newInstance(size_t);
  static RexxVariableDictionary *newInstance(RexxObject *);

protected:

  RexxActivity  *reservingActivity;    /* current reserving activity        */
  RexxHashTable *contents;             /* vdict hashtable                   */
  RexxList *waitingActivities;         /* list of waiting activities        */
  unsigned short flags;                /* dictionary control flags          */
  unsigned short reserveCount;         /* number of times reserved          */
  RexxVariableDictionary *next;        /* chained object dictionary         */
  RexxObject *scope;                   /* scopy of this object dictionary   */
};


inline RexxVariableDictionary *new_variableDictionary(size_t s) { return RexxVariableDictionary::newInstance(s); }
inline RexxVariableDictionary *new_objectVariableDictionary(RexxObject *s) { return RexxVariableDictionary::newInstance(s); }
#endif

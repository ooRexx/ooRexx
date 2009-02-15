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
/* REXX Kernel                                           otplocalvariables.hpp*/
/*                                                                            */
/* Primitive Run time local variable cache                                    */
/*                                                                            */
/******************************************************************************/

#ifndef Included_RexxLocalVariables
#define Included_RexxLocalVariables

#include "RexxVariableDictionary.hpp"

#define VDICT_NOVALUE    0x0001u       /* novalue traps enabled             */
#define NESTED_INTERNAL  0x0002u       /* this is an internal call without procedure */
#define METHOD_CONTEXT   0x0004u       /* this is a method context */

#define VARIABLE_SELF    1             /* variable lookaside indices        */
#define VARIABLE_SUPER   2
#define VARIABLE_RESULT  3
#define VARIABLE_RC      4
#define VARIABLE_SIGL    5
#define FIRST_VARIABLE_INDEX 5         /* variable index list first slot    */

class RexxLocalVariables {
 public:
  inline void *operator new(size_t size, void *ptr) { return ptr;};
  RexxLocalVariables(RexxObject **frames, size_t items) { locals = (RexxVariable **)frames; size = items; }
  RexxLocalVariables() { locals = OREF_NULL; size = 0; }

  void live(size_t);
  void liveGeneral(int reason);
  void migrate(RexxActivity *);

  /* NOTE:  we add one because the size is actually the index */
  /* number of the last variable in the cache.   The zero-th */
  /* element is used to trigger cache lookup failures. */
  inline void init(RexxActivation *creator, size_t poolSize) { this->owner = creator; this->size = poolSize + 1; dictionary = OREF_NULL; flags = 0; }
  inline void setFrame(RexxObject **frame)
  {
      locals = (RexxVariable **)frame;
      memset(locals, 0, sizeof(RexxVariable *) * size);
      // NOTE:  We do NOT reset the variable dictionary.  For a new activation,
      // init() has already reset this.  If we're migrating to a new frame after a reply,
      // then we need to keep the old set of variables active.
  }

  RexxVariable *lookupVariable(RexxString *name, size_t index);

  RexxVariable *findVariable(RexxString *name, size_t index);
  RexxVariable *lookupStemVariable(RexxString *name, size_t index);

  void createDictionary();

  inline RexxVariableDictionary *getDictionary()
  {
      if (dictionary == OREF_NULL) {
          createDictionary();
      }
      return dictionary;
  }

  inline void putVariable(RexxVariable *variable, size_t index)
  {
      /* this may be a dynamic addition, so we might not know the */
      /* index. */
      if (index != 0) {
          locals[index] = variable;
          if (dictionary != OREF_NULL) {
              dictionary->put(variable, variable->getName());
          }
      }
      else {
          if (dictionary == OREF_NULL) {
              createDictionary();
          }
          dictionary->put(variable, variable->getName());
      }
  }

  void updateVariable(RexxVariable*);

  inline RexxVariable *get(size_t index) { return locals[index]; }
  inline RexxVariable *find(RexxString *name, size_t index)
  {
      RexxVariable *variable = get(index);
      if (variable == OREF_NULL) {
          variable = findVariable(name, index);
      }
      return variable;
  }

  inline void       setNovalueOn() { this->flags |= VDICT_NOVALUE; };
  inline void       setNovalueOff() { this->flags &= ~VDICT_NOVALUE; };
  inline bool       getNovalue() {return (this->flags & VDICT_NOVALUE) != 0; };
  inline void       setNested()  { flags |= NESTED_INTERNAL; }
  inline void       clearNested()  { flags &= ~NESTED_INTERNAL; }
  inline bool       isNested() { return (flags&NESTED_INTERNAL) != 0; }

  inline void       procedure(RexxActivation *activation) { this->owner = activation; dictionary = OREF_NULL;  flags &= ~NESTED_INTERNAL; }
  inline void       setDictionary(RexxVariableDictionary *dict) { dictionary = dict; }
  inline RexxVariableDictionary *getNestedDictionary() { return dictionary; }

  size_t flags;                        /* dictionary control flags          */
  size_t size;                         /* size of the expstack              */
  RexxActivation *owner;               /* the owning activation             */
  RexxVariable **locals;               /* the frame of local variables      */
  RexxVariableDictionary *dictionary;  /* dictionary used for dynamic lookups */
};
#endif

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
/* REXX Kernel                                       IdentityTableClass.hpp   */
/*                                                                            */
/* Primitive IdentityTable Collection Class Definition                        */
/*                                                                            */
/******************************************************************************/
#ifndef Included_RexxIdentityTable
#define Included_RexxIdentityTable

#include "TableClass.hpp"

class RexxIdentityTable : public RexxTable {
  public:
   void * operator new(size_t size, void *objectPtr) { return objectPtr; };
                                       /* So it doesn't need to do anythin*/
   inline RexxIdentityTable(RESTORETYPE restoreType) { ; };

   virtual RexxObject *remove(RexxObject *key);
   virtual RexxObject *get(RexxObject *key);
   virtual RexxObject *put(RexxObject *, RexxObject *);
   virtual RexxObject *add(RexxObject *, RexxObject *);
   virtual RexxObject *removeItem(RexxObject *value);
   virtual RexxObject *hasItem(RexxObject *targetIndex);
   virtual RexxObject *getIndex(RexxObject * value);

   inline RexxObject  *hasItem(RexxObject *newValue, RexxObject *targetIndex) {return this->contents->primitiveHasItem(newValue, targetIndex); };
   inline RexxArray   *allAt(RexxObject *key) {return this->contents->primitiveGetAll(key);}
   inline RexxObject  *findSuperScope(RexxObject *v) { return this->contents->primitiveNextItem(v, TheNilObject); };

   RexxObject *newRexx(RexxObject **, size_t);
   static void createInstance();
   static RexxIdentityTable *newInstance(size_t size);
   static RexxClass *classInstance;
};


inline RexxIdentityTable *new_identity_table() { return RexxIdentityTable::newInstance(RexxHashTable::DEFAULT_HASH_SIZE); }

#endif


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
/* REXX Kernel                                            DirectoryClass.hpp  */
/*                                                                            */
/* Primitive Directory Class Definitions                                      */
/*                                                                            */
/******************************************************************************/
#ifndef Included_RexxDirectory
#define Included_RexxDirectory

#include "RexxCollection.hpp"

class RexxDirectory : public RexxHashTableCollection {
 public:
  inline void * operator new(size_t size, void *objectPtr) { return objectPtr; };
  inline RexxDirectory(RESTORETYPE restoreType) { ; };

  void          live(size_t);
  void          liveGeneral(int reason);
  void          flatten(RexxEnvelope *);
  RexxObject   *unflatten(RexxEnvelope *);
  RexxObject   *copy();
  RexxArray    *makeArray();

  RexxArray    *requestArray();
  RexxObject   *mergeItem(RexxObject *, RexxObject *);
  RexxObject   *at(RexxString *);
  RexxObject   *fastAt(RexxString *name) { return this->contents->stringGet(name);}
  RexxObject   *atRexx(RexxString *);
  RexxObject   *put(RexxObject *, RexxString *);
  RexxObject   *entry(RexxString *);
  RexxObject   *entryRexx(RexxString *);
  RexxObject   *hasEntry(RexxString *);
  RexxObject   *hasIndex(RexxString *);
  size_t        items();
  RexxObject   *itemsRexx();
  RexxObject   *remove(RexxString *);
  RexxObject   *removeRexx(RexxString *);
  RexxObject   *setEntry(RexxString *, RexxObject *);
  RexxObject   *setMethod(RexxString *, RexxMethod *);
  RexxObject   *unknown(RexxString *, RexxArray *);
  RexxSupplier *supplier();
  RexxArray    *allItems();
  RexxArray    *allIndexes();
  void          reset();
  RexxObject   *empty();
  RexxObject   *isEmpty();
  RexxObject   *indexRexx(RexxObject *);
  RexxObject   *hasItem(RexxObject *);
  RexxObject   *removeItem(RexxObject *);

  RexxObject   *newRexx(RexxObject **init_args, size_t);

  RexxTable  *method_table;            /* table of added methods            */
  RexxMethod *unknown_method;          /* unknown method entry              */

  static RexxDirectory *newInstance();

  static void createInstance();
  // singleton class instance;
  static RexxClass *classInstance;
};


inline RexxDirectory *new_directory() { return RexxDirectory::newInstance(); }

#endif

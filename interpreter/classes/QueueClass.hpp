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
/* REXX Kernel                                               QueueClass.hpp   */
/*                                                                            */
/* Primitive Queue Class Definitions                                          */
/*                                                                            */
/******************************************************************************/
#ifndef Included_RexxQueue
#define Included_RexxQueue

#include "ListClass.hpp"

class RexxQueue : public RexxList {
 public:

 void *operator new(size_t);
 inline void *operator new(size_t size, void *ptr) {return ptr;};
 inline RexxQueue() {;};
 inline RexxQueue(RESTORETYPE restoreType) { ; };
 RexxObject *pullRexx();
 RexxObject *pushRexx(RexxObject *);
 RexxObject *queueRexx(RexxObject *);
 LISTENTRY *locateEntry(RexxObject *, RexxObject *);
 RexxObject *put(RexxObject *, RexxObject *);
 RexxObject *at(RexxObject *);
 RexxObject *remove(RexxObject *);
 RexxObject *hasindex(RexxObject *);
 RexxObject *peek();
 RexxObject *supplier();
 RexxObject *newRexx(RexxObject **, size_t);
 RexxQueue  *ofRexx(RexxObject **, size_t);
 RexxObject *append(RexxObject *);
 RexxArray  *allIndexes();
 RexxObject *index(RexxObject *);
 RexxObject *firstRexx();
 RexxObject *lastRexx();
 RexxObject *next(RexxObject *);
 RexxObject *previous(RexxObject *);
 size_t      entryToIndex(size_t target);
 RexxObject *insert(RexxObject *, RexxObject *);

 inline RexxObject *pop() { return this->removeFirst();};
 inline void push(RexxObject *obj) { this->addFirst(obj);};
 inline void queue(RexxObject *obj) { this->addLast(obj);};

 static void createInstance();
 static RexxClass *classInstance;

};

inline RexxQueue *new_queue() { return new RexxQueue; }
#endif

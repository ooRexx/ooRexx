/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2024 Rexx Language Association. All rights reserved.    */
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
/* REXX Kernel                                               QueueClass.hpp   */
/*                                                                            */
/* Primitive Queue Class Definitions                                          */
/*                                                                            */
/******************************************************************************/
#ifndef Included_QueueClass
#define Included_QueueClass

#include "ArrayClass.hpp"

class QueueClass : public ArrayClass
{
 public:

     void * operator new(size_t, size_t = DefaultArraySize, size_t = DefaultArraySize);

     inline QueueClass() {;};
     inline QueueClass(RESTORETYPE restoreType) { ; };

     RexxString *primitiveMakeString() override;
     RexxString *makeString() override;
     bool validateIndex(RexxObject **index, size_t indexCount, size_t argPosition, size_t boundsError, size_t &position) override;
     void checkInsertIndex(size_t position) override;

     RexxObject *pullRexx();
     RexxObject *pushRexx(RexxObject *item);
     RexxObject *queueRexx(RexxObject *item);
     RexxInternalObject *peek();
     RexxObject *putRexx(RexxObject *value, RexxObject *index);
     RexxObject *deleteRexx(RexxObject *index);
     RexxObject *initRexx(RexxObject *initialSize);

     inline RexxInternalObject *pop() { return deleteItem(1);}
     inline RexxInternalObject *pull() { return deleteItem(1);}
     inline void push(RexxInternalObject *obj) { addFirst(obj); }
     inline void queue(RexxInternalObject *obj) { addLast(obj); }
     RexxInternalObject *remove(size_t index) override;

     RexxObject *newRexx(RexxObject **init_args, size_t argCount);
     QueueClass *ofRexx(RexxObject **args, size_t argCount);

     static void createInstance();
     static RexxClass *classInstance;
};

inline QueueClass *new_queue() { return new(ArrayClass::DefaultArraySize) QueueClass; }
inline QueueClass *new_queue(size_t s) { return new(s) QueueClass; }
#endif

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
/* REXX Kernel                                           ExpressionStack.hpp  */
/*                                                                            */
/* Primitive Expression Stack Class Definitions                               */
/*                                                                            */
/******************************************************************************/

#ifndef Included_RexxExpressionStack
#define Included_RexxExpressionStack

#include "ArrayClass.hpp"

class ProtectedObject;
class RexxActivity;

class RexxExpressionStack {
 public:

  inline void *operator new(size_t size, void *ptr) { return ptr;};
  RexxExpressionStack(RexxObject **frames, size_t items) { stack = frames; size = items; top = stack; }
  RexxExpressionStack() { stack = OREF_NULL; size = 0; top = stack; }
  void live(size_t);
  void liveGeneral(int reason);
  void flatten(RexxEnvelope *);
  void         expandArgs(size_t, size_t, size_t, const char *);
  RexxString * requiredStringArg(size_t);
  RexxString * optionalStringArg(size_t);
  RexxInteger *requiredIntegerArg(size_t, size_t, const char *);
  RexxInteger *optionalIntegerArg(size_t, size_t, const char *);
  void         migrate(RexxActivity *);

  inline void setFrame(RexxObject **frames, size_t items) { stack = frames; size = items; top = stack; *top = OREF_NULL; }

  inline void send(RexxString *message, RexxObject *scope, size_t count, ProtectedObject &result) {
                 (*(this->top - count))->messageSend(message, (RexxObject **)(this->top - count + 1), count, scope, result); };
  inline void send(RexxString *message, size_t count, ProtectedObject &result) {
                 (*(this->top - count))->messageSend(message, (RexxObject **)(this->top -count + 1), count, result); };
  inline void         push(RexxObject *value) { *(++this->top) = value; };
  inline RexxObject * pop() { return *(this->top--); };
  inline RexxObject * fastPop() { return *(this->top--); };
  inline RexxArray  * argumentArray(size_t count) { return (new (count, (RexxObject **)(this->top - (count - 1))) RexxArray); };
  inline RexxObject **arguments(size_t count) { return (RexxObject **)(this->top - (count - 1)); };
  inline void         replace(size_t offset, RexxObject *value) { *(this->top - offset) = value; };
  inline size_t       getSize() {return this->size;};
  inline RexxObject * getTop()  {return *(this->top);};
  inline void         operatorResult(RexxObject *value) { *(--this->top) = value; };
  inline void         prefixResult(RexxObject *value)   { *(this->top) = value; };
  inline void         popn(size_t c) {this->top -= c;};
  inline void         clear() {this->top = this->stack;};
  inline RexxObject * peek(size_t v) {return *(this->top - v);};
  inline RexxObject **pointer(size_t v) {return (this->top - v); };
  inline size_t       location() {return this->top - this->stack;};
  inline void         setTop(size_t v) {this->top = this->stack + v;};
  inline void         toss() { this->top--; };
  inline RexxObject **getFrame() { return stack; }

protected:

  size_t size;                         /* size of the expstack              */
  RexxObject **top;                    /* current expstack top location     */
  RexxObject **stack;                  /* actual stack values               */
};
#endif

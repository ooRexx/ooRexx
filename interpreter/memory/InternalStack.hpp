/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2019 Rexx Language Association. All rights reserved.    */
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
/* REXX Kernel                                                                */
/*                                                                            */
/* Primitive Expression Stack Class Definitions                               */
/*                                                                            */
/******************************************************************************/

#ifndef Included_InternalStack
#define Included_InternalStack

class InternalStack : public RexxInternalObject
{
 public:
           void *operator new(size_t size, size_t stackSize);
    inline void  operator delete(void *) { ; }

    InternalStack() { ; }
    InternalStack(size_t stackSize);
    inline InternalStack(RESTORETYPE restoreType) { ; }

    void live(size_t) override;
    void liveGeneral(MarkReason reason) override;

    inline void         push(RexxInternalObject *value) { *(++top) = value; };
    inline RexxInternalObject *pop() { return *(top--); };
    inline void         replace(size_t offset, RexxInternalObject *value) { *(top - offset) = value; };
    inline size_t       getSize() {return size;};
    inline RexxInternalObject *getTop()  {return *(top);};
    inline void         popn(size_t c) {top -= c;};
    inline void         clear() {top = stack;};
    inline RexxInternalObject * peek(size_t v) {return *(top - v);};
    inline RexxInternalObject **pointer(size_t v) {return (top - v); };
    inline size_t       location() {return top - stack;};
    inline void         setTop(size_t v) {top = stack + v;};
    inline void         toss() { top--; };
    inline bool         isEmpty() { return top == stack; }
    inline bool         isFull() { return top >= stack + size; }

protected:

    size_t size;                         // size of the expression stack
    RexxInternalObject **top;            // current expression stack top position
    RexxInternalObject *stack[1];        // start of actual stack values
};


inline InternalStack *new_internalstack(size_t s) { return new (s) InternalStack(s); }

#endif

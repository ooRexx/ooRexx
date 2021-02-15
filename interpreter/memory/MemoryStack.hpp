/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2014 Rexx Language Association. All rights reserved.    */
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
/* REXX Kernel                                             MemoryStack.hpp    */
/*                                                                            */
/* Primitive Stack Class Definitions                                          */
/*                                                                            */
/******************************************************************************/
#ifndef Included_MemoryStack
#define Included_MemoryStack

#include <string.h>

/**
 * A simple and fast stack object for doing memory management
 * live marking.  This has minimal overrun protections, so
 * the using code needs to perform appropriate "stack is full"
 * checks.
 */
class LiveStack
{
 public:
    void        *operator new(size_t, size_t);
    void         operator delete(void *);

    LiveStack(size_t size);

    // the position is origin zero, relative to the top, which is an empty slot.  So, position 0
    // is the top element, 1 is the penultimate elements, etc.
    inline RexxInternalObject *get(size_t pos)
    {
        // we only return something if within the bounds
        if (pos < top)
        {
            return stack[top - (pos + 1)];
        }
        else
        {
            return OREF_NULL;
        }
    }


    /**
     * Push an object on to the stack
     *
     * @param obj    The object to push
     *
     * @return
     */
    inline void push(RexxInternalObject *obj)
    {
        // we have no overrun protection here.  The using piece either
        // needs to accurately predict how large the stack needs to be or
        // it needs to check if things are full before pushing.
        stack[top++] = obj;
    }


    /**
     * Pop an item off of the stack.  Has some underrun protection,
     * but that's it.
     *
     * @return The popped object (or OREF_NULL if we're empty)
     */
    RexxInternalObject *pop()
    {
        // protect from an underrun
        if (top == 0)
        {
            return OREF_NULL;
        }
        // decrement the top and return the pointer
        return stack[--top];
    }

    LiveStack  *reallocate(size_t increment);
    LiveStack  *ensureSpace(size_t needed);

    inline bool        checkRoom() { return top < size; }
    inline bool        checkRoom(size_t needed) { return size - top > needed; }
    inline size_t      stackSize() { return size; };
    inline RexxInternalObject *stackTop() { return top == 0 ? OREF_NULL : stack[top - 1]; };
    inline void        copyEntries(LiveStack *other) { memcpy((char *)stack, (char *)other->stack, other->size * sizeof(RexxInternalObject *)); top = other->top; }
    inline void        clear() { memset(stack, 0, sizeof(RexxInternalObject*) * size); }

 protected:

    size_t   size;                      // the stack size
    size_t   top;                       // the next position we push on the stack
    RexxInternalObject *stack[1];       // the stack entries
};


/**
 * A wrap-around marking stack.  This stack can hold n
 * elements, and if more than n elements are pushed on the
 * stack, the oldest element is removed.
 */
class PushThroughStack : public RexxInternalObject
{
 public:
    void        *operator new(size_t, size_t);
    void        *operator new(size_t, size_t, bool temporary);
    inline void  operator delete(void *) { };

    inline PushThroughStack(RESTORETYPE restoreType) { ; };
    PushThroughStack(size_t size);

    void init(size_t);

    void live(size_t) override;
    void liveGeneral(MarkReason reason) override;

    // the position is origin zero, relative to the current.  Current
    // is the position of the last item pushed on to the stack.
    inline RexxInternalObject *get(size_t pos)
    {
        // if they a really searching back, reduce it modulo size.
        pos = pos % size;
        // Since we wrap, the position within the bounds
        if (pos > current)
        {
            // this is relative to the top of the stack
            // if stack
            return stack[size + current - pos];
        }
        else
        {
            // just pos items back from current
            return stack[current - pos];
        }
    }


    /**
     * Push an object on to the stack
     *
     * @param obj    The object to push
     *
     * @return
     */
    inline void push(RexxInternalObject *obj)
    {
        // this will wrap, as necessary, wiping out the reference at the bottom of the stack
        incrementCurrent();
        stack[current] = obj;
    }


    /**
     * Pop an item off of the stack.  Has some underrun protection,
     * but that's it.
     *
     * @return The popped object (or OREF_NULL if we're empty)
     */
    RexxInternalObject *pop()
    {
        // get the current referenced item
        RexxInternalObject *obj = stack[current];
        // because we make everything in the stack and this is used for GC
        // protection, null out the removed entry
        stack[current] = OREF_NULL;
        // now move back an item, with wrap protection.
        decrementCurrent();
        return obj;
    }

    LiveStack  *reallocate(size_t increment);

    inline size_t      stackSize() { return size; };
    inline RexxInternalObject *stackTop() { return stack[current]; };

    // increment and decrement will wrap
    inline void        decrementCurrent() { current = (current == 0) ? size - 1 : current - 1; }
    inline void        incrementCurrent() { if (++current >= size) current = 0; }

    inline void        copyEntries(PushThroughStack *other) { memcpy((char *)stack, other->stack, other->size * sizeof(RexxInternalObject *)); current = other->current; }
    inline void        clear() { memset(stack, 0, sizeof(RexxInternalObject*) * size); }
           void        extend(size_t);
           void        remove(RexxInternalObject *, bool search = false);

 protected:

    size_t   size;                      // the stack size
    size_t   current;                   // the last object on the stack (starts as if OREF_NULL had been written first)
    RexxInternalObject *stack[1];       // the stack entries
};

#endif

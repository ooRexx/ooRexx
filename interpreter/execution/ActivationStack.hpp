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
/* REXX Kernel                                                                */
/*                                                                            */
/* Primitive Activation Stack Frame Definitions                               */
/*                                                                            */
/******************************************************************************/

#ifndef Included_ActivationStack
#define Included_ActivationStack


/**
 * A frame buffer used to as backing for an activation
 * expression stack and variable frame.
 */
class ActivationFrameBuffer : public RexxInternalObject
{
    public:
            void *operator new(size_t size, size_t entries);
     inline void  operator delete(void *) { ; }

     ActivationFrameBuffer() { ; }
     ActivationFrameBuffer(size_t entries);
     inline ActivationFrameBuffer(RESTORETYPE restoreType) { ; }

     void live(size_t) override;
     void liveGeneral(MarkReason reason) override;

     inline bool hasCapacity(size_t entries) { return size - next >= entries; }
     inline RexxInternalObject **allocateFrame(size_t entries)
     {
         RexxInternalObject **frame = &buffer[next];
         next += entries;
         return frame;
     }

     inline bool contains(RexxInternalObject **frame)
     {
         return frame >= &buffer[0] && frame <= &buffer[size];
     }

     inline void releaseFrame(RexxInternalObject **frame)
     {
         next = frame - &buffer[0];
     }

     inline void push(ActivationFrameBuffer *p)
     {
         previous = p;    // chain this up
     }


     inline void reset() { next = 0; }    // reset a cached frame buffer

     inline ActivationFrameBuffer *getPrevious() { return previous; }

protected:

     size_t size;                        // size of the buffer (in slots)
     size_t next;                        // location of next allocation
     ActivationFrameBuffer *previous;    // previous entry in the stack
     RexxInternalObject *buffer[1];      // start of the buffer location
};


/**
 * Special stack for managing the stack frames used by the
 * activations running on an Activity.
 */
class ActivationStack
{
 public:

    enum { DefaultFrameBufferSize = 2048 };
    ActivationStack() { ; }

    void live(size_t);
    void liveGeneral(MarkReason reason);

    void init();
    void expandCapacity(size_t entries);

    inline void ensureCapacity(size_t entries) { if (!current->hasCapacity(entries)) { expandCapacity(entries); } }
    inline RexxInternalObject **allocateFrame(size_t entries)
    {
        // make sure we have space first
        ensureCapacity(entries);
        // now allocate from the current stack buffer
        return current->allocateFrame(entries);
    }
    void releaseFrame(RexxInternalObject **frame)
    {
        // we may be popping back one or more buffers.  We deactivate the newer ones
        while (!current->contains(frame))
        {
            // we need to pop at least one buffer off of the stack
            ActivationFrameBuffer *released = current;
            current = released->getPrevious();
            // we'll keep at least one buffer around for reuse.  If
            // we've already got one in the cache, just let this one
            // get GCed.
            if (unused == OREF_NULL)
            {
                unused = released;
                unused->reset();   // reset to clean state
            }
        }

        // now back this up to the release point
        current->releaseFrame(frame);
    }

protected:

    ActivationFrameBuffer *current;     // our current frame buffer
    ActivationFrameBuffer *unused;      // our cached unused buffer(s)
};


inline ActivationFrameBuffer *new_activationFrameBuffer(size_t s) { return new (s) ActivationFrameBuffer(s); }

#endif

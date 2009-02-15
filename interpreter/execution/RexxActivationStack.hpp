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
/* REXX Kernel                                       RexxActivationStack.hpp  */
/*                                                                            */
/* Primitive Activation Stack Frame Definitions                               */
/*                                                                            */
/******************************************************************************/

#ifndef Included_RexxActivationStack
#define Included_RexxActivationStack


class RexxActivationFrameBuffer : public RexxInternalObject {
    public:
     inline void *operator new(size_t size, void *ptr) { return ptr;};
     inline void  operator delete(void *) { ; }
     inline void  operator delete(void *, void *) { ; }

     RexxActivationFrameBuffer() { ; }
     inline RexxActivationFrameBuffer(RESTORETYPE restoreType) { ; }
     void live(size_t);
     void liveGeneral(int reason);
     void flatten(RexxEnvelope *);

     inline bool hasCapacity(size_t entries) { return size - next >= entries; }
     inline RexxObject **allocateFrame(size_t entries)
     {
         RexxObject **frame = &buffer[next];
         next += entries;
         return frame;
     }

     inline bool contains(RexxObject **frame)
     {
         return frame >= &buffer[0] && frame <= &buffer[size];
     }

     inline void releaseFrame(RexxObject **frame)
     {
         next = frame - &buffer[0];
     }

     inline void push(RexxActivationFrameBuffer *p)
     {
         previous = p;    // chain this up
     }


     inline void reset() { next = 0; }    // reset a cached frame buffer

     inline RexxActivationFrameBuffer *getPrevious() { return previous; }

     static RexxActivationFrameBuffer *newInstance(size_t);

protected:


     size_t size;                        /* size of the buffer (in slots) */
     size_t next;                        /* location of next allocation */
     RexxActivationFrameBuffer *previous;/* previous entry in the stack */
     RexxObject *buffer[1];              /* start of the buffer location */
};


class RexxActivationStack {
 public:

  enum { DefaultFrameBufferSize = 2048 };

  inline void *operator new(size_t size, void *ptr) { return ptr;};
  RexxActivationStack() { ; }
  void live(size_t);
  void liveGeneral(int reason);

  void init();
  void expandCapacity(size_t entries);

  inline void ensureCapacity(size_t entries) { if (!current->hasCapacity(entries)) { expandCapacity(entries); } }
  inline RexxObject **allocateFrame(size_t entries)
  {
      /* make sure we have space first */
      ensureCapacity(entries);
      /* now allocate from the current stack buffer */
      return current->allocateFrame(entries);
  }
  void releaseFrame(RexxObject **frame)
    {
        /* we may be popping back one or more buffers.  We deactivate */
        /* the newer ones */
        while (!current->contains(frame)) {
            /* we need to pop at least one buffer off of the stack */
            RexxActivationFrameBuffer *released = current;
            current = released->getPrevious();
            /* we'll keep at least one buffer around for reuse.  If */
            /* we've already got one in the cache, just let this one */
            /* get GCed. */
            if (unused == OREF_NULL) {
                unused = released;
                unused->reset();   // reset to clean state
            }
        }

        /* now back this up to the release point */
        current->releaseFrame(frame);
    }

protected:

  RexxActivationFrameBuffer *current;
  RexxActivationFrameBuffer *unused;
};


inline RexxActivationFrameBuffer *new_activationFrameBuffer(size_t s) { return RexxActivationFrameBuffer::newInstance(s); }

#endif

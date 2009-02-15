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
/* REXX Kernel                                                  winmem.c      */
/*                                                                            */
/* Windows memory management                                                  */
/******************************************************************************/

#include "RexxCore.h"
#include "RexxMemory.hpp"
#include "ActivityManager.hpp"
#include "SystemInterpreter.hpp"

// retrofit by IH
#define MEMSIZE   4194304   /* get pools in 4M chunks. */
#define PAGESIZE  4096

/*********************************************************************/
/*                                                                   */
/*   Function:  Allocate a block of memory suitable to pass back to  */
/*              another program as a return result                   */
/*                                                                   */
/*********************************************************************/
void *SystemInterpreter::allocateResultMemory(
   size_t   Size )                     /* size to allocate                  */
{
    return (void *)GlobalAlloc(GMEM_FIXED, Size);
}


/*********************************************************************/
/*                                                                   */
/*   Function:  Release a block of memory given to us by an outside  */
/*              agent as a return result                             */
/*                                                                   */
/*********************************************************************/
void SystemInterpreter::releaseResultMemory(
  void  *MemoryBlock)                  /* pointer to the result memory      */
{
    /* free for DB2 2.1.1 version */
    if (GlobalSize(MemoryBlock))
      GlobalFree(MemoryBlock);
    else
        free(MemoryBlock);           /* release this block */
}


/**
 * Create a the initial memory pool for the interpreter.
 *
 * @param pool   pointer for the returned pool.
 *
 * @return
 */
MemorySegmentPool *MemorySegmentPool::createPool()
{
    MemorySegmentPool *  newPool;
    void  *tmpPtr;
    size_t             segmentSize;
    /* create the memory segemnt         */
    /* if already exists then this       */
    /* isn't a coldstart.                */
    tmpPtr = VirtualAlloc(NULL, MEMSIZE, MEM_RESERVE|MEM_TOP_DOWN, PAGE_READWRITE);
    if (tmpPtr != NULL)
    {
        segmentSize = RXROUNDUP(SegmentSize, PAGESIZE);
        newPool = (MemorySegmentPool *)tmpPtr;
        /* Now commit the storage.           */
        tmpPtr = VirtualAlloc(tmpPtr, segmentSize, MEM_COMMIT, PAGE_READWRITE);
        if (!tmpPtr)                       /* Error on commit?                  */
        {
            reportException(Error_System_resources);
            return NULL;                    /* return NULL.                      */
        }

        /* Since memory may not be ready     */
        /* for a segment we keep it as a spar*/
        /* and will give it to memory on     */
        /* 1st request for a new segment.    */

        newPool->spareSegment = new (((char *)newPool) + MemorySegmentPoolOverhead) MemorySegment (segmentSize - MemorySegmentPoolOverhead);
        /* compute actual usable space.      */
        newPool->uncommitted = MEMSIZE - segmentSize;
#ifdef _DEBUG
        newPool->reserved = MEMSIZE;
#endif
        /* compute starting point of next    */
        /* allocation.                       */
        newPool->nextAlloc = ((char *)newPool) + segmentSize;
        /* start allocating large segments   */
        /* from the end of the pool.         */
        newPool->nextLargeAlloc = ((char*) newPool) + MEMSIZE;

        new (newPool) MemorySegmentPool;   /* Initialize as a segmentPool       */
        return newPool;                    /* newly created.                    */
    }
    else
    {
        reportException(Error_System_resources);
    }
    return NULL;
}


void *MemorySegmentPool::operator new(size_t size, size_t minSize)
/*********************************************************************/
/* Function:: Create a new MemoryPool block, which is basically      */
/*   a new OS/2 Shared memory block.                                 */
/*                                                                   */
/*********************************************************************/
{
  MemorySegmentPool *newPool;
  void  *tmpPtr;
  size_t initialSegSize;
  size_t poolSize;

                                       /* Add pool object overhead to minSiz*/
  minSize += MemorySegmentPoolOverhead;
  if (minSize > MEMSIZE)               /* Asking for more than default pool */
                                       /* compute actual request size.      */
   poolSize = RXROUNDUP(minSize + MemorySegmentPoolOverhead, PAGESIZE);
  else
   poolSize = MEMSIZE;
                                       /* create another shared memory      */
                                       /* segment, unnamed, Gettable.       */
                                       /* and initialiiy uncommited.        */

  tmpPtr = VirtualAlloc(NULL, poolSize, MEM_RESERVE|MEM_TOP_DOWN, PAGE_READWRITE);
  if (!tmpPtr)                        /* Error on reserve?                 */
  {
     reportException(Error_System_resources);
     return NULL;                        /* return NULL.                    */
  }
                                       /* Need to commit some potion of     */
                                       /* new pool, so create a segment from*/
                                       /* from it.                          */

  if (minSize < SegmentSize)           /* need more than a segment?         */
                                       /* nope make a normal segment        */
    initialSegSize = RXROUNDUP(SegmentSize, PAGESIZE);
  else
                                       /* yup, commit enough for this alloc */
    initialSegSize = RXROUNDUP(minSize, PAGESIZE);

                                       /* Now commit the storage.           */
  tmpPtr = VirtualAlloc(tmpPtr, initialSegSize, MEM_COMMIT, PAGE_READWRITE);
                                       /* Since memory may not be ready     */
                                       /* for a segment we keep it as a spar*/
                                       /* and will give it to memory on     */
                                       /* 1st request for a new segment.    */
  newPool = (MemorySegmentPool *) tmpPtr;
  if (!tmpPtr)                        /* Error on commit?                  */
  {
     reportException(Error_System_resources);
     return NULL;                        /* return NULL.                   */
  }

  newPool->spareSegment = new (((char *)newPool) + MemorySegmentPoolOverhead) MemorySegment (initialSegSize - MemorySegmentPoolOverhead);
                                       /* compute actual usable space.      */
  newPool->uncommitted = poolSize - initialSegSize;
#ifdef _DEBUG
  newPool->reserved = poolSize;
#endif
                                       /* compute starting poin of next     */
                                       /* allocation.                       */
  newPool->nextAlloc = ((char *)newPool) + initialSegSize;
  // note: the whole code here is a duplicate of the code in the constructor...
  //       ...why not use the constructor?
                                       /* start allocating large segments   */
                                       /* from the end of the pool.         */
  newPool->nextLargeAlloc = ((char*) newPool) + poolSize;
  return newPool;
}

MemorySegmentPool::MemorySegmentPool()
/*********************************************************************/
/* Function:: Constructor for memoryPool block.                      */
/*   set next to NULL, size to size,                                 */
/*                                                                   */
/*********************************************************************/
{
   this->next  = NULL;                 /* No next pointer right now         */
}


MemorySegment *MemorySegmentPool::newSegment(size_t minSize)
/*********************************************************************/
/* Function:: Constructor for memoryPool block.                      */
/*   set next to NULL, size to size,                                 */
/*                                                                   */
/*********************************************************************/
{
    size_t segmentSize;
    MemorySegment *newSeg;
    MemorySegmentPool *newPool;
    void  *tmpPtr;

    /* Any spare segments left over      */
    /* from initial pool alloc?          */
    /* And big enough for this request?  */
    if (this->spareSegment && this->spareSegment->size() >= minSize)
    {
        newSeg = this->spareSegment;      /* no longer have a spare            */
        this->spareSegment = NULL;
        return newSeg;                    /* return this segment for allocation*/
    }
    segmentSize = RXROUNDUP(minSize, PAGESIZE); /* compute commit size       */
    /* enough space for request          */
    if (this->uncommitted >= segmentSize)
    {
        /* commit next portion               */
        tmpPtr = VirtualAlloc((void *)this->nextAlloc, segmentSize, MEM_COMMIT,PAGE_READWRITE);
        this->nextAlloc = (char *)tmpPtr;
        if (!tmpPtr)
        {
            reportException(Error_System_resources);
        }
        /* Create new segment.               */
        newSeg = new (this->nextAlloc) MemorySegment (segmentSize);
        this->uncommitted -= segmentSize; /* Adjust uncommitted amount         */
        this->nextAlloc += segmentSize;   /* Adjust to next Allocation addr    */
        return newSeg;
    }
    else
    {
        newPool = new (minSize) MemorySegmentPool;
        /* Did we get a new Pool?            */
        if (newPool)
        {
            // make sure the memory object knows we've added a new pool to the set
            memoryObject.memoryPoolAdded(newPool);  // tell the memory object we have a new pool
            this->next = newPool;         /* Anchor it to end of chain         */
            return newPool->newSegment(minSize);
        }
        else
        {
            return NULL;
        }
    }
}

MemorySegment *MemorySegmentPool::newLargeSegment(size_t minSize)
/*********************************************************************/
/* Function: Allocate a large segment from the other end of the pool */
/*********************************************************************/
{
    size_t segmentSize;
    MemorySegment *newSeg;
    MemorySegmentPool *newPool;
    void  *tmpPtr;

    /* Any spare segments left over      */
    /* from initial pool alloc?          */
    /* And big enough for this request?  */
    if (this->spareSegment && this->spareSegment->size() >= minSize)
    {
        newSeg = this->spareSegment;      /* no longer have a spare            */
        this->spareSegment = NULL;
        return newSeg;                    /* return this segment for allocation*/
    }

    /* compute commit size               */
    segmentSize = RXROUNDUP(minSize, PAGESIZE);
    /* enough space for request          */
    if (this->uncommitted >= segmentSize)
    {
        /* commit next portion               */
        tmpPtr = VirtualAlloc((void *)(this->nextLargeAlloc - segmentSize), segmentSize, MEM_COMMIT,PAGE_READWRITE);
        this->nextLargeAlloc = (char *)tmpPtr;
        if (!tmpPtr)
        {
            reportException(Error_System_resources);
        }
        /* Create new segment.               */
        newSeg = new (this->nextLargeAlloc) MemorySegment (segmentSize);
        this->uncommitted -= segmentSize; /* Adjust uncommitted amount         */
        // this->nextLargeAlloc does not have to be adjusted, will be correct
        return newSeg;
    }
    else
    {
        newPool = new (minSize) MemorySegmentPool;
        if (newPool)                      /* Did we get a new Pool?            */
        {
            memoryObject.memoryPoolAdded(newPool);
            this->next = newPool;         /* Anchor it to end of chain         */
            return newPool->newLargeSegment(minSize);
        }
        else
        {
            return NULL;
        }
    }
}

/* return a pointer to the next valid pool */
void MemorySegmentPool::freePool()
/*********************************************************************/
/* Function:: DosFreeMem this pool object                            */
/*********************************************************************/
{
   VirtualFree(this, 0, MEM_RELEASE);
}


void MemorySegmentPool::setNext( MemorySegmentPool *nextPool )
/*********************************************************************/
/* Function:: set the next pointer                                   */
/*********************************************************************/
{
   this->next = nextPool;
}


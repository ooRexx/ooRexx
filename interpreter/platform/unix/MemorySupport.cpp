/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2014 Rexx Language Association. All rights reserved.    */
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
/* REXX Unix Support                                                          */
/*                                                                            */
/* Unix memory management routines                                            */
/*                                                                            */
/******************************************************************************/


#include <stdlib.h>
#include "RexxCore.h"
#include "RexxMemory.hpp"
#include "ActivityManager.hpp"
#include "SystemInterpreter.hpp"
#include "Memory.hpp"


/**
 * Allocate a block of memory suitable to pass back to
 * another program as a return result
 *
 * @param size   The memory size to allocate.
 *
 * @return The pointer to the allocated memory.
 */
void * SystemInterpreter::allocateResultMemory(size_t size)
{
    return malloc(size);
}


/**
 * Release a block of memory given to us by an outside
 * agent as a return result
 *
 * @param memoryBlock
 *               The block of memory to release.
 */
void SystemInterpreter::releaseResultMemory(void *memoryBlock)
{
    free(memoryBlock);
}


/**
 * Create a new MemoryPool block.
 *
 * @param size    The size of the block
 * @param minSize The minimum size we can accept.
 *
 * @return The memory for a new block
 */
void *MemorySegmentPool::operator new(size_t size, size_t minSize)
{
    /* Add pool object overhead to minSiz*/
    minSize += MemorySegment::MemorySegmentPoolOverhead;

    size_t poolSize = Memory::MemoryAllocationSize;
    if (minSize > Memory::MemoryAllocationSize)               /* Asking for more than default pool */
    {
        /* compute actual request size.      */
        poolSize = MemorySegment::roundToSegmentPoolSize(minSize);
    }

    void *tmpPtr = calloc(poolSize,1);
    if (!tmpPtr)                       /* Error on commit?                  */
    {
        reportException(Error_System_resources);
    }

    MemorySegmentPool *newPool = (MemorySegmentPool *) tmpPtr;

    size_t initialSegSize;
    if (minSize < MemorySegment::SegmentSize)
    {
        initialSegSize = Memory::roundPageSize(MemorySegment::SegmentSize);
    }
    else
    {
        initialSegSize = Memory::roundPageSize(minSize);
    }

    newPool->spareSegment = new (((char *)newPool) + MemorySegment::MemorySegmentPoolOverhead)
                            MemorySegment (initialSegSize - MemorySegment::MemorySegmentPoolOverhead);
    newPool->uncommitted = poolSize - initialSegSize;
#ifdef _DEBUG
    newPool->reserved = poolSize;
#endif

    newPool->nextAlloc = (char *)newPool + initialSegSize;

    /* start allocating large segments   */
    /* from the end of the pool.         */
    newPool->nextLargeAlloc = ((char*) newPool) + poolSize;

    return newPool;
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
    void *tmpPtr = (void *)calloc(Memory::MemoryAllocationSize, 1);
    if (tmpPtr == NULL)              /* Error on commit?                  */
    {
        reportException(Error_System_resources);
    }

    MemorySegmentPool *newPool = (MemorySegmentPool *)tmpPtr;
    size_t segmentSize = Memory::roundPageSize(MemorySegment::SegmentSize);

    /* Since memory may not be ready     */
    /* for a segment we keep it as a spar*/
    /* and will give it to memory on     */
    /* 1st request for a new segment.    */

    newPool->spareSegment = new (((char *)newPool) + MemorySegment::MemorySegmentPoolOverhead) MemorySegment (segmentSize - MemorySegment::MemorySegmentPoolOverhead);
    /* compute actual usable space.      */
    newPool->uncommitted = Memory::MemoryAllocationSize - segmentSize;
#ifdef _DEBUG
    newPool->reserved = Memory::MemoryAllocationSize;
#endif
    /* compute starting poin of next     */
    /* allocation.                       */
    newPool->nextAlloc = (char *)newPool + segmentSize;

    /* start allocating large segments   */
    /* from the end of the pool.         */
    newPool->nextLargeAlloc = ((char*) newPool) + Memory::MemoryAllocationSize;

    ::new ((void *)newPool) MemorySegmentPool;   /* Initialize as a segmentPool       */
    return newPool;                    /* newly created.                    */
}


/**
 * Constructor for a MemorySementPool object.
 */
MemorySegmentPool::MemorySegmentPool()
{
    next  = NULL;                 /* No next pointer right now         */
}


/**
 * Create a new segment of a given minimum size
 *
 * @param minSize The minimum required size.
 *
 * @return The new memory segment object.
 */
MemorySegment *MemorySegmentPool::newSegment(size_t minSize)
{
    /* Any spare segments left over      */
    /* from initial pool alloc?          */
    /* And big enough for this request?  */
    if (spareSegment && spareSegment->size() >= minSize)
    {
        MemorySegment *newSeg = spareSegment;      /* no longer have a spare            */
        spareSegment = NULL;
        return newSeg;                    /* return this segment for allocation*/
    }

    size_t segmentSize = Memory::roundPageSize(minSize); /* compute commit size       */
    /* enough space for request          */
    if (uncommitted >= segmentSize)
    {
        MemorySegment *newSeg = new (nextAlloc) MemorySegment (segmentSize);

        uncommitted -= segmentSize; /* Adjust uncommitted amount         */
        nextAlloc += segmentSize;   /* Adjust to next Allocation addr    */
        return newSeg;
    }
    else                                /* uncommitted not enough need a new pool */
    {
        MemorySegmentPool *newPool = new (minSize) MemorySegmentPool;
        if (newPool)
        {                    /* Did we get a new Pool?            */
            next = newPool;         /* Anchor it to end of chain         */
            memoryObject.memoryPoolAdded(newPool);  // tell the memory object we'v added a new pool
            return newPool->newSegment(minSize);
        }
        else
        {
            return NULL;
        }
    }
}


/**
 * Create a new segment of a given minimum size
 *
 * @param minSize The minimum required size.
 *
 * @return The new memory segment object.
 */
MemorySegment *MemorySegmentPool::newLargeSegment(size_t minSize)
{
    /* Any spare segments left over      */
    /* from initial pool alloc?          */
    /* And big enough for this request?  */
    if (spareSegment && spareSegment->size() >= minSize)
    {
        MemorySegment *newSeg = spareSegment;      /* no longer have a spare            */
        spareSegment = NULL;
        return newSeg;                    /* return this segment for allocation*/
    }

    /* compute commit size               */
    size_t segmentSize = Memory::roundPageSize(minSize);
    /* enough space for request          */
    if (uncommitted >= segmentSize)
    {
        nextLargeAlloc = nextLargeAlloc - segmentSize; // already calloc'd on AIX, just move pointer

        /* Create new segment.               */
        MemorySegment *newSeg = new (nextLargeAlloc) MemorySegment (segmentSize);
        uncommitted -= segmentSize; /* Adjust uncommitted amount         */
        // nextLargeAlloc does not have to be adjusted, will be correct
        return newSeg;
    }
    else
    {
        MemorySegmentPool *newPool = new (minSize) MemorySegmentPool;
        if (newPool)                      /* Did we get a new Pool?            */
        {
            /* Make sure new pool is added to the end of list   */
            next = newPool;           /* Anchor it to end of chain         */
            memoryObject.memoryPoolAdded(newPool);  // tell the memory object we've added a new pool
            return newPool->newLargeSegment(minSize);
        }
        else
        {
            return NULL;
        }
    }
}


/**
 * Release the memory associated with a pool
 */
void MemorySegmentPool::freePool()       // add return value
{
    free(this);
}


/**
 * Chain a memory pool to another pool
 *
 * @param nextPool The next pool in the chain.
 */
void MemorySegmentPool::setNext( MemorySegmentPool *nextPoolPtr )
{
   next = nextPoolPtr;
}

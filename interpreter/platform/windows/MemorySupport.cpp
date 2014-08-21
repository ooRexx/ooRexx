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
/* REXX Kernel                                                                */
/*                                                                            */
/* Windows memory management                                                  */
/******************************************************************************/

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
void *SystemInterpreter::allocateResultMemory(size_t size)
{
    return (void *)GlobalAlloc(GMEM_FIXED, size);
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
    // The DB2 interfaces were using the wrong allocation method for the
    // storage.  So, of course, we're the ones who have to adjust.
    if (GlobalSize(memoryBlock))
    {
        GlobalFree(memoryBlock);
    }
    else
    {
        free(memoryBlock);
    }
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
    MemorySegmentPool *newPool;
    void  *tmpPtr;
    size_t initialSegSize;

    // add the pool overhead to the minimum size
    minSize += MemorySegment::MemorySegmentPoolOverhead;
    // we have a minimum allocation amount...
    size_t poolSize = Numerics::maxVal(Memory::MemoryAllocationSize, MemorySegment::roundToSegmentPoolSize(minSize));
                                         /* create another shared memory      */
                                         /* segment, unnamed, Gettable.       */
                                         /* and initialiiy uncommited.        */
    // try to to allocate another chunk of memory.
    tmpPtr = VirtualAlloc(NULL, poolSize, MEM_RESERVE|MEM_TOP_DOWN, PAGE_READWRITE);
    if (!tmpPtr)
    {
       reportException(Error_System_resources);
       return NULL;
    }

    // If this is smaller than a single segment, just allocate a segment, otherwise
    // round up the allocation amount to a pagesize
    if (minSize < MemorySegment::SegmentSize)
    {

        initialSegSize = Memory::roundPageSize(MemorySegment::SegmentSize);
    }
    else
    {

        initialSegSize = Memory::roundPageSize(minSize);
    }

    // now commit that amound
    tmpPtr = VirtualAlloc(tmpPtr, initialSegSize, MEM_COMMIT, PAGE_READWRITE);
    // Since memory may not be ready for a segment we keep it as a spare
    // and will give it to memory on 1st request for a new segment.
    newPool = (MemorySegmentPool *) tmpPtr;
    if (!tmpPtr)
    {
       reportException(Error_System_resources);
       return NULL;
    }

    newPool->spareSegment = new (((char *)newPool) + MemorySegment::MemorySegmentPoolOverhead)
        MemorySegment (initialSegSize - MemorySegment::MemorySegmentPoolOverhead);
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
    tmpPtr = VirtualAlloc(NULL, Memory::MemoryAllocationSize, MEM_RESERVE|MEM_TOP_DOWN, PAGE_READWRITE);
    if (tmpPtr != NULL)
    {
        segmentSize = Memory::roundPageSize(MemorySegment::SegmentSize);
        newPool = (MemorySegmentPool *)tmpPtr;
        // now commit the storage.  If this did not commit ok, we have a problem,
        // so indicate an allocation failure
        tmpPtr = VirtualAlloc(tmpPtr, segmentSize, MEM_COMMIT, PAGE_READWRITE);
        if (!tmpPtr)
        {
            reportException(Error_System_resources);
            return NULL;
        }

        // Since memory may not be ready for a segment we keep it as a spare
        // and will give it to memory on 1st request for a new segment.
        newPool->spareSegment = new (((char *)newPool) + MemorySegment::MemorySegmentPoolOverhead)
            MemorySegment (segmentSize - MemorySegment::MemorySegmentPoolOverhead);
        // compute actual usable space.
        newPool->uncommitted = Memory::MemoryAllocationSize - segmentSize;
#ifdef _DEBUG
        newPool->reserved = Memory::MemoryAllocationSize;
#endif
        // compute starting point of next allocation.
        newPool->nextAlloc = ((char *)newPool) + segmentSize;
        // start allocating large segments from the end of the pool.
        newPool->nextLargeAlloc = ((char*) newPool) + Memory::MemoryAllocationSize;

        // Initialize this as a segment pool
        ::new (newPool) MemorySegmentPool;
        return newPool;
    }
    else
    {
        reportException(Error_System_resources);
    }
    return NULL;
}

MemorySegmentPool::MemorySegmentPool()
/*********************************************************************/
/* Function:: Constructor for memoryPool block.                      */
/*   set next to NULL, size to size,                                 */
/*                                                                   */
/*********************************************************************/
{
    next  = NULL;                 /* No next pointer right now         */
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
    if (spareSegment && spareSegment->size() >= minSize)
    {
        newSeg = spareSegment;      /* no longer have a spare            */
        spareSegment = NULL;
        return newSeg;                    /* return this segment for allocation*/
    }
    segmentSize = Memory::roundPageSize(minSize); /* compute commit size       */
    /* enough space for request          */
    if (uncommitted >= segmentSize)
    {
        /* commit next portion               */
        tmpPtr = VirtualAlloc((void *)nextAlloc, segmentSize, MEM_COMMIT,PAGE_READWRITE);
        nextAlloc = (char *)tmpPtr;
        if (!tmpPtr)
        {
            reportException(Error_System_resources);
        }
        /* Create new segment.               */
        newSeg = new (nextAlloc) MemorySegment (segmentSize);
        uncommitted -= segmentSize; /* Adjust uncommitted amount         */
        nextAlloc += segmentSize;   /* Adjust to next Allocation addr    */
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
            next = newPool;         /* Anchor it to end of chain         */
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
    if (spareSegment && spareSegment->size() >= minSize)
    {
        newSeg = spareSegment;      /* no longer have a spare            */
        spareSegment = NULL;
        return newSeg;                    /* return this segment for allocation*/
    }

    /* compute commit size               */
    segmentSize = Memory::roundPageSize(minSize);
    /* enough space for request          */
    if (uncommitted >= segmentSize)
    {
        /* commit next portion               */
        tmpPtr = VirtualAlloc((void *)(nextLargeAlloc - segmentSize), segmentSize, MEM_COMMIT,PAGE_READWRITE);
        nextLargeAlloc = (char *)tmpPtr;
        if (!tmpPtr)
        {
            reportException(Error_System_resources);
        }
        /* Create new segment.               */
        newSeg = new (nextLargeAlloc) MemorySegment (segmentSize);
        uncommitted -= segmentSize; /* Adjust uncommitted amount         */
        // this->nextLargeAlloc does not have to be adjusted, will be correct
        return newSeg;
    }
    else
    {
        newPool = new (minSize) MemorySegmentPool;
        if (newPool)                      /* Did we get a new Pool?            */
        {
            memoryObject.memoryPoolAdded(newPool);
            next = newPool;         /* Anchor it to end of chain         */
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
    next = nextPool;
}


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
/* REXX AIX Support                                             aixmem.c      */
/*                                                                            */
/* AIX memory management routines                                             */
/*                                                                            */
/******************************************************************************/


#include <stdlib.h>
#include "RexxCore.h"
#include "RexxMemory.hpp"
#include "ActivityManager.hpp"
#include "SystemInterpreter.hpp"

#define MEMSIZE     4194304            /* memory pool                       */
#ifdef LINUX
#define PAGESIZE    4096               /* page size                         */
#endif

/*********************************************************************/
/*                                                                   */
/*   Function:  Allocate a block of memory suitable to pass back to  */
/*              another program as a return result                   */
/*                                                                   */
/*********************************************************************/
void * SystemInterpreter::allocateResultMemory(
   size_t   Size )                     /* size to allocate                  */
{
  return malloc(Size);
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
  free(MemoryBlock);                   /* release this block                */
}


MemorySegmentPool *MemorySegmentPool::createPool()
/*********************************************************************/
/*   Function:  Access/Create the 1st block as Named Storage         */
/*              return true is an access, false is created.          */
/*********************************************************************/
{
    MemorySegmentPool *  newPool;
    void  *tmpPtr;
    size_t segmentSize;
    /* create the shared memory segemnt  */
    /* if already exists then this       */
    /* isn't a coldstart.                */
    tmpPtr = (void *)calloc(MEMSIZE,1);
    if (tmpPtr == NULL)              /* Error on commit?                  */
    {
        reportException(Error_System_resources);
    }

    newPool = (MemorySegmentPool *)tmpPtr;
    segmentSize = RXROUNDUP(SegmentSize, PAGESIZE);

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
    /* compute starting poin of next     */
    /* allocation.                       */
    newPool->nextAlloc = (char *)newPool + segmentSize;

    /* start allocating large segments   */
    /* from the end of the pool.         */
    newPool->nextLargeAlloc = ((char*) newPool) + MEMSIZE;

    new (newPool) MemorySegmentPool;   /* Initialize as a segmentPool       */
    return newPool;                    /* newly created.                    */
}

void *MemorySegmentPool::operator new(size_t size, size_t minSize)
/*********************************************************************/
/* Function:: Create a new MemoryPool block                          */
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


   tmpPtr = calloc(poolSize,1);
   if (!tmpPtr)                       /* Error on commit?                  */
   {
      reportException(Error_System_resources);
   }

   newPool = (MemorySegmentPool *) tmpPtr;

   if (minSize < SegmentSize)
      initialSegSize = RXROUNDUP(SegmentSize, PAGESIZE);
   else
      initialSegSize = RXROUNDUP(minSize, PAGESIZE);

   newPool->spareSegment = new (((char *)newPool) + MemorySegmentPoolOverhead) MemorySegment (initialSegSize - MemorySegmentPoolOverhead);
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
    size_t  segmentSize;
    MemorySegment *newSeg;
    MemorySegmentPool *newPool;
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
        newSeg = new (this->nextAlloc) MemorySegment (segmentSize);

        this->uncommitted -= segmentSize; /* Adjust uncommitted amount         */
        this->nextAlloc += segmentSize;   /* Adjust to next Allocation addr    */
        return newSeg;
    }
    else                                /* uncommitted not enough need a new pool */
    {
        newPool = new (minSize) MemorySegmentPool;
        if (newPool)
        {                    /* Did we get a new Pool?            */
            this->next = newPool;         /* Anchor it to end of chain         */
            memoryObject.memoryPoolAdded(newPool);  // tell the memory object we'v added a new pool
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
        this->nextLargeAlloc = this->nextLargeAlloc - segmentSize; // already calloc'd on AIX, just move pointer

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
            /* Make sure new pool is added to the end of list   */
            this->next = newPool;           /* Anchor it to end of chain         */
            memoryObject.memoryPoolAdded(newPool);  // tell the memory object we've added a new pool
            return newPool->newLargeSegment(minSize);
        }
        else
        {
            return NULL;
        }
    }
}

void MemorySegmentPool::freePool()       // add return value
/*********************************************************************/
/* Function:: Free this pool object                                  */
/*********************************************************************/
{
    free(this);
}

/******************** added by weigold *******************************/
void MemorySegmentPool::setNext( MemorySegmentPool *nextPoolPtr )
/*********************************************************************/
/* Function:: set the next pointer                                   */
/*********************************************************************/
{
   this->next = nextPoolPtr;
}

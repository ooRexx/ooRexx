/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2006 Rexx Language Association. All rights reserved.    */
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
                                       /* Local and global memory Pools     */
                                       /*  last one accessed.               */
extern MemorySegmentPool *ProcessCurrentPool;
extern MemorySegmentPool *GlobalCurrentPool;
//extern BOOL  ProcessColdStart;         /* we're coldstarting this         */
//extern BOOL  ProcessRestoreImage;      /* we're restoring the image       */
//extern BOOL  ProcessSaveImage;         /* we're saving    the image       */

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
PVOID SysAllocateResultMemory(
   LONG     Size )                     /* size to allocate                  */
{
  PVOID  MemoryBlock;                  /* allocated memory block            */

                                       /* try to allocate the storage       */
  if (MemoryBlock=(PVOID)malloc( Size))
    return MemoryBlock;                /* return the allocated memory       */
  else
    return NULL;                       /* return the allocated memory       */
}

/*********************************************************************/
/*                                                                   */
/*   Function:  Release a block of memory given to us by an outside  */
/*              agent as a return result                             */
/*                                                                   */
/*********************************************************************/
void SysReleaseResultMemory(
  PVOID  MemoryBlock)                  /* pointer to the result memory      */
{
  free(MemoryBlock);                   /* release this block                */
}
//BOOL SysAccessPool(MemorySegmentPool **pool)
/*********************************************************************/
/* Function:  Access/Create the 1st block as Named Storage           */
/*            return TRUE as an access, FALSE is created.            */
/*********************************************************************/
//{
//  return FALSE;
//}

BOOL SysAccessPool(MemorySegmentPool **pool)
/*********************************************************************/
/*   Function:  Access/Create the 1st block as Named Storage         */
/*              return TRUE is an access, FALSE is created.          */
/*********************************************************************/
{
  APIRET rc;                           /* return code                  */
  MemorySegmentPool *  newPool;
  PVOID  tmpPtr;
  ULONG              segmentSize;
                                       /* create the shared memory segemnt  */
                                       /* if already exists then this       */
                                       /* isn't a coldstart.                */
/*
  tmpPtr = VirtualAlloc(NULL, MEMSIZE, MEM_RESERVE|MEM_TOP_DOWN, PAGE_READWRITE);
*/
  tmpPtr = calloc(MEMSIZE,1);
  if (!tmpPtr)                       /* Error on commit?                  */
  {
     report_exception(Error_System_resources);
     return NULL;                    /* return NULL.                      */
  }

  *pool = (MemorySegmentPool *)tmpPtr;
  newPool = *pool;
//segmentSize = roundup4096(SEGSIZE);
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
    return FALSE;                      /* newly created.                    */
}

void     *MemorySegmentPool::operator new(size_t size, size_t minSize)
/*********************************************************************/
/* Function:: Create a new MemoryPool block                          */
/*                                                                   */
/*********************************************************************/
{
  APIRET rc;
  MemorySegmentPool *newPool;
  MemorySegment     *newSeg;
  PVOID  tmpPtr;
  ULONG  initialSegSize;
  ULONG  poolSize;

                                       /* Add pool object overhead to minSiz*/
  minSize += MemorySegmentPoolOverhead;
  if (minSize > MEMSIZE)               /* Asking for more than default pool */
                                       /* compute actual request size.      */
// poolSize = roundup4096(minSize + MEMORYSEGMENTPOOLOVERHEAD);
   poolSize = RXROUNDUP(minSize + MemorySegmentPoolOverhead, PAGESIZE);
  else
   poolSize = MEMSIZE;


   tmpPtr = calloc(poolSize,1);
   if (!tmpPtr)                       /* Error on commit?                  */
   {
      report_exception(Error_System_resources);
      return NULL;                    /* return NULL.                      */
   }

   newPool = (MemorySegmentPool *) tmpPtr;

// if (minSize < SEGSIZE)
//    initialSegSize = roundup4096(SEGSIZE);
// else
//    initialSegSize = roundup4096(minSize);

   if (minSize < SegmentSize)
      initialSegSize = RXROUNDUP(SegmentSize, PAGESIZE);
   else
      initialSegSize = RXROUNDUP(minSize, PAGESIZE);

   newPool->spareSegment = new (((char *)newPool) + MemorySegmentPoolOverhead) MemorySegment (initialSegSize - MemorySegmentPoolOverhead);
// newPool = (MemorySegmentPool *) tmpPtr;
   newPool->uncommitted = poolSize - initialSegSize;
#ifdef _DEBUG
   newPool->reserved = poolSize;
#endif

   newPool->nextAlloc = (char *)newPool + initialSegSize;

                                       /* start allocating large segments   */
                                       /* from the end of the pool.         */
   newPool->nextLargeAlloc = ((char*) newPool) + poolSize;

   return newPool;
// just as it was with the memory leak
//                                     /* Create another memory             */
//                                     /* segment, unnamed, Gettable.       */
//                                     /* and initialiiy uncommited.        */
//if (NULL == (newPool = (MemorySegmentPool *)malloc(size)))
//                                     /* Error on allocation?              */
//  return NULL;                       /* return NULL.                      */
//else
//  return newPool;

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

/*THU001A  begin*/
MemorySegment *MemorySegmentPool::newSegment(size_t minSize)
/*********************************************************************/
/* Function:: Constructor for memoryPool block.                      */
/*   set next to NULL, size to size,                                 */
/*                                                                   */
/*********************************************************************/
{
   APIRET rc;
   size_t  segmentSize;
// ULONG  poolSize;
   MemorySegment *newSeg;
   MemorySegmentPool *newPool;
   PVOID  tmpPtr;
                                       /* Any spare segments left over      */
                                       /* from initial pool alloc?          */
                                       /* And big enough for this request?  */
// if (this->spareSegment && this->spareSegment->size >= minSize)
   if (this->spareSegment && this->spareSegment->size() >= minSize)
   {
     newSeg = this->spareSegment;      /* no longer have a spare            */
     this->spareSegment = NULL;
     return newSeg;                    /* return this segment for allocation*/
   }
// else if (this->spareSegment)
// {
// I'm not sure this makes sense any more...having memoryObject add a
// segment doesn't really have any meaning, as it's not clear what the context
// of adding this really should be.  This probably should be kept around in
// case it can be used later on.
//                                     /* have a spare, let memory add it to*/
//                                     /* one of its segment lists          */
//   memoryObject.addSegment(this->spareSegment);
//   this->spareSegment = NULL;        /* no longer have a spare.           */
// }

// segmentSize = roundup4096(minSize); /* compute commit size               */
   segmentSize = RXROUNDUP(minSize, PAGESIZE); /* compute commit size       */
                                       /* enough space for request          */
   if (this->uncommitted >= segmentSize)
   {
/* don't need that
     tmpPtr = VirtualAlloc(PVOID(this->nextAlloc), segmentSize, MEM_COMMIT,PAGE_READWRITE);
         this->nextAlloc = (char *)tmpPtr;
     if (!tmpPtr)
     {
        report_exception(Error_System_resources);
        return NULL;
     }
*/
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
       /* Make sure new pool is added to the end of list   */
       if ( this->next )
       {
          /* anyone else added a new segment while we where here? */
          MemorySegmentPool *lastPool = this->next;

          while ( lastPool->next )
          {
             lastPool = lastPool->next;
          } /* endwhile */
          lastPool->next = newPool;
          memoryObject.accessPools(this);
       }
       else
       {
         this->next = newPool;         /* Anchor it to end of chain         */
       }
       ProcessCurrentPool = newPool;   /* update last pool accessed for proc*/
       GlobalCurrentPool = newPool;    /* update global pool addr           */
//     if (this->uncommitted)
//     {                               /* Anything left to commit?          */
                                       /* Commit remainder.                 */
// Again, we have the same problem.  The memory pool should not be pushing
// segments into the memory object, but rather satisfy this on demand.
/*
          tmpPtr = VirtualAlloc(PVOID(this->nextAlloc), this->uncommitted, MEM_COMMIT, PAGE_READWRITE);
              this->nextAlloc = (char *)tmpPtr;
          if (!tmpPtr)
          {
             report_exception(Error_System_resources);
             return NULL;
          }
*/
                                       /* Let memory add it to its segments */

//        memoryObject.addSegment(new (this->nextAlloc) MemorySegment (this->uncommitted));
//                                     /* update alloc ptr.                 */
//        this->nextAlloc += this->uncommitted;
//        this->uncommitted = 0;          /* and amnount uncomitted.           */
//     }
       return newPool->newSegment(minSize);
     }
     else
     {
       return NULL;
     }
   }
}
/* only remember....                                                        */
/*
   space = malloc(segmentSize);
   if (!space)
     return NULL;

   newSeg = new (space) MemorySegment (segmentSize);
   return newSeg;
}
*/

MemorySegment *MemorySegmentPool::newLargeSegment(size_t minSize)
/*********************************************************************/
/* Function: Allocate a large segment from the other end of the pool */
/*********************************************************************/
{
   size_t segmentSize;
   MemorySegment *newSeg;
   MemorySegmentPool *newPool;
   PVOID  tmpPtr;

                                       /* Any spare segments left over      */
                                       /* from initial pool alloc?          */
                                       /* And big enough for this request?  */
   if (this->spareSegment && this->spareSegment->size() >= minSize) {
     newSeg = this->spareSegment;      /* no longer have a spare            */
     this->spareSegment = NULL;
     return newSeg;                    /* return this segment for allocation*/
   } else if (this->spareSegment) {
#if 0
//                                     /* have a spare, let memory add it to*/
//                                     /* one of its segment lists          */
//   memoryObject.addSegment(this->spareSegment);
//   this->spareSegment = NULL;        /* no longer have a spare.           */
#endif
   }

                                       /* compute commit size               */
   segmentSize = RXROUNDUP(minSize, PAGESIZE);
                                       /* enough space for request          */
   if (this->uncommitted >= segmentSize) {
//   tmpPtr = VirtualAlloc(PVOID(this->nextLargeAlloc - segmentSize), segmentSize, MEM_COMMIT,PAGE_READWRITE);
     this->nextLargeAlloc = this->nextLargeAlloc - segmentSize; // already calloc'd on AIX, just move pointer

//   if (!tmpPtr)
//   {
//      report_exception(Error_System_resources);
//      return NULL;                        /* return NULL.                   */
//   }
                                       /* Create new segment.               */
     newSeg = new (this->nextLargeAlloc) MemorySegment (segmentSize);
     this->uncommitted -= segmentSize; /* Adjust uncommitted amount         */
     // this->nextLargeAlloc does not have to be adjusted, will be correct
     return newSeg;
   } else {
     newPool = new (minSize) MemorySegmentPool;
     if (newPool) {                    /* Did we get a new Pool?            */
       /* Make sure new pool is added to the end of list   */
       if ( this->next ) {
          /* anyone else added a new segment while we where here? */
          MemorySegmentPool *lastPool = this->next;

          while ( lastPool->next )
            lastPool = lastPool->next;
          lastPool->next = newPool;
          memoryObject.accessPools(this);
       } else
         this->next = newPool;         /* Anchor it to end of chain         */
       ProcessCurrentPool = newPool;   /* update last pool accessed for proc*/
       GlobalCurrentPool = newPool;    /* update global pool addr           */
       if (this->uncommitted) {        /* Anything left to commit?          */
#if 0
//                                     /* Commit remainder (as a "normal" segment) */
//        tmpPtr = VirtualAlloc(PVOID(this->nextAlloc), this->uncommitted, MEM_COMMIT, PAGE_READWRITE);
//            this->nextAlloc = (char *)tmpPtr;
//        if (!tmpPtr)
//        {
//           report_exception(Error_System_resources);
//           return NULL;                        /* return NULL.                   */
//        }
//                                     /* Let memory add it to its segments */
//        memoryObject.addSegment(new (this->nextAlloc) MemorySegment (this->uncommitted));
//                                     /* update alloc ptr.                 */
//        this->nextAlloc += this->uncommitted;
//        this->uncommitted = 0;          /* and amnount uncomitted.           */
#endif
       }
       return newPool->newLargeSegment(minSize);
     } else
       return NULL;
   }
}

BOOL    MemorySegmentPool::accessNextPool()
/*********************************************************************/
/* Function:: Gain access to all existing memoryPools.               */
/*                                                                   */
/*********************************************************************/
{
                                       /* Is there a next MemoryPool        */
    if (this->next)
    {
      return TRUE;                      /* Return one accessed               */
    }
    else
    {
      return FALSE;                     /* At the end, return FALSE (NO_MORE)*/
    }
    return FALSE;                     /* At the end, return FALSE (NO_MORE)*/
}

MemorySegmentPool  * MemorySegmentPool::freePool()       // add return value
/*********************************************************************/
/* Function:: Free this pool object                                  */
/*********************************************************************/
{
    MemorySegmentPool *nextPtr;
    nextPtr = this->next;
    free(this);
    return nextPtr;
}

/* do you remember...
   free(this);
*/

/******************** added by weigold *******************************/
void MemorySegmentPool::setNext( MemorySegmentPool *nextPool )
/*********************************************************************/
/* Function:: set the next pointer                                   */
/*********************************************************************/
{
   this->next = nextPool;
}

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
/* REXX Kernel                                                                */
/*                                                                            */
/* Primitive Memory segment management                                        */
/*                                                                            */
/******************************************************************************/
#include "RexxCore.h"
#include "ActivityManager.hpp"


void MemorySegment::dump(const char *owner, size_t counter, FILE *keyfile, FILE *dumpfile)
/******************************************************************************/
/* Function:  Dump information about an individual segment                    */
/******************************************************************************/
{
                                       /* print header for segment          */
      fprintf(stderr,"Dumping %s Segment %d from %p for %u\n", owner, counter, &segmentStart, segmentSize);
                                       /* now dump the segment              */
      fprintf(keyfile, "%s addr.%d = %p\n", owner, counter, &segmentStart);
      fprintf(keyfile, "%s size.%d = %u\n", owner, counter, segmentSize);
      fwrite(&segmentStart, 1, segmentSize, dumpfile);
}


DeadObject *MemorySegment::lastDeadObject()
/******************************************************************************/
/* Function:  return a pointer to the last object in a segment if, and only if*/
/* the object is a dead one.  This is used to check the trailing deadspace    */
/* in the segment for purposes of combining the segments.                     */
/******************************************************************************/
{
    char *objectPtr, *endPtr;
    char *lastObjectPtr = NULL;

    /* just scan all of the objects until we've reached the end of */
    /* the segment */
    for (objectPtr = start(), endPtr = end();
        objectPtr < endPtr;
        objectPtr += ((RexxObject *)objectPtr)->getObjectSize())
    {
        lastObjectPtr = objectPtr;
    }

    if (!((RexxObject *)lastObjectPtr)->isObjectLive(memoryObject.markWord))
    {
        return(DeadObject *)lastObjectPtr;
    }
    return NULL;
}


DeadObject *MemorySegment::firstDeadObject()
/******************************************************************************/
/* Function:  return a pointer to the first object in a segment if, and only  */
/* if the object is a dead one.  This is used to check the leading deadspace  */
/* in the segment for purposes of combining the segments.                     */
/******************************************************************************/
{
    if (!((RexxObject *)start())->isObjectLive(memoryObject.markWord))
    {
        return(DeadObject *)start();
    }
    return NULL;
}


void MemorySegment::gatherObjectStats(MemoryStats *memStats, SegmentStats *stats)
/******************************************************************************/
/* Function:  Accumulate memory statistics for a segment                      */
/******************************************************************************/
{
    char *op;
    char *ep;
    /* for all objects in this segment   */
    for (op = start(), ep = end(); op < ep; op += ((RexxObject *)op)->getObjectSize())
    {
        /* record the information about this object */
        stats->recordObject(memStats, op);
    }
}


void MemorySegmentSet::dumpSegments(FILE *keyfile, FILE *dumpfile)
/******************************************************************************/
/* Function:  Dump information about the each of the segments                 */
/******************************************************************************/
{
    MemorySegment *segment;
    size_t counter = 0;

    for (segment = first(); segment != NULL; segment = next(segment))
    {
        segment->dump(name, ++counter, keyfile, dumpfile);
    }
}

void MemorySegmentSet::dumpMemoryProfile(FILE *outfile)
/******************************************************************************/
/* Function:  Dump profile informaton about a segment set (empty for base     */
/* class)                                                                     */
/******************************************************************************/
{
}

void LargeSegmentSet::dumpMemoryProfile(FILE *outfile)
/******************************************************************************/
/* Function:  Dump profile informaton about the large allocation segment set  */
/******************************************************************************/
{
    fprintf(outfile, "Memory profile for large object allocations\n\n");
    /* all the work is done by the dead caches */
    deadCache.dumpMemoryProfile(outfile);
}


void NormalSegmentSet::dumpMemoryProfile(FILE *outfile)
/******************************************************************************/
/* Function:  Dump profile informaton about the normal allocation segment set */
/******************************************************************************/
{
    int i;

    fprintf(outfile, "Memory profile for normal object allocations\n\n");
    /* all the work is done by the dead caches */
    largeDead.dumpMemoryProfile(outfile);

    for (i = FirstDeadPool; i < DeadPools; i++) {
        subpools[i].dumpMemoryProfile(outfile);
    }
}


void NormalSegmentSet::checkObjectOverlap(DeadObject *obj)
/******************************************************************************/
/* Function:  do dead object overlap validation checking.                     */
/******************************************************************************/
{
    /* all the work is done by the dead caches */
    largeDead.checkObjectOverlap(obj);

    for (int i = FirstDeadPool - 1; i < DeadPools; i++)
    {
        subpools[i].checkObjectOverlap(obj);
    }
}


/******************************************************************************/
/* Function:  Constructor for the large segment pool.                         */
/******************************************************************************/
LargeSegmentSet::LargeSegmentSet(RexxMemory *mem) :
    MemorySegmentSet(mem, SET_NORMAL, "Large Allocation Segments"),
    deadCache("Large Block Allocation Pool"), requests(0), smallestObject(0), largestObject(0) { }


/******************************************************************************/
/* Function:  Constructor for the large segment pool.                         */
/******************************************************************************/
OldSpaceSegmentSet::OldSpaceSegmentSet(RexxMemory *mem) :
    MemorySegmentSet(mem, SET_OLDSPACE, "Old Space Segments"),
    deadCache("Old Space Allocation Pool")
{

}


/******************************************************************************/
/* Function:  Constructor for the normal segment pool.                        */
/******************************************************************************/
NormalSegmentSet::NormalSegmentSet(RexxMemory *mem) :
    MemorySegmentSet(mem, SET_NORMAL, "Normal Allocation Segments"),
    largeDead("Large Normal Allocation Pool")
{
    /* finish setting up the allocation subpools.  We set up one    */
    /* additional one, to act as a guard pool to redirect things to */
    /* the large pool. */
    int i;
    for (i = 0; i < DeadPools; i++)
    {  /* there are only                    */
        /* DeadPools subpools! (<, not <=)   */
        char buffer[100];
        sprintf(buffer, "Normal allocation subpool %d for blocks of size %d", i, DeadPoolToLength(i));
        subpools[i].setID(buffer);
        /* make sure these are properly set up as single size */
        /* keepers */
        subpools[i].emptySingle();
        /* initially, all subpools are empty, so have them skip */
        /* directly to the dead chains. */
        lastUsedSubpool[i] = DeadPools;
    }
    lastUsedSubpool[i] = DeadPools;    /* add the final entry in            */
                                       /* the lastUsedSubpool table (extra) */

                                       /* This must be the first seg created*/
                                       /* Create a segment for recovery     */
                                       /* doesn't need to be a full segment */
                                       /* worth, 1/2 should do              */
    recoverSegment = memory->newSegment(RecoverSegmentSize, RecoverSegmentSize);
}


void NormalSegmentSet::getInitialSet()
/******************************************************************************/
/* Function:  Set up the intial normal memory allocation.                     */
/******************************************************************************/
{
    /* get our first set of segments */
    addSegments(InitialNormalSegmentSpace);
}


MemorySegment *MemorySegmentSet::allocateSegment(size_t requestLength, size_t minimumLength)
/******************************************************************************/
/* Function:  Allocate a segment...designed for subclass overrides            */
/******************************************************************************/
{
    return memory->newSegment(requestLength, minimumLength);
}


MemorySegment *LargeSegmentSet::allocateSegment(size_t requestLength, size_t minimumLength)
/******************************************************************************/
/* Function:  Allocate a segment...designed for subclass overrides            */
/******************************************************************************/
{
    return memory->newLargeSegment(requestLength, minimumLength);
}


MemorySegment *MemorySegmentSet::getSegment(size_t requestLength, size_t minimumLength)
/******************************************************************************/
/* Function:  Locate a for a request.  We try to allocate one first from      */
/* the pool of empty segments, then go to the memory pool manager if we don't */
/* have one in reserve.                                                       */
/******************************************************************************/
{
    /* see if we have an empty segment available for the full size */
    MemorySegment *segment = findEmptySegment(requestLength);
    /* if we can't find one, check for the minimum size first, */
    /* before we go to the memory pool manager to get more. */
    if (segment == NULL) {
        segment = findEmptySegment(minimumLength);
        /* No luck, go get a real thing to add to this. */
        if (segment == NULL) {
            segment = allocateSegment(requestLength, minimumLength);
        }
    }
    return segment;
}


MemorySegment *MemorySegmentSet::findEmptySegment(size_t requestLength)
/******************************************************************************/
/* Function:  Find a segment in our empty segment cache.                      */
/******************************************************************************/
{
    /* scan the empty segments list */
    MemorySegment *segment = emptySegments.next;
    while (segment->isReal())
    {
        /* if this one is large enough, use it. */
        if (segment->size() > requestLength)
        {
            segment->remove();
            return segment;
        }
        segment = segment->next;
    }
    return NULL;
}


void MemorySegmentSet::activateEmptySegments()
/******************************************************************************/
/* Function:  Move all of our active segments back into the active cache.  We */
/* do this because we want to try to merge segments to create larger blocks.  */
/******************************************************************************/
{
    /* scan the empty segments list */
    MemorySegment *segment = emptySegments.next;
    while (segment->isReal())
    {
        /* grab the next segment in the chain */
        MemorySegment *nextSeg = segment->next;

        /* remove the current one from the chain */
        segment->remove();
        /* insert back into the active list */
        addSegment(segment);
        segment = nextSeg;
    }
}


bool MemorySegmentSet::newSegment(size_t requestLength, size_t minimumLength)
/******************************************************************************/
/* Function:  Allocate a segment and add it to the segment pool               */
/******************************************************************************/
{
    /* try to allocate a segment, and add it to the list if */
    /* successful.  This also adds the space to our dead object */
    /* pool. */
    MemorySegment *segment = allocateSegment(requestLength, minimumLength);
    if (segment != NULL)
    {
        addSegment(segment);
        return true;
    }
    return false;
}


void MemorySegmentSet::addDeadObject(DeadObject *object)
/******************************************************************************/
/* Function:  Add a dead object to the segment set.                           */
/******************************************************************************/
{
 // The base class doesn't maintain a dead object pool, so this is a nop
}


void LargeSegmentSet::addDeadObject(DeadObject *object)
/******************************************************************************/
/* Function:  Add a dead object to the segment set.                           */
/******************************************************************************/
{
    /* we want to keep our cache sorted by the size. */
    deadCache.addSortedBySize(object);
}


void OldSpaceSegmentSet::addDeadObject(DeadObject *object)
/******************************************************************************/
/* Function:  Add a dead object to the segment set.                           */
/******************************************************************************/
{
    /* we want to keep our cache sorted by the size. */
    deadCache.addSortedBySize(object);
}


void NormalSegmentSet::addDeadObject(DeadObject *object)
/******************************************************************************/
/* Function:  Add a dead object to the segment set.                           */
/******************************************************************************/
{
//  checkObjectOverlap(object);
    size_t length = object->getObjectSize();

    /* if the length is larger than the biggest subpool we */
    /* maintain, we add this to the large block list. */
    if (length > LargestSubpool)
    {
        /* ideally, we'd like to add this sorted by size, but */
        /* this is called so frequently, attempting to sort */
        /* degrades performance by about 10%. */
        largeDead.add(object);
    }
    else
    {
        /* calculate the dead chain          */
        /* and add that to the appropriate chain */
        size_t deadChain = LengthToDeadPool(length);
        subpools[deadChain].addSingle(object);
        /* we can mark this subpool as having items again */
        lastUsedSubpool[deadChain] = deadChain;
    }
}


void MemorySegmentSet::addDeadObject(char *object, size_t length)
/******************************************************************************/
/* Function:  Add a dead object to the segment set.                           */
/******************************************************************************/
{
 // The base class doesn't maintain a dead object pool, so this is a nop
}


void LargeSegmentSet::addDeadObject(char *object, size_t length)
/******************************************************************************/
/* Function:  Add a dead object to the segment set.                           */
/******************************************************************************/
{
#ifdef VERBOSE_GC
    if (length > largestObject)
    {
        largestObject = length;
    }
    if (length < smallestObject)
    {
        smallestObject = length;
    }
#endif
    /* we want to keep our cache sorted by the size. */
    deadCache.addSortedBySize(new (object) DeadObject(length));
}


void OldSpaceSegmentSet::addDeadObject(char *object, size_t length)
/******************************************************************************/
/* Function:  Add a dead object to the segment set.                           */
/******************************************************************************/
{
    /* we want to keep our cache sorted by the size. */
    deadCache.addSortedBySize(new (object) DeadObject(length));
}


void NormalSegmentSet::addDeadObject(char *object, size_t length)
/******************************************************************************/
/* Function:  Add a dead object to the segment set.                           */
/******************************************************************************/
{
    /* if the length is larger than the biggest subpool we */
    /* maintain, we add this to the large block list. */
    if (length > LargestSubpool)
    {
        /* ideally, we'd like to add this sorted by size, but */
        /* this is called so frequently, attempting to sort */
        /* degrades performance by about 10%. */
        largeDead.add(new (object) DeadObject(length));
    }
    else
    {
        /* calculate the dead chain          */
        /* and add that to the appropriate chain */
        size_t deadChain = LengthToDeadPool(length);
        subpools[deadChain].addSingle(new (object) DeadObject(length));
        /* we can mark this subpool as having items again */
        lastUsedSubpool[deadChain] = deadChain;
    }
}


void MemorySegmentSet::addSegment(MemorySegment *segment, bool createDeadObject)
/******************************************************************************/
/* Function:  Add a segment to the segment pool.                              */
/******************************************************************************/
{
    /* we want to keep these segments ordered by address so we can */
    /* potentially combine them later. */
    MemorySegment *insertPosition = anchor.next;
    while (insertPosition->isReal())
    {
        /* we want to insert these in sorted order, by address. */
        /* This allows us to merge segments later, if necessary. */
        if (segment < insertPosition)
        {
            break;
        }
        insertPosition = insertPosition->next;
    }

    /* first check to see if we can merge this with the previous */
    /* segment.  This will give us larger segment sections for reuse. */
    MemorySegment *previous = insertPosition->previous;
    if (previous->isReal() && previous->isAdjacentTo(segment))
    {
        /* just combine this with the previous segment and add the */
        /* entire block as a dead object. */
        size_t deadLength = segment->realSize();
        previous->combine(segment);
        memory->verboseMessage("Combining newly allocated segment of %d bytes to create new segment of %d bytes\n", deadLength, previous->size());
        addDeadObject((char *)segment, deadLength);
    }
    else
    {
        /* insert this into position */
        insertPosition->insertBefore(segment);
        /* Insert the segment's dead space into the proper chain */
        if (createDeadObject)
        {
            DeadObject *ptr = segment->createDeadObject();
            addDeadObject(ptr);
        }
    }
}


DeadObject *MemorySegmentSet::donateObject(size_t allocationLength)
/******************************************************************************/
/* Function:  Search the cache of dead objects for an object of sufficient    */
/* size to fulfill this request.  For the default implementation, we always   */
/* return NULL.  Different segment sets may have different strategies for     */
/* determining which block is most suitable for donation.                     */
/******************************************************************************/
{
    return NULL;
}


DeadObject *NormalSegmentSet::donateObject(size_t allocationLength)
/******************************************************************************/
/* Function:  Search the cache of dead objects for an object of sufficient    */
/* size to fulfill this request.  This just uses normal large block           */
/* allocation strategy to decide on the block to donate, give a best fit      */
/* for the allocation.                                                        */
/******************************************************************************/
{
    /* find a large object and return it */
    return (DeadObject *)findLargeDeadObject(allocationLength);
}


DeadObject *LargeSegmentSet::donateObject(size_t allocationLength)
/******************************************************************************/
/* Function:  Search the cache of dead objects for an object of sufficient    */
/* size to fulfill this request.  This just uses normal large block           */
/* allocation strategy to decide on the block to donate, give a best fit      */
/* for the allocation.                                                        */
/******************************************************************************/
{
    /* find an object in the cache return it.  This will likely be */
    /* larger than the request, as we shouldn't have may small */
    /* objects in the dead chains. */
    return deadCache.findSmallestFit(allocationLength);
}


MemorySegment *MemorySegmentSet::donateSegment(size_t allocationLength)
/******************************************************************************/
/* Function:  Search the set of segments to see if it is possible to donate   */
/* a segment to another segment set.  Our preference is to give up an empty   */
/* segment.  If we don't have an empty one of sufficient size, then we'll     */
/* see if we can split one of our segments to create a new one.               */
/******************************************************************************/
{
    /* first check for the empty one... */
    MemorySegment *segment = findEmptySegment(allocationLength);
    /* if no empties available, we might still be able to split off */
    /* an empty bit. */
    if (segment == NULL)
    {
        segment = splitSegment(allocationLength);
    }
    return segment;
}


MemorySegment *MemorySegmentSet::splitSegment(size_t allocationLength)
/******************************************************************************/
/* Function:  Search the set of large segments looking for a dead block large */
/* enough that we can split the segment and give part to the normal           */
/* allocation heap.   Our preference is a block that is at the start or end   */
/* of a segment, but since we're in a critical memory situation right now,    */
/* any block of sufficient size will do.  NOTE:  This routine can             */
/* only be called immediately after a complete mark and sweep operation has   */
/* been performed.  We are dependent on there having been no object           */
/* allocations since the segment information was updated.                     */
/******************************************************************************/
{
    typedef enum
    {
        NO_SEGMENT, SPLIT_FRONT, SPLIT_REAR, SPLIT_MIDDLE
    } SplitType;

    SplitType split = NO_SEGMENT;
    MemorySegment *candidateSegment = NULL;
    char    *splitBlock = NULL;
    size_t   splitLength = MaximumObjectSize;

    char *objectPtr;

    /* We're basically going to perform a sweep operation on the */
    /* large heap segment looking for a candidate segment to split. */
    /* Our preference is for an empty block at the end of a */
    /* segment.  Second choice is an empty block at the beginning */
    /* of the segment.  Failing either of these, we'll split in the */
    /* middle to create 3 segments. */
    MemorySegment *segment = first();
    while (segment != NULL)
    {
        char *endPtr;
        size_t deadLength;
        /* ok, sweep all of the objects in this segment, looking */
        /* for one large enough. */
        for (objectPtr = segment->start(), endPtr = segment->end(), deadLength = ((RexxObject *)objectPtr)->getObjectSize();
            objectPtr < endPtr;
            objectPtr += deadLength, deadLength = ((RexxObject *)objectPtr)->getObjectSize())
        {
            /* We're only interested in the dead objects.  Note */
            /* that since we've just finished a GC operation, we */
            /* shouldn't see any adjacent dead objects. */
            if (!((RexxObject *)objectPtr)->isObjectLive(memoryObject.markWord))
            {
                /* have we found an empty part large enough to */
                /* convert into a segment? */
                if (deadLength >= allocationLength && deadLength>= MinimumSegmentSize)
                {
                    /* is this at the end of the segment?  This is a */
                    /* perfect candidate.  we'll remember this for */
                    /* later. */
                    if (segment->isLastBlock(objectPtr, deadLength))
                    {
                        /* we might already have a rear split candidate. */
                        /* Take the smaller of the possibilities. */
                        if (split == SPLIT_REAR)
                        {
                            /* already have a smaller one?  We'll just skip this */
                            if (splitLength < deadLength)
                            {
                                break;
                            }
                        }
                        /* record the information, and skip to the next */
                        /* segment. */
                        split = SPLIT_REAR;
                        candidateSegment = segment;
                        splitBlock = objectPtr;
                        splitLength = deadLength;
                        break;
                    }
                    /* This might be at the beginning of the */
                    /* segment.  We only use this if we don't have */
                    /* something more suitable already. */
                    else if (segment->isFirstBlock(objectPtr))
                    {
                        /* if we've already found a rear split, we */
                        /* don't want this one. */
                        if (split == SPLIT_REAR)
                        {
                            continue;
                        }
                        /* we'll also ignore this if we've found a shorter front block */
                        if (split == SPLIT_FRONT && splitLength < deadLength)
                        {
                            continue;
                        }
                        split = SPLIT_FRONT;
                        candidateSegment = segment;
                        splitBlock = objectPtr;
                        splitLength = deadLength;
                        /* we continue rather than break here, */
                        /* because this segment might have a better */
                        /* candidate on the end. */
                        continue;
                    }
                    /* we've found a block on the middle.  This is */
                    /* our last choice, so we only use this if we */
                    /* haven't found anything better. */
                    else
                    {
                        /* we'll also ignore this if we've found a */
                        /* shorter front block */
                        if ((split == SPLIT_MIDDLE && splitLength < deadLength))
                        {
                            continue;
                        }
                        /* have we found any of the other options?  Skip it also. */
                        if (split != NO_SEGMENT)
                        {
                            continue;
                        }
                        split = SPLIT_MIDDLE;
                        candidateSegment = segment;
                        splitBlock = objectPtr;
                        splitLength = deadLength;
                        /* we continue rather than break here, */
                        /* because this segment might have a better */
                        /* candidate on the end. */
                        continue;
                    }
                }
            }
        }
        segment = next(segment);
    }

    /* now we need to process the "best" split candidate block. */
    switch (split)
    {
        /* no block found that we can convert into a segment. */
        /* We'll just have to steal a dead block off of the chain. */
        case NO_SEGMENT: {
                return NULL;
            }
            /* The preferred (and easiest split).  Just remove the */
            /* block from the dead chain, adjust the original segment */
            /* size, and make a new one from this block. */
        case SPLIT_REAR: {
                DeadObject *deadObject = (DeadObject *)splitBlock;
                /* remove this from the dead chain. */
                deadObject->remove();
                /* And turn this into a segment */
                MemorySegment *newSeg = new (splitBlock) MemorySegment(splitLength - MemorySegmentOverhead);
                /* reduce the length of the segment we took this from */
                candidateSegment->shrink(splitLength);
                return newSeg;
            }
            /* split a segment in the front.  We need to create two */
            /* segments for this one. */
        case SPLIT_FRONT: {
                DeadObject *deadObject = (DeadObject *)splitBlock;
                /* remove this from the dead chain. */
                deadObject->remove();
                /* remove the existing segment from the chain. */
                removeSegment(candidateSegment);
                /* calculate the adjusted length of this */
                size_t tailLength = candidateSegment->realSize() - splitLength;
                /* go to the start of the tail end segment we're */
                /* leaving behind.  Note that since we increment the */
                /* length of the starting block from the front of the */
                /* segment, we end up leaving a header space at the */
                /* front of the created tail portion. */
                MemorySegment *tailSegment = (MemorySegment *)( ((char*) candidateSegment) + splitLength);
                /* create two segments out of this */
                MemorySegment *newSeg = new (candidateSegment) MemorySegment(splitLength);
                tailSegment = new (tailSegment) MemorySegment(tailLength);
                /* Anchor new segment at end of list */
                addSegment(tailSegment, false);
                return newSeg;
            }
            /* we're taking a block from the middle of the segment.  We */
            /* need to create segments in front, and back. */
        case SPLIT_MIDDLE: {
                DeadObject *deadObject = (DeadObject *)splitBlock;
                /* remove this from the dead chain. */
                deadObject->remove();
                /* remove the existing segment from the chain. */
                removeSegment(candidateSegment);
                /* get the length of data in the front segment part */
                size_t frontLength = splitBlock - candidateSegment->start();
                /* calculate the size of the data in the tail end piece */
                size_t tailLength = candidateSegment->size() - (frontLength + splitLength);
                /* address the start of the tail segment, accounting */
                /* for the segment header we're adding on to the front */
                /* of it (which comes from the end of the segment block */
                /* we're stealing) */
                MemorySegment *tailSegment = (MemorySegment *)(splitBlock + splitLength - MemorySegmentOverhead);
                /* we need to reduce this by two segment headers...one */
                /* for the segment we're stealing, and one for the */
                /* trailing segment */
                splitLength -= (2 * MemorySegmentOverhead);
                /* create two segments out of this */
                MemorySegment *newSeg = new (splitBlock) MemorySegment(splitLength);
                tailSegment = new (tailSegment) MemorySegment(tailLength);
                candidateSegment = new (candidateSegment) MemorySegment(frontLength);
                /* Anchor the original pieces on the segment chain */
                addSegment(tailSegment, false);
                addSegment(candidateSegment, false);
                return newSeg;
            }
    }
    return NULL;
}


size_t NormalSegmentSet::suggestMemoryExpansion()
/******************************************************************************/
/* Function:  Decide if we should add additional segments to the normal heap  */
/* based on the amount of free space we have after a garbage collection.  If  */
/* we're starting to run full, then we should expand the heap to prevent      */
/* recycling the garbage collector to reclaim minimal amounts of storage that */
/* is immediately reused, causing another GC cycle.                           */
/******************************************************************************/
{
    float freePercent = freeMemoryPercentage();

    memory->verboseMessage("Normal segment set free memory percentage is %d\n", (int)(freePercent * 100.0));

    /* if we are less than 30% full, we should try to expand to the */
    /* 30% mark. */
    if (freePercent < NormalMemoryExpansionThreshold)
    {
        /* get a recommendation on how large the heap should be */
        size_t recommendedSize = recommendedMemorySize();
        size_t newDeadBytes = recommendedSize - liveObjectBytes;
        /* calculate the number of new bytes we need to add reach */
        /* our memory threshold.  It will be the job of others to */
        /* translate this into segment sized units. */
        size_t expansionSize = newDeadBytes - deadObjectBytes;
        return expansionSize;
    }
    /* no expansion required yet. */
    return 0;
}


size_t MemorySegmentSet::suggestMemoryExpansion()
/******************************************************************************/
/* Function:  Decide if we should add additional segments to the normal heap  */
/* based on the amount of free space we have after a garbage collection.  If  */
/* we're starting to run full, then we should expand the heap to prevent      */
/* recycling the garbage collector to reclaim minimal amounts of storage that */
/* is immediately reused, causing another GC cycle.                           */
/******************************************************************************/
{
    /* the base memory set has no expansion algorithm...always return 0 */
    return 0;
}


size_t MemorySegmentSet::suggestMemoryContraction()
/******************************************************************************/
/* Function:  Decide if we should add try to remove memory from a segment set.*/
/* The value returned is the suggested number of bytes we should try to.      */
/* This is only a suggestion based on a best guess, and we'll only remove     */
/* things in full segment units.                                              */
/******************************************************************************/
{
    /* the base memory set has no contraction algorithm...always return 0 */
    return 0;
}


size_t NormalSegmentSet::suggestMemoryContraction()
/******************************************************************************/
/* Function:  Decide if we should add try to remove memory from a segment set.*/
/* The value returned is the suggested number of bytes we should try to.      */
/* This is only a suggestion based on a best guess, and we'll only remove     */
/* things in full segment units.                                              */
/******************************************************************************/
{
    float freePercent = freeMemoryPercentage();

    /* if we have predominately free space in the heap, we should try to give some back */
    if (freePercent > NormalMemoryContractionThreshold)
    {
        /* if we're still working on our initial allocation set, we */
        /* don't want to try to shrink that. */
        if (totalFreeMemory() <= InitialNormalSegmentSpace)
        {
            return 0;
        }
        /* Calculate an amount to shrink this by.  If it ends up */
        /* being a small amount, we'll live with the overage. */
        return totalFreeMemory() - recommendedMaximumMemorySize();
    }
    /* no expansion required yet. */
    return 0;
}


void MemorySegmentSet::adjustMemorySize()
/******************************************************************************/
/* Function:  Decide if we should add or removed segments from the segment    */
/* set based on the current state of the heap immediately following a sweep.  */
/* We want to avoid thrashing the garbage collector because of chronic low    */
/* memory conditions.  If we wait until we get an actual allocation failure,  */
/* we will end up in a state where we're constantly running a mark-and-sweep  */
/*to  reclaim just a tiny amount of storage, and then repeat.  We will try to */
/* keep the heap with appropriate free space limits.                          */
/*                                                                            */
/* Conversely, if we find ourselves with a lot of empty space, we'll see if   */
/* we can remove segments from the segment set.  For the small allocation     */
/* pools, it is not likley we'd be able to do this, but since we try to       */
/* maintain large blocks whenever possible, it can happen.  For the larger    */
/* allocation pool, we are more likely to end up with empty segments we can   */
/* give back.                                                                 */
/******************************************************************************/
{
    /* see if a proactive expansion is advised, based on our */
    /* current state. */
    size_t suggestedExpansion = suggestMemoryExpansion();
    /* if we've decided we need to expand,  */
    if (suggestedExpansion > 0)
    {
        /* go add as many segments as are required to reach that */
        /* level. */
        memory->verboseMessage("Expanding normal segment set by %d\n", suggestedExpansion);
        addSegments(suggestedExpansion);
    }
#if 0
//  else {
        /* get a suggested contraction amount.  There is no sense */
        /* in trying to release less than a segment. */
//      size_t suggestedContraction = suggestMemoryContraction();
//      if (suggestedContraction >= SegmentDeadSpace) {
            /* see if we can't release some of the space we have. */
//          releaseEmptySegments(suggestedContraction);
//      }
//  }
#endif
}


void MemorySegmentSet::releaseEmptySegments(size_t releaseSize)
/******************************************************************************/
/* Function:  Scan the loaded segments, searching for empty segments that     */
/* can be released.  We will release up to the maximum recommended amount.    */
/******************************************************************************/
{
    /* round this up to the next segment boundary. We're already at */
    /* a pretty good overage point, so we can afford to round this */
    /* up. */
    releaseSize = roundSegmentBoundary(releaseSize);
    MemorySegment *segment = first();
    for (;segment != NULL; segment = next(segment))
    {
        /* is this an empty segment that fits within our criteria? */
        if (segment->isEmpty() && segment->size() <= releaseSize)
        {
            /* we need to step back one segment for the looping, as */
            /* we're going to cut this segment out from the herd. */
            MemorySegment *previous = segment->previous;
            /* remove this from the pool */
            removeSegmentAndStorage(segment);
            /* return this segment to the memory pool */
            releaseSegment(segment);
            /* pick up the loop with the previous segment (which */
            /* will immediately step to the segment following the */
            /* one we just released). */
            segment = previous;
        }
    }
}


void MemorySegmentSet::releaseSegment(MemorySegment *segment)
{
    /* just move this to the empty segment chain */
    emptySegments.insertBefore(segment);
    /* the count is only of the active segments */
    count--;
}


void MemorySegmentSet::addSegments(size_t requiredSpace)
/******************************************************************************/
/* Function:  Add segments to the segment set until the required space is     */
/* achieved or we've had a real memory failure.  Our preference is to add this*/
/* in one chunk, but we'll subdivide down to smaller units as long as we're   */
/* still getting the segments we require.                                     */
/******************************************************************************/
{
    /* this may take several passes to achieve */
    for (;;)
    {
        /* figure out how large this should be */
        size_t segmentSize = calculateSegmentAllocation(requiredSpace);
        size_t minSize = segmentSize >= LargeSegmentDeadSpace ? LargeSegmentDeadSpace : SegmentDeadSpace;
        /* try to allocate a new segment */
        MemorySegment *newSeg = allocateSegment(segmentSize, minSize);
        if (newSeg == NULL)
        {
            /* if we already failed to get our "minimum minimum", just quit now. */
            if (minSize == SegmentDeadSpace)
            {
                return;
            }
            /* try for a single segment allocation.  If that */
            /* fails...we have nothing else we can do. */
            newSeg = allocateSegment(SegmentDeadSpace, SegmentDeadSpace);
            if (newSeg == NULL)
            {
                return;
            }
        }

        /* we have a segment.  Add this to the segment pool */
        /* (and add the dead space to the available memory). */
        addSegment(newSeg);
        segmentSize = newSeg->size();
        /* if we've got what's needed, we're out of here. */
        if (segmentSize >= requiredSpace)
        {
            return;
        }
        /* reduce the size and try for some more */
        requiredSpace -= segmentSize;
    }
}


void MemorySegmentSet::prepareForSweep()
/******************************************************************************/
/* Function:  Handle all segment set pre-sweep initialization.                */
/******************************************************************************/
{
    liveObjectBytes = 0;
    deadObjectBytes = 0;
}


void NormalSegmentSet::prepareForSweep()
/******************************************************************************/
/* Function:  Handle all segment set pre-sweep initialization.                */
/******************************************************************************/
{
    MemorySegmentSet::prepareForSweep();

    /* we're about to rebuild the dead chains during the sweep, so */
    /* initialize all of these now. */
    largeDead.empty();
    for (int i = FirstDeadPool; i < DeadPools; i++)
    {
        subpools[i].emptySingle();
    }
}


void LargeSegmentSet::prepareForSweep()
/******************************************************************************/
/* Function:  Handle all segment set pre-sweep initialization.                */
/******************************************************************************/
{
    MemorySegmentSet::prepareForSweep();

    /* we're about to rebuild the dead chains during the sweep, so */
    /* initialize all of these now. */
    deadCache.empty();
    requests = 0;
    largestObject = 0;
    smallestObject = 999999999;
}


void MemorySegmentSet::completeSweepOperation()
/******************************************************************************/
/* Function:  Handle all segment set post sweep activities.                   */
/******************************************************************************/
{
}


void NormalSegmentSet::completeSweepOperation()
/******************************************************************************/
/* Function:  Handle all segment set post sweep activities.                   */
/******************************************************************************/
{
    /* Now we can optimize the look-aside entries for the small */
    /* dead chains.  By checking to see which chains have blocks in */
    /* them, we can potentially skip a lot of check/searching on an */
    /* allocation request. */
    for (int i = FirstDeadPool; i < DeadPools; i++)
    {
        if (!subpools[i].isEmptySingle())
        {
            /* point this back at itself */
            lastUsedSubpool[i] = i;
        }
        else
        {
            /* default to the "skip to the end" location */
            int usePool = DeadPools;
            int j;
            /* scan all of the higher pools looking for the first hit */
            for (j = i + 1; j < DeadPools; j++)
            {
                if (!subpools[i].isEmptySingle())
                {
                    usePool = j;
                    break;
                }
            }

            /* this will now be guaranteed to go to the correct */
            /* location on each request. */
            lastUsedSubpool[i] = usePool;
        }
    }
}


void LargeSegmentSet::completeSweepOperation()
/******************************************************************************/
/* Function:  Handle all segment set post sweep activities.                   */
/******************************************************************************/
{
#ifdef VERBOSE_GC
    memory->verboseMessage("Large segment sweep complete.  Largest block is %d, smallest block is %d\n", largestObject, smallestObject);
#endif
}

inline bool objectIsLive(char *obj, size_t mark) {return ((RexxObject *)obj)->isObjectLive(mark); }
inline bool objectIsNotLive(char *obj, size_t mark) {return ((RexxObject *)obj)->isObjectDead(mark); }

void MemorySegmentSet::sweep()
/******************************************************************************/
/*                                                                            */
/* This routine does a sweep of all segments devoted to "normal" size storage */
/* allocations.                                                               */
/******************************************************************************/
{
    MemorySegment *sweepSegment;
    char  *objectPtr, *endPtr, *nextObjectPtr;
    size_t deadLength;
    size_t bytes;
    size_t mark = memoryObject.markWord;

    /* do the sweep preparation (this differs for particular segment */
    /* set implemenation) */
    prepareForSweep();
    /* go through the segments in order, until we've swept the */
    /* entire set */
    sweepSegment = first();
    while (sweepSegment != NULL)
    {
        /* clear our live objects counter    */
        sweepSegment->liveObjects = 0;
        /* for all objects in segment */
        for (objectPtr = sweepSegment->start(), endPtr = sweepSegment->end(); objectPtr < endPtr; )
        {
            /* this a live object?               */
            if (objectIsLive(objectPtr, mark))
            {
                /* Get size of object for stats and  */
                bytes = ((RexxObject *)objectPtr)->getObjectSize();
                /* do any reference checking         */
                validateObject(bytes);
                /* update our tracking counters */
                liveObjectBytes += bytes;
                /* point to next object in segment.  */
                objectPtr += bytes;
                /* bump the live object counter      */
                sweepSegment->liveObjects++;
            }

            else
            {
                /* get the object's size */
                deadLength = ((RexxObject *)objectPtr)->getObjectSize();
                /* do any reference checking         */
                validateObject(deadLength);

                for (nextObjectPtr = objectPtr + deadLength;
                    (nextObjectPtr < endPtr) && objectIsNotLive(nextObjectPtr, mark);
                    nextObjectPtr += bytes)
                {
                    /* get the object size */
                    bytes = ((RexxObject *)nextObjectPtr)->getObjectSize();
                    /* do any reference checking         */
                    validateObject(bytes);
                    /* add in the size of this dead body */
                    deadLength += bytes;
                }
                /* add this to the dead counters     */
                deadObjectBytes += deadLength;
                /* now add to the dead chain */
                addDeadObject((char *)objectPtr, deadLength);
                /* update object Pointers.           */
                objectPtr += deadLength;
            }
        }
        /* go to the next segment in the pool*/
        sweepSegment = next(sweepSegment);
    }
    /* now do any of the sweep post-processing. */
    completeSweepOperation();
}


void MemorySegmentSet::collectEmptySegments()
/******************************************************************************/
/* Function:  Scan a segment set after a sweep operation, collecting          */
/* any empty segments.  This is a nop for the base MemorySegmentSet class.    */
/******************************************************************************/
{
}


RexxObject *MemorySegmentSet::splitDeadObject(
    DeadObject *object,                /* the object we're splitting */
    size_t allocationLength,           /* the request length, already rounded to an appropriate boundary */
    size_t splitMinimum)               /* the minimum size we're willing to split */
/******************************************************************************/
/* Function:  Process a dead object and potentially split the object into     */
/* an allocated object and a second dead object that will be added back into  */
/* the appropriate pool of dead objects.                                      */
/******************************************************************************/
{
    size_t deadLength = object->getObjectSize() - allocationLength;
    /* we need to keep all of these sizes as ObjectGrain multiples, */
    /* so round it down...the allocation will get all of the extra. */
    /* deadLength = rounddown(deadLength, ObjectGrain);             */

    /* remainder too small or this is a very large request */
    /* is the remainder two small to reuse? */
    if (deadLength < splitMinimum)
    {
        /* over allocate this object */
        allocationLength += deadLength;
    }
    else
    {
        /* Yes, so pull new object out of the front of the dead */
        /* object, adjust the size of the Dead object.  We want */
        /* to use the front rather than the back so that if we */
        /* hit the need to split a segment because of low memory */
        /* conditions, we increase the probability that we'll be */
        /* able to use the end of the segment. */
        DeadObject *largeObject = (DeadObject *)(((char *)object) + allocationLength);
        /* and reinsert into the the dead object pool */
        addDeadObject((char *)largeObject, deadLength);
    }
    /* Adjust the size of this object to the requested allocation length */
    ((RexxObject *)object)->setObjectSize(allocationLength);
    return(RexxObject *)object;
}


RexxObject *NormalSegmentSet::findLargeDeadObject(
    size_t allocationLength)           /* the request length, already rounded to an appropriate boundary */
/******************************************************************************/
/* Function:  Search the segment set for a block of the requested length.     */
/* If a block is found, the block will be subdivided if necessary, with the   */
/* remainder portion placed back on the proper chain.                         */
/******************************************************************************/
{
    /* Go through the LARGEDEAD object looking for the 1st */
    /* one we can use either our object is too big for all the */
    /* small chains, or the small chains are depleted.... */
    DeadObject *largeObject = largeDead.findFit(allocationLength);
    if (largeObject != NULL)
    {         /* did we find an object?            */
        /* potentially split this object into a smaller unit so we */
        /* can reuse the remainder. */
        return splitDeadObject(largeObject, allocationLength, MinimumObjectSize);
    }
    return OREF_NULL;
}


RexxObject *NormalSegmentSet::findObject(size_t allocationLength)
/******************************************************************************/
/* Function:  Find a dead object in the caches for the NormalSegmentSet.      */
/******************************************************************************/
{
    /* this is just a non-inline version of the allocation routine */
    /* routine to be used during low memory situations. */
    return allocateObject(allocationLength);
}


RexxObject *NormalSegmentSet::handleAllocationFailure(size_t allocationLength)
/******************************************************************************/
/* Function:  Allocate an object from the normal object segment pool.         */
/******************************************************************************/
{
    /* Step 2, force a GC */
    memory->collect();
    /* now that we have good GC data, decide if we need to adjust */
    /* the heap size. */
    adjustMemorySize();
    /* Step 3, see if can allocate now */
    RexxObject *newObject = findObject(allocationLength);
    /* still no luck?                    */
    if (newObject == OREF_NULL)
    {
        /* it is possible that the adjustment routines did not add */
        /* anything because, yet we were unable to allocate a block */
        /* because we're highly fragmented.  Try adding a new segment */
        /* now, before going to the extreme methods. */
        addSegments(SegmentSize);
        /* see if can allocate now */
        newObject = findObject(allocationLength);
        /* still no luck?                    */
        if (newObject == OREF_NULL)
        {
            /* We might be able to steal something from the */
            /* large allocations.  This will also steal the recovery */
            /* segment as a last ditch effort */
            memory->scavengeSegmentSets(this, allocationLength);
            /* see if can allocate now */
            newObject = findObject(allocationLength);
            /* still no luck?                    */
            if (newObject == OREF_NULL)
            {
                /* absolute last chance to fix this.  We allocated a */
                /* recovery segment at start up and have been hiding this */
                /* in our back pocket.  It is now time to bring this into */
                /* play, because we're running on fumes! */
                if (recoverSegment != NULL)
                {
                    addSegment(recoverSegment);
                    recoverSegment = NULL;
                    /* And try to find it once more */
                    newObject = findObject(allocationLength);
                }
                if (newObject == OREF_NULL)
                {
                    /* can't allocate, report resource error. */
                    reportException(Error_System_resources);
                }
            }
        }
    }
    return newObject;
}


RexxObject *LargeSegmentSet::findObject(size_t allocationLength)
/******************************************************************************/
/* Function:  Locate an object is the dead object cache maintained for large  */
/* object allocations.                                                        */
/******************************************************************************/
{
    /* this is just a space-saving non-inline version of allocateObject */
    return allocateObject(allocationLength);
}


RexxObject *LargeSegmentSet::handleAllocationFailure(size_t allocationLength)
/******************************************************************************/
/* Function:  Allocate a large block of memory.  This will manage allocation  */
/* of larger memory requests.  This will search our large block allocation    */
/* chains, and if a block is not found, will decide whether to immediately    */
/* allocate additional memory for large allocations or force a garbage        */
/* collection cycle.  As a last gasp effort, we'll also try to "borrow" a     */
/* block of memory from the pool reserved for smaller allocations.            */
/******************************************************************************/
{
    /* Step 2, decide to expand the heap or GC, or both */
    expandOrCollect(allocationLength);
    /* Step 3, see if can allocate now */
    RexxObject *newObject = findObject(allocationLength);
    if (newObject == OREF_NULL)
    {    /* still no luck?                    */
        /* Step 4, force a heap expansion, if we can */
        expandSegmentSet(allocationLength);
        /* merge the expanded segments into our current set. */
        /* This could potentially prolong the interval between */
        /* collection cycles. */
        mergeSegments(allocationLength);
        /* Step 5, see if can allocate now */
        newObject = findObject(allocationLength);
        /* still no luck?                    */
        if (newObject == OREF_NULL)
        {
            /* We'll take one last chance.  We don't really like */
            /* to do this, but we might be able to steal a block */
            /* off of the normal allocation chain. */
            memory->scavengeSegmentSets(this, allocationLength);
            /* Last chance, see if can allocate now */
            newObject = findObject(allocationLength);
            if (newObject == OREF_NULL)
            {/* still no luck?                    */
                /* can't allocate, report resource error. */
                reportException(Error_System_resources);
            }
        }
    }
    /* if we got a real object, count this.... */
    if (newObject != OREF_NULL)
    {
        requests++;
    }
    /* return our allocated object       */
    return newObject;
}


RexxObject *OldSpaceSegmentSet::findObject(size_t allocationLength)
/******************************************************************************/
/* Function:  Locate an object is the dead object cache maintained for large  */
/* object allocations.                                                        */
/******************************************************************************/
{
    /* go through the LARGEDEAD object looking for the 1st one we can */
    /* use either our object is too big for all the small chains, or */
    /* the small chain are depleted.... */
    DeadObject *largeObject = deadCache.findFit(allocationLength);
    /* did we find an object?            */
    if (largeObject != NULL)
    {
        /* split and prepare this object for use */
        return splitDeadObject(largeObject, allocationLength, LargeAllocationUnit);
    }
    return OREF_NULL;                    /* we couldn't get this              */
}


RexxObject *OldSpaceSegmentSet::allocateObject(size_t requestLength)
/******************************************************************************/
/* Function:  Allocate an object from the old space.  This is a very simple   */
/* management strategy, with no heroic efforts made to locate memory.  Since  */
/* objects here can only come from the OldSpace area, there is no garbage     */
/* collection forced for failures.                                            */
/******************************************************************************/
{
    /* round this allocation up to the appropriate large boundary */
    size_t allocationLength = roundLargeObjectAllocation(requestLength);
    /* Step 1, try to find an object in the current heap.  We don't */
    /* try to round the object size up at all.  This will take place */
    /* once we've found a fit.  The allocations will be rounded up to */
    /* the next appropriate boundary. */
    RexxObject *newObject = findObject(allocationLength);
    /* can't allocate one?               */
    if (newObject == OREF_NULL)
    {
        /* add space to this and try again */
        newSegment(allocationLength, allocationLength);
        /* Step 3, see if can allocate now */
        newObject = findObject(allocationLength);
    }
    /* return our allocated object       */
    return newObject;
}


MemorySegment *MemorySegmentSet::largestActiveSegment()
/******************************************************************************/
/* Function:  Find the largest segment in the segment set.                    */
/******************************************************************************/
{
    MemorySegment *largest = &anchor;
    MemorySegment *segment;

    for (segment = anchor.next; segment->isReal(); segment = segment->next)
    {
        if (segment->size() > largest->size())
        {
            largest = segment;
        }
    }

    /* we return this unconditionally, as our anchor segment will */
    /* report a size of zero. */
    return largest;
}


MemorySegment *MemorySegmentSet::largestEmptySegment()
/******************************************************************************/
/* Function:  Find the largest empty segment in the segment set.              */
/******************************************************************************/
{
    MemorySegment *largest = &emptySegments;
    MemorySegment *segment;

    for (segment = emptySegments.next; segment->isReal(); segment = segment->next)
    {
        if (segment->size() > largest->size())
        {
            largest = segment;
        }
    }

    /* we return this unconditionally, as our anchor segment will */
    /* report a size of zero. */
    return largest;
}


void LargeSegmentSet::expandOrCollect(
    size_t allocationLength)
/******************************************************************************/
/* Function:  Handle the decision process for a failed large block allocation.*/
/* This function will decide whether to force a garbage collection, or add    */
/* additional segments to the heap.  The cheaper alternative is to avoid      */
/* forcing the mark and sweep operations, so we see if we can predict if a    */
/* garbage collection is likely to fail to find a block large enough.         */
/******************************************************************************/
{
    /* first check our set of empty segments.  If we have an empty */
    /* in our list big enough for this, we'll just reactivate it, */
    /* and go on.  We don't need to (and shouldn't) force a garbage */
    /* collection yet. */
    MemorySegment *largestEmpty = largestEmptySegment();
    if (largestEmpty->size() > allocationLength)
    {
        /* just move this over, and get out of here...we should be */
        /* able to satisfy this request now. */
        MemorySegment *segment = findEmptySegment(allocationLength);
        addSegment(segment);
        return;
    }

    MemorySegment *largestActive = largestActiveSegment();

    /* if our largest segment can't hold this block, then this is a */
    /* certain failure.  It is time to add more segments for large */
    /* allocations. */
    if (largestActive->size() < allocationLength)
    {
        expandSegmentSet(allocationLength);
        return;
    }
    /* we can get into a situation where we need to expand */
    /* frequently.  If we've reached that point, just add */
    /* additional memorhy. */
    if (requests <= MemoryThrashingThreshold)
    {
        expandSegmentSet(allocationLength);
        /* we only do the expansion once per cycle for this.  Bump */
        /* the requests count up so we don't keep expanding without */
        /* a collection. */
        requests = MemoryThrashingThreshold + 1;
        return;
    }
    else
    {
        /* make sure all of our empty segments are included.  This */
        /* will allow us to merge these into larger blocks. */
        activateEmptySegments();
        /* go ahead and try to reclaim everything.  We may still */
        /* end up expanding after this is done. */
        memory->collect();
        /* see if we can merge segments together to create a block large enough to satisfy this request. */
        mergeSegments(allocationLength);
    }
}


void MemorySegmentSet::mergeSegments(size_t allocationLength)
/******************************************************************************/
/* Function:  Attempt to merge the set of large segments to create a dead     */
/* block of at least the requested size.  This process can only be done       */
/* immediately after a complete GC cycle has been performed, as we need to    */
/* have accurate information about the live and dead blocks within the        */
/* segments.                                                                  */
/*                                                                            */
/* We'll attempt the merger in multiple passes to see if we can create a      */
/* block large enough.  If we already have a block of at least this size on   */
/* the chains, then we'll do nothing.                                         */
/*                                                                            */
/* For the first pass, we'll combine adjacent empty segments to create a      */
/* larger block.  If the first pass fails, then we'll try to find adjacent    */
/* segments with trailing and leading deadblocks we can combine into a larger */
/* unit (possibly including intervening empty segments).                      */
/******************************************************************************/
{
    /* check our largest block.  If we can allocate something of */
    /* this length, we don't combine empties now.  Deferring this */
    /* makes it easier for use to give segments back. */
    MemorySegment *largestEmpty = largestActiveSegment();
    if (largestEmpty->size() > allocationLength)
    {
        return;
    }

    /* the segments are sorted in ascending order, so we can easily */
    /* find the adjacent segments. */
    MemorySegment *segment;
    /* scan the entire list */
    for (segment = anchor.next; segment->isReal(); segment = segment->next)
    {
        if (segment->isEmpty())
        {
            MemorySegment *nextSeg = segment->next;
            /* loop until we we've collapsed all of the adjacent */
            /* empty segments */
            for (;segment->isAdjacentTo(nextSeg) && nextSeg->isEmpty(); nextSeg = segment->next)
            {
                memory->verboseMessage("Combining two empty segments\n");
                /* combine these segments */
                combineEmptySegments(segment, nextSeg);
            }
        }
    }

    /* Check our largest block again.  If combining the empty */
    /* segments created a new large block, we're finished.  If not, */
    /* then we've got a hard job ahead of us */
    largestEmpty = largestActiveSegment();
    if (largestEmpty->size() > allocationLength)
    {
        return;
    }


    /* scan the entire list again, looking for opportunities to */
    /* combine partially empty segments.  This could potentially */
    /* create a contiguous block large enough for our purposes. */
    for (segment = anchor.next; segment->isReal(); segment = segment->next)
    {
        /* this segment is only a candidate for merging if it has a */
        /* dead object on the end (fairly common, since we allocate */
        /* from the front) */
        DeadObject *lastBlock = segment->lastDeadObject();
        MemorySegment *emptySegment = NULL;
        MemorySegment *tailSegment = NULL;
        if (lastBlock != NULL)
        {
            /* we only do this if we can get a block of sufficient */
            /* size for the request we've received.  So see how */
            /* much we can reclaim first. */
            size_t deadLength = lastBlock->getObjectSize();
            /* now go to the next segment, but only continue if */
            /* they abutt */
            MemorySegment *nextSeg = segment->next;
            if (!segment->isAdjacentTo(nextSeg) || !nextSeg->isReal())
            {
                continue;
            }
            /* we could have a single empty segment after us, as */
            /* we've already merged multiples. */
            if (nextSeg->isEmpty())
            {
                /* add the size in and continue */
                deadLength += nextSeg->realSize();
                emptySegment = nextSeg;
                nextSeg = nextSeg->next;
            }
            /* we should now be looking at a potential merger */
            /* candidate. */
            if (segment->isAdjacentTo(nextSeg) && nextSeg->isReal())
            {
                /* see if we have an empty block at the front of this */
                DeadObject *firstBlock = nextSeg->firstDeadObject();
                if (firstBlock != NULL)
                {
                    deadLength += firstBlock->getObjectSize() + MemorySegmentOverhead;
                    tailSegment = nextSeg;
                }
            }
            /* start by removing the last block from the deadchain. */
            lastBlock->remove();
            /* if there is an intervening empty segment, merge this */
            /* into this segment. */
            if (emptySegment != NULL)
            {
                /* remove all of the dead blocks from the chain */
                emptySegment->removeAll();
                /* remove the segment, and combine with this one. */
                removeSegment(emptySegment);
                segment->combine(emptySegment);
            }
            /* we may be able to combine the front block  */
            if (tailSegment != NULL)
            {
                /* remove the first dead block from the chain */
                tailSegment->firstDeadObject()->remove();
                /* merge these two segments together. */
                removeSegment(tailSegment);
                segment->combine(tailSegment);
                memory->verboseMessage("Non-empty segments combined to create segment of %d bytes\n", segment->size());
                /* Now that this has been combined with one or more */
                /* segments, the merged segment may still be a */
                /* candidate for one more level of merge.  Step to the */
                /* previous one so we take a second look at this. */
                segment = segment->previous;
            }

            /* finally resize this block to the combined size. */
            lastBlock->setObjectSize(deadLength);
            /* add this back into the chain as appropriate. */
            addDeadObject(lastBlock);
        }
    }
}


void MemorySegmentSet::combineEmptySegments(
    MemorySegment *front,
    MemorySegment *back)
/******************************************************************************/
/* Function:  Merge two empty segments into a single segment and add the block*/
/* create to the dead object cache.                                           */
/******************************************************************************/
{
    DeadObject *frontObject = front->firstObject();
    DeadObject *backObject = back->firstObject();

    /* remove the dead space for these two objects from the cache */
    frontObject->remove();
    backObject->remove();

    /* remove the back segment */
    removeSegment(back);

    /* add the space to the front segment */
    front->combine(back);

    memory->verboseMessage("Two segments combined to create segment of %d bytes\n", front->size());
    /* and add the resulting dead object to the cache */
    DeadObject *ptr = front->createDeadObject();
    addDeadObject(ptr);
}


void LargeSegmentSet::expandSegmentSet(
    size_t allocationLength)
/******************************************************************************/
/* Function:  Expand the size of the large allocation heap by adding an       */
/* additional segment.  We'll employ three different strategies for adding    */
/* a segment to the heap.  For "really, really big" requests, we'll just      */
/* round to the a page boundary and allocate.                                 */
/*                                                                            */
/* For requests larger than a single segment, but not large enough  to qualify*/
/* "really, really big", we'll round up to the next "convenient segment" to   */
/* add some additional usable storage.  The convenience factor will           */
/* over allocate by at least SEGMENT/2 bytes.  If the overallocation request  */
/* fails, then we'll just round up to the page boundary and try again.        */
/*                                                                            */
/* For the smaller allocations, we'll add this heap space in LARGESEGMENT     */
/* increments so we can get more suballocations before needing to force       */
/* another GC cycle.  If the attempt to allocate a LARGESEGMENT fails, we'll  */
/* try again with just a single SEGMENT size allocation.                      */
/******************************************************************************/
{
    /* do we have a really big request? */
    if (allocationLength > LargeSegmentDeadSpace)
    {
        /* we allocate this as the size we need.  No sense in */
        /* trying to over allocate here, as we'd like to free */
        /* this up as soon as we can. */
        memory->verboseMessage("Expanding large segment set by %d\n", allocationLength);
        newSegment(allocationLength, allocationLength);
    }
    /* for the smaller of the large blocks, we expand by a full */
    /* LargeSegment so we can fit more allocations in a segment. */
    /* If we can't get a full real on, then fall back to the */
    /* smaller size segment. */
    else if (allocationLength < SegmentDeadSpace)
    {
        memory->verboseMessage("Expanding large segment set by %d\n", LargeSegmentDeadSpace);
        newSegment(LargeSegmentDeadSpace, SegmentDeadSpace);
    }
    /* we've got a "tweener" block.  We'll round up to the next */
    /* segment boundary to give a little extra.  If the extra is */
    /* less than half a segment in size, add one more segment unit */
    /* to the allocation to increase the probability we'd be able */
    /* to use the extra. */
    else
    {
        size_t requestLength = roundSegmentBoundary(allocationLength);
        if ((requestLength - allocationLength) < MinimumSegmentSize)
        {
            requestLength += SegmentDeadSpace;
        }
        memory->verboseMessage("Expanding large segment set by %d\n", requestLength);
        newSegment(requestLength, allocationLength);
    }
}


void MemorySegmentSet::gatherStats(MemoryStats *memStats, SegmentStats *stats)
/******************************************************************************/
/* Function:  Accumulate memory statistics for a segment                      */
/******************************************************************************/
{
    stats->count = count;

    MemorySegment *seg;
    for (seg = first(); seg != NULL; seg = next(seg))
    {
        seg->gatherObjectStats(memStats, stats);
        stats->largestSegment = Numerics::maxVal(stats->largestSegment, seg->size());
        stats->smallestSegment = Numerics::maxVal(stats->smallestSegment, seg->size());
    }
}


void MemorySegment::markAllObjects()
/******************************************************************************/
/* Function:  Perform a marking operation on all objects in a segment         */
/******************************************************************************/
{
    char *op, *ep;
    for (op = start(), ep = end(); op < ep; )
    {
        /* mark behaviour live               */
        memory_mark_general(((RexxObject *)op)->behaviour);

        /* Does this object have other Obj   */
        /*refs?                              */
        if (((RexxObject *)op)->hasReferences())
        {
            /*  yes, Then lets mark them         */
            ((RexxObject *)op)->liveGeneral(RESTORINGIMAGE);
        }
        op += ((RexxObject *)op)->getObjectSize();   /* move to next object               */
    }
}


void OldSpaceSegmentSet::markOldSpaceObjects()
/******************************************************************************/
/* Function:  Perform a marking operation on all OldSpace objects             */
/******************************************************************************/
{
    /* mark the restored image segment */
    MemorySegment *segment;
    for (segment = first(); segment != NULL; segment = next(segment))
    {
        segment->markAllObjects();
    }
}


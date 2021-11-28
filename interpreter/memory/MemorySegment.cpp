/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2021 Rexx Language Association. All rights reserved.    */
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
/* Primitive Memory segment management                                        */
/*                                                                            */
/******************************************************************************/
#include "RexxCore.h"
#include "ActivityManager.hpp"

#include <stdio.h>

// the threshold to trigger expansion of the normal segment set.
const double NormalSegmentSet::NormalMemoryExpansionThreshold = .30;
// the threshold to trigger expansion of the large segment set. Since these
// tend to be large blocks and can be frequently longer lived, we try to run with
// a little more head room than the normal set
const double LargeSegmentSet::LargeMemoryExpansionThreshold = .40;
// the absolute smallest segment we will allocate.
const size_t MemorySegmentSet::MinimumSegmentSize = (MemorySegment::SegmentSize/2);
// amount of usable space in a minimum sized segment
const size_t MemorySegmentSet::MinimumSegmentDeadSpace = (MinimumSegmentSize - MemorySegment::MemorySegmentOverhead);
// the large segment set handles objects up to segment size, so lets set the default
// large enough to handle a few of them.
const size_t MemorySegmentSet::LargeSegmentSize = (MemorySegment::SegmentSize * 4);;
// allocation available in a default segment
const size_t MemorySegmentSet::SegmentDeadSpace = (MemorySegment::SegmentSize - MemorySegment::MemorySegmentOverhead);
// space available in a larger allocation.
const size_t MemorySegmentSet::LargeSegmentDeadSpace = (LargeSegmentSize - MemorySegment::MemorySegmentOverhead);
const size_t NormalSegmentSet::InitialNormalSegmentSpace = ((MemorySegment::SegmentSize * 12) - MemorySegment::MemorySegmentOverhead);
const size_t LargeSegmentSet::InitialLargeSegmentSpace = (LargeSegmentSize - MemorySegment::MemorySegmentOverhead);

/**
 * Dump information about an individual segment
 *
 * @param owner    The name of the owning segment set.
 * @param counter  The current object counter.
 * @param keyfile  The output file for the dump information.
 * @param dumpfile The full dump file.
 */
void MemorySegment::dump(const char *owner, size_t counter, FILE *keyfile, FILE *dumpfile)
{
                                     /* print header for segment          */
    fprintf(stderr,"Dumping %s Segment %zu from %p for %zu\n", owner, counter, &segmentStart, segmentSize);
                                     /* now dump the segment              */
    fprintf(keyfile, "%s addr.%zu = %p\n", owner, counter, &segmentStart);
    fprintf(keyfile, "%s size.%zu = %zu\n", owner, counter, segmentSize);
    fwrite(&segmentStart, 1, segmentSize, dumpfile);
}


/**
 * Accumulate memory statistics for a segment
 *
 * @param memStats The memory stats accumulator
 * @param stats    The segment stats accumulator
 */
void MemorySegment::gatherObjectStats(MemoryStats *memStats, SegmentStats *stats)
{
    RexxInternalObject *op = startObject();
    RexxInternalObject *ep = endObject();

    // for all objects in this segment, record the object information
    for (; op < ep; op = op->nextObject())
    {
        stats->recordObject(memStats, op);
    }
}


/**
 * Dump information about the each of the segments
 *
 * @param keyfile  The keyfile for the output
 * @param dumpfile The dumpfile for dump information.
 */
void MemorySegmentSet::dumpSegments(FILE *keyfile, FILE *dumpfile)
{
    size_t counter = 0;

    for (MemorySegment *segment = first(); segment != NULL; segment = next(segment))
    {
        segment->dump(name, ++counter, keyfile, dumpfile);
    }
}


/**
 * Dump profile informaton about a segment set (empty for base
 * class)
 *
 * @param outfile The profile output file.
 */
void MemorySegmentSet::dumpMemoryProfile(FILE *outfile)
{
}


/**
 * Dump profile informaton about the large allocation segment set
 *
 * @param outfile The target dump file.
 */
void LargeSegmentSet::dumpMemoryProfile(FILE *outfile)
{
#ifdef MEMPROFILE                      /* doing memory profiling            */
    fprintf(outfile, "Memory profile for large object allocations\n\n");
    // all the work is done by the dead caches
    deadCache.dumpMemoryProfile(outfile);
#endif
}


/**
 * Dump profile informaton about the single object allocation
 * segment set
 *
 * @param outfile The target dump file.
 */
void SingleObjectSegmentSet::dumpMemoryProfile(FILE *outfile)
{
#ifdef MEMPROFILE                      /* doing memory profiling            */
    fprintf(outFile, "Statistics for Largest Object Pool %s\n, id");
    fprintf(outFile, "    Total memory allocations:  %zu\n", allocationCount);
    fprintf(outFile, "    Segments return to system:  %zu\n", returnCount);
    fprintf(outFile, "    Largest object allocated:  %zu\n", largestObject);
    fprintf(outFile, "    Smallest object allocated:  %zu\n", smallestObject);
#endif
}


/**
 * Dump profile informaton about the normal allocation segment set
 *
 * @param outfile The target dump file.
 */
void NormalSegmentSet::dumpMemoryProfile(FILE *outfile)
{
#ifdef MEMPROFILE                      /* doing memory profiling            */
    fprintf(outfile, "Memory profile for normal object allocations\n\n");
    // all the work is done by the dead caches
    largeDead.dumpMemoryProfile(outfile);

    for (int i = FirstDeadPool; i < DeadPools; i++)
    {
        subpools[i].dumpMemoryProfile(outfile);
    }
#endif
}


/**
 * do dead object overlap validation checking.
 *
 * @param obj    The object to check.
 */
void NormalSegmentSet::checkObjectOverlap(DeadObject *obj)
{
    // all the work is done by the dead caches
    largeDead.checkObjectOverlap(obj);

    for (int i = FirstDeadPool - 1; i < DeadPools; i++)
    {
        subpools[i].checkObjectOverlap(obj);
    }
}


/**
 * Constructor for the large segment pool.
 *
 * @param mem    The hosting memory object.
 */
LargeSegmentSet::LargeSegmentSet(MemoryObject *mem) :
    MemorySegmentSet(mem, SET_NORMAL, "Large Allocation Segments"),
    deadCache("Large Block Allocation Pool"), requests(0), smallestObject(0), largestObject(0) { }


/**
 * Constructor for a single object segment set.
 *
 * @param mem    The hosting memory object.
 */
SingleObjectSegmentSet::SingleObjectSegmentSet(MemoryObject *mem) :
    MemorySegmentSet(mem, SET_SINGLEOBJECT, "Single Object Segments"),
    allocationsSinceLastGC(0), allocationCount(0), returnCount(0) { }


/**
 * Constructor for the oldspace segment pool.
 *
 * @param mem    The master memory object instance
 */
OldSpaceSegmentSet::OldSpaceSegmentSet(MemoryObject *mem) :
    MemorySegmentSet(mem, SET_OLDSPACE, "Old Space Segments"),
    deadCache("Old Space Allocation Pool") {
}


/**
 * Constructor for the normal segment pool.
 *
 * @param mem    The master memory object.
 */
NormalSegmentSet::NormalSegmentSet(MemoryObject *mem) :
    MemorySegmentSet(mem, SET_NORMAL, "Normal Allocation Segments"),
    largeDead("Large Normal Allocation Pool")
{
    // finish setting up the allocation subpools.  We set up one
    // additional one, to act as a guard pool to redirect things to
    // the large pool.

    // there are only
    // DeadPools subpools! (<, not <=)
    for (int i = 0; i < DeadPools; i++)
    {
        char buffer[100];
        sprintf(buffer, "Normal allocation subpool %d for blocks of size %zu", i, deadPoolToLength(i));
        subpools[i].setID(buffer);
        // make sure these are properly set up as single size keepers
        subpools[i].emptySingle();
        // initially, all subpools are empty, so have them skip
        // directly to the dead chains.
        lastUsedSubpool[i] = DeadPools;
    }
    // add the final entry in the lastUsedSubpool table (extra)
    lastUsedSubpool[DeadPools] = DeadPools;

    // This must be the first seg created. Create a segment to give use some space
    // to potentiall recover from errors in case there is an out of memory error. This
    // doesn't need to be a full segment worth, 1/2 should do
    recoverSegment = memory->newSegment(RecoverSegmentSize, RecoverSegmentSize);
}


/**
 * Set up the intial normal memory allocation.
 */
void NormalSegmentSet::getInitialSet()
{
    // get our first set of segments (well, one big one really)
    newSegment(InitialNormalSegmentSpace, InitialNormalSegmentSpace);
}


/**
 * Set up the intial normal memory allocation.
 */
void LargeSegmentSet::getInitialSet()
{
    // get our first set of segments (well, one big one really)
    newSegment(InitialLargeSegmentSpace, InitialLargeSegmentSpace);
}


/**
 * Allocate a segment...designed for subclass overrides to provide specialized handling.
 *
 * @param requestLength
 *               The requested segment size.
 * @param minimumLength
 *               The minimum segment size we can take as a fall back
 *
 * @return A newly allocated memory segment or NULL if there is an allocation failure.
 */
MemorySegment *MemorySegmentSet::allocateSegment(size_t requestLength, size_t minimumLength)
{
    return memory->newSegment(requestLength, minimumLength);
}


/**
 * Allocate a segment for the larget segment set.
 *
 * @param requestLength
 *               The requested segment size.
 * @param minimumLength
 *               The minimum size we can take as a fallback.
 *
 * @return The newly allocated segment or NULL if there was an allocation failure.
 */
MemorySegment *LargeSegmentSet::allocateSegment(size_t requestLength, size_t minimumLength)
{
    return memory->newLargeSegment(requestLength, minimumLength);
}


/**
 * Allocate a memory segment for the SingleObject set. This will allocate
 * the object to the minimum size.
 *
 * @param requestLength
 *               The requested length.
 * @param minimumLength
 *               The minimum length that will satisfy the request.
 *
 * @return A new memory segment object, or NULL if this could not be allocated.
 */
MemorySegment *SingleObjectSegmentSet::allocateSegment(size_t requestLength, size_t minimumLength)
{
    // use the minimum length for both because we want a best-fit here.
    return memory->newLargeSegment(requestLength, requestLength);
}


/**
 * Allocate a segment and add it to the segment pool
 *
 * @param requestLength
 *               The length requested.
 * @param minimumLength
 *               A minimum acceptable length
 *
 * @return true if the allocation succeeded, false otherwise. If allocated, this
 *         is also added to the segment set memory pool.
 */
bool MemorySegmentSet::newSegment(size_t requestLength, size_t minimumLength)
{
    // try to allocate a segment, and add it to the list if
    // successful.  This also adds the space to our dead object pool
    MemorySegment *segment = allocateSegment(requestLength, minimumLength);
    if (segment != NULL)
    {
        addSegment(segment);
        return true;
    }
    return false;
}


/**
 * Add a dead object to the segment set.
 *
 * @param object The newly swept up dead object.
 */
void MemorySegmentSet::addDeadObject(DeadObject *object)
{
 // The base class doesn't maintain a dead object pool, so this is a nop
}


/**
 * Add a dead object to the segment set.
 *
 * @param object The newly swept up dead object.
 */
void LargeSegmentSet::addDeadObject(DeadObject *object)
{
    // we want to keep our cache sorted by the size.
    deadCache.addSortedBySize(object);
}


/**
 * Add a dead object to the segment set during the sweep operation.
 *
 * @param object The newly dead object.
 */
void SingleObjectSegmentSet::addDeadObject(DeadObject *object)
{
    // Normally, we are a single object and after the garbage collection the segment
    // will not contain any live objects, so this dead object represents the entire
    // segment. However, it is possible that an array was initially allocated from this
    // storage pool and needed to be expanded. In that case, we have a short array object
    // at the front of the segment followed by the remnants of the original array as dead
    // space. We have two choices, we can do nothing, which means a lot of storage is dead
    // unused space until the array stub gets GC'd or we can move this segment into the normal
    // storage pool. It will never end up getting released, but at least the memory won't be
    // going to waste. However, we ourselves do not keep any objects in a cache because
    // we never hand them out.
}


/**
 * Add a dead object to the segment set during the sweep operation.
 *
 * @param object The newly dead object.
 */
void OldSpaceSegmentSet::addDeadObject(DeadObject *object)
{
    /* we want to keep our cache sorted by the size. */
    deadCache.addSortedBySize(object);
}


/**
 * Add a dead object to the segment set during the sweep operation.
 *
 * @param object The newly dead object.
 */
void NormalSegmentSet::addDeadObject(DeadObject *object)
{
//  checkObjectOverlap(object);
    size_t length = object->getObjectSize();

    // if the length is larger than the biggest subpool we
    // maintain, we add this to the large block list.
    if (length > LargestSubpool)
    {
        // ideally, we'd like to add this sorted by size, but
        // this is called so frequently, attempting to sort
        // degrades performance by about 10%.
        largeDead.add(object);
    }
    else
    {
        // calculate the dead chain
        // and add that to the appropriate chain
        size_t deadChain = lengthToDeadPool(length);
        subpools[deadChain].addSingle(object);
        // we can mark this subpool as having items again
        lastUsedSubpool[deadChain] = deadChain;
    }
}


/**
 * Add a block of storage to the dead object chains
 *
 * @param object The pointer to the start of the object.
 * @param length The length of the object.
 */
void MemorySegmentSet::addDeadObject(char *object, size_t length)
{
 // The base class doesn't maintain a dead object pool, so this is a nop
}


/**
 * Add a block of storage to the dead object chains
 *
 * @param object The pointer to the start of the object.
 * @param length The length of the object.
 */
void LargeSegmentSet::addDeadObject(char *object, size_t length)
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
    // we want to keep our cache sorted by the size.
    deadCache.addSortedBySize(new (object) DeadObject(length));
}


/**
 * Add a section of memory as a dead object to our pool.
 *
 * @param object The address of the memory.
 * @param length The length of the dead memory.
 */
void SingleObjectSegmentSet::addDeadObject(char *object, size_t length)
{
    // we don't keep a deadpool cache, but we do want to mark this as a dead object
    // so that we know the segment is a candidate for releasing
    new (object) DeadObject(length);
}


/**
 * Add a section of memory as a dead object to our pool.
 *
 * @param object The address of the memory.
 * @param length The length of the dead memory.
 */
void OldSpaceSegmentSet::addDeadObject(char *object, size_t length)
{
    // we want to keep our cache sorted by the size.
    deadCache.addSortedBySize(new (object) DeadObject(length));
}


/**
 * Add a section of memory as a dead object to our pool.
 *
 * @param object The address of the memory.
 * @param length The length of the dead memory.
 */
void NormalSegmentSet::addDeadObject(char *object, size_t length)
{
    // if the length is larger than the biggest subpool we
    // maintain, we add this to the large block list.
    if (length > LargestSubpool)
    {
        // ideally, we'd like to add this sorted by size, but
        // this is called so frequently, attempting to sort
        // degrades performance by about 10%.
        largeDead.add(new (object) DeadObject(length));
    }
    else
    {
        // calculate the dead chain
        // and add that to the appropriate chain
        size_t deadChain = lengthToDeadPool(length);
        subpools[deadChain].addSingle(new (object) DeadObject(length));
        // we can mark this subpool as having items again
        lastUsedSubpool[deadChain] = deadChain;
    }
}


/**
 * Add a segment to the segment pool.
 *
 * @param segment The new segment to insert
 */
void MemorySegmentSet::addSegment(MemorySegment *segment)
{
    memory->verboseMessage("Adding a segment of %zu bytes to %s\n", segment->size(), name);

    MemorySegment *insertPosition = anchor.next;

    // insert this at the head of the chain.
    insertPosition->insertBefore(segment);
    // all of the storage in the segment is added as a dead object.
    DeadObject *ptr = segment->createDeadObject();
    addDeadObject(ptr);
}


/**
 * Accept transfer of a memory segment from another segment set. This
 * occurs at the end of the sweep phase, so we will add the segment to
 * our list and then perform a sweep on it to add the dead storage
 * to our chains. Normally this segment will have just one live object
 * and one large dead object.
 *
 * @param segment The segment to transfer.
 */
void MemorySegmentSet::transferSegment(MemorySegment *segment)
{
    memory->verboseMessage("Segment of %zu bytes transferred to %s\n", segment->size(), name);
    MemorySegment *insertPosition = anchor.next;

    // insert this at the head of the chain.
    insertPosition->insertBefore(segment);
    // now perform a sweep on this segment
    sweepSingleSegment(segment);
}


/**
 * Search the cache of dead objects for an object of sufficient
 * size to fulfill this request.  For the default implementation, we always
 * return NULL.  Different segment sets may have different strategies for
 * determining which block is most suitable for donation.
 *
 * @param allocationLength
 *               The size of the object needed.
 *
 * @return Either a pointer to a dead object or NULL if none are available in the size required.
 */
DeadObject *MemorySegmentSet::donateObject(size_t allocationLength)
{
    return NULL;
}


/**
 * Search the cache of dead objects for an object of sufficient
 * size to fulfill this request.  This just uses normal large block
 * allocation strategy to decide on the block to donate, give a best fit
 * for the allocation.
 *
 * @param allocationLength
 *               The required object size
 *
 * @return A DeadObject of the required size.
 */
DeadObject *NormalSegmentSet::donateObject(size_t allocationLength)
{
    // If we're being asked to donate an object, then the request is larger
    // than the threshold for objects we manage. Therefore, we can look directly
    // in our large object cache.
    return (DeadObject *)findLargeDeadObject(allocationLength);
}


/**
 * Search the cache of dead objects for an object of sufficient
 * size to fulfill this request.  This just uses normal large block
 * allocation strategy to decide on the block to donate, give a best fit
 * for the allocation.
 *
 * @param allocationLength
 *               The required object size
 *
 * @return A DeadObject of the required size.
 */
DeadObject *LargeSegmentSet::donateObject(size_t allocationLength)
{
    // find an object in the cache return it.  This will likely be
    // larger than the request, as we shouldn't have may small
    // objects in the dead chains. */
    return deadCache.findSmallestFit(allocationLength);
}


/**
 * Decide if we should add additional segments to the normal heap
 * based on the amount of free space we have after a garbage collection.  If
 * we're starting to run full, then we should expand the heap to prevent
 * recycling the garbage collector to reclaim minimal amounts of storage that
 * is immediately reused, causing another GC cycle.
 *
 * @return A recommended expansion size.
 */
size_t MemorySegmentSet::suggestMemoryExpansion()
{
    // the base memory set has no expansion algorithm...always return 0
    return 0;
}


/**
 * Decide if we should add additional segments to the normal heap
 * based on the amount of free space we have after a garbage collection.  If
 * we're starting to run full, then we should expand the heap to prevent
 * recycling the garbage collector to reclaim minimal amounts of storage that
 * is immediately reused, causing another GC cycle.
 *
 * @return A recommended expansion size.
 */
size_t NormalSegmentSet::suggestMemoryExpansion()
{
    // since we have just done a GC, we have metrics on the size of live
    // and dead storage. Get the current percentage
    float freePercent = freeMemoryPercentage();

    memory->verboseMessage("Normal segment set free memory percentage is %d\n", (int)(freePercent * 100.0));

    // if we are less than 30% full, we should try to expand to the
    // 30% mark.
    if (freePercent < NormalMemoryExpansionThreshold)
    {
        // get a recommendation on how large the heap should be
        size_t recommendedSize = recommendedMemorySize();
        size_t newDeadBytes = recommendedSize - liveObjectBytes;
        // calculate the number of new bytes we need to add reach
        // our memory threshold.  It will be the job of others to
        // translate this into segment sized units.
        size_t expansionSize = newDeadBytes - deadObjectBytes;
        return expansionSize;
    }
    // no expansion required yet.
    return 0;
}


/**
 * Decide if we should add additional segments to the normal heap
 * based on the amount of free space we have after a garbage collection.  If
 * we're starting to run full, then we should expand the heap to prevent
 * recycling the garbage collector to reclaim minimal amounts of storage that
 * is immediately reused, causing another GC cycle.
 *
 * @return A recommended expansion size.
 */
size_t LargeSegmentSet::suggestMemoryExpansion()
{
    // since we have just done a GC, we have metrics on the size of live
    // and dead storage. Get the current percentage
    float freePercent = freeMemoryPercentage();

    memory->verboseMessage("Large segment set free memory percentage is %d\n", (int)(freePercent * 100.0));

    // We use different percentages for the large object pool and the normal
    // pool, attempting to give the large pool a better opportunity to fit in large size blocks.
    if (freePercent < LargeMemoryExpansionThreshold)
    {
        // get a recommendation on how large the heap should be
        size_t recommendedSize = recommendedMemorySize();
        size_t newDeadBytes = recommendedSize - liveObjectBytes;
        // calculate the number of new bytes we need to add reach
        // our memory threshold.  It will be the job of others to
        // translate this into segment sized units.
        size_t expansionSize = newDeadBytes - deadObjectBytes;
        return expansionSize;
    }
    // no expansion required yet.
    return 0;
}


/**
 * Decide if we should add  segments from the segment set based
 * on the current state of the heap immediately following a sweep.
 * We want to avoid thrashing the garbage collector because of chronic low
 * memory conditions.  If we wait until we get an actual allocation failure,
 * we will end up in a state where we're constantly running a mark-and-sweep
 * to  reclaim just a tiny amount of storage, and then repeat.  We will try to
 * keep the heap with appropriate free space limits.
 */
void MemorySegmentSet::adjustMemorySize()
{
    // see if a proactive expansion is advised, based on our
    // current state.
    size_t suggestedExpansion = suggestMemoryExpansion();
    // if we've decided we need to expand,
    if (suggestedExpansion > 0)
    {
        // go add as many segments as are required to reach that
        // level.
        memory->verboseMessage("Expanding %s by %zu bytes\n", name, suggestedExpansion);
        addSegments(suggestedExpansion);
    }
}


/**
 * Add segments to the segment set until the required space is
 * achieved or we've had a real memory failure.  Our preference is to add this
 * in one chunk, but we'll subdivide down to smaller units as long as we're
 * still getting the segments we require.
 *
 * @param requiredSpace
 *               The required total memory we need.
 */
void MemorySegmentSet::addSegments(size_t requiredSpace)
{
    // this may take several passes to achieve, but typically works the first time.
    for (;;)
    {
        // figure out how large this should be
        size_t segmentSize = calculateSegmentAllocation(requiredSpace);
        // we have a minimum size segment based on how large the request is
        size_t minSize = segmentSize >= LargeSegmentDeadSpace ? LargeSegmentDeadSpace : SegmentDeadSpace;
        // try to allocate a new segment
        MemorySegment *newSeg = allocateSegment(segmentSize, minSize);
        if (newSeg == NULL)
        {
            // if we already failed to get our "minimum minimum", just quit now.
            if (minSize == SegmentDeadSpace)
            {
                return;
            }
            // try for a single minimum segment allocation.  If that
            // fails...we have nothing else we can do.
            newSeg = allocateSegment(SegmentDeadSpace, SegmentDeadSpace);
            if (newSeg == NULL)
            {
                return;
            }
        }

        // we have a segment.  Add this to the segment pool
        // (and add the dead space to the available memory).
        addSegment(newSeg);
        segmentSize = newSeg->size();
        // if we've got what's needed, we're out of here+
        if (segmentSize >= requiredSpace)
        {
            return;
        }
        // reduce the size and try for some more
        requiredSpace -= segmentSize;
    }
}


void MemorySegmentSet::prepareForSweep()
{
    // reset the collection counters
    liveObjectBytes = 0;
    deadObjectBytes = 0;
}


/**
 * Handle all segment set pre-sweep initialization.
 */
void NormalSegmentSet::prepareForSweep()
{
    MemorySegmentSet::prepareForSweep();

    // we're about to rebuild the dead chains during the sweep, so
    // initialize all of these now.
    largeDead.empty();
    for (int i = FirstDeadPool; i < DeadPools; i++)
    {
        subpools[i].emptySingle();
    }
}


/**
 * Handle all segment set pre-sweep initialization.
 */
void LargeSegmentSet::prepareForSweep()
{
    MemorySegmentSet::prepareForSweep();

    // we're about to rebuild the dead chains during the sweep, so
    // initialize all of these now.
    deadCache.empty();
    requests = 0;
    largestObject = 0;
    smallestObject = 999999999;
}


/**
 * Handle all segment set post sweep activities.
 */
void MemorySegmentSet::completeSweepOperation()
{
    // This is a NOP for the base class.
}


void NormalSegmentSet::completeSweepOperation()
{
    // Now we can optimize the look-aside entries for the small
    // dead chains.  By checking to see which chains have blocks in
    // them, we can potentially skip a lot of check/searching on an
    // allocation request.
    for (int i = FirstDeadPool; i < DeadPools; i++)
    {
        if (!subpools[i].isEmptySingle())
        {
            // point this back at itself
            lastUsedSubpool[i] = i;
        }
        else
        {
            // default to the "skip to the end" location
            int usePool = DeadPools;
            int j;
            // scan all of the higher pools looking for the first hit
            for (j = i + 1; j < DeadPools; j++)
            {
                if (!subpools[i].isEmptySingle())
                {
                    usePool = j;
                    break;
                }
            }

            // this will now be guaranteed to go to the correct
            // location on each request.
            lastUsedSubpool[i] = usePool;
        }
    }
}


/**
 * Handle post-sweep processing for the set.
 */
void LargeSegmentSet::completeSweepOperation()
{
    memory->verboseMessage("Large segment sweep complete.  Largest block is %zu, smallest block is %zu\n", largestObject, smallestObject);
}


/**
 * Handle post-sweep processing for the set.
 */
void SingleObjectSegmentSet::completeSweepOperation()
{
    // Run the chain of segments checking for A) empty segments or B) segments where there is one
    // large object that is smaller than the segment. The first segment type can be returned to
    // the system. Category B) is a top-level array object that has been expanded so the
    // segment contains a small array stub and a very large dead object.
    MemorySegment *sweepSegment = first();
    while (sweepSegment != NULL)
    {
        // Since we might be removing this current segment,
        // get the next one in the chain now.
        MemorySegment *nextSegment = next(sweepSegment);
        // the sweep operation keeps a counter of live objects, so this is a
        // quick test for whether we can return the storage for this segment
        if (sweepSegment->isEmpty())
        {
            // remove this from the segment set chain and asked the memory object to return
            // it to the system.
            sweepSegment->remove();
            memory->verboseMessage("Returning memory segment of %zu bytes\n", sweepSegment->size());
            memory->freeSegment(sweepSegment);
        }
        else
        {
            RexxInternalObject *objectPtr = sweepSegment->startObject();
            // Get size of object for stats and
            size_t bytes = objectPtr->getObjectSize();
            // if the object does not take up the entire segment, then we have had an
            // object split. We need to transfer this segment into the Normal pool so
            // the rest of the segment can be used.
            if (bytes != sweepSegment->size())
            {
                sweepSegment->remove();
                memory->verboseMessage("Transfering memory segment of %zu bytes to the Normal segment set\n", sweepSegment->size());
                memory->transferSegmentToNormalSet(sweepSegment);
            }
        }
        // go to the next segment in the pool
        sweepSegment = nextSegment;
    }
}


inline bool objectIsLive(char *obj, size_t mark) {return ((RexxInternalObject *)obj)->isObjectLive(mark); }
inline bool objectIsNotLive(char *obj, size_t mark) {return ((RexxInternalObject *)obj)->isObjectDead(mark); }

/**
 * Perform a sweep of all segments controlled by this segment
 * set.
 */
void MemorySegmentSet::sweep()
{
    size_t mark = memoryObject.markWord;

    // do the sweep preparation (this differs for particular segment
    // set implemenation
    prepareForSweep();
    // go through the segments in order, until we've swept the
    // entire set */
    MemorySegment *sweepSegment = first();
    while (sweepSegment != NULL)
    {
        // go sweep that segment
        sweepSingleSegment(sweepSegment);
        // go to the next segment in the pool
        sweepSegment = next(sweepSegment);
    }
    // now do any of the sweep post-processing.
    completeSweepOperation();
}


/**
 * Sweep a single memory segment and return the dead objects to the
 * segment set dead object cache for reuse.
 *
 * @param sweepSegment
 *               The segment to sweep.
 */
void MemorySegmentSet::sweepSingleSegment(MemorySegment *sweepSegment)
{
    size_t mark = memoryObject.markWord;

    // clear the live objects counter for tracking
    sweepSegment->liveObjects = 0;

    RexxInternalObject *objectPtr = sweepSegment->startObject();
    RexxInternalObject *endPtr = sweepSegment->endObject();

    // now we need to look at all of the objects in the segment looking
    // for ones that don't have the current live mark.
    while (objectPtr < endPtr)
    {
        // live objects have been tagged by the marking operation.
        if (objectPtr->isObjectLive(mark))
        {
            // Get size of object for stats and do validation if in debug mode
            size_t bytes = objectPtr->getObjectSize();
            validateObject(objectPtr);
            // update our live object counters
            liveObjectBytes += bytes;
            sweepSegment->liveObjects++;
            // point to next object in segment.
            objectPtr = objectPtr->nextObject();
        }
        // this is a dead object (the first in a potential string of dead objects. We'll
        // scan to the next live object (or the end) looking to combine all of these
        // dead objects into a single block
        else
        {
            // get the size of the first object and validate things if in debug mode.
            size_t deadLength = objectPtr->getObjectSize();
            validateObject(objectPtr);

            // now scan forward to sweep up as many dead objects as we can.
            RexxInternalObject *nextObjectPtr = objectPtr->nextObject();

            for (; (nextObjectPtr < endPtr) && nextObjectPtr->isObjectDead(mark); nextObjectPtr = nextObjectPtr->nextObject())
            {
                // get the size and add to the accumulator
                size_t bytes = nextObjectPtr->getObjectSize();
                validateObject(nextObjectPtr);
                deadLength += bytes;
            }
            // add this value to the accumulators we use to make
            // expansion decisions
            deadObjectBytes += deadLength;
            // objectPtr points to the start of the string of dead objects,
            // deadLength is the total deadlength. We add this to the
            // segment set dead object caches.
            addDeadObject((char *)objectPtr, deadLength);
            // adding this block created a DeadObject instance with the appropriate
            // length, so we use that to step to the next object.
            objectPtr = objectPtr->nextObject(deadLength);
        }
    }
}


/**
 * Process a dead object and potentially split the object into
 * an allocated object and a second dead object that will be added back into
 * the appropriate pool of dead objects.
 *
 * @param object The object to split.
 * @param allocationLength
 *               The front length we require, already rounded to
 *               the appropriate grain size.
 * @param splitMinimum
 *               The minimum required length for the back object. If the trailing
 *               portion is short than this, then the original dead object is
 *               left the same size.
 *
 * @return The split off real object.
 */
RexxInternalObject *MemorySegmentSet::splitDeadObject(DeadObject *object, size_t allocationLength, size_t splitMinimum)
{
    size_t deadLength = object->getObjectSize() - allocationLength;

    // remainder too small or this is a very large request
    // is the remainder two small to reuse?
    if (deadLength < splitMinimum)
    {
        // over allocate this object
        allocationLength += deadLength;
    }
    else
    {
        // Yes, so pull new object out of the front of the dead
        // object, adjust the size of the Dead object.
        char *largeObject = ((char *)object) + allocationLength;
        // and reinsert into the the dead object pool
        addDeadObject((char *)largeObject, deadLength);
    }
    // Adjust the size of this object to the requested allocation length
    // and return it as an object pointer
    ((RexxInternalObject *)object)->setObjectSize(allocationLength);
    return(RexxInternalObject *)object;
}


/**
 * Search the segment set for a block of the requested length.
 * If a block is found, the block will be subdivided if necessary, with the
 * remainder portion placed back on the proper chain.
 *
 * @param allocationLength
 *               The required object length (already rounded to an appropriate grain size).
 *
 * @return An appropriately sized object or NULL if one was not found.
 */
RexxInternalObject *NormalSegmentSet::findLargeDeadObject(size_t allocationLength)
{
    // Go through the LARGEDEAD object looking for the 1st
    // one we can use either our object is too big for all the
    // small chains, or the small chains are depleted....
    DeadObject *largeObject = largeDead.findFit(allocationLength);
    if (largeObject != NULL)
    {
        // if we found an object that will fit, try to split an object off from
        // from the whole. There must be at least a minimum size object left.
        return splitDeadObject(largeObject, allocationLength, Memory::MinimumObjectSize);
    }
    // nothing found, return NULL
    return OREF_NULL;
}


/**
 * Find a dead object in the caches for the NormalSegmentSet.
 *
 * @param allocationLength
 *               The required allocation size.
 *
 * @return A located object or NULL if unable to allocate.
 */
RexxInternalObject *NormalSegmentSet::findObject(size_t allocationLength)
{
    // this is just a non-inline virtual version of the allocation routine
    // routine to be used during low memory situations.
    return allocateObject(allocationLength);
}


/**
 * Handle an allocation failure for a request that was directed to the
 * normal allocation segment set.
 *
 * @param allocationLength
 *               The length of the failing allocation attempt.
 *
 * @return An allocated object, or NULL if this still fails after the all failure
 *         methods have been attempted.
 */
RexxInternalObject *NormalSegmentSet::handleAllocationFailure(size_t allocationLength)
{
    memory->verboseMessage("Normal allocation failure for %zu bytes\n", allocationLength);

    // Step 1, force a GC
    memory->collect();
    // Step 2, now that we have good GC data, decide if we need to adjust
    // the heap size.
    adjustMemorySize();
    // Step 3, see if can allocate now
    RexxInternalObject *newObject = findObject(allocationLength);
    // still no luck?
    if (newObject == OREF_NULL)
    {
        // it is possible that the adjustment routines did not add
        // anything, yet we were unable to allocate a block
        // because we're highly fragmented.  Try adding a new segment
        // now, before going to the extreme methods.
        addSegments(MemorySegment::SegmentSize);
        // see if can allocate now
        newObject = findObject(allocationLength);
        // still no luck? We need to go to more extreme lengths
        // to try find a block to satisfy this request.
        if (newObject == OREF_NULL)
        {
            // We might be able to steal something from the
            // large allocations.  This will also steal the recovery
            // segment as a last ditch effort
            memory->scavengeSegmentSets(this, allocationLength);
            // see if can allocate now
            newObject = findObject(allocationLength);
            // still no luck?
            if (newObject == OREF_NULL)
            {
                // We have a recovery segment in our back pocket, but this
                // is needed to allow us to actually report that we have an
                // out of memory condition. We add this to the segment set, but
                // we will not try to use it to satisfy this request. We need this
                // to keep from crashing.
                if (recoverSegment != NULL)
                {
                    addSegment(recoverSegment);
                    recoverSegment = NULL;
                }

                // if we have gone through all of that and still have nothing,
                // this is a real out-of-memory situation.
                // can't allocate, report resource error.
                reportException(Error_System_resources);
            }
        }
    }
    return newObject;
}


/**
 * Locate an object is the dead object cache maintained for large
 * object allocations.
 *
 * @param allocationLength
 *               The required object size.
 *
 * @return A located object form the dead space or NULL if unable to allocate.
 */
RexxInternalObject *LargeSegmentSet::findObject(size_t allocationLength)
{
    /* this is just a space-saving non-inline version of allocateObject */
    return allocateObject(allocationLength);
}


/**
 * Handle a first-attempt allocation failure from the large block segment
 * set. This will decide whether to expand and/or perofrom a garbage collection in
 * order to allocate a block of the requested size.
 *
 * @param allocationLength
 *               The size of the object that caused the original allocation failure.
 *
 * @return An allocated object. If unable to allocate, this raised the resource
 *         exception.
 */
RexxInternalObject *LargeSegmentSet::handleAllocationFailure(size_t allocationLength)
{
    memory->verboseMessage("Large object allocation failure for %zu bytes\n", allocationLength);

    // Step 1, force a GC
    memory->collect();
    // Step 2, now that we have good GC data, decide if we need to adjust
    // the heap size.
    adjustMemorySize();
    // Step 2, see if can allocate now
    RexxInternalObject *newObject = findObject(allocationLength);
    // Nothing in the current space?
    if (newObject == OREF_NULL)
    {
        // Step 4, force a heap expansion, if we can
        // this is a more targeted size
        expandSegmentSet(allocationLength);
        // Step 4, see if can allocate now */
        newObject = findObject(allocationLength);
        // still no luck?                    */
        if (newObject == OREF_NULL)
        {
            // We'll take one last chance.  We don't really like
            // to do this, but we might be able to steal a block
            // off of the normal allocation chain. We will managed
            // this as one of ours until the next GC cycle.
            memory->scavengeSegmentSets(this, allocationLength);
            // Last chance, see if can allocate now
            newObject = findObject(allocationLength);
            // if still nothing, we are really and truly out of memory.
            if (newObject == OREF_NULL)
            {
                reportException(Error_System_resources);
            }
        }
    }
    // if we got a real object, count this....
    if (newObject != OREF_NULL)
    {
        requests++;
    }
    // return our allocated object
    return newObject;
}


/**
 * Handle a first-attempt allocation failure from the single
 * object segment set. A failure here means we were unable to obtain
 * a block of the required size from the system. Our only option
 * now is to force a garbage collection which might free up enough
 * memory that the system can fullfil the request.
 *
 * @param allocationLength
 *               The size of the object that caused the original allocation failure.
 *
 * @return An allocated object. If unable to allocate, this raised the resource
 *         exception.
 */
RexxInternalObject *SingleObjectSegmentSet::handleAllocationFailure(size_t allocationLength)
{
    memory->verboseMessage("Single object allocation failure for %zu bytes\n", allocationLength);

    // to ahead and do the collection. If we're lucky, we can free up enough memory
    // to satisify this request.
    memory->collect();

    // reset the allocations trap
    allocationsSinceLastGC = 0;
    RexxInternalObject *newObject = allocateObject(allocationLength);
    // if still nothing, we are really and truly out of memory.
    if (newObject == OREF_NULL)
    {
        reportException(Error_System_resources);
    }
    return newObject;
}


/**
 * Locate an object is the dead object cache maintained for old space
 * object allocations.
 *
 * @param allocationLength
 *               The required object size
 *
 * @return An allocated object, or NULL if not enough there.
 */
RexxInternalObject *OldSpaceSegmentSet::findObject(size_t allocationLength)
{
    // go through the LARGEDEAD object looking for the 1st one we can
    // use
    DeadObject *largeObject = deadCache.findFit(allocationLength);
    // if we found something, split off the ammount we need and return it.
    if (largeObject != NULL)
    {
        return splitDeadObject(largeObject, allocationLength, Memory::LargeAllocationUnit);
    }
    // nothing of sufficient size here.
    return OREF_NULL;
}


/**
 * Allocate an extremely large object. These are contained in a single
 * segment so that we can release the large blocks of storage once
 * the object is dead.
 *
 * @param requestLength
 *               The size of the required object.
 *
 * @return A newly allocated object or NULL for an allocation failure.
 */
RexxInternalObject *SingleObjectSegmentSet::allocateObject(size_t requestLength)
{
    memory->verboseMessage("Allocating a single segment object of %zu bytes to %s\n", requestLength, name);

    // it is possible that we could have a pattern where a large number of
    // short-lived objects get allocated without triggering a garbage collection. Since
    // this segment set expands for each object allocation, we would never trigger a GC
    // until we had an actual allocation failure. It is better to be proactive and
    // prune the dead segments before attempting the allocation if they have been
    // accumulating
    if (allocationsSinceLastGC > ForceGCThreshold)
    {
        memory->verboseMessage("Single object force threshold reached\n");
        return OREF_NULL;       // this will trigger the GC
    }

    // allocate a memory segment sufficiently large for the requested object
    MemorySegment *segment = allocateSegment(requestLength, requestLength);
    // a real allocation failure, time to take out the trash
    if (segment == NULL)
    {
        return OREF_NULL;
    }

    allocationsSinceLastGC++;          // update the allocation counters
    allocationCount++;

    memory->verboseMessage("Adding a segment of %zu bytes to %s\n", segment->size(), name);

    // we don't attempt and recombinations in this set, so just push it on
    // to the head of our segment chain
    anchor.next->insertBefore(segment);
    // now create a dead object in the segment, that is our return value
    DeadObject *ptr = segment->createDeadObject();
    return (RexxInternalObject *)ptr;
}


/**
 * Allocate an object from the old space.  This is a very simple
 * management strategy, with no heroic efforts made to locate memory.  Since
 * objects here can only come from the OldSpace area, there is no garbage
 * collection forced for failures.
 *
 * @param requestLength
 *               The required object length.
 *
 * @return An object of appropriate size or NULL if not enough space.
 */
RexxInternalObject *OldSpaceSegmentSet::allocateObject(size_t requestLength)
{
    // round this allocation up to the appropriate large boundary
    size_t allocationLength = Memory::roundLargeObjectAllocation(requestLength);
    // Step 1, try to find an object in the current heap.
    RexxInternalObject *newObject = findObject(allocationLength);
    // can't allocate one? Add more space and try again. We don't GC this space.
    if (newObject == OREF_NULL)
    {
        // add space to this and try again. Note that this is only used during image builds,
        // which are a little more controlled, so we expect this to succeed.
        newSegment(allocationLength, allocationLength);
        newObject = findObject(allocationLength);
    }
    return newObject;
}


/**
 * Find the largest segment in the segment set.
 *
 * @return The largest segment in the set (this assumes the segment set has
 *         at least one allocated segment).
 */
MemorySegment *MemorySegmentSet::largestActiveSegment()
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

    // we return this unconditionally, as our anchor segment will
    // report a size of zero.
    return largest;
}


/**
 * The large allocation heap only handles requests up to the
 * size of a Normal segment. Therefore, we will always add in
 * large segment increments. If that fails, we will try adding a
 * segment just slightly larger than the request size.
 *
 * @param allocationLength
 *               The allocation size that caused the allocation failure.
 */
void LargeSegmentSet::expandSegmentSet(size_t allocationLength)
{

    // We manage allocations in mid-range sizes here. The request will
    // always be smaller than our default expansion size, so that is our
    // first attempt. If that fails, we'll round to a page boundary and
    // try for the smaller size
    memory->verboseMessage("Expanding large segment set by %d\n", LargeSegmentDeadSpace);
    newSegment(LargeSegmentDeadSpace, MemorySegment::roundSegmentBoundary(allocationLength));
}


/**
 * Accumulate memory statistics for a segment
 *
 * @param memStats Gather current statistics from each memory segment.
 * @param stats
 */
void MemorySegmentSet::gatherStats(MemoryStats *memStats, SegmentStats *stats)
{
    stats->count = count;

    MemorySegment *seg;
    for (seg = first(); seg != NULL; seg = next(seg))
    {
        seg->gatherObjectStats(memStats, stats);
        stats->largestSegment = std::max(stats->largestSegment, seg->size());
        stats->smallestSegment = std::max(stats->smallestSegment, seg->size());
    }
}


/**
 * Perform a marking operation on all objects in a segment
 * (only called for an image restore)
 */
void MemorySegment::markAllObjects()
{
    RexxInternalObject *op = startObject();
    RexxInternalObject *ep = endObject();
    while (op < ep)
    {
        // all objects have an associated behaviour, make sure it is marked
        memory_mark_general(op->behaviour);

        // not every object has references to other objects (strings are
        // a good example). We can skip calling the live method if we know they don't
        if (op->hasReferences())
        {
            // yes, there are references, we need to fix them up.
            op->liveGeneral(RESTORINGIMAGE);
        }
        op =op->nextObject();   // we are just marking each object in order.
    }
}


/**
 * Perform a marking operation on all OldSpace objects
 */
void OldSpaceSegmentSet::markOldSpaceObjects()
{
    // mark each of the oldspace segments
    for (MemorySegment *segment = first(); segment != NULL; segment = next(segment))
    {
        segment->markAllObjects();
    }
}

